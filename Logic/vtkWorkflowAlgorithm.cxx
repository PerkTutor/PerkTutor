
#include "vtkWorkflowAlgorithm.h"
#include "vtkRecordLog.h"
#include "vtkMarkovModel.h"
#include "vtkTrackingRecord.h"

#include "vtkSmartPointer.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataParser.h"
#include "RecordType.h"

#include <string>
#include <sstream>
#include <vector>
#include <cmath>


//----------------------------------------------------------------
// Helper Functions
//----------------------------------------------------------------


//-------------------------------------------------------------------------
// Class functions
//-------------------------------------------------------------------------


vtkStandardNewMacro( vtkWorkflowAlgorithm );

vtkWorkflowAlgorithm
::vtkWorkflowAlgorithm()
{
  this->procedureRT = NULL;
  this->derivativeProcedureRT = NULL;
  this->filterProcedureRT = NULL;
  this->orthogonalProcedureRT = NULL;
  this->principalProcedureRT = NULL;
  this->centroidProcedureRT = NULL;
  this->MarkovRT = NULL;

  this->indexLastProcessed = 0;
  this->currentTask = -1;
  this->prevTask = -1;

  this->toolName = "";
}



vtkWorkflowAlgorithm
::~vtkWorkflowAlgorithm()
{
  for ( int i = 0 ; i < procedures.size(); i++ )
  {
    procedures.at(i)->Delete();
  }
  procedures.clear();

  if ( this->procedureRT != NULL )
  {
    this->procedureRT->Delete();
    this->derivativeProcedureRT->Delete();
    this->filterProcedureRT->Delete();
    this->orthogonalProcedureRT->Delete();
    this->principalProcedureRT->Delete();
    this->centroidProcedureRT->Delete();
    this->MarkovRT->Delete();
  }

  this->ModuleNode = NULL;
}



void vtkWorkflowAlgorithm
::SetModuleNode( vtkMRMLWorkflowSegmentationNode* newModuleNode )
{
  this->ModuleNode = newModuleNode;
}


void vtkWorkflowAlgorithm
::SetToolName( std::string newToolName )
{
  this->toolName = newToolName;
}


Tool vtkWorkflowAlgorithm
::GetTool()
{
  return this->ModuleNode->toolCollection.GetTool( this->toolName );
}


std::vector<double> vtkWorkflowAlgorithm
::CalculateTaskProportions()
{
  // Create a vector of counts for each label
  std::vector<int> taskCounts;
  for ( int i = 0; i < this->GetTool().perkProc.GetNumTasks(); i++ )
  {
    taskCounts.push_back( 0 );
  }

  int sum = 0;

  // Iterate over all record logs and count label (task) instances
  for ( int i = 0; i < procedures.size(); i++ )
  {
    for ( int j = 0; j < procedures.at(i)->Size(); j++ )
	{
	  taskCounts[ procedures.at(i)->GetRecordAt(j).getLabel() ] += 1;
	  sum++;
	}
  }

  std::vector<double> taskProportions;
  // Calculate the proportion of each task
  for ( int i = 0; i < this->GetTool().perkProc.GetNumTasks(); i++ )
  {
    taskProportions.push_back( ( 1.0 * taskCounts[i] ) / sum );
  }

  return taskProportions;
}




std::vector<int> vtkWorkflowAlgorithm
::CalculateTaskCentroids()
{
  // Create a vector of counts for each label
  std::vector<double> taskProportions = this->CalculateTaskProportions();
  std::vector<double> taskRawCentroids ( this->GetTool().perkProc.GetNumTasks(), 0 );
  std::vector<double> fracPart ( this->GetTool().perkProc.GetNumTasks(), 0 );

  for ( int i = 0; i < this->GetTool().perkProc.GetNumTasks(); i++ )
  {
    taskRawCentroids[i] = taskProportions[i] * this->GetTool().inputParam.NumCentroids;
  }

  bool rounded = true;
  int sumCentroids = 0;
  for ( int i = 0; i < this->GetTool().perkProc.GetNumTasks(); i++ )
  {
    sumCentroids += floor( taskRawCentroids[i] );
  }    

  while ( sumCentroids < this->GetTool().inputParam.NumCentroids )
  {

    rounded = true;
    for ( int i = 0; i < this->GetTool().perkProc.GetNumTasks(); i++ )
	{
      if ( floor( taskRawCentroids[i] ) != ceil( taskRawCentroids[i] ) )
	  {
        rounded = false;
	  }
	}

	if ( rounded )
	{
      break;
	}

    // Find the highest "priority" (with largest fractional part) centroid count to increase
	int priority = 0;
    for ( int i = 0; i < this->GetTool().perkProc.GetNumTasks(); i++ )
    {
      fracPart[i] = taskRawCentroids[i] - floor( taskRawCentroids[i] );
	  if ( fracPart[i] > fracPart[priority] )
	  {
        priority = i;
	  }
    }

	taskRawCentroids[ priority ] = ceil( taskRawCentroids[ priority ] );

	sumCentroids = 0;

	for ( int i = 0; i < this->GetTool().perkProc.GetNumTasks(); i++ )
	{
      sumCentroids += taskRawCentroids[i];
	}    

  }

  std::vector<int> taskCentroids ( this->GetTool().perkProc.GetNumTasks(), 0 );
  for ( int i = 0; i < this->GetTool().perkProc.GetNumTasks(); i++ )
  {
    taskCentroids[i] = floor( taskRawCentroids[i] );
  } 

  return taskCentroids;  

}





void vtkWorkflowAlgorithm
::ReadAllProcedures( std::vector<std::string> fileNames )
{
  for ( int i = 0; i < fileNames.size(); i++ )
  {
    this->ReadProcedure( fileNames[i] );
  }
}



void vtkWorkflowAlgorithm
::ReadProcedure( std::string fileName )
{
  // Create a new tool with the same attributes as the tool associated with this algorithm instance
  Tool tempTool; // This should be a deep copy (since we refer to the object, not pointer)

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( fileName.c_str() );
  parser->Parse();

  vtkXMLDataElement* rootElement = parser->GetRootElement();
  
  tempTool.transBuff.FromXMLElement( rootElement, this->toolName );
  tempTool.segBuff.FromXMLElement( rootElement, this->GetTool().perkProc );
  

  // Now, use the records messages (manual segmentation) to determine the label (task) at each recorded transform
  vtkTrackingRecord* conversionRecord = vtkTrackingRecord::New();
  vtkRecordLog* currProcedure = vtkRecordLog::New();

  for ( int i = 0; i < tempTool.transBuff.GetBufferSize(); i++ )
  {

    // Convert the transform in the transform buffer to a label record
    TransformRecord currTransform = tempTool.transBuff.GetTransformAt(i);
    TimeLabelRecord currRecord;

	conversionRecord->fromMatrixString( tempTool.transBuff.GetTransformAt(i).Transform )
	currRecord.values = conversionRecord->GetVector();
	currRecord.setTime( currTransform.TimeStampSec + 1.0e-9 * currTransform.TimeStampNSec );

    // Find the record with the largest smaller time stamp
	std::string currTask = "";
	double currTimeDiff = tempTool.transBuff.GetTotalTime();

	for ( int j = 0; j < tempTool.segBuff.GetBufferSize(); j++ )
	{
      if ( tempTool.segBuff.GetTimeAt(j) < tempTool.transBuff.GetTimeAt(i) && tempTool.transBuff.GetTimeAt(i) - tempTool.segBuff.GetTimeAt(j) < currTimeDiff )
	  {
        currTask = tempTool.segBuff.GetMessageAt(j).Message;
		currTimeDiff = tempTool.transBuff.GetTimeAt(i) - tempTool.segBuff.GetTimeAt(j);
	  }
	}

	if ( strcmp( currTask.c_str(), "" ) != 0 )
	{
	  currRecord.setLabel( currTask );
	  currProcedure->AddRecord( currRecord );
	}

  }

  conversionRecord->Delete();

  procedures.push_back( currProcedure );

}


// TODO: This is for testing only. Eventually we might like a similar method for segmenting previously recorded procedures.
void vtkWorkflowAlgorithm
::SegmentProcedure( std::string fileName )
{

  // Create a new tool with the same attributes as the tool associated with this algorithm instance
  Tool tempTool; // This should be a deep copy (since we refer to the object, not pointer)

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( fileName.c_str() );
  parser->Parse();

  vtkXMLDataElement* rootElement = parser->GetRootElement();
  
  tempTool.transBuff.FromXMLElement( rootElement, this->toolName );

  // Now, iterate over all transforms and add to the corresponding tool in the MRML
  for ( int i = 0; i < tempTool.transBuff.GetBufferSize(); i++ )
  {
    this->ModuleNode->toolCollection.GetTool( this->toolName ).transBuff.AddTransform( tempTool.transBuff.GetTransformAt(i) );
	this->UpdateTask();
  }


}




// Return whether or not training was successful
bool vtkWorkflowAlgorithm
::Train()
{
  // There must exist procedures
  if ( procedures.empty() )
  {
    return false;
  }

  // Calculate the number of centroids for each task
  std::vector<int> taskCentroids;
  std::vector<double> taskProportions;
  std::vector<int> cumulativeCentroids;
  int currSum = 0;
  taskCentroids = this->CalculateTaskCentroids();

  // Calculate the cumulative number of centroids for cluster numbering
  for ( int i = 0; i < this->GetTool().perkProc.NumTasks; i ++ )
  {
	// Make sure that every task is represented in the procedures
	if ( taskCentroids[i] == 0 )
	{
      return false;
	}

	cumulativeCentroids.push_back( currSum );
	currSum += taskCentroids[i];
  }

  // Apply Gaussian filtering to each record log
  std::vector<vtkRecordLog*> filterProcedures;
  for ( int i = 0; i < procedures.size(); i++ )
  {
    filterProcedures.push_back( procedures[i]->GaussianFilter( this->GetTool().inputParam.FilterWidth ) );
  }


  // Use velocity also
  std::vector<vtkRecordLog*> derivativeProcedures;
  for ( int i = 0; i < procedures.size(); i++ )
  {
    derivativeProcedures.push_back( filterProcedures[i]->DeepCopy() );
    for ( int d = 1; d <= this->GetTool().inputParam.Derivative; d++ )
	{
	  vtkRecordLog* currDerivativeProcedure = derivativeProcedures[i];
	  vtkRecordLog* orderDerivativeProcedure = filterProcedures[i]->Derivative(d);	  
	  derivativeProcedures[i] = currDerivativeProcedure->ConcatenateValues( orderDerivativeProcedure );
	  currDerivativeProcedure->Delete();
	  orderDerivativeProcedure->Delete();	  
	}
  }


  // Apply orthogonal transformation
  std::vector<vtkRecordLog*> orthogonalProcedures;
  for ( int i = 0; i < filterProcedures.size(); i++ )
  {
    orthogonalProcedures.push_back( derivativeProcedures[i]->OrthogonalTransformation( this->GetTool().inputParam.OrthogonalWindow, this->GetTool().inputParam.OrthogonalOrder ) );
  }

  // Concatenate all of the record logs into one record log
  vtkRecordLog* orthogonalCat = vtkRecordLog::New();
  orthogonalCat->Initialize( 0, orthogonalProcedures[0]->RecordSize() );
  for ( int i = 0; i < orthogonalProcedures.size(); i++ )
  {
    vtkRecordLog* currOrthogonalCat = orthogonalCat;
    orthogonalCat = currOrthogonalCat->Concatenate( orthogonalProcedures[i] );
	currOrthogonalCat->Delete();
  }

  // Calculate and apply the PCA transform
  this->GetTool().trainingParam.Mean.values = orthogonalCat->Mean().values;
  this->GetTool().trainingParam.Mean.setLabel( 0 );
  this->GetTool().trainingParam.PrinComps = orthogonalCat->CalculatePCA( this->GetTool().inputParam.NumPrinComps );
  std::vector<vtkRecordLog*> principalProcedures;
  for ( int i = 0; i < orthogonalProcedures.size(); i++ )
  {
    principalProcedures.push_back( orthogonalProcedures[i]->TransformPCA( this->GetTool().trainingParam.PrinComps, this->GetTool().trainingParam.Mean ) );
  }
  vtkRecordLog* principalCat = orthogonalCat->TransformPCA( this->GetTool().trainingParam.PrinComps, this->GetTool().trainingParam.Mean );

  // Put together all the tasks together for task by task clustering
  std::vector<vtkRecordLog*> recordsByTask = principalCat->GroupRecordsByLabel( this->GetTool().perkProc.NumTasks );

  // Add the centroids from each task
  // TODO: Change NumCentroids back to 700
  for ( int i = 0; i < this->GetTool().perkProc.NumTasks; i++ )
  {
	std::vector<LabelRecord> currTaskCentroids = recordsByTask[i]->fwdkmeans( taskCentroids[i] );
	for ( int j = 0; j < taskCentroids[i]; j++ )
	{
	  currTaskCentroids[j].setLabel( currTaskCentroids[j].getLabel() + cumulativeCentroids[i] );
      this->GetTool().trainingParam.Centroids.push_back( currTaskCentroids[j] );
	}
  }

  // Calculate the sequence of centroids for each procedure
  std::vector<vtkRecordLog*> centroidProcedures;
  for ( int i = 0; i < principalProcedures.size(); i++ )
  {
    centroidProcedures.push_back( principalProcedures[i]->fwdkmeansTransform( this->GetTool().trainingParam.Centroids ) );
  }

  // Assume that all the estimation matrices are ones
  LabelRecord PseudoPi;
  for ( int j = 0; j < NumTasks; j++ )
  {
    PseudoPi.add( 1.0 * this->GetTool().inputParam.MarkovPseudoScalePi );
  }
  PseudoPi.setLabel(0);

  std::vector<LabelRecord> PseudoA;
  for ( int i = 0; i < NumTasks; i++ )
  {
	LabelRecord currPseudoA;
	for ( int j = 0; j < NumTasks; j++ )
	{
      currPseudoA.add( 1.0 * this->GetTool().inputParam.MarkovPseudoScaleA );
	}
	currPseudoA.setLabel(i);
	PseudoA.push_back( currPseudoA );
  }

  std::vector<LabelRecord> PseudoB;
  for ( int i = 0; i < NumTasks; i++ )
  {
	LabelRecord currPseudoB;
	for ( int j = 0; j < NumCentroids; j++ )
	{
      currPseudoB.add( 1.0 * this->GetTool().inputParam.MarkovPseudoScaleB );
	}
	currPseudoB.setLabel(i);
	PseudoB.push_back( currPseudoB );
  }

  // Create a new Markov Model, and estimate its parameters
  vtkMarkovModel* Markov = vtkMarkovModel::New();
  Markov->SetSize( this->GetTool().perkProc.NumTasks, this->GetTool().inputParam.NumCentroids );
  Markov->InitializeEstimation( this->GetTool().perkProc.NumTasks, this->GetTool().inputParam.NumCentroids );
  Markov->AddPseudoData( PseudoPi, PseudoA, PseudoB );
  for ( int i = 0; i < centroidProcedures.size(); i++ )
  {
    this->Markov->AddEstimationData( centroidProcedures[i]->ToMarkovRecordVector() );
  }
  this->Markov->EstimateParameters();

  this->GetTool().trainingParam.MarkovPi = Markov.GetPi();
  this->GetTool().trainingParam.MarkovA = Markov.GetA();
  this->GetTool().trainingParam.MarkovB = Markov.GetB();


  // Delete objects we have created
  for ( int i = 0; i < filterProcedures.size(); i++ )
  {
    filterProcedures[i]->Delete();
  }
  for ( int i = 0; i < derivativeProcedures.size(); i++ )
  {
    derivativeProcedures[i]->Delete();
  }
  for ( int i = 0; i < orthogonalProcedures.size(); i++ )
  {
    orthogonalProcedures[i]->Delete();
  }
  for ( int i = 0; i < principalProcedures.size(); i++ )
  {
    principalProcedures[i]->Delete();
  }
  for ( int i = 0; i < recordsByTask.size(); i++ )
  {
    recordsByTask[i]->Delete();
  }
  for ( int i = 0; i < centroidProcedures.size(); i++ )
  {
    centroidProcedures[i]->Delete();
  }
  
  filterProcedures.clear();
  derivativeProcedures.clear();
  orthogonalProcedures.clear();
  principalProcedures.clear();
  recordsByTask.clear();
  centroidProcedures.clear();

  orthogonalCat->Delete();
  principalCat->Delete();
  Markov->Delete();

  return true;

}



void vtkWorkflowAlgorithm
::Reset()
{

  // If the algorithm isn't trained, then we can't initialize it for real-time segmentation
  if ( ! this->ModuleNode->toolCollection.GetTrained() )
  {
    return;
  }

  if ( this->procedureRT != NULL )
  {
    this->procedureRT->Delete();
    this->derivativeProcedureRT->Delete();
    this->filterProcedureRT->Delete();
    this->orthogonalProcedureRT->Delete();
    this->principalProcedureRT->Delete();
    this->centroidProcedureRT->Delete();
    this->MarkovRT->Delete();
  }

  procedureRT = vtkRecordLogRT::New();
  derivativeProcedureRT = vtkRecordLogRT::New();
  filterProcedureRT = vtkRecordLogRT::New();
  orthogonalProcedureRT = vtkRecordLogRT::New();
  principalProcedureRT = vtkRecordLogRT::New();
  centroidProcedureRT = vtkRecordLogRT::New();
  MarkovRT = vtkMarkovModelRT::New();
  
  this->GetTrainingParametersFromMRMLNode();

  MarkovRT->SetSize( this->GetTool().perkProc.NumTasks, this->GetTool().inputParam.NumCentroids );
  MarkovRT->SetPi( this->GetTool().trainingParam.MarkovPi );
  MarkovRT->SetA( this->GetTool().trainingParam.MarkovA );
  MarkovRT->SetB( this->GetTool().trainingParam.MarkovB );

  indexLastProcessed = 0;
  currentTask = -1;
  prevTask = -1;

}



void vtkWorkflowAlgorithm
::addRecord( TransformRecord t )
{

  TimeLabelRecord currRecord;
  vtkTrackingRecord* currTrackingRecord = vtkTrackingRecord::New();
  currTrackingRecord->fromMatrixString( t.Transform );
  currRecord.values = currTrackingRecord->GetVector();
  currRecord.setTime( t.TimeStampSec + 1.0e-9 * t.TimeStampNSec );
  currRecord.setLabel( 0 );
  procedureRT->AddRecord( currRecord );
  currTrackingRecord->Delete();

}



void vtkWorkflowAlgorithm
::addSegmentRecord( TransformRecord t )
{
  addRecord( t );

  // TODO: Only keep the most recent observations (a few for filtering, a window for orthogonal transformation)
  // TODO: Use these statements to help find bottlenecks (check how much time each operation takes) and delete when finished
  std::vector<double> elapseTime( 7, 0.0 );
  double currTime = this->MRMLNode->GetTimestamp();

  // Apply Gaussian filtering to each previous records
  filterProcedureRT->AddRecord( procedureRT->GaussianFilterRT( this->GetTool().inputParam.FilterWidth ) );

  elapseTime[0] = currTime - this->MRMLNode->GetTimestamp();
  currTime = this->MRMLNode->GetTimestamp();
  
  // Concatenate with derivative (velocity, acceleration, etc...)
  TimeLabelRecord derivativeRecord = filterProcedureRT->GetRecordRT();
  for ( int d = 1; d <= this->GetTool().inputParam.Derivative; d++ )
  {
    TimeLabelRecord currDerivativeRecord = procedureRT->DerivativeRT(d);
	for ( int j = 0; j < currDerivativeRecord.size(); j++ )
	{
      derivativeRecord.add( currDerivativeRecord.get(j) );
	}
  }
  derivativeProcedureRT->AddRecord( derivativeRecord );

  elapseTime[1] = currTime - this->MRMLNode->GetTimestamp();
  currTime = this->MRMLNode->GetTimestamp();

  // Apply orthogonal transformation
  orthogonalProcedureRT->AddRecord( derivativeProcedureRT->OrthogonalTransformationRT( this->GetTool().inputParam.OrthogonalWindow, this->GetTool().inputParam.OrthogonalOrder ) );

  elapseTime[2] = currTime - this->MRMLNode->GetTimestamp();
  currTime = this->MRMLNode->GetTimestamp();

  // Apply PCA transformation
  principalProcedureRT->AddRecord( orthogonalProcedureRT->TransformPCART( this->GetTool().trainingParam.PrinComps, this->GetTool().trainingParam.Mean ) );

  elapseTime[3] = currTime - this->MRMLNode->GetTimestamp();
  currTime = this->MRMLNode->GetTimestamp();

  // Apply centroid transformation
  centroidProcedureRT->AddRecord( principalProcedureRT->fwdkmeansTransformRT( this->GetTool().trainingParam.Centroids ) );

  elapseTime[4] = currTime - this->MRMLNode->GetTimestamp();
  currTime = this->MRMLNode->GetTimestamp();

  // Use Markov Model calculate states to come up with the current most likely state...
  MarkovRecord markovState = MarkovRT->CalculateStateRT( centroidProcedureRT->ToMarkovRecordRT() );

  elapseTime[5] = currTime - this->MRMLNode->GetTimestamp();
  currTime = this->MRMLNode->GetTimestamp();

  // Now, we will keep a recording of the workflow segmentation in procedureRT, in addition to returning it
  TimeLabelRecord newRecord;
  newRecord.values = procedureRT->GetRecordRT().values;
  newRecord.setTime( procedureRT->GetRecordRT().getTime() );
  newRecord.setLabel( markovState.getState() );
  procedureRT->SetRecordRT( newRecord );

  this->currentTask = markovState.getState();

  elapseTime[6] = currTime - this->MRMLNode->GetTimestamp();
  currTime = this->MRMLNode->GetTimestamp();

}



void vtkWorkflowAlgorithm
::UpdateTask()
{
  // Check if there are any new transforms to process
  if ( this->GetTool().transBuff.GetBufferSize() > indexLastProcessed )
  {

    TransformRecord currentTransform = this->GetTool().transBuff.GetTransformAt( indexLastProcessed );

    if ( this->ModuleNode->toolCollection.GetTrained() )
	{
	  addSegmentRecord( currentTransform );
	}
	else
	{
	  addRecord( currentTransform );
	}
	// Add to the segmentation buffer
	if ( this->currentTask != this->prevTask )
	{
      prevTask = currentTask;
	  this->GetTool().segBuff.AddMessage( this->getCurrentTask(), currentTransform.TimeStampSec, currentTransform.TimeStampNSec );
	}

	indexLastProcessed++;
  }

}


std::string vtkWorkflowAlgorithm
::getCurrentTask()
{
  if ( this->currentTask < 0 || this->currentTask > NumTasks )
  {
    return "-";
  }
  return TaskName[ this->currentTask ];
}


std::string vtkWorkflowAlgorithm
::getCurrentInstruction()
{
  if ( this->currentTask < 0 || this->currentTask > NumTasks )
  {
    return "-";
  }
  return TaskInstruction[ this->currentTask ];
}


std::string vtkWorkflowAlgorithm
::getNextTask()
{
  if ( this->currentTask < 0 || this->currentTask > NumTasks )
  {
    return "-";
  }

  int nextTaskIndex  = FindTaskIndex( TaskNext[ this->currentTask ] );

  if ( nextTaskIndex < 0 || nextTaskIndex > NumTasks )
  {
    return "-";
  }

  return TaskName[ nextTaskIndex ];
}


std::string vtkWorkflowAlgorithm
::getNextInstruction()
{
  if ( this->currentTask < 0 || this->currentTask > NumTasks )
  {
    return "-";
  }

  int nextTaskIndex  = FindTaskIndex( TaskNext[ this->currentTask ] );

  if ( nextTaskIndex < 0 || nextTaskIndex > NumTasks )
  {
    return "-";
  }

  return TaskInstruction[ nextTaskIndex ];
}