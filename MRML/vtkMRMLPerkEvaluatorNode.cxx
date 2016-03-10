

#include "vtkMRMLPerkEvaluatorNode.h"

// Constants ------------------------------------------------------------------
static const char* TRANSFORM_BUFFER_REFERENCE_ROLE = "TransformBuffer";
static const char* METRICS_TABLE_REFERENCE_ROLE = "MetricsTable";
static const char* METRIC_INSTANCE_REFERENCE_ROLE = "MetricInstance";


// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLPerkEvaluatorNode* vtkMRMLPerkEvaluatorNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLPerkEvaluatorNode" );
  if( ret )
    {
      return ( vtkMRMLPerkEvaluatorNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLPerkEvaluatorNode();
}


vtkMRMLNode* vtkMRMLPerkEvaluatorNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLPerkEvaluatorNode" );
  if( ret )
    {
      return ( vtkMRMLPerkEvaluatorNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLPerkEvaluatorNode();
}



void vtkMRMLPerkEvaluatorNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLPerkEvaluatorNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  
  of << indent << "AutoUpdateMeasurementRange=\"" << this->AutoUpdateMeasurementRange << "\"";
  of << indent << "AutoUpdateTransformRoles=\"" << this->AutoUpdateTransformRoles << "\"";
  of << indent << "MarkBegin=\"" << this->MarkBegin << "\"";
  of << indent << "MarkEnd=\"" << this->MarkEnd << "\"";
  of << indent << "NeedleOrientation=\"" << this->NeedleOrientation << "\"";
  of << indent << "MetricsDirectory=\"" << this->MetricsDirectory << "\"";
  of << indent << "PlaybackTime=\"" << this->PlaybackTime << "\"";
  of << indent << "RealTimeProcessing=\"" << this->RealTimeProcessing << "\"";
}


void vtkMRMLPerkEvaluatorNode
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
 
    if ( ! strcmp( attName, "AutoUpdateMeasurementRange" ) )
    {
      this->AutoUpdateMeasurementRange = atoi( attValue );
    }
    if ( ! strcmp( attName, "AutoUpdateTransformRoles" ) )
    {
      this->AutoUpdateTransformRoles = atoi( attValue );
    }
    if ( ! strcmp( attName, "MarkBegin" ) )
    {
      this->MarkBegin = atof( attValue );
    }
    if ( ! strcmp( attName, "MarkEnd" ) )
    {
      this->MarkEnd = atof( attValue );
    }
    if ( ! strcmp( attName, "NeedleOrientation" ) )
    {
      this->NeedleOrientation = ( NeedleOrientationEnum ) atoi( attValue );
    }
    if ( ! strcmp( attName, "MetricsDirectory" ) )
    {
      this->MetricsDirectory = std::string( attValue );
    }
    if ( ! strcmp( attName, "PlaybackTime" ) )
    {
      this->PlaybackTime = atof( attValue );
    }
    if ( ! strcmp( attName, "RealTimeProcessing" ) )
    {
      this->RealTimeProcessing = atof( attValue );
    }
    
  }

  // Ignore the maps for now...
}


void vtkMRMLPerkEvaluatorNode
::Copy( vtkMRMLNode *anode )
{
  Superclass::Copy( anode );
  vtkMRMLPerkEvaluatorNode *node = ( vtkMRMLPerkEvaluatorNode* ) anode;

  this->AutoUpdateMeasurementRange = node->AutoUpdateMeasurementRange;
  this->AutoUpdateTransformRoles = node->AutoUpdateTransformRoles;
  this->MarkBegin = node->MarkBegin;
  this->MarkEnd = node->MarkEnd;
  this->NeedleOrientation = node->NeedleOrientation;
  this->MetricsDirectory = std::string( node->MetricsDirectory );
  this->PlaybackTime = node->PlaybackTime;
  this->RealTimeProcessing = node->RealTimeProcessing;
}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLPerkEvaluatorNode
::vtkMRMLPerkEvaluatorNode()
{
  this->AutoUpdateMeasurementRange = true;
  this->AutoUpdateTransformRoles = true;

  this->MarkBegin = 0.0;
  this->MarkEnd = 0.0;
  
  this->NeedleOrientation = vtkMRMLPerkEvaluatorNode::PlusX;
  this->MetricsDirectory = "";

  this->PlaybackTime = 0.0;

  this->RealTimeProcessing = false;

  this->AddNodeReferenceRole( TRANSFORM_BUFFER_REFERENCE_ROLE );
  this->AddNodeReferenceRole( METRICS_TABLE_REFERENCE_ROLE );
  this->AddNodeReferenceRole( METRIC_INSTANCE_REFERENCE_ROLE );
}


vtkMRMLPerkEvaluatorNode
::~vtkMRMLPerkEvaluatorNode()
{
}


// Getters and setters -----------------------------------------------------------------------------


bool vtkMRMLPerkEvaluatorNode
::GetAutoUpdateMeasurementRange()
{
  return this->AutoUpdateMeasurementRange;
}


void vtkMRMLPerkEvaluatorNode
::SetAutoUpdateMeasurementRange( bool update )
{
  if ( update != this->AutoUpdateMeasurementRange )
  {
    this->AutoUpdateMeasurementRange = update;
    this->Modified();
  }
}


bool vtkMRMLPerkEvaluatorNode
::GetAutoUpdateTransformRoles()
{
  return this->AutoUpdateTransformRoles;
}


void vtkMRMLPerkEvaluatorNode
::SetAutoUpdateTransformRoles( bool update )
{
  if ( update != this->AutoUpdateTransformRoles )
  {
    this->AutoUpdateTransformRoles = update;
    this->Modified();
  }
}

// Let the user set whatever values the want for MarkEnd and MarkBegin
// If they are too large/small, that is ok. Only analyze within the range.
double vtkMRMLPerkEvaluatorNode
::GetMarkBegin()
{
  return this->MarkBegin;
}


void vtkMRMLPerkEvaluatorNode
::SetMarkBegin( double begin )
{
  if ( begin != this->MarkBegin )
  {
    this->MarkBegin = begin;
    this->Modified();
  }
}


double vtkMRMLPerkEvaluatorNode
::GetMarkEnd()
{
  return this->MarkEnd;
}


void vtkMRMLPerkEvaluatorNode
::SetMarkEnd( double end )
{
  if ( end != this->MarkEnd )
  {
    this->MarkEnd = end;
    this->Modified();
  }
}


void vtkMRMLPerkEvaluatorNode
::GetNeedleBase( double needleBase[ 4 ] )
{
  if ( this->NeedleOrientation == vtkMRMLPerkEvaluatorNode::PlusX )
  {
    needleBase[ 0 ] = 1; needleBase[ 1 ] = 0; needleBase[ 2 ] = 0;
  }
  if ( this->NeedleOrientation == vtkMRMLPerkEvaluatorNode::MinusX )
  {
    needleBase[ 0 ] = -1; needleBase[ 1 ] = 0; needleBase[ 2 ] = 0;
  }
  if ( this->NeedleOrientation == vtkMRMLPerkEvaluatorNode::PlusY )
  {
    needleBase[ 0 ] = 0; needleBase[ 1 ] = 1; needleBase[ 2 ] = 0;
  }
  if ( this->NeedleOrientation == vtkMRMLPerkEvaluatorNode::MinusY )
  {
    needleBase[ 0 ] = 0; needleBase[ 1 ] = -1; needleBase[ 2 ] = 0;
  }
  if ( this->NeedleOrientation == vtkMRMLPerkEvaluatorNode::PlusZ )
  {
    needleBase[ 0 ] = 0; needleBase[ 1 ] = 0; needleBase[ 2 ] = 1;
  }
  if ( this->NeedleOrientation == vtkMRMLPerkEvaluatorNode::MinusZ )
  {
    needleBase[ 0 ] = 0; needleBase[ 1 ] = 0; needleBase[ 2 ] = -1;
  }
  needleBase[ 3 ] = 1;
}


vtkMRMLPerkEvaluatorNode::NeedleOrientationEnum vtkMRMLPerkEvaluatorNode
::GetNeedleOrientation()
{
  return this->NeedleOrientation;
}


void vtkMRMLPerkEvaluatorNode
::SetNeedleOrientation( vtkMRMLPerkEvaluatorNode::NeedleOrientationEnum newNeedleOrientation )
{
  if ( newNeedleOrientation != this->NeedleOrientation )
  {
    this->NeedleOrientation = newNeedleOrientation;\
    this->Modified();
  }
}


double vtkMRMLPerkEvaluatorNode
::GetPlaybackTime()
{
  return this->PlaybackTime;
}


void vtkMRMLPerkEvaluatorNode
::SetPlaybackTime( double newPlaybackTime, bool analysis )
{
  if ( newPlaybackTime != this->PlaybackTime )
  {
    this->PlaybackTime = newPlaybackTime;
    if ( ! analysis )
    {
      this->Modified();
    }
  }
}


bool vtkMRMLPerkEvaluatorNode
::GetRealTimeProcessing()
{
  return this->RealTimeProcessing;
}


void vtkMRMLPerkEvaluatorNode
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


// Metric scripts ------------------------------------------------------------------------------------------------


void vtkMRMLPerkEvaluatorNode
::AddMetricInstanceID( std::string metricInstanceID )
{
  this->AddAndObserveNodeReferenceID( METRIC_INSTANCE_REFERENCE_ROLE, metricInstanceID.c_str() );
}


void vtkMRMLPerkEvaluatorNode
::RemoveMetricInstanceID( std::string transformID )
{
  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( METRIC_INSTANCE_REFERENCE_ROLE ); i++ )
  {
    if ( transformID.compare( this->GetNthNodeReferenceID( METRIC_INSTANCE_REFERENCE_ROLE, i ) ) == 0 )
    {
      this->RemoveNthNodeReferenceID( METRIC_INSTANCE_REFERENCE_ROLE, i );
      i--;
    }
  }
}


std::vector< std::string > vtkMRMLPerkEvaluatorNode
::GetMetricInstanceIDs()
{
  std::vector< std::string > metricInstanceIDs;

  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( METRIC_INSTANCE_REFERENCE_ROLE ); i++ )
  {
    metricInstanceIDs.push_back( this->GetNthNodeReferenceID( METRIC_INSTANCE_REFERENCE_ROLE, i ) );
  }

  return metricInstanceIDs;
}


void vtkMRMLPerkEvaluatorNode
::SetMetricInstanceIDs( std::vector< std::string > metricInstanceIDs )
{
  // Remove all of the active transform IDs
  while( this->GetNumberOfNodeReferences( METRIC_INSTANCE_REFERENCE_ROLE ) > 0 )
  {
    this->RemoveNthNodeReferenceID( METRIC_INSTANCE_REFERENCE_ROLE, 0 );
  }

  // Add all of the specified IDs
  for ( int i = 0; i < metricInstanceIDs.size(); i++ )
  {
    this->AddAndObserveNodeReferenceID( METRIC_INSTANCE_REFERENCE_ROLE, metricInstanceIDs.at( i ).c_str() );
  }
}

bool vtkMRMLPerkEvaluatorNode
::IsMetricInstanceID( std::string metricInstanceID )
{
  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( METRIC_INSTANCE_REFERENCE_ROLE ); i++ )
  {
    if ( metricInstanceID.compare( this->GetNthNodeReferenceID( METRIC_INSTANCE_REFERENCE_ROLE, i ) ) == 0 )
    {
      return true;
    }
  }

  return false;
}


// Transform buffer/metrics table References ----------------------------------------------------------------------

std::string vtkMRMLPerkEvaluatorNode
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


vtkMRMLTransformBufferNode* vtkMRMLPerkEvaluatorNode
::GetTransformBufferNode()
{
  return vtkMRMLTransformBufferNode::SafeDownCast( this->GetNodeReference( TRANSFORM_BUFFER_REFERENCE_ROLE ) );
}


std::string vtkMRMLPerkEvaluatorNode
::GetTransformBufferID()
{
  return this->GetNodeReferenceIDString( TRANSFORM_BUFFER_REFERENCE_ROLE );
}


void vtkMRMLPerkEvaluatorNode
::SetTransformBufferID( std::string newTransformBufferID )
{
  vtkNew< vtkIntArray > events;
  events->InsertNextValue( vtkMRMLTransformBufferNode::TransformAddedEvent );
  events->InsertNextValue( vtkMRMLTransformBufferNode::RecordingStateChangedEvent );
  events->InsertNextValue( vtkMRMLTransformBufferNode::ActiveTransformAddedEvent );
  this->SetAndObserveNodeReferenceID( TRANSFORM_BUFFER_REFERENCE_ROLE, newTransformBufferID.c_str(), events.GetPointer() );

  // Auto-update as necessary
  if ( this->GetTransformBufferNode() == NULL )
  {
    return;
  }

  // Playback time
  this->SetPlaybackTime( this->GetTransformBufferNode()->GetMinimumTime() ); // Set to minimum for convenience

  // Measurement range
  if ( this->GetAutoUpdateMeasurementRange() )
  {
    this->SetMarkBegin( 0.0 );
    this->SetMarkEnd( this->GetTransformBufferNode()->GetTotalTime() );
  }

  // Transform roles
  if ( this->GetAutoUpdateTransformRoles() )
  {
    this->InvokeEvent( BufferActiveTransformAddedEvent );
  }

}


vtkMRMLTableNode* vtkMRMLPerkEvaluatorNode
::GetMetricsTableNode()
{
  return vtkMRMLTableNode::SafeDownCast( this->GetNodeReference( METRICS_TABLE_REFERENCE_ROLE ) );
}


std::string vtkMRMLPerkEvaluatorNode
::GetMetricsTableID()
{
  return this->GetNodeReferenceIDString( METRICS_TABLE_REFERENCE_ROLE );
}


void vtkMRMLPerkEvaluatorNode
::SetMetricsTableID( std::string newMetricsTableID )
{
  this->SetAndObserveNodeReferenceID( METRICS_TABLE_REFERENCE_ROLE, newMetricsTableID.c_str() );
}


// MRML node event processing -----------------------------------------------------------------

void vtkMRMLPerkEvaluatorNode
::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  this->vtkMRMLTransformableNode::ProcessMRMLEvents( caller, event, callData );

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
      this->SetRealTimeProcessing( false ); // Stop real-time processing if recording has stopped (note: real-time processing should not necessarily be started whenever recording is started)
    }
  }

  // The active transforms of the buffer have changed
  if ( event == vtkMRMLTransformBufferNode::ActiveTransformAddedEvent )
  {
    this->InvokeEvent( BufferActiveTransformAddedEvent );
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