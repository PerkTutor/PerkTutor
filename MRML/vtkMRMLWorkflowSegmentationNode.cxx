
// WorkflowSegmentation MRML includes
#include "vtkMRMLWorkflowSegmentationNode.h"

// Constants ------------------------------------------------------------------
static const char* TOOL_REFERENCE_ROLE = "Tool";
static const char* TRANSFORM_BUFFER_REFERENCE_ROLE = "TransformBuffer";


// Constructors and Destructors
// ----------------------------------------------------------------------------

vtkMRMLWorkflowSegmentationNode* vtkMRMLWorkflowSegmentationNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSegmentationNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSegmentationNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSegmentationNode;
}


vtkMRMLNode* vtkMRMLWorkflowSegmentationNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSegmentationNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSegmentationNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSegmentationNode;
}



// Scene: Save and load
// ----------------------------------------------------------------------------


void vtkMRMLWorkflowSegmentationNode::WriteXML( ostream& of, int nIndent )
{
  this->Superclass::WriteXML(of, nIndent);
}



void vtkMRMLWorkflowSegmentationNode::ReadXMLAttributes( const char** atts )
{
  this->Superclass::ReadXMLAttributes(atts);
}



// Slicer Scene
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode::Copy( vtkMRMLNode *anode )
{  
  this->Superclass::Copy( anode );
}



void vtkMRMLWorkflowSegmentationNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


// Constructor/destructor
// ----------------------------------------------------------------------------
vtkMRMLWorkflowSegmentationNode
::vtkMRMLWorkflowSegmentationNode()
{
  this->AddNodeReferenceRole( TOOL_REFERENCE_ROLE );
}



vtkMRMLWorkflowSegmentationNode
::~vtkMRMLWorkflowSegmentationNode()
{
}


std::string vtkMRMLWorkflowSegmentationNode
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


// Transform buffer node
// ----------------------------------------------------------------------------

vtkMRMLTransformBufferNode* vtkMRMLWorkflowSegmentationNode
::GetTransformBufferNode()
{
  return vtkMRMLTransformBufferNode::SafeDownCast( this->GetNodeReference( TRANSFORM_BUFFER_REFERENCE_ROLE ) );
}


std::string vtkMRMLWorkflowSegmentationNode
::GetTransformBufferID()
{
  return this->GetNodeReferenceIDString( TRANSFORM_BUFFER_REFERENCE_ROLE );
}


void vtkMRMLWorkflowSegmentationNode
::SetTransformBufferID( std::string newTransformBufferID )
{
  vtkNew< vtkIntArray > events;
  events->InsertNextValue( vtkMRMLTransformBufferNode::TransformAddedEvent );
  events->InsertNextValue( vtkMRMLTransformBufferNode::RecordingStateChangedEvent );
  this->SetAndObserveNodeReferenceID( TRANSFORM_BUFFER_REFERENCE_ROLE, newTransformBufferID.c_str(), events.GetPointer() );
  // TODO: Reset all tools
}


bool vtkMRMLWorkflowSegmentationNode
::GetRealTimeProcessing()
{
  return this->RealTimeProcessing;
}


void vtkMRMLWorkflowSegmentationNode
::SetRealTimeProcessing( bool newRealTimeProcessing )
{
  if ( newRealTimeProcessing != this->RealTimeProcessing )
  {
    this->RealTimeProcessing = newRealTimeProcessing;
    this->Modified();
    if ( newRealTimeProcessing )
    {
      this->InvokeEvent( RealTimeProcessingStartedEvent );
    }
  }
}



// Deal with references to tool nodes
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode
::AddToolID( std::string toolID )
{
  this->AddAndObserveNodeReferenceID( TOOL_REFERENCE_ROLE, toolID.c_str() );
}


void vtkMRMLWorkflowSegmentationNode
::RemoveToolID( std::string toolID )
{
  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( TOOL_REFERENCE_ROLE ); i++ )
  {
    if ( toolID.compare( this->GetNthNodeReferenceID( TOOL_REFERENCE_ROLE, i ) ) == 0 )
    {
      this->RemoveNthNodeReferenceID( TOOL_REFERENCE_ROLE, i );
	    i--;      
	  }
  }
}


std::vector< std::string > vtkMRMLWorkflowSegmentationNode
::GetToolIDs()
{
  std::vector< std::string > toolIDs;

  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( TOOL_REFERENCE_ROLE ); i++ )
  {
    toolIDs.push_back( this->GetNthNodeReferenceID( TOOL_REFERENCE_ROLE, i ) );
  }

  return toolIDs;
}
  

bool vtkMRMLWorkflowSegmentationNode
::IsToolID( std::string toolID )
{
  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( TOOL_REFERENCE_ROLE ); i++ )
  {
    if ( toolID.compare( this->GetNthNodeReferenceID( TOOL_REFERENCE_ROLE, i ) ) == 0 )
    {
      return true;
    }
  }

  return false;
}


void vtkMRMLWorkflowSegmentationNode
::SetToolIDs( std::vector< std::string > toolIDs )
{
  int modifyState = this->StartModify();

  // Remove all of the active transform IDs
  while( this->GetNumberOfNodeReferences( TOOL_REFERENCE_ROLE ) > 0 )
  {
    this->RemoveNthNodeReferenceID( TOOL_REFERENCE_ROLE, 0 );
  }

  // Add all of the specified IDs
  for ( int i = 0; i < toolIDs.size(); i++ )
  {
    this->AddToolID( toolIDs.at( i ) );
  }

  this->EndModify( modifyState );
}



// MRML node events processing
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode
::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  // Propagate modified events
  if ( event == vtkCommand::ModifiedEvent )
  {
    this->Modified();
  }

  // The caller will be the node that was modified
  vtkMRMLTransformBufferNode* transformBuffer = vtkMRMLTransformBufferNode::SafeDownCast( caller );
  if ( transformBuffer == NULL )
  {
    return;
  }

  // Recording state of buffer changed
  if ( event == vtkMRMLTransformBufferNode::RecordingStateChangedEvent )
  { 
    bool* eventData = reinterpret_cast< bool* >( callData );
    if ( *eventData == false )
    {
      this->SetRealTimeProcessing( eventData ); // Stop real-time processing if recording has stopped (note: real-time processing should not necessarily be started whenever recording is started)
    }
  }


  // Transform added to buffer
  if ( event == vtkMRMLTransformBufferNode::TransformAddedEvent && this->RealTimeProcessing )
  { 
    vtkMRMLTransformBufferNode::TransformEventDataType* eventData = reinterpret_cast< vtkMRMLTransformBufferNode::TransformEventDataType* >( callData );
    if ( transformBuffer->GetTransformRecordBuffer( eventData->first )->GetNumRecords() == eventData->second + 1 )
    {
      this->InvokeEvent( TransformRealTimeAddedEvent, &eventData->first );
    }
  }

}