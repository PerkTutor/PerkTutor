
#include "vtkMRMLWorkflowToolNode.h"

// Constants ------------------------------------------------------------------
static const char* WORKFLOW_PROCEDURE_REFERENCE_ROLE = "ProcedureDefinition";
static const char* WORKFLOW_INPUT_REFERENCE_ROLE = "ProcedureInput";
static const char* WORKFLOW_TRAINING_REFERENCE_ROLE = "ProcedureTraining";

// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLWorkflowToolNode* vtkMRMLWorkflowToolNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowToolNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowToolNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowToolNode();
}


vtkMRMLNode* vtkMRMLWorkflowToolNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowToolNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowToolNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowToolNode();
}



void vtkMRMLWorkflowToolNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLWorkflowToolNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  
  of << indent << "Name=\"" << this->Name << "\"";
}


void vtkMRMLWorkflowToolNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);
  
  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
 
    if ( ! strcmp( attName, "Name" ) )
    {
      this->ToolName = std::string( attValue );
    }
  }
}


void vtkMRMLWorkflowToolNode
::Copy( vtkMRMLNode *anode )
{
  this->vtkMRMLNode::Copy( anode ); // this will copy all of the node references
  vtkMRMLWorkflowToolNode *node = ( vtkMRMLWorkflowToolNode* ) anode;
  if ( node == NULL )
  {
    return;
  }
  
  this->SetToolName( node->GetToolName() );
}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLWorkflowToolNode
::vtkMRMLWorkflowToolNode()
{
  this->Name = "";
  this->ResetBuffers();
}


vtkMRMLWorkflowToolNode
::~vtkMRMLWorkflowToolNode()
{
}


void vtkMRMLWorkflowToolNode
::ResetBuffers()
{
  this->RawBuffer = vtkSmartPointer< vtkWorkflowLogRecordBufferRT >::New();
  this->FilterBuffer = vtkSmartPointer< vtkWorkflowLogRecordBufferRT >::New();
  this->DerivativeBuffer = vtkSmartPointer< vtkWorkflowLogRecordBufferRT >::New();
  this->OrthogonalBuffer = vtkSmartPointer< vtkWorkflowLogRecordBufferRT >::New();
  this->PcaBuffer = vtkSmartPointer< vtkWorkflowLogRecordBufferRT >::New();
  this->CentroidBuffer = vtkSmartPointer< vtkWorkflowLogRecordBufferRT >::New();
  
  this->CurrentTask = vtkSmartPointer< vtkWorkflowTask >::New();
}


bool vtkMRMLWorkflowToolNode
::GetDefined()
{
  return ( this->GetWorkflowProcedureNode() != NULL );
}


bool vtkMRMLWorkflowToolNode
::GetInputted()
{
  return ( this->GetWorkflowInputNode() != NULL );
}


bool vtkMRMLWorkflowToolNode
::GetTrained()
{
  return ( this->GetWorkflowTrainingNode() != NULL );
}

std::string vtkMRMLWorkflowToolNode
::GetNodeReferenceIDString( std::string referenceRole )
{
  const char* refID = this->GetNodeReferenceID( referenceRole.c_str() );
  std::string refIDString;

  if ( refID == NULL )
  {
    refIDString = "";
  }
  else
  {
    refIDString = refID;
  }

  return refIDString;
}


vtkMRMLWorkflowProcedureNode* vtkMRMLWorkflowToolNode
::GetWorkflowProcedureNode()
{
  return vtkMRMLWorkflowProcedureNode::SafeDownCast( this->GetNodeReference( WORKFLOW_PROCEDURE_REFERENCE_ROLE ) );
}


std::string vtkMRMLWorkflowToolNode
::GetWorkflowProcedureID()
{
  return this->GetNodeReferenceIDString( WORKFLOW_PROCEDURE_REFERENCE_ROLE );
}


void vtkMRMLWorkflowToolNode
::SetWorkflowProcedureID( std::string newWorkflowProcedureID )
{
  this->SetAndObserveNodeReferenceID( WORKFLOW_PROCEDURE_REFERENCE_ROLE, newWorkflowProcedureID.c_str() );
}


vtkMRMLWorkflowInputNode* vtkMRMLWorkflowToolNode
::GetWorkflowInputNode()
{
  return vtkMRMLWorkflowInputNode::SafeDownCast( this->GetNodeReference( WORKFLOW_INPUT_REFERENCE_ROLE ) );
}


std::string vtkMRMLWorkflowToolNode
::GetWorkflowInputID()
{
  return this->GetNodeReferenceIDString( WORKFLOW_INPUT_REFERENCE_ROLE );
}


void vtkMRMLWorkflowToolNode
::SetWorkflowInputID( std::string newWorkflowInputID )
{
  this->SetAndObserveNodeReferenceID( WORKFLOW_INPUT_REFERENCE_ROLE, newWorkflowInputID.c_str() );
}


vtkMRMLWorkflowTrainingNode* vtkMRMLWorkflowToolNode
::GetWorkflowTrainingNode()
{
  return vtkMRMLWorkflowTrainingNode::SafeDownCast( this->GetNodeReference( WORKFLOW_TRAINING_REFERENCE_ROLE ) );
}


std::string vtkMRMLWorkflowToolNode
::GetWorkflowTrainingID()
{
  return this->GetNodeReferenceIDString( WORKFLOW_TRAINING_REFERENCE_ROLE );
}


void vtkMRMLWorkflowToolNode
::SetWorkflowTrainingID( std::string newWorkflowTrainingID )
{
  this->SetAndObserveNodeReferenceID( WORKFLOW_TRAINING_REFERENCE_ROLE, newWorkflowTrainingID.c_str() );
}


// Computational methods
// Return whether or not training was successful
bool vtkMRMLWorkflowToolNode
::Train( std::vector< vtkSmartPointer< vtkWorkflowLogRecordBuffer > > trainingBuffers )
{
  // There must exist procedures
  if ( trainingBuffers.empty() )
  {
    return false;
  }

  // Calculate the number of centroids for each task  
  std::vector< std::string > taskNames = this->GetWorkflowProcedureNode()->GetAllTaskNames();
  
  std::map< std::string, int > taskNumCentroids = this->CalculateTaskNumCentroids( trainingBuffers );
  std::map< std::string, int > taskCumCentroids;
  int currSum = 0;

  // Calculate the cumulative number of centroids for cluster numbering
  std::map< std::string, int > cumulativeCentroids;
  std::map< std::string, int >::iterator itrInt;
  for ( itrInt = taskNumCentroids.begin(); itrInt != taskNumCentroids.end(); itrInt++ )
  {
	// Make sure that every task is represented in the procedures
	  if ( itrInt->second == 0 )
	  {
      return false;
	  }

	  cumulativeCentroids[ itrInt->first ] = currSum;
	  currSum += itrInt->second;
  }

  // Apply Gaussian filtering to each record log
  std::vector< vtkSmartPointer< vtkWorkflowLogRecordBuffer > > filterBuffers;
  for ( int i = 0; i < trainingBuffers.size(); i++ )
  {
    vtkSmartPointer< vtkWorkflowLogRecordBuffer > currFilterBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
    currFilterBuffer->Copy( trainingBuffers.at( i ) );
    currFilterBuffer->GaussianFilter( this->GetWorkflowInputNode()->GetFilterWidth() );
    filterBuffers.push_back( currFilterBuffer );
  }


  // Use velocity and higher order derivatives also
  std::vector< vtkSmartPointer< vtkWorkflowLogRecordBuffer > > derivativeBuffers;
  for ( int i = 0; i < filterBuffers.size(); i++ )
  {
    vtkSmartPointer< vtkWorkflowLogRecordBuffer > currDerivativeBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
    currDerivativeBuffer->Copy( filterBuffers.at( i ) );
    
    for ( int d = 1; d <= this->GetWorkflowInputNode()->GetDerivative(); d++ )
	  {
	    vtkSmartPointer< vtkWorkflowLogRecordBuffer > currOrderBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
      currOrderBuffer->Copy( filterBuffers.at( i )  );
      currOrderBuffer->Differentiate( d );
      
      currDerivativeBuffer->ConcatenateValues( currOrderBuffer );
	  }
    
    derivativeBuffers.push_back( currDerivativeBuffer );
  }


  // Apply orthogonal transformation
  std::vector< vtkSmartPointer< vtkWorkflowLogRecordBuffer > > orthogonalBuffers;
  for ( int i = 0; i < derivativeBuffers.size(); i++ )
  {
    vtkSmartPointer< vtkWorkflowLogRecordBuffer > currOrthogonalBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
    currOrthogonalBuffer->Copy( derivativeBuffers.at( i ) );
    currOrthogonalBuffer->OrthogonalTransformation( this->GetWorkflowInputNode()->GetOrthogonalWindow(), this->GetWorkflowInputNode()->GetOrthogonalOrder() );
    orthogonalBuffers.push_back( currOrthogonalBuffer );
  }

  // Concatenate all of the record logs into one record log
  // Observe that the concatenated buffers are sorted by time stamp - its order is not maintained by procedure, but this is ok
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > concatenatedOrthogonalBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
  for ( int i = 0; i < orthogonalBuffers.size(); i++ )
  {
    concatenatedOrthogonalBuffer->Concatenate( orthogonalBuffers.at( i ) );
  }

  // Calculate PCA transform
  this->GetWorkflowTrainingNode()->SetMean( concatenatedOrthogonalBuffer->Mean() );
  this->GetWorkflowTrainingNode()->SetPrinComps( concatenatedOrthogonalBuffer->CalculatePCA( this->GetWorkflowInputNode()->GetNumPrinComps() ) );

  // Apply PCA transformation
  std::vector< vtkSmartPointer< vtkWorkflowLogRecordBuffer > > pcaBuffers;
  for ( int i = 0; i < orthogonalBuffers.size(); i++ )
  {
    vtkSmartPointer< vtkWorkflowLogRecordBuffer > currPCABuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
    currPCABuffer->Copy( orthogonalBuffers.at( i ) );
    currPCABuffer->TransformPCA( this->GetWorkflowTrainingNode()->GetPrinComps(), this->GetWorkflowTrainingNode()->GetMean() );
    pcaBuffers.push_back( currPCABuffer );
  }
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > concatenatedPCABuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
  concatenatedPCABuffer->Copy( concatenatedOrthogonalBuffer );
  concatenatedPCABuffer->TransformPCA( this->GetWorkflowTrainingNode()->GetPrinComps(), this->GetWorkflowTrainingNode()->GetMean() );

  // Put together all the tasks together for task by task clustering
  std::map< std::string, vtkSmartPointer< vtkWorkflowLogRecordBuffer > > taskwiseBuffers;
  for ( int i = 0; i < taskNames.size(); i++ )
  {
    std::vector< std::string > currTask;
    currTask.push_back( taskNames.at( i ) );    
    taskwiseBuffers[ taskNames.at( i ) ] = concatenatedPCABuffer->GetLabelledRange( currTask );
  }

  // Calculate and add the centroids from each task
  std::map< std::string, vtkSmartPointer< vtkWorkflowLogRecordBuffer > >::iterator itrBuffer;
  for ( itrBuffer = taskwiseBuffers.begin(); itrBuffer != taskwiseBuffers.end(); itrBuffer++ )
  {
	  std::vector< vtkSmartPointer< vtkLabelVector > > currTaskCentroids = itrBuffer->second->fwdkmeans( taskNumCentroids[ itrBuffer->first ] );
    
	  for ( int j = 0; j < currTaskCentroids.size(); j++ )
	  {
      // Make the centroid numbering continuous over all centroids
      currTaskCentroids.at( j )->SetLabel( atoi( currTaskCentroids.at( j )->GetLabel().c_str() ) + cumulativeCentroids[ itrBuffer->first ] );
      this->GetWorkflowTrainingNode()->GetCentroids().push_back( currTaskCentroids.at( j ) );
	  }    
  }

  // Calculate the sequence of centroids for each procedure
  std::vector< vtkSmartPointer< vtkWorkflowLogRecordBuffer > > centroidBuffers;
  for ( int i = 0; i < pcaBuffers.size(); i++ )
  {
    vtkSmartPointer< vtkWorkflowLogRecordBuffer > currCentroidBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
    currCentroidBuffer->Copy( pcaBuffers.at( i ) );
    currCentroidBuffer->fwdkmeansTransform( this->GetWorkflowTrainingNode()->GetCentroids() );
    pcaBuffers.push_back( currCentroidBuffer );
  }

  // Assume that all the estimation matrices are associated with the pseudo scales
  vtkSmartPointer< vtkLabelVector > PseudoPi = vtkSmartPointer< vtkLabelVector >::New();
  PseudoPi->FillElements( this->GetWorkflowProcedureNode()->GetNumTasks(), this->GetWorkflowInputNode()->GetMarkovPseudoScalePi() );
  PseudoPi->SetLabel( "Pi" );

  std::vector< vtkSmartPointer< vtkLabelVector > > PseudoA;
  for ( int i = 0; i < taskNames.size(); i++ ) // Observe that we still have the task names
  {
	  vtkSmartPointer< vtkLabelVector > currPseudoA = vtkSmartPointer< vtkLabelVector >::New();
	  currPseudoA->FillElements( this->GetWorkflowProcedureNode()->GetNumTasks(), this->GetWorkflowInputNode()->GetMarkovPseudoScaleA() );
	  currPseudoA->SetLabel( taskNames.at( i ) );
	  PseudoA.push_back( currPseudoA );
  }

  std::vector< vtkSmartPointer< vtkLabelVector > > PseudoB;
  for ( int i = 0; i < taskNames.size(); i++ )
  {
    vtkSmartPointer< vtkLabelVector > currPseudoB = vtkSmartPointer< vtkLabelVector >::New();
	  currPseudoB->FillElements( this->GetWorkflowInputNode()->GetNumCentroids(), this->GetWorkflowInputNode()->GetMarkovPseudoScaleB() );
	  currPseudoB->SetLabel( taskNames.at( i ) );
	  PseudoB.push_back( currPseudoB );
  }

  // Create a new Markov Model, and estimate its parameters
  vtkSmartPointer< vtkMarkovModel > Markov = vtkSmartPointer< vtkMarkovModel >::New();
  Markov->SetStates( taskNames );
  Markov->SetSymbols( this->GetWorkflowInputNode()->GetNumCentroids() );
  Markov->InitializeEstimation();
  Markov->AddPseudoData( PseudoPi, PseudoA, PseudoB );
  for ( int i = 0; i < centroidBuffers.size(); i++ )
  {
    std::vector< vtkSmartPointer< vtkMarkovVector > > markovVectors = centroidBuffers.at(i)->ToMarkovVectors();
    Markov->AddEstimationData( markovVectors );
  }
  Markov->EstimateParameters();

  this->GetWorkflowTrainingNode()->GetMarkov()->vtkMarkovModel::Copy( Markov ); // Need to use the superclass copy

  return true;
}


void vtkMRMLWorkflowToolNode
::AddAndSegmentRecord( vtkLabelRecord* newRecord )
{
  this->RawBuffer->AddRecord( newRecord );

  // Apply Gaussian filtering to each previous records
  this->FilterBuffer->AddRecord( this->RawBuffer->GaussianFilterRT( this->GetWorkflowInputNode()->GetFilterWidth() ) );
  
  // Concatenate with derivative (velocity, acceleration, etc...)
  vtkSmartPointer< vtkLabelRecord > derivativeRecord = vtkSmartPointer< vtkLabelRecord >::New();
  derivativeRecord->Copy( vtkLabelRecord::SafeDownCast( this->FilterBuffer->GetCurrentRecord() ) );
  for ( int d = 1; d <= this->GetWorkflowInputNode()->GetDerivative(); d++ )
  {
    vtkSmartPointer< vtkLabelRecord > currDerivativeRecord = this->FilterBuffer->DifferentiateRT( d );
    derivativeRecord->GetVector()->Concatenate( currDerivativeRecord->GetVector() );
  }
  this->DerivativeBuffer->AddRecord( derivativeRecord );

  // Apply orthogonal transformation
  this->OrthogonalBuffer->AddRecord( this->DerivativeBuffer->OrthogonalTransformationRT( this->GetWorkflowInputNode()->GetOrthogonalWindow(), this->GetWorkflowInputNode()->GetOrthogonalOrder() ) );

  // Apply PCA transformation
  this->PcaBuffer->AddRecord( this->OrthogonalBuffer->TransformPCART( this->GetWorkflowTrainingNode()->GetPrinComps(), this->GetWorkflowTrainingNode()->GetMean() ) );

  // Apply centroid transformation
  this->CentroidBuffer->AddRecord( this->PcaBuffer->fwdkmeansTransformRT( this->GetWorkflowTrainingNode()->GetCentroids() ) );

  // Use Markov Model calculate states to come up with the current most likely state...
  vtkSmartPointer< vtkMarkovVector > markovVectorRT = this->CentroidBuffer->ToMarkovVectorRT();
  this->GetWorkflowTrainingNode()->GetMarkov()->CalculateStateRT( markovVectorRT );

  // Now, we will keep a recording of the workflow segmentation in RawBuffer - add the label
  vtkSmartPointer< vtkLabelRecord > currLabelRecord = vtkLabelRecord::SafeDownCast( this->RawBuffer->GetCurrentRecord() );
  currLabelRecord->GetVector()->SetLabel( markovVectorRT->GetState() );

  this->CurrentTask = this->GetWorkflowProcedureNode()->GetTask( markovVectorRT->GetState() );
}


vtkWorkflowTask* vtkMRMLWorkflowToolNode
::GetCurrentTask()
{
  return this->CurrentTask;
}



// Helpers for computation
// -----------------------------------------------------------------------------------------

std::map< std::string, double > vtkMRMLWorkflowToolNode
::CalculateTaskProportions( std::vector< vtkSmartPointer< vtkWorkflowLogRecordBuffer > > trainingBuffers )
{
  // Create a vector of counts for each label
  std::map< std::string, double > taskProportions;
  std::vector< std::string > taskNames = this->GetWorkflowProcedureNode()->GetAllTaskNames();
  for ( int i = 0; i < taskNames.size(); i++ )
  {
    taskProportions[ taskNames.at( i ) ] = 0;
  }

  int totalRecords = 0;
  // Iterate over all record logs and count label (task) instances
  for ( int i = 0; i < trainingBuffers.size(); i++ )
  {
    for ( int j = 0; j < trainingBuffers.at(i)->GetNumRecords(); j++ )
	  {
      vtkSmartPointer< vtkLabelRecord > currRecord = vtkLabelRecord::SafeDownCast( trainingBuffers.at( i )->GetRecord( j ) );
      std::string currTaskName = currRecord->GetVector()->GetLabel();
      for ( int k = 0; k < taskNames.size(); k++ )
      {
        if ( currTaskName.compare( taskNames.at( k ) ) == 0 )
	      {
	        taskProportions[ currTaskName ]++;
	        totalRecords++;
	      }
      }
	  }
  }

    // Calculate the proportion of each task
  std::map< std::string, double >::iterator itr;
  for ( itr = taskProportions.begin(); itr != taskProportions.end(); itr++ )
  {
    itr->second = itr->second / totalRecords;
  }

  return taskProportions;
}



std::map< std::string, double > vtkMRMLWorkflowToolNode
::EqualizeTaskProportions( std::vector< vtkSmartPointer< vtkWorkflowLogRecordBuffer > > trainingBuffers )
{
  //Find the mean and standard deviation of the task centroids
  std::map< std::string, double > taskProportions = this->CalculateTaskProportions( trainingBuffers );

  double mean = 0;
  std::map< std::string, double >::iterator itrDouble;
  for ( itrDouble = taskProportions.begin(); itrDouble != taskProportions.end(); itrDouble++ )
  {
    mean += itrDouble->second;
  }
  mean = mean / taskProportions.size();

  // Reduce the standard deviation by the equalizing parameter
  for ( itrDouble = taskProportions.begin(); itrDouble != taskProportions.end(); itrDouble++ )
  {
    itrDouble->second = ( itrDouble->second - mean ) / this->GetWorkflowInputNode()->GetEqualization() + mean;
  }

  return taskProportions;
}




std::map< std::string, int > vtkMRMLWorkflowToolNode
::CalculateTaskNumCentroids( std::vector< vtkSmartPointer< vtkWorkflowLogRecordBuffer > > trainingBuffers )
{
  // Create a vector of counts for each label
  std::map< std::string, double > taskProportions = this->EqualizeTaskProportions( trainingBuffers );
  std::map< std::string, double > taskRawCentroids;

  std::map< std::string, double >::iterator itrDouble;
  int sumCentroids = 0;
  for ( itrDouble = taskProportions.begin(); itrDouble != taskProportions.end(); itrDouble++ )
  {
    taskRawCentroids[ itrDouble->first ] = itrDouble->second * this->GetWorkflowInputNode()->GetNumCentroids();
    sumCentroids += floor( itrDouble->second );    
  }

  while ( sumCentroids < this->GetWorkflowInputNode()->GetNumCentroids() )
  {
    // It can never happen that all the centroid have been rounded and the sum is insufficient
    // So there is no need to check

    // Find the highest "priority" (with largest fractional part) centroid count to increase
    double priorityFraction = 0.0;
	  std::string priority = "";
    for ( itrDouble = taskRawCentroids.begin(); itrDouble != taskRawCentroids.end(); itrDouble++ )
    {
      double currFracPart = itrDouble->second - floor( itrDouble->second );
	    if ( currFracPart > priorityFraction )
	    {
        priority = itrDouble->first;
        priorityFraction = currFracPart;
	    }
    }

	  taskRawCentroids[ priority ] = ceil( taskRawCentroids[ priority ] );

    sumCentroids = 0;
    for ( itrDouble = taskProportions.begin(); itrDouble != taskProportions.end(); itrDouble++ )
    {
      sumCentroids += floor( itrDouble->second );
	  }    

  }

  // Actually create the correct integer number of centroids
  std::map< std::string, int > taskCentroids;
  for ( itrDouble = taskRawCentroids.begin(); itrDouble != taskRawCentroids.end(); itrDouble++ )
  {
    taskCentroids[ itrDouble->first ] = floor( itrDouble->second );
  } 

  return taskCentroids;
}