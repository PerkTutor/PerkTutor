
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
    for ( int j = 0; i < records[i].size(); j++ )
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
	  currRecord.set( j, val );
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
::GetInputParamtersFromMRMLNode()
{
  this->NumTasks = this->MRMLNode->inputParam.NumTasks;
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
  int SizePrinComps = ( OrthogonalOrder + 1 ) * TRACKINGRECORD_SIZE; // TODO: * this->MRMLNode->numTools;

  this->PrinComps = StringToLabelRecordVector( this->MRMLNode->trainingParam.PrinComps, NumPrinComps, SizePrinComps );
  this->Centroids = StringToLabelRecordVector( this->MRMLNode->trainingParam.Centroids, NumCentroids, NumPrinComps );
  this->Markov->SetPi( StringToLabelRecord( this->MRMLNode->trainingParam.MarkovPi, NumTasks ) );
  this->Markov->SetA( StringToLabelRecordVector( this->MRMLNode->trainingParam.MarkovA, NumTasks, NumTasks ) );
  this->Markov->SetA( StringToLabelRecordVector( this->MRMLNode->trainingParam.MarkovA, NumTasks, NumCentroids ) );
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
	  currMessage.setLabel( atoi( noteElement->GetAttribute( "message" ) ) );
	  currMessage.setTime( elementTime );
      messages.push_back( currMessage );
    }

    if ( strcmp( elementType, "transform" ) == 0 )
    {
	  TimeLabelRecord currRecord;
	  std::string transformString = std::string( noteElement->GetAttribute( "transform" ) );
	  vtkTrackingRecord* currTrackingRecord = new vtkTrackingRecord();
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
    int currTask = 0;
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
  }
  
  // Only add the record to the log if there is a previous message or before stop message(otherwise task undefinfed -> discard)
  vtkRecordLog* currProcedure = new vtkRecordLog();
  for ( int i = 0; i < records.size(); i++ )
  {
    if ( records[i].getLabel() >= 1 && records[i].getLabel() <= NumTasks )
	{
      currProcedure->AddRecord( records[i] );
	}
  }

  procedures.push_back( currProcedure );

}





void vtkWorkflowAlgorithm
::train()
{

  // Calculate the number of centroids for each task
  std::vector<int> taskCentroids;
  std::vector<double> taskProportions;
  taskProportions = CalculateTaskProportions();
  for ( int i = 0; i < NumTasks; i ++ )
  {
    taskCentroids.push_back( taskProportions[i] * this->NumCentroids );
  }

  // Apply Gaussian filtering to each record log
  std::vector<vtkRecordLog*> filterProcedures;
  for ( int i = 0; i < procedures.size(); i++ )
  {
    filterProcedures.push_back( procedures[i]->GaussianFilter( this->FilterWidth ) );
  }

  // Apply orthogonal transformation
  std::vector<vtkRecordLog*> orthogonalProcedures;
  for ( int i = 0; i < filterProcedures.size(); i++ )
  {
    orthogonalProcedures.push_back( filterProcedures[i]->OrthogonalTransformation( OrthogonalWindow, OrthogonalOrder ) );
  }

  // Concatenate all of the record logs into one record log
  vtkRecordLog* orthogonalCat = new vtkRecordLog();
  for ( int i = 0; i < orthogonalProcedures.size(); i++ )
  {
    orthogonalCat->Concatenate( orthogonalProcedures[i] );
  }

  // Calculate and apply the PCA transform
  this->PrinComps = orthogonalCat->CalculatePCA( this->NumPrinComps );
  std::vector<vtkRecordLog*> principalProcedures;
  for ( int i = 0; i < orthogonalProcedures.size(); i++ )
  {
    principalProcedures.push_back( orthogonalProcedures[i]->TransformPCA( this->PrinComps ) );
  }
  vtkRecordLog* principalCat = orthogonalCat->TransformPCA( this->PrinComps );

  // Put together all the tasks together for task by task clustering
  std::vector<vtkRecordLog*> recordsByTask = principalCat->GroupRecordsByLabel( this->NumTasks );

  // Add the centroids from each task
  for ( int i = 0; i < this->NumTasks; i++ )
  {
	std::vector<LabelRecord> currTaskCentroids = recordsByTask[i]->fwdkmeans( taskCentroids[i] );
	for ( int j = 0; j < taskCentroids[i]; j++ )
	{
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
  std::vector<double> ones;
  ones.assign( 1.0 * this->MarkovPseudoScalePi, this->NumTasks );
  PseudoPi.values = ones;
  PseudoPi.setLabel(0);

  std::vector<LabelRecord> PseudoA;
  for ( int i = 0; i < NumTasks; i++ )
  {
	LabelRecord currPseudoA;
	std::vector<double> ones;
	ones.assign( 1.0 * this->MarkovPseudoScaleA, this->NumTasks );
	currPseudoA.values = ones;
	currPseudoA.setLabel(i);
	PseudoA.push_back( currPseudoA );
  }

  std::vector<LabelRecord> PseudoB;
  for ( int i = 0; i < NumTasks; i++ )
  {
	LabelRecord currPseudoB;
	std::vector<double> ones;
	ones.assign( 1.0 * this->MarkovPseudoScaleB, this->NumCentroids );
	currPseudoB.values = ones;
	currPseudoB.setLabel(i);
	PseudoB.push_back( currPseudoB );
  }

  // Create a new Markov Model, and estimate its parameters
  this->Markov->InitializeEstimation( NumTasks, NumCentroids );
  this->Markov->AddPseudoData( PseudoPi, PseudoA, PseudoB );
  for ( int i = 0; i < centroidProcedures.size(); i++ )
  {
    this->Markov->AddEstimationData( centroidProcedures[i]->ToMarkovRecordVector() );
  }
  this->Markov->EstimateParameters();


  // Now, change the associated values in the MRML
  // Assume that the input parameters are ok (if they are not then this whole training procedure was useless anyway)
  this->MRMLNode->trainingParam.PrinComps = LabelRecordVectorToString( this->PrinComps );
  this->MRMLNode->trainingParam.Centroids = LabelRecordVectorToString( this->Centroids );
  this->MRMLNode->trainingParam.MarkovA = LabelRecordVectorToString( this->Markov->GetA() );
  this->MRMLNode->trainingParam.MarkovB = LabelRecordVectorToString( this->Markov->GetB() );
  this->MRMLNode->trainingParam.MarkovPi = LabelRecordToString( this->Markov->GetPi() );


}
