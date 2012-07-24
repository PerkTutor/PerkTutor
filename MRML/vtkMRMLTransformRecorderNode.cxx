
// VTK includes
#include <vtkCleanPolyData.h>
#include <vtkCommand.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkExtractSelectedPolyDataIds.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkPlanes.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"

// TransformRecorder MRML includes
#include "vtkMRMLTransformRecorderNode.h"

// MRML includes
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationROINode.h>
#include "vtkMRMLLinearTransformNode.h"
// STD includes
#include <math.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

// Max needle speed that is used in measurements. Filters outliers.
#define MAX_NEEDLE_SPEED_MMPERS 500

// MACROS ---------------------------------------------------------------------

#define DELETE_IF_NOT_NULL(x) \
  if ( x != NULL ) \
    { \
    x->Delete(); \
    x = NULL; \
    }

#define WRITE_STRING_XML(x) \
  if ( this->x != NULL ) \
  { \
    of << indent << " "#x"=\"" << this->x << "\"\n"; \
  }

#define READ_AND_SET_STRING_XML(x) \
    if ( !strcmp( attName, #x ) ) \
      { \
      this->SetAndObserve##x( NULL ); \
      this->Set##x( attValue ); \
      }


// For testing.
void fileOutput( std::string str, double num )
{
  std::ofstream output( "_vtkMRMLTransformRecorderNode.txt", std::ios_base::app );
  time_t seconds;
  seconds = time( NULL );
  
  output << seconds << " : " << str << " - " << num << std::endl;
  output.close();
}



// Helper functions.
// ----------------------------------------------------------------------------

bool
StringToBool( std::string str )
{
  bool var;
  std::stringstream ss( str );
  ss >> var;
  return var;
}

// ----------------------------------------------------------------------------

vtkMRMLTransformRecorderNode* vtkMRMLTransformRecorderNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLTransformRecorderNode" );
  if( ret )
    {
      return ( vtkMRMLTransformRecorderNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransformRecorderNode;
}



vtkMRMLNode* vtkMRMLTransformRecorderNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLTransformRecorderNode" );
  if( ret )
    {
      return ( vtkMRMLTransformRecorderNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransformRecorderNode;
}


void
vtkMRMLTransformRecorderNode
::StartReceiveServer()
{
  this->Active = true;
}



void
vtkMRMLTransformRecorderNode
::StopReceiveServer()
{
  this->Active = false;
}

void
vtkMRMLTransformRecorderNode
::UpdateFileFromBuffer()
{
  std::ofstream output( this->LogFileName.c_str() );
  
  if ( ! output.is_open() )
    {
    vtkErrorMacro( "Record file could not be opened!" );
    return;
    }
  
  output << "<TransformRecorderLog>" << std::endl;
  
    // Save transforms.
  
  for ( unsigned int i = 0; i < this->TransformsBuffer.size(); ++ i )
    {
    output << "  <log";
    output << " TimeStampSec=\"" << this->TransformsBuffer[ i ].TimeStampSec << "\"";
    output << " TimeStampNSec=\"" << this->TransformsBuffer[ i ].TimeStampNSec << "\"";
    output << " type=\"transform\"";
    output << " DeviceName=\"" << this->TransformsBuffer[ i ].DeviceName << "\"";
    output << " transform=\"" << this->TransformsBuffer[ i ].Transform << "\"";
    output << " />" << std::endl;
    }
  
  
    // Save messages.
  
  for ( unsigned int i = 0; i < this->MessagesBuffer.size(); ++ i )
    {
    output << "  <log";
    output << " TimeStampSec=\"" << this->MessagesBuffer[ i ].TimeStampSec << "\"";
    output << " TimeStampNSec=\"" << this->MessagesBuffer[ i ].TimeStampNSec << "\"";
    output << " type=\"message\"";
    output << " message=\"" << this->MessagesBuffer[ i ].Message << "\"";
    output << " />" << std::endl;
    }
  
  
  output << "</TransformRecorderLog>" << std::endl;
  output.close();
  this->ClearBuffer();
  
}





void
vtkMRMLTransformRecorderNode
::ClearBuffer()
{
  this->TransformsBuffer.clear();
  this->MessagesBuffer.clear();
  
  this->TotalNeedlePath = 0.0;
  this->TotalNeedlePathInside = 0.0;
  
  if ( this->LastNeedleTransform != NULL )
  {
    this->LastNeedleTransform->Delete();
    this->LastNeedleTransform = NULL;
  }
}



void vtkMRMLTransformRecorderNode
::GetTimestamp( int &sec, int &nsec )
{
  clock_t clock1 = clock();
  double seconds = double( clock1 - this->Clock0 ) / CLOCKS_PER_SEC;
  sec = floor( seconds );
  nsec = ( seconds - sec ) * 1e9;    
}



vtkMRMLTransformRecorderNode
::vtkMRMLTransformRecorderNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  // this->SetModifiedSinceRead( true );
  
  this->Recording = false;
  
  this->LogFileName = "";
  
  this->Active = true;
  
  // Initialize zero time point.
  this->Clock0 = clock();
  this->IGTLTimeOffsetSeconds = 0.0;
  this->IGTLTimeSynchronized = false;
  
  this->TotalNeedlePath = 0.0;
  this->TotalNeedlePathInside = 0.0;
  this->LastNeedleTransform = NULL;
  this->LastNeedleTime = -1.0;
  this->NeedleInside = false;
}




vtkMRMLTransformRecorderNode
::~vtkMRMLTransformRecorderNode()
{
  this->ClearObservedTranformNodes();
  
  if ( this->LastNeedleTransform != NULL )
  {
    this->LastNeedleTransform->Delete();
    this->LastNeedleTransform = NULL;
  }
}



void vtkMRMLTransformRecorderNode::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  
  of << indent << " Recording=\"" << this->Recording << "\"";
  of << indent << " LogFileName=\"" << this->LogFileName << "\"";
}



void vtkMRMLTransformRecorderNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
    {
    attName  = *(atts++);
    attValue = *(atts++);
    
    // todo: Handle observed transform nodes and connector node.
    
    if ( ! strcmp( attName, "Recording" ) )
      {
      this->SetRecording( StringToBool( std::string( attValue ) ) );
      }
    
    if ( ! strcmp( attName, "LogFileName" ) )
      {
      this->SetLogFileName( attValue );
      }
    }
}


//----------------------------------------------------------------------------
void vtkMRMLTransformRecorderNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode );
  vtkMRMLTransformRecorderNode *node = ( vtkMRMLTransformRecorderNode* ) anode;

    // Observers must be removed here, otherwise MRML updates would activate nodes on the undo stack
  
  for ( unsigned int i = 0; i < this->ObservedTransformNodes.size(); ++ i )
    {
    this->ObservedTransformNodes[ i ]->RemoveObservers( vtkMRMLLinearTransformNode::TransformModifiedEvent );
    }
  
  this->SetRecording( node->GetRecording() );
  this->SetLogFileName( node->GetLogFileName() );
}


//----------------------------------------------------------------------------
void vtkMRMLTransformRecorderNode::UpdateReferences()
{
  Superclass::UpdateReferences();
      // MRML node ID's should be checked. If Scene->GetNodeByID( id ) returns NULL,
    // the reference should be deleted (set to NULL).
  
}



void vtkMRMLTransformRecorderNode::UpdateReferenceID( const char *oldID, const char *newID )
{
  Superclass::UpdateReferenceID( oldID, newID );
  
  /*
  if ( this->ConnectorNodeID && !strcmp( oldID, this->ConnectorNodeID ) )
    {
    this->SetAndObserveConnectorNodeID( newID );
    }
  */
  
  // TODO: This needs to be written for observed transforms.
  
}



void vtkMRMLTransformRecorderNode::UpdateScene( vtkMRMLScene *scene )
{
  Superclass::UpdateScene( scene );
  // this->SetAndObserveConnectorNodeID( this->ConnectorNodeID );
  // TODO: Deal with observed transforms.
}



void vtkMRMLTransformRecorderNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "LogFileName: " << this->LogFileName << "\n";
}



void vtkMRMLTransformRecorderNode
::AddObservedTransformNode( const char* TransformNodeID )
{
  if ( TransformNodeID == NULL )
  {
    return;
  }
  
  
    // Check if this ID is already observed.
  
  bool alreadyObserved = false;  
  
  for ( unsigned int i = 0; i < this->ObservedTransformNodeIDs.size(); ++ i )
  {
    if ( strcmp( TransformNodeID, this->ObservedTransformNodeIDs[ i ] ) == 0 )
    {
    alreadyObserved = true;
    }
  }
  
  if ( alreadyObserved == true )
  {
    return;  // No need to add this node again.
  }
  
  
  if ( this->GetScene() )
  {
    this->GetScene()->AddReferencedNodeID( TransformNodeID, this );
  }
  
  
  vtkMRMLLinearTransformNode* transformNode = NULL;
  this->ObservedTransformNodeIDs.push_back( (char*)TransformNodeID );
  vtkMRMLLinearTransformNode* tnode = this->GetObservedTransformNode( TransformNodeID );
  vtkSetAndObserveMRMLObjectMacro( transformNode, tnode );
  this->ObservedTransformNodes.push_back( transformNode );
  if ( tnode )
  {
    tnode->AddObserver( vtkMRMLLinearTransformNode::TransformModifiedEvent, (vtkCommand*)this->MRMLCallbackCommand );
  }
}



void vtkMRMLTransformRecorderNode
::RemoveObservedTransformNode( const char* TransformNodeID )
{
  if ( TransformNodeID == NULL )
  {
    return;
  }
  
  
    // Remove observer, and erase node from the observed node vector.
  
  std::vector< vtkMRMLLinearTransformNode* >::iterator nodeIt;
  for ( nodeIt = this->ObservedTransformNodes.begin();
        nodeIt != this->ObservedTransformNodes.end();
        nodeIt ++ )
  {
    if ( strcmp( TransformNodeID, (*nodeIt)->GetID() ) == 0 )
    {
      (*nodeIt)->RemoveObserver( (vtkCommand*)this->MRMLCallbackCommand );
      vtkSetAndObserveMRMLObjectMacro( *nodeIt, NULL );
      this->ObservedTransformNodes.erase( nodeIt );
    }
  }
  
  
    // Remove node ID and reference.
  
  std::vector< char* >::iterator transformIt;
  for ( transformIt = this->ObservedTransformNodeIDs.begin();
        transformIt != this->ObservedTransformNodeIDs.end();
        transformIt ++ )
  {
    if ( strcmp( TransformNodeID, *transformIt ) == 0 )
    {
      if ( this->GetScene() )
      {
        this->GetScene()->RemoveReferencedNodeID( TransformNodeID, this );
      }
      
      this->ObservedTransformNodeIDs.erase( transformIt );
      break;
    }
  }
}



void vtkMRMLTransformRecorderNode
::ClearObservedTranformNodes()
{
  std::vector< char* >::iterator transformIDIt;
  for ( transformIDIt = this->ObservedTransformNodeIDs.begin();
        transformIDIt != this->ObservedTransformNodeIDs.end();
        transformIDIt ++ )
  {
    this->RemoveObservedTransformNode( *transformIDIt );
  }
}



vtkMRMLLinearTransformNode* vtkMRMLTransformRecorderNode
::GetObservedTransformNode( const char* TransformNodeID )
{
  vtkMRMLLinearTransformNode* node = NULL;
  if ( this->GetScene()
       && TransformNodeID != NULL )
  {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID( TransformNodeID );
    node = vtkMRMLLinearTransformNode::SafeDownCast( snode );
  }
  return node;
}



void vtkMRMLTransformRecorderNode::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{
  if ( this->Recording == false )
  {
    return;
  }
  
  
    // Handle modified event of any observed transform node.
  
  std::vector< vtkMRMLLinearTransformNode* >::iterator tIt;
  for ( tIt = this->ObservedTransformNodes.begin();
        tIt != this->ObservedTransformNodes.end();
        tIt ++ )
  {
    if ( *tIt == vtkMRMLLinearTransformNode::SafeDownCast( caller )
         && event == vtkMRMLLinearTransformNode::TransformModifiedEvent )
    {
      vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( caller );
      if ( transformNode != NULL )
      {
        this->AddNewTransform( transformNode->GetID() );
      }
    }
  }
}



void vtkMRMLTransformRecorderNode::RemoveMRMLObservers()
{
  for ( unsigned int i = 0; i < this->ObservedTransformNodes.size(); ++ i )
  {
    this->ObservedTransformNodes[ i ]->RemoveObservers( vtkMRMLLinearTransformNode::TransformModifiedEvent );
  }
}



/**
 * @param selections Should contain as many elements as the number of incoming
 *        transforms throught the active connector. Order follows the order in
 *        the connector node. 0 means transform is not tracked, 1 means it's tracked.
 */
void vtkMRMLTransformRecorderNode::SetTransformSelections( std::vector< int > selections )
{
  this->TransformSelections.clear();
  
  for ( unsigned int i = 0; i < selections.size(); ++ i )
  {
    this->TransformSelections.push_back( selections[ i ] );
  }


}



void vtkMRMLTransformRecorderNode::SetLogFileName( std::string fileName )
{
  this->LogFileName = fileName;
}



void vtkMRMLTransformRecorderNode::SaveIntoFile( std::string fileName )
{
  this->LogFileName = fileName;
  this->UpdateFileFromBuffer();

}



std::string vtkMRMLTransformRecorderNode::GetLogFileName()
{
  return this->LogFileName;
}



void vtkMRMLTransformRecorderNode::CustomMessage( std::string message, int sec, int nsec )
{
  if ( sec == -1  &&  nsec == -1 )
  {
    this->GetTimestamp( sec, nsec );
  }
  
  MessageRecord rec;
    rec.Message = message;
    rec.TimeStampSec = sec;
    rec.TimeStampNSec = nsec;
  
  this->MessagesBuffer.push_back( rec );
  
  
    // This shoulb probably be redesigned at some point.
  
  if ( message.compare( "IN" ) == 0 )
  {
    this->NeedleInside = true;
  }
  else if ( message.compare( "OUT" ) == 0 )
  {
    this->NeedleInside = false;
  }
}



unsigned int vtkMRMLTransformRecorderNode::GetTransformsBufferSize()
{
  return this->TransformsBuffer.size();

}



unsigned int vtkMRMLTransformRecorderNode::GetMessagesBufferSize()
{
  return this->MessagesBuffer.size();
}



double vtkMRMLTransformRecorderNode::GetTotalTime()
{
  double totalTime = 0.0;
  unsigned int n = this->TransformsBuffer.size();
  
  if ( n < 2 )
  {
    return totalTime;
  }
  
  double diffSec = double( this->TransformsBuffer[ n - 1 ].TimeStampSec - this->TransformsBuffer[ 0 ].TimeStampSec );
  double diffNSec = double( this->TransformsBuffer[ n - 1 ].TimeStampNSec - this->TransformsBuffer[ 0 ].TimeStampNSec );
  
  totalTime = diffSec + 1.0e-9 * diffNSec;
  
  return totalTime;
}




double vtkMRMLTransformRecorderNode::GetTotalPath()
{
  return this->TotalNeedlePath;
}



double vtkMRMLTransformRecorderNode::GetTotalPathInside()
{
  return this->TotalNeedlePathInside;
}




void vtkMRMLTransformRecorderNode::SetRecording( bool newState )
{
  this->Recording = newState;
  if ( this->Recording )
  {
    this->InvokeEvent( this->RecordingStartEvent, NULL );
  }
  else
  {
    this->InvokeEvent( this->RecordingStopEvent, NULL );
  }
}



void vtkMRMLTransformRecorderNode::AddNewTransform( const char* TransformNodeID )
{
  int sec = 0;
  int nsec = 0;
  vtkSmartPointer< vtkMatrix4x4 > m = vtkSmartPointer< vtkMatrix4x4 >::New();
  std::string deviceName;
  
  this->GetTimestamp( sec, nsec );
  
  
    // Get the new transform matrix.
  
  vtkMRMLLinearTransformNode* ltn = this->GetObservedTransformNode( TransformNodeID );
  
  if ( ltn != NULL )
  {
    m->DeepCopy( ltn->GetMatrixTransformToParent() );
  }
  else
  {
    vtkErrorMacro( "Transform node not found." );
  }
  
    
    // Get the device name for the new transform.
  
  deviceName = std::string( ltn->GetName() );
  
  
  
    // Compute path lengths for needle.
  
  if (    deviceName.compare( "Needle" ) == 0
       || deviceName.compare( "NeedleToReference" ) == 0
       || deviceName.compare( "Stylus" ) == 0
       || deviceName.compare( "StylusTipToRAS" ) == 0
       || deviceName.compare( "StylusTip" ) == 0
       || deviceName.compare( "StylusTipToReference" ) == 0 )
  {
    double needleTime = double( sec ) + 1.0e-9 * double( nsec );
    double vNeedle = 1000000.0;  // TODO: Just a very big number. It could be solved nicer.
    
    if ( this->LastNeedleTransform == NULL  ||  this->LastNeedleTime < 0.0 )  // This is the first needle transform.
    {
      this->LastNeedleTransform = vtkTransform::New();
      this->LastNeedleTransform->SetMatrix( m );
    }
    else  // Add to length.
    {
      double distVector[ 3 ] = { 0.0, 0.0, 0.0 };
      for ( int i = 0; i < 3; ++ i )
      {
        distVector[ i ] = m->GetElement( i, 3 ) - this->LastNeedleTransform->GetMatrix()->GetElement( i, 3 );
      }
      double dist = vtkMath::Norm( distVector );
      double dt = needleTime - this->LastNeedleTime;
      if ( dt > 0.0 )
      {
        vNeedle = dist / dt; // mm / s
        // fileOutput( "Needle velocity", vNeedle );  // TODO: Should be removed to improve performance!
        if ( vNeedle < MAX_NEEDLE_SPEED_MMPERS )
        {
          this->TotalNeedlePath += dist;
          if ( this->NeedleInside )
          {
            this->TotalNeedlePathInside += dist;
          }
          this->LastNeedleTransform->SetMatrix( m );
        }
      }
    }
    
    this->LastNeedleTime = needleTime;
  }
  
  
    // Record the transform.
  
  std::stringstream mss;
  for ( int row = 0; row < 4; ++ row )
  {
    for ( int col = 0; col < 4; ++ col )
    {
      mss << m->GetElement( row, col ) << " ";
    }
  }
  
  TransformRecord rec;
    rec.DeviceName = deviceName;
    rec.TimeStampSec = sec;
    rec.TimeStampNSec = nsec;
    rec.Transform = mss.str();
  
  if ( this->Recording == false )
  {
    return;
  }
  
  
    // Look for the most recent value of this transform.
    // If the value hasn't changed, we don't record.
  
  std::vector< TransformRecord >::iterator trIt;
  trIt = this->TransformsBuffer.end();
  bool duplicate = false;
  while ( trIt != this->TransformsBuffer.begin() )
  {
    trIt --;
    if ( rec.DeviceName.compare( (*trIt).DeviceName ) == 0 )
    {
      if ( rec.Transform.compare( (*trIt).Transform ) == 0 )
      {
        duplicate = true;
      }
      break;
    }
  }
  
  if ( duplicate == false )
  {
    this->TransformsBuffer.push_back( rec );
  }
}
