

#include "vtkMRMLPerkEvaluatorNode.h"

// Constants ------------------------------------------------------------------
static const char* TRANSFORM_BUFFER_REFERENCE_ROLE = "TransformBuffer";
static const char* METRICS_TABLE_REFERENCE_ROLE = "MetricsTable";


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
  
  // Add the transform role map
  int transformRoleCounter = 0;
  for ( std::map< std::string, std::string >::iterator itr = this->TransformRoleMap.begin(); itr != this->TransformRoleMap.end(); itr++ )
  {
    of << indent << "TransformRoleMap" << transformRoleCounter << "=\"" << itr->first << " " << itr->second << "\"";
    transformRoleCounter++;
  }
  // Add the anatomy node map
  int anatomyNodeCounter = 0;
  for ( std::map< std::string, std::string >::iterator itr = this->AnatomyNodeMap.begin(); itr != this->AnatomyNodeMap.end(); itr++ )
  {
    of << indent << "AnatomyNodeMap" << anatomyNodeCounter << "=\"" << itr->first << " " << itr->second << "\"";
    anatomyNodeCounter++;
  }
}


void vtkMRMLPerkEvaluatorNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);

  // Don't maintain any roles that aren't in the read xml
  this->TransformRoleMap.clear();
  this->AnatomyNodeMap.clear();

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

    if ( std::string( attName ).find( "TransformRoleMap" ) != std::string::npos )
    {
      std::stringstream transformRoleStream( attValue );
      std::string transformNodeName; transformRoleStream >> transformNodeName; 
      std::string transformRole; transformRoleStream >> transformRole;
      this->TransformRoleMap[ transformNodeName ] = transformRole;
    }
    
    if ( std::string( attName ).find( "AnatomyNodeMap" ) != std::string::npos )
    {
      std::stringstream anatomyNodeStream( attValue );
      std::string anatomyRole; anatomyNodeStream >> anatomyRole; 
      std::string anatomyNodeName; anatomyNodeStream >> anatomyNodeName;
      this->AnatomyNodeMap[ anatomyRole ] = anatomyNodeName;
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
  
  this->TransformRoleMap = std::map< std::string, std::string >( node->TransformRoleMap );
  this->AnatomyNodeMap = std::map< std::string, std::string >( node->AnatomyNodeMap );
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

  this->AddNodeReferenceRole( TRANSFORM_BUFFER_REFERENCE_ROLE );
  this->AddNodeReferenceRole( METRICS_TABLE_REFERENCE_ROLE );
}


vtkMRMLPerkEvaluatorNode
::~vtkMRMLPerkEvaluatorNode()
{
  this->TransformRoleMap.clear();
  this->AnatomyNodeMap.clear();
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


std::string vtkMRMLPerkEvaluatorNode
::GetMetricsDirectory()
{
  return this->MetricsDirectory;
}


void vtkMRMLPerkEvaluatorNode
::SetMetricsDirectory( std::string newMetricsDirectory )
{
  if ( newMetricsDirectory.compare( this->MetricsDirectory ) != 0 )
  {
    this->MetricsDirectory = newMetricsDirectory;
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



// Transform/Anatomy roles ---------------------------------------------------------------------------------------

std::string vtkMRMLPerkEvaluatorNode
::GetTransformRole( std::string transformNodeName )
{
  if ( this->TransformRoleMap.find( transformNodeName ) != this->TransformRoleMap.end() )
  {
    return this->TransformRoleMap[ transformNodeName ];
  }
  else
  {
    return "";
  }
}


std::string vtkMRMLPerkEvaluatorNode
::GetFirstTransformNodeName( std::string transformRole )
{
  for( std::map< std::string, std::string >::iterator itr = this->TransformRoleMap.begin(); itr != this->TransformRoleMap.end(); itr++ )
  {
    if ( transformRole.compare( itr->second ) == 0 )
    {
      return itr->first;
    }
  }
  return "";
}


void vtkMRMLPerkEvaluatorNode
::SetTransformRole( std::string transformNodeName, std::string newTransformRole )
{
  if ( newTransformRole.compare( this->TransformRoleMap[ transformNodeName ] ) != 0 )
  {
    this->TransformRoleMap[ transformNodeName ] = newTransformRole;
    this->Modified();
  }
}


std::string vtkMRMLPerkEvaluatorNode
::GetAnatomyNodeName( std::string anatomyRole )
{
  if ( this->AnatomyNodeMap.find( anatomyRole ) != this->AnatomyNodeMap.end() )
  {
    return this->AnatomyNodeMap[ anatomyRole ];
  }
  else
  {
    return "";
  }
}


std::string vtkMRMLPerkEvaluatorNode
::GetFirstAnatomyRole( std::string anatomyNodeName )
{
  for( std::map< std::string, std::string >::iterator itr = this->AnatomyNodeMap.begin(); itr != this->AnatomyNodeMap.end(); itr++ )
  {
    if ( anatomyNodeName.compare( itr->second ) == 0 )
    {
      return itr->first;
    }
  }
  return "";
}


void vtkMRMLPerkEvaluatorNode
::SetAnatomyNodeName( std::string anatomyRole, std::string newAnatomyNodeName )
{
  if ( newAnatomyNodeName.compare( this->AnatomyNodeMap[ anatomyRole ] ) != 0 )
  {
    this->AnatomyNodeMap[ anatomyRole ] = newAnatomyNodeName;
    this->Modified();
  }
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
  this->SetAndObserveNodeReferenceID( TRANSFORM_BUFFER_REFERENCE_ROLE, newTransformBufferID.c_str() );
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