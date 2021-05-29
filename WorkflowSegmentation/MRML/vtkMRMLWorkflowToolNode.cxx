
#include "vtkMRMLWorkflowToolNode.h"

// Constants ------------------------------------------------------------------
static const char* TOOL_TRANSFORM_REFERENCE_ROLE = "ToolTransform";
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
  
  of << indent << "ToolName=\"" << this->ToolName << "\"";
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
 
    if ( ! strcmp( attName, "ToolName" ) )
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
  this->CurrentTaskNew = false;
  this->ToolName = "";
  this->ResetWorkflowSequences();

  vtkNew< vtkIntArray > events;
  events->InsertNextValue( vtkCommand::ModifiedEvent );
  this->AddNodeReferenceRole( WORKFLOW_PROCEDURE_REFERENCE_ROLE, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( WORKFLOW_INPUT_REFERENCE_ROLE, NULL, events.GetPointer() );
  this->AddNodeReferenceRole( WORKFLOW_TRAINING_REFERENCE_ROLE, NULL, events.GetPointer() );
}


vtkMRMLWorkflowToolNode
::~vtkMRMLWorkflowToolNode()
{
}


void vtkMRMLWorkflowToolNode
::ResetWorkflowSequences()
{
  this->RawWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceOnlineNode >::New();
  this->FilterWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceOnlineNode >::New();
  this->DerivativeWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceOnlineNode >::New();
  this->OrthogonalWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceOnlineNode >::New();
  this->PcaWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceOnlineNode >::New();
  this->CentroidWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceOnlineNode >::New();
  
  this->CurrentTask = vtkSmartPointer< vtkWorkflowTask >::New();
}


bool vtkMRMLWorkflowToolNode
::IsWorkflowProcedureSet()
{
  return ( this->GetWorkflowProcedureNode() != NULL );
}


bool vtkMRMLWorkflowToolNode
::IsWorkflowInputSet()
{
  return ( this->GetWorkflowInputNode() != NULL );
}


bool vtkMRMLWorkflowToolNode
::IsWorkflowTrainingSet()
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


vtkMRMLLinearTransformNode* vtkMRMLWorkflowToolNode
::GetToolTransformNode()
{
  return vtkMRMLLinearTransformNode::SafeDownCast( this->GetNodeReference( TOOL_TRANSFORM_REFERENCE_ROLE ) );
}


std::string vtkMRMLWorkflowToolNode
::GetToolTransformID()
{
  return this->GetNodeReferenceIDString( TOOL_TRANSFORM_REFERENCE_ROLE );
}


void vtkMRMLWorkflowToolNode
::SetToolTransformID( std::string newToolTransformID )
{
  this->SetAndObserveNodeReferenceID( TOOL_TRANSFORM_REFERENCE_ROLE, newToolTransformID.c_str() );
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

  // Also, set the name of this tool
  if ( this->GetWorkflowProcedureNode() != NULL )
  {
    this->SetToolName( this->GetWorkflowProcedureNode()->GetProcedureName() );
  }
  else
  {
    this->SetToolName( "" );
  }
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
::Train( vtkCollection* trainingWorkflowSequences )
{
  // Calculate the number of centroids for each task  
  std::vector< std::string > taskNames = this->GetWorkflowProcedureNode()->GetAllTaskNames();
  
  std::map< std::string, int > taskNumCentroids = this->CalculateTaskNumCentroids( trainingWorkflowSequences );
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
  vtkNew< vtkCollection > filterWorkflowSequences;
  vtkNew< vtkCollectionIterator > workflowSequencesIt; workflowSequencesIt->SetCollection( trainingWorkflowSequences );
  for ( workflowSequencesIt->InitTraversal(); ! workflowSequencesIt->IsDoneWithTraversal(); workflowSequencesIt->GoToNextItem() )
  {
    vtkMRMLWorkflowSequenceNode* currWorkflowSequence = vtkMRMLWorkflowSequenceNode::SafeDownCast( workflowSequencesIt->GetCurrentObject() );
    if ( currWorkflowSequence == NULL )
    {
      continue;
    }

    vtkSmartPointer< vtkMRMLWorkflowSequenceNode > currFilterWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
    currFilterWorkflowSequence->Copy( currWorkflowSequence );
    currFilterWorkflowSequence->GaussianFilter( this->GetWorkflowInputNode()->GetFilterWidth() );
    filterWorkflowSequences->AddItem( currFilterWorkflowSequence );
  }


  // Use velocity and higher order derivatives also
  vtkNew< vtkCollection > derivativeWorkflowSequences;
  vtkNew< vtkCollectionIterator > filterWorkflowSequencesIt; filterWorkflowSequencesIt->SetCollection( filterWorkflowSequences.GetPointer() );
  for ( filterWorkflowSequencesIt->InitTraversal(); ! filterWorkflowSequencesIt->IsDoneWithTraversal(); filterWorkflowSequencesIt->GoToNextItem() )
  {
    vtkMRMLWorkflowSequenceNode* currFilterWorkflowSequence = vtkMRMLWorkflowSequenceNode::SafeDownCast( filterWorkflowSequencesIt->GetCurrentObject() );
    if ( currFilterWorkflowSequence == NULL )
    {
      continue;
    }

    vtkSmartPointer< vtkMRMLWorkflowSequenceNode > currDerivativeWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
    currDerivativeWorkflowSequence->Copy( currFilterWorkflowSequence );
    
    for ( int d = 1; d <= this->GetWorkflowInputNode()->GetDerivative(); d++ )
	  {
	    vtkSmartPointer< vtkMRMLWorkflowSequenceNode > currOrderWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
      currOrderWorkflowSequence->Copy( currFilterWorkflowSequence );
      currOrderWorkflowSequence->Differentiate( d );
      
      currDerivativeWorkflowSequence->ConcatenateValues( currOrderWorkflowSequence );
	  }
    
    derivativeWorkflowSequences->AddItem( currDerivativeWorkflowSequence );
  }


  // Apply orthogonal transformation
  vtkNew< vtkCollection > orthogonalWorkflowSequences;
  vtkNew< vtkCollectionIterator > derivativeWorkflowSequencesIt; derivativeWorkflowSequencesIt->SetCollection( derivativeWorkflowSequences.GetPointer() );
  for ( derivativeWorkflowSequencesIt->InitTraversal(); ! derivativeWorkflowSequencesIt->IsDoneWithTraversal(); derivativeWorkflowSequencesIt->GoToNextItem() )
  {
    vtkMRMLWorkflowSequenceNode* currDerivativeWorkflowSequence = vtkMRMLWorkflowSequenceNode::SafeDownCast( derivativeWorkflowSequencesIt->GetCurrentObject() );
    if ( currDerivativeWorkflowSequence == NULL )
    {
      continue;
    }

    vtkSmartPointer< vtkMRMLWorkflowSequenceNode > currOrthogonalWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
    currOrthogonalWorkflowSequence->Copy( currDerivativeWorkflowSequence );
    currOrthogonalWorkflowSequence->OrthogonalTransformation( this->GetWorkflowInputNode()->GetOrthogonalWindow(), this->GetWorkflowInputNode()->GetOrthogonalOrder() );
    orthogonalWorkflowSequences->AddItem( currOrthogonalWorkflowSequence );
  }

  // Concatenate all of the record logs into one record log
  // Observe that the concatenated buffers are sorted by time stamp - its order is not maintained by procedure, but this is ok
  vtkNew< vtkMRMLWorkflowSequenceNode > concatenatedOrthogonalWorkflowSequence;
  vtkNew< vtkCollectionIterator > orthogonalWorkflowSequencesIt; orthogonalWorkflowSequencesIt->SetCollection( orthogonalWorkflowSequences.GetPointer() );
  for ( orthogonalWorkflowSequencesIt->InitTraversal(); ! orthogonalWorkflowSequencesIt->IsDoneWithTraversal(); orthogonalWorkflowSequencesIt->GoToNextItem() )
  {
    vtkMRMLWorkflowSequenceNode* currOrthogonalWorkflowSequence = vtkMRMLWorkflowSequenceNode::SafeDownCast( orthogonalWorkflowSequencesIt->GetCurrentObject() );
    if ( currOrthogonalWorkflowSequence == NULL )
    {
      continue;
    }

    concatenatedOrthogonalWorkflowSequence->Concatenate( currOrthogonalWorkflowSequence, true );
  }

  // Calculate PCA transform
  vtkSmartPointer< vtkDoubleArray > mean = vtkSmartPointer< vtkDoubleArray >::New();
  concatenatedOrthogonalWorkflowSequence->Mean( mean );
  this->GetWorkflowTrainingNode()->SetMean( mean );

  vtkSmartPointer< vtkDoubleArray > prinComps = vtkSmartPointer< vtkDoubleArray >::New();
  concatenatedOrthogonalWorkflowSequence->CalculatePrincipalComponents( this->GetWorkflowInputNode()->GetNumPrinComps(), prinComps );
  this->GetWorkflowTrainingNode()->SetPrinComps( prinComps );

  // Apply PCA transformation
  vtkNew< vtkCollection > pcaWorkflowSequences;
  for ( orthogonalWorkflowSequencesIt->InitTraversal(); ! orthogonalWorkflowSequencesIt->IsDoneWithTraversal(); orthogonalWorkflowSequencesIt->GoToNextItem() )
  {
    vtkMRMLWorkflowSequenceNode* currOrthogonalWorkflowSequence = vtkMRMLWorkflowSequenceNode::SafeDownCast( orthogonalWorkflowSequencesIt->GetCurrentObject() );
    if ( currOrthogonalWorkflowSequence == NULL )
    {
      continue;
    }

    vtkSmartPointer< vtkMRMLWorkflowSequenceNode > currPCAWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
    currPCAWorkflowSequence->Copy( currOrthogonalWorkflowSequence );
    currPCAWorkflowSequence->TransformByPrincipalComponents( this->GetWorkflowTrainingNode()->GetPrinComps(), this->GetWorkflowTrainingNode()->GetMean() );
    pcaWorkflowSequences->AddItem( currPCAWorkflowSequence );
  }
  vtkSmartPointer< vtkMRMLWorkflowSequenceNode > concatenatedPCAWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
  concatenatedPCAWorkflowSequence->Copy( concatenatedOrthogonalWorkflowSequence.GetPointer() );
  concatenatedPCAWorkflowSequence->TransformByPrincipalComponents( this->GetWorkflowTrainingNode()->GetPrinComps(), this->GetWorkflowTrainingNode()->GetMean() );

  // Put together all the tasks together for task by task clustering
  std::map< std::string, vtkSmartPointer< vtkMRMLWorkflowSequenceNode > > taskwiseWorkflowSequences;
  for ( int i = 0; i < taskNames.size(); i++ )
  {
    std::vector< std::string > currTask;
    currTask.push_back( taskNames.at( i ) );

    vtkSmartPointer< vtkMRMLWorkflowSequenceNode > currTaskwiseWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
    concatenatedPCAWorkflowSequence->GetLabelledSubsequence( currTask, currTaskwiseWorkflowSequence );
    taskwiseWorkflowSequences[ taskNames.at( i ) ] = currTaskwiseWorkflowSequence;
  }

  // Calculate and add the centroids from each task
  vtkSmartPointer< vtkDoubleArray > allCentroids = vtkSmartPointer< vtkDoubleArray >::New();
  allCentroids->SetNumberOfComponents( this->GetWorkflowInputNode()->GetNumPrinComps() );
  allCentroids->SetNumberOfTuples( 0 ); // We will append tuples

  std::map< std::string, vtkSmartPointer< vtkMRMLWorkflowSequenceNode > >::iterator taskwiseWorfklowSequencesIt;
  for ( taskwiseWorfklowSequencesIt = taskwiseWorkflowSequences.begin(); taskwiseWorfklowSequencesIt != taskwiseWorkflowSequences.end(); taskwiseWorfklowSequencesIt++ )
  {
    vtkNew< vtkDoubleArray > currTaskCentroids;
	  taskwiseWorfklowSequencesIt->second->fwdkmeans( taskNumCentroids[ taskwiseWorfklowSequencesIt->first ], currTaskCentroids.GetPointer() ); // Second is the workflow sequence node

    allCentroids->InsertTuples( allCentroids->GetNumberOfTuples(), currTaskCentroids->GetNumberOfTuples(), 0, currTaskCentroids.GetPointer() );
  }
  this->GetWorkflowTrainingNode()->SetCentroids( allCentroids );

  // Calculate the sequence of centroids for each procedure
  vtkNew< vtkCollection > centroidWorkflowSequences;
  vtkNew< vtkCollectionIterator > pcaWorkflowSequencesIt; pcaWorkflowSequencesIt->SetCollection( pcaWorkflowSequences.GetPointer() );
  for ( pcaWorkflowSequencesIt->InitTraversal(); ! pcaWorkflowSequencesIt->IsDoneWithTraversal(); pcaWorkflowSequencesIt->GoToNextItem() )
  {
    vtkMRMLWorkflowSequenceNode* currPCAWorkflowSequence = vtkMRMLWorkflowSequenceNode::SafeDownCast( pcaWorkflowSequencesIt->GetCurrentObject() );
    if ( currPCAWorkflowSequence == NULL )
    {
      continue;
    }

    vtkSmartPointer< vtkMRMLWorkflowSequenceNode > currCentroidWorkflowSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
    currCentroidWorkflowSequence->Copy( currPCAWorkflowSequence );
    currCentroidWorkflowSequence->fwdkmeansTransform( this->GetWorkflowTrainingNode()->GetCentroids() );
    centroidWorkflowSequences->AddItem( currCentroidWorkflowSequence );
  }

  // Assume that all the estimation matrices are associated with the pseudo scales
  vtkSmartPointer< vtkDoubleArray > PseudoPi = vtkSmartPointer< vtkDoubleArray >::New();
  PseudoPi->SetNumberOfComponents( this->GetWorkflowProcedureNode()->GetNumTasks() );
  PseudoPi->SetNumberOfTuples( 1 );
  for ( int j = 0; j < PseudoPi->GetNumberOfComponents(); j++ )
  {
    PseudoPi->FillComponent( j, this->GetWorkflowInputNode()->GetMarkovPseudoScalePi() ); // TODO: We want to call the "Fill" function, but it is not yet available in Slicer's VTK
  }

  vtkSmartPointer< vtkDoubleArray > PseudoA = vtkSmartPointer< vtkDoubleArray >::New();
  PseudoA->SetNumberOfComponents( this->GetWorkflowProcedureNode()->GetNumTasks() );
  PseudoA->SetNumberOfTuples( this->GetWorkflowProcedureNode()->GetNumTasks() );
  for ( int j = 0; j < PseudoA->GetNumberOfComponents(); j++ )
  {
    PseudoA->FillComponent( j, this->GetWorkflowInputNode()->GetMarkovPseudoScaleA() ); // TODO: We want to call the "Fill" function, but it is not yet available in Slicer's VTK
  }

  vtkSmartPointer< vtkDoubleArray > PseudoB = vtkSmartPointer< vtkDoubleArray >::New();
  PseudoB->SetNumberOfComponents( this->GetWorkflowInputNode()->GetNumCentroids() );
  PseudoB->SetNumberOfTuples( this->GetWorkflowProcedureNode()->GetNumTasks() );
  for ( int j = 0; j < PseudoB->GetNumberOfComponents(); j++ )
  {
    PseudoB->FillComponent( j, this->GetWorkflowInputNode()->GetMarkovPseudoScaleB() ); // TODO: We want to call the "Fill" function, but it is not yet available in Slicer's VTK
  }

  // Create a new Markov Model, and estimate its parameters
  vtkSmartPointer< vtkMarkovModel > Markov = vtkSmartPointer< vtkMarkovModel >::New();
  Markov->SetStates( taskNames );
  Markov->SetSymbols( this->GetWorkflowInputNode()->GetNumCentroids() );
  Markov->InitializeEstimation();

  vtkNew< vtkCollectionIterator > centroidWorkflowSequencesIt;

  Markov->AddPseudoData( PseudoPi, PseudoA, PseudoB );

  // TODO: Dedugging
  centroidWorkflowSequencesIt->SetCollection( centroidWorkflowSequences.GetPointer() );
  for ( centroidWorkflowSequencesIt->InitTraversal(); ! centroidWorkflowSequencesIt->IsDoneWithTraversal(); centroidWorkflowSequencesIt->GoToNextItem() )
  {
    vtkMRMLWorkflowSequenceNode* currCentroidWorkflowSequence = vtkMRMLWorkflowSequenceNode::SafeDownCast( centroidWorkflowSequencesIt->GetCurrentObject() );
    if ( currCentroidWorkflowSequence == NULL )
    {
      continue;
    }

    currCentroidWorkflowSequence->AddMarkovModelAttributes();
    Markov->AddEstimationData( currCentroidWorkflowSequence );
  }
  Markov->EstimateParameters();

  this->GetWorkflowTrainingNode()->GetMarkov()->vtkMarkovModel::Copy( Markov ); // Need to use the superclass copy

  return true;
}


void vtkMRMLWorkflowToolNode
::AddAndSegmentTransform( vtkMRMLLinearTransformNode* newTransformNode, std::string newTimeString )
{
  vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode > rawDoubleArrayNode = vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode >::New();
  vtkMRMLWorkflowSequenceNode::LinearTransformToDoubleArray( newTransformNode, rawDoubleArrayNode, vtkMRMLWorkflowSequenceNode::QUATERNION_ARRAY );
  this->RawWorkflowSequence->SetDataNodeAtValue( rawDoubleArrayNode, newTimeString );

  // Apply Gaussian filtering to each previous records
  vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode > gaussDoubleArrayNode = vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode >::New();
  this->RawWorkflowSequence->GaussianFilterOnline( this->GetWorkflowInputNode()->GetFilterWidth(), gaussDoubleArrayNode->GetArray() );
  this->FilterWorkflowSequence->SetDataNodeAtValue( gaussDoubleArrayNode, newTimeString );
  
  // Concatenate with derivative (velocity, acceleration, etc...)
  
  vtkNew< vtkDoubleArray > derivativeDoubleArray;
  derivativeDoubleArray->DeepCopy( this->FilterWorkflowSequence->GetNthDoubleArray( this->FilterWorkflowSequence->GetNumberOfDataNodes() - 1 ) );
  for ( int d = 1; d <= this->GetWorkflowInputNode()->GetDerivative(); d++ )
  {    
    vtkSmartPointer< vtkDoubleArray > tempDerivativeDoubleArray = vtkSmartPointer< vtkDoubleArray >::New();
    tempDerivativeDoubleArray->DeepCopy( derivativeDoubleArray.GetPointer() );

    vtkSmartPointer< vtkDoubleArray > currDerivativeDoubleArray = vtkSmartPointer< vtkDoubleArray >::New();
    this->FilterWorkflowSequence->DifferentiateOnline( d, currDerivativeDoubleArray );

    vtkMRMLWorkflowSequenceNode::ConcatenateDoubleArrays( tempDerivativeDoubleArray, currDerivativeDoubleArray, derivativeDoubleArray.GetPointer() );
  }
  vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode > derivativeDoubleArrayNode = vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode >::New();
  derivativeDoubleArrayNode->GetArray()->DeepCopy( derivativeDoubleArray.GetPointer() );
  this->DerivativeWorkflowSequence->SetDataNodeAtValue( derivativeDoubleArrayNode, newTimeString );

  // Apply orthogonal transformation
  vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode > orthogonalDoubleArrayNode = vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode >::New();
  this->DerivativeWorkflowSequence->OrthogonalTransformationOnline( this->GetWorkflowInputNode()->GetOrthogonalWindow(), this->GetWorkflowInputNode()->GetOrthogonalOrder(), orthogonalDoubleArrayNode->GetArray() );
  this->OrthogonalWorkflowSequence->SetDataNodeAtValue( orthogonalDoubleArrayNode, newTimeString );

  // Apply PCA transformation
  vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode > pcaDoubleArrayNode = vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode >::New();
  this->OrthogonalWorkflowSequence->TransformByPrincipalComponentsOnline( this->GetWorkflowTrainingNode()->GetPrinComps(), this->GetWorkflowTrainingNode()->GetMean(), pcaDoubleArrayNode->GetArray() );
  this->PcaWorkflowSequence->SetDataNodeAtValue( pcaDoubleArrayNode, newTimeString );

  // Apply centroid transformation
  vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode > fwdkmeansDoubleArrayNode = vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode >::New();
  this->PcaWorkflowSequence->fwdkmeansTransformOnline( this->GetWorkflowTrainingNode()->GetCentroids(), fwdkmeansDoubleArrayNode->GetArray() );
  this->CentroidWorkflowSequence->SetDataNodeAtValue( fwdkmeansDoubleArrayNode, newTimeString );

  // Use Markov Model calculate states to come up with the current most likely state...
  // Now, we will keep a recording of the workflow segmentation
  vtkMRMLNode* currCentroidWorkflowSequenceNode = vtkMRMLNode::SafeDownCast( this->CentroidWorkflowSequence->GetNthDataNode( this->RawWorkflowSequence->GetNumberOfDataNodes() - 1 ) );
  this->CentroidWorkflowSequence->AddMarkovModelAttributesOnline( currCentroidWorkflowSequenceNode );
  this->GetWorkflowTrainingNode()->GetMarkov()->CalculateStateOnline( currCentroidWorkflowSequenceNode, newTimeString );
  currCentroidWorkflowSequenceNode->SetAttribute( "Message", currCentroidWorkflowSequenceNode->GetAttribute( "MarkovState" ) );

  this->SetCurrentTask( this->GetWorkflowProcedureNode()->GetTask( currCentroidWorkflowSequenceNode->GetAttribute( "Message" ) ) );
}


vtkWorkflowTask* vtkMRMLWorkflowToolNode
::GetCurrentTask()
{
  return this->CurrentTask;
}


void vtkMRMLWorkflowToolNode
::SetCurrentTask( vtkWorkflowTask* newCurrentTask )
{
  if ( newCurrentTask == this->CurrentTask )
  {
    return;
  }

  this->CurrentTask = newCurrentTask;
  this->Modified();
  this->InvokeEvent( CurrentTaskChangedEvent );
}


// Helpers for computation
// -----------------------------------------------------------------------------------------

std::map< std::string, double > vtkMRMLWorkflowToolNode
::CalculateTaskProportions( vtkCollection* trainingWorkflowSequenceNodes )
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
  vtkNew< vtkCollectionIterator > workflowSequencesIt; workflowSequencesIt->SetCollection( trainingWorkflowSequenceNodes );
  for ( workflowSequencesIt->InitTraversal(); ! workflowSequencesIt->IsDoneWithTraversal(); workflowSequencesIt->GoToNextItem() )
  {
    vtkMRMLWorkflowSequenceNode* currWorkflowSequence = vtkMRMLWorkflowSequenceNode::SafeDownCast( workflowSequencesIt->GetCurrentObject() );
    if ( currWorkflowSequence == NULL )
    {
      continue;
    }
    
    for ( int j = 0; j < currWorkflowSequence->GetNumberOfDataNodes(); j++ )
	  {
      vtkMRMLWorkflowDoubleArrayNode* currDoubleArrayNode = vtkMRMLWorkflowDoubleArrayNode::SafeDownCast( currWorkflowSequence->GetNthDataNode( j ) );
      if ( currDoubleArrayNode == NULL )
      {
        continue;
      }

      for ( int k = 0; k < taskNames.size(); k++ )
      {
        if ( currDoubleArrayNode->GetAttribute( "Message" ) != NULL
          && taskNames.at( k ).compare( currDoubleArrayNode->GetAttribute( "Message" ) ) == 0 )
	      {
	        taskProportions[ taskNames.at( k ) ]++;
	        totalRecords++;
	      }
      }

	  }
  }

  // If all of the record were unlabelled then do not divide, just return
  if ( totalRecords == 0 )
  {
    return taskProportions;
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
::EqualizeTaskProportions( vtkCollection* trainingWorkflowSequenceNodes )
{
  //Find the mean and standard deviation of the task centroids
  std::map< std::string, double > taskProportions = this->CalculateTaskProportions( trainingWorkflowSequenceNodes );

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
::CalculateTaskNumCentroids( vtkCollection* trainingWorkflowSequenceNodes )
{
  // Create a vector of counts for each label
  std::map< std::string, double > taskProportions = this->EqualizeTaskProportions( trainingWorkflowSequenceNodes );
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

    // Could not find anything to ceil (probably because there are no labelled records)
    if ( taskRawCentroids.find( priority ) == taskRawCentroids.end() )
    {
      break;
    }

	  taskRawCentroids[ priority ] = ceil( taskRawCentroids[ priority ] );

    sumCentroids = 0;
    for ( itrDouble = taskRawCentroids.begin(); itrDouble != taskRawCentroids.end(); itrDouble++ )
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


// MRML node events processing
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowToolNode
::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  // This will propagate the ReferenceNodeModifiedEvent
  this->vtkMRMLNode::ProcessMRMLEvents( caller, event, callData );
  
  // Propagate ModifiedEvent
  if ( event == vtkCommand::ModifiedEvent )
  {
    this->Modified();
  }

}