
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

std::string
LabelRecordVectorToString( std::vector<LabelRecord> records )
{
  std::stringstream ss;

  for ( int i = 0; i < records.size(); i++ )
  {
    ss << records[i].getLabel() << " ";
    for ( int j = 0; j < records[i].size(); j++ )
	{
      ss << records[i].get(j) << " ";
	}
  }

  return ss.str();

}


std::vector<LabelRecord>
StringToLabelRecordVector( std::string s, int numRecords, int recordSize )
{
  std::stringstream ss( s );
  std::vector<LabelRecord> recordVector;
  double val;

  for ( int i = 0; i < numRecords; i++ )
  {
    LabelRecord currRecord;
	ss >> val;
    currRecord.setLabel( val );
    for ( int j = 0; j < recordSize; j++ )
	{
      ss >> val;
	  currRecord.add( val );
	}
	recordVector.push_back( currRecord );
  }

  return recordVector;
}



std::string
LabelRecordToString( LabelRecord record )
{
  std::vector<LabelRecord> records;
  records.push_back( record );
  return LabelRecordVectorToString( records );
}



LabelRecord
StringToLabelRecord( std::string s, int recordSize )
{
  return StringToLabelRecordVector( s, 1, recordSize ).at(0);
}


//-------------------------------------------------------------------------
// Class functions
//-------------------------------------------------------------------------


vtkStandardNewMacro( vtkWorkflowAlgorithm );

vtkWorkflowAlgorithm
::vtkWorkflowAlgorithm()
{
}

vtkWorkflowAlgorithm
::~vtkWorkflowAlgorithm()
{
  for ( int i = 0 ; i < procedures.size(); i++ )
  {
    delete [] procedures.at(i);
  }
  procedures.clear();
}




void vtkWorkflowAlgorithm
::setMRMLNode( vtkMRMLWorkflowSegmentationNode* newMRMLNode )
{
  this->MRMLNode = newMRMLNode;
}



void vtkWorkflowAlgorithm
::GetProcedureDefinitionFromMRMLNode()
{
  this->NumTasks = this->MRMLNode->procDefn.NumTasks;
  this->TaskName = this->MRMLNode->procDefn.TaskName;
  this->TaskInstruction = this->MRMLNode->procDefn.TaskInstruction;
  this->TaskNext = this->MRMLNode->procDefn.TaskNext;

}



void vtkWorkflowAlgorithm
::GetInputParamtersFromMRMLNode()
{
  this->Derivative = this->MRMLNode->inputParam.Derivative;
  this->FilterWidth = this->MRMLNode->inputParam.FilterWidth;
  this->OrthogonalWindow = this->MRMLNode->inputParam.OrthogonalWindow;
  this->OrthogonalOrder = this->MRMLNode->inputParam.OrthogonalOrder;
  this->NumPrinComps = this->MRMLNode->inputParam.NumPrinComps;
  this->NumCentroids = this->MRMLNode->inputParam.NumCentroids;
  this->MarkovPseudoScalePi = this->MRMLNode->inputParam.MarkovPseudoScalePi;
  this->MarkovPseudoScaleA = this->MRMLNode->inputParam.MarkovPseudoScaleA;
  this->MarkovPseudoScaleB = this->MRMLNode->inputParam.MarkovPseudoScaleB;
}



void vtkWorkflowAlgorithm
::GetTrainingParametersFromMRMLNode()
{
  int SizePrinComps = ( this->OrthogonalOrder + 1 ) * ( TRACKINGRECORD_SIZE ) * ( this->Derivative + 1 ); // TODO: * this->MRMLNode->numTools;

  this->PrinComps = StringToLabelRecordVector( this->MRMLNode->trainingParam.PrinComps, NumPrinComps, SizePrinComps );
  this->Mean = StringToLabelRecord( this->MRMLNode->trainingParam.Mean, SizePrinComps );
  this->Centroids = StringToLabelRecordVector( this->MRMLNode->trainingParam.Centroids, NumCentroids, NumPrinComps );

  Markov = vtkMarkovModel::New();
  Markov->SetSize( NumTasks, NumCentroids );
  this->Markov->SetPi( StringToLabelRecord( this->MRMLNode->trainingParam.MarkovPi, NumTasks ) );
  this->Markov->SetA( StringToLabelRecordVector( this->MRMLNode->trainingParam.MarkovA, NumTasks, NumTasks ) );
  this->Markov->SetB( StringToLabelRecordVector( this->MRMLNode->trainingParam.MarkovB, NumTasks, NumCentroids ) );
}




int vtkWorkflowAlgorithm
::FindTaskIndex( std::string name )
{
  // Just iterate through all of them... otherwise assign -1
  for ( int i = 0; i < NumTasks; i++ )
  {
    if ( strcmp( TaskName.at(i).c_str(), name.c_str() ) == 0 )
	{
	  return i;
	}
  }

  return -1;
}





std::vector<double> vtkWorkflowAlgorithm
::CalculateTaskProportions()
{
  // Create a vector of counts for each label
  std::vector<int> taskCounts;
  for ( int i = 0; i < this->NumTasks; i++ )
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
  for ( int i = 0; i < this->NumTasks; i++ )
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
  std::vector<double> taskRawCentroids ( this->NumTasks, 0 );
  std::vector<double> fracPart ( this->NumTasks, 0 );

  for ( int i = 0; i < this->NumTasks; i++ )
  {
    taskRawCentroids[i] = taskProportions[i] * this->NumCentroids;
  }

  bool rounded = true;
  int sumCentroids = 0;
  for ( int i = 0; i < this->NumTasks; i++ )
  {
    sumCentroids += floor( taskRawCentroids[i] );
  }    

  while ( sumCentroids < this->NumCentroids )
  {

    rounded = true;
    for ( int i = 0; i < this->NumTasks; i++ )
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
    for ( int i = 0; i < this->NumTasks; i++ )
    {
      fracPart[i] = taskRawCentroids[i] - floor( taskRawCentroids[i] );
	  if ( fracPart[i] > fracPart[priority] )
	  {
        priority = i;
	  }
    }

	taskRawCentroids[ priority ] = ceil( taskRawCentroids[ priority ] );

	sumCentroids = 0;

	for ( int i = 0; i < this->NumTasks; i++ )
	{
      sumCentroids += taskRawCentroids[i];
	}    

  }

  std::vector<int> taskCentroids ( this->NumTasks, 0 );
  for ( int i = 0; i < this->NumTasks; i++ )
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
  // Create vectors of time records and message records
  std::vector<TimeLabelRecord> messages;
  std::vector<TimeLabelRecord> records;

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( fileName.c_str() );
  parser->Parse();
  
  // Get the root element (and check it exists)
  vtkXMLDataElement* rootElement = parser->GetRootElement();
  if ( ! rootElement )
  {
    return;
  }
  
  int num = rootElement->GetNumberOfNestedElements();  // Number of saved records (including transforms and messages).
  
  for ( int i = 0; i < num; ++ i )
  {

    vtkXMLDataElement* noteElement = rootElement->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "log" ) != 0 )
    {
      continue;  // If it's not a "log", jump to the next.
    }

	const char* elementType = noteElement->GetAttribute( "type" );
   
	int timeSec = atoi( noteElement->GetAttribute( "TimeStampSec" ) );
	int timeNSec = atoi( noteElement->GetAttribute( "TimeStampNSec" ) );
	double elementTime = timeSec + 1.0e-9 * timeNSec;
    
    
    if ( strcmp( elementType, "message" ) == 0 )
    {
	  TimeLabelRecord currMessage;
	  currMessage.setLabel( FindTaskIndex( std::string( noteElement->GetAttribute( "message" ) ) ) );
	  currMessage.setTime( elementTime );
      messages.push_back( currMessage );
    }

    if ( strcmp( elementType, "transform" ) == 0 )
    {
	  TimeLabelRecord currRecord;
	  std::string transformString = std::string( noteElement->GetAttribute( "transform" ) );
	  vtkTrackingRecord* currTrackingRecord = vtkTrackingRecord::New();
	  currTrackingRecord->fromMatrixString( transformString );
	  currRecord.values = currTrackingRecord->GetVector();
	  currRecord.setTime( elementTime );
	  records.push_back( currRecord );
    }

  }

  // Now, use the records messages (manual segmentation) to determine the label (task) at each recorded transform
  for ( int i = 0; i < records.size(); i++ )
  {
    // Find the record with the largest smaller time stamp
    int currTask = -1;
	double currTimeDiff = ( records[records.size()-1].getTime() - records[0].getTime() );
	for ( int j = 0; j < messages.size(); j++ )
	{
      if ( messages[j].getTime() < records[i].getTime() && records[i].getTime() - messages[j].getTime() < currTimeDiff )
	  {
        currTask = messages[j].getLabel();
		currTimeDiff = records[i].getTime() - messages[j].getTime();
	  }
	}
	// Set the label
	records[i].setLabel( currTask );
	// We shall represent tasks starting at zero here
  }
  
  // Only add the record to the log if there is a previous message or before stop message(otherwise task undefinfed -> discard)
  vtkRecordLog* currProcedure = new vtkRecordLog();
  for ( int i = 0; i < records.size(); i++ )
  {
    if ( records[i].getLabel() >= 0 && records[i].getLabel() < NumTasks )
	{
      currProcedure->AddRecord( records[i] );
	}
  }

  procedures.push_back( currProcedure );

}


// TODO: This is for testing only. Eventually we might like a similar method for segmenting previously recorded procedures.
void vtkWorkflowAlgorithm
::SegmentProcedure( std::string fileName )
{

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( fileName.c_str() );
  parser->Parse();
  
  // Get the root element (and check it exists)
  vtkXMLDataElement* rootElement = parser->GetRootElement();
  if ( ! rootElement )
  {
    return;
  }
  
  int num = rootElement->GetNumberOfNestedElements();  // Number of saved records (including transforms and messages).
  
  for ( int i = 0; i < num; ++ i )
  {

    vtkXMLDataElement* noteElement = rootElement->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "log" ) != 0 )
    {
      continue;  // If it's not a "log", jump to the next.
    }

	const char* elementType = noteElement->GetAttribute( "type" );
   
	int timeSec = atoi( noteElement->GetAttribute( "TimeStampSec" ) );
	int timeNSec = atoi( noteElement->GetAttribute( "TimeStampNSec" ) );
	double elementTime = timeSec + 1.0e-9 * timeNSec;
    
    if ( strcmp( elementType, "transform" ) == 0 )
    {
	  TransformRecord rec;
	  rec.DeviceName = "Test";
	  rec.Transform = std::string( noteElement->GetAttribute( "transform" ) );
	  rec.TimeStampNSec = timeNSec;
	  rec.TimeStampSec = timeSec;
	  addSegmentRecord( rec );
    }

  }


}




// Return whether or not training was successful
bool vtkWorkflowAlgorithm
::train()
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
  for ( int i = 0; i < NumTasks; i ++ )
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
  // TODO: This is very slow, can we make it faster by only using "nearby" time stamps
  std::vector<vtkRecordLog*> filterProcedures;
  for ( int i = 0; i < procedures.size(); i++ )
  {
    filterProcedures.push_back( procedures[i]->GaussianFilter( this->FilterWidth ) );
  }


  // Use velocity also
  std::vector<vtkRecordLog*> derivativeProcedures;
  for ( int i = 0; i < procedures.size(); i++ )
  {
    derivativeProcedures.push_back( filterProcedures[i] );
    for ( int d = 1; d <= this->Derivative; d++ )
	{
	  derivativeProcedures[i] = derivativeProcedures[i]->ConcatenateValues( filterProcedures[i]->Derivative(d) );
	}
  }


  // Apply orthogonal transformation
  std::vector<vtkRecordLog*> orthogonalProcedures;
  for ( int i = 0; i < filterProcedures.size(); i++ )
  {
    orthogonalProcedures.push_back( derivativeProcedures[i]->OrthogonalTransformation( OrthogonalWindow, OrthogonalOrder ) );
  }

  // Concatenate all of the record logs into one record log
  vtkRecordLog* orthogonalCat = vtkRecordLog::New();
  orthogonalCat->Initialize( 0, orthogonalProcedures[0]->RecordSize() );
  for ( int i = 0; i < orthogonalProcedures.size(); i++ )
  {
    orthogonalCat = orthogonalCat->Concatenate( orthogonalProcedures[i] );
  }

  // Calculate and apply the PCA transform
  this->Mean.values = orthogonalCat->Mean().values;
  this->Mean.setLabel( 0 );
  this->PrinComps = orthogonalCat->CalculatePCA( this->NumPrinComps );
  std::vector<vtkRecordLog*> principalProcedures;
  for ( int i = 0; i < orthogonalProcedures.size(); i++ )
  {
    principalProcedures.push_back( orthogonalProcedures[i]->TransformPCA( this->PrinComps, this->Mean ) );
  }
  vtkRecordLog* principalCat = orthogonalCat->TransformPCA( this->PrinComps, this->Mean );

  // Put together all the tasks together for task by task clustering
  std::vector<vtkRecordLog*> recordsByTask = principalCat->GroupRecordsByLabel( this->NumTasks );

  // Add the centroids from each task
  // TODO: Change NumCentroids back to 700
  for ( int i = 0; i < this->NumTasks; i++ )
  {
	std::vector<LabelRecord> currTaskCentroids = recordsByTask[i]->fwdkmeans( taskCentroids[i] );
	for ( int j = 0; j < taskCentroids[i]; j++ )
	{
	  currTaskCentroids[j].setLabel( currTaskCentroids[j].getLabel() + cumulativeCentroids[i] );
      this->Centroids.push_back( currTaskCentroids[j] );
	}
  }

  // Calculate the sequence of centroids for each procedure
  std::vector<vtkRecordLog*> centroidProcedures;
  for ( int i = 0; i < principalProcedures.size(); i++ )
  {
    centroidProcedures.push_back( principalProcedures[i]->fwdkmeansTransform( this->Centroids ) );
  }

  // Assume that all the estimation matrices are ones
  LabelRecord PseudoPi;
  for ( int j = 0; j < NumTasks; j++ )
  {
    PseudoPi.add( 1.0 * this->MarkovPseudoScalePi );
  }
  PseudoPi.setLabel(0);

  std::vector<LabelRecord> PseudoA;
  for ( int i = 0; i < NumTasks; i++ )
  {
	LabelRecord currPseudoA;
	for ( int j = 0; j < NumTasks; j++ )
	{
      currPseudoA.add( 1.0 * this->MarkovPseudoScaleA );
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
      currPseudoB.add( 1.0 * this->MarkovPseudoScaleB );
	}
	currPseudoB.setLabel(i);
	PseudoB.push_back( currPseudoB );
  }

  // Create a new Markov Model, and estimate its parameters
  Markov = vtkMarkovModel::New();
  Markov->SetSize( this->NumTasks, this->NumCentroids );
  this->Markov->InitializeEstimation( NumTasks, this->NumCentroids );
  this->Markov->AddPseudoData( PseudoPi, PseudoA, PseudoB );
  for ( int i = 0; i < centroidProcedures.size(); i++ )
  {
    this->Markov->AddEstimationData( centroidProcedures[i]->ToMarkovRecordVector() );
  }
  this->Markov->EstimateParameters();


  // Now, change the associated values in the MRML
  // Assume that the input parameters are ok (if they are not then this whole training procedure was useless anyway)
  this->MRMLNode->trainingParam.PrinComps = LabelRecordVectorToString( this->PrinComps );
  this->MRMLNode->trainingParam.Mean = LabelRecordToString( this->Mean );
  this->MRMLNode->trainingParam.Centroids = LabelRecordVectorToString( this->Centroids );
  this->MRMLNode->trainingParam.MarkovA = LabelRecordVectorToString( this->Markov->GetA() );
  this->MRMLNode->trainingParam.MarkovB = LabelRecordVectorToString( this->Markov->GetB() );
  this->MRMLNode->trainingParam.MarkovPi = LabelRecordToString( this->Markov->GetPi() );

  return true;

}



void vtkWorkflowAlgorithm
::InitializeSegmentationRT()
{
  procedureRT = vtkRecordLogRT::New();
  derivativeProcedureRT = vtkRecordLogRT::New();
  filterProcedureRT = vtkRecordLogRT::New();
  orthogonalProcedureRT = vtkRecordLogRT::New();
  principalProcedureRT = vtkRecordLogRT::New();
  centroidProcedureRT = vtkRecordLogRT::New();

  MarkovRT = vtkMarkovModelRT::New();
  MarkovRT->SetSize( this->NumTasks, this->NumCentroids );
  MarkovRT->SetPi( this->Markov->GetPi() );
  MarkovRT->SetA( this->Markov->GetA() );
  MarkovRT->SetB( this->Markov->GetB() );

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

}



void vtkWorkflowAlgorithm
::addSegmentRecord( TransformRecord t )
{
  addRecord( t );

  // TODO: Only keep the most recent observations (a few for filtering, a window for orthogonal transformation)

  // Apply Gaussian filtering to each previous records
  filterProcedureRT->AddRecord( procedureRT->GaussianFilterRT( this->FilterWidth ) );
  
  // Concatenate with derivative (velocity, acceleration, etc...)
  TimeLabelRecord derivativeRecord = filterProcedureRT->GetRecordRT();
  for ( int d = 1; d <= this->Derivative; d++ )
  {
    TimeLabelRecord currDerivativeRecord = procedureRT->DerivativeRT(d);
	for ( int j = 0; j < currDerivativeRecord.size(); j++ )
	{
      derivativeRecord.add( currDerivativeRecord.get(j) );
	}
  }
  derivativeProcedureRT->AddRecord( derivativeRecord );

  // Apply orthogonal transformation
  orthogonalProcedureRT->AddRecord( derivativeProcedureRT->OrthogonalTransformationRT( this->OrthogonalWindow, this->OrthogonalOrder ) );

  // Apply PCA transformation
  principalProcedureRT->AddRecord( orthogonalProcedureRT->TransformPCART( this->PrinComps, this->Mean ) );

  // Apply centroid transformation
  centroidProcedureRT->AddRecord( principalProcedureRT->fwdkmeansTransformRT( this->Centroids ) );

  // Use Markov Model calculate states to come up with the current most likely state...
  MarkovRecord markovState = MarkovRT->CalculateStateRT( centroidProcedureRT->ToMarkovRecordRT() );

  // Now, we will keep a recording of the workflow segmentation in procedureRT, in addition to returning it
  TimeLabelRecord newRecord;
  newRecord.values = procedureRT->GetRecordRT().values;
  newRecord.setTime( procedureRT->GetRecordRT().getTime() );
  newRecord.setLabel( markovState.getState() );
  procedureRT->SetRecordRT( newRecord );

  this->currentTask = markovState.getState();

}



void vtkWorkflowAlgorithm
::UpdateTask()
{
  // Check if there are any new transforms to process
  if ( this->MRMLNode->GetTransformsBufferSize() > indexLastProcessed )
  {
    if ( this->MRMLNode->GetIsTrained() )
	{
	  addSegmentRecord( this->MRMLNode->GetTransformAt( indexLastProcessed ) );
	}
	else
	{
	  addRecord( this->MRMLNode->GetTransformAt( indexLastProcessed ) );
	}
	// Add to the segmentation buffer
	if ( this->currentTask != this->prevTask )
	{
      prevTask = currentTask;
	  std::stringstream ss;
	  ss << currentTask;
	  this->MRMLNode->AddSegmentation( ss.str() );
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