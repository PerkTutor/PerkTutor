
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


// Standard MRML Node Functions ----------------------------------------------------------------------------

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


void vtkMRMLTransformRecorderNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "FileName: " << this->fileName << "\n";
  for ( int i = 0; i < this->ObservedTransformNodes.size(); i++ )
  {
    os << indent << " ObservedTransformNode=\"" << this->ObservedTransformNodes.at(i)->GetName() << "\"";
  }
}


void vtkMRMLTransformRecorderNode
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
    
    // TODO: Handle observed transform nodes and connector node.   
    if ( ! strcmp( attName, "FileName" ) )
    {
      this->SetFileName( attValue );
    }
	if ( ! strcmp( attName, "ObservedTransformNode" ) )
    {
	  StoredTransformNodeNames.push_back( std::string( attValue ) );
    }

  }

}



void vtkMRMLTransformRecorderNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  
  of << indent << " FileName=\"" << this->fileName << "\"";
  for ( int i = 0; i < this->ObservedTransformNodes.size(); i++ )
  {
    of << indent << " ObservedTransformNode=\"" << this->ObservedTransformNodes.at(i)->GetName() << "\"";
  }
}



void vtkMRMLTransformRecorderNode
::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode );
  vtkMRMLTransformRecorderNode *node = ( vtkMRMLTransformRecorderNode* ) anode;

    // Observers must be removed here, otherwise MRML updates would activate nodes on the undo stack
  
  for ( unsigned int i = 0; i < this->ObservedTransformNodes.size(); ++ i )
    {
    this->ObservedTransformNodes[ i ]->RemoveObservers( vtkMRMLLinearTransformNode::TransformModifiedEvent );
    }
  
  this->SetRecording( node->GetRecording() );
  this->SetFileName( node->GetFileName() );
}


void vtkMRMLTransformRecorderNode
::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
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
        this->AddTransform( transformNode->GetID() );
      }
    }
  }
}




// Constructors and Destructors -----------------------------------------------------------

vtkMRMLTransformRecorderNode
::vtkMRMLTransformRecorderNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  // this->SetModifiedSinceRead( true );
  
  this->Recording = false;
  
  this->fileName = "";
  
  // Initialize zero time point.
  this->Clock0 = clock();
  
  this->TotalNeedlePath = 0.0;
  this->TotalNeedlePathInside = 0.0;
  this->LastNeedleTransform = NULL;
  this->LastNeedleTime = -1.0;
  this->NeedleInside = false;

  this->TransformBuffer = vtkMRMLTransformBufferNode::New();
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
  if ( this->TransformBuffer != NULL )
  {
    this->TransformBuffer->Delete();
    this->TransformBuffer = NULL;
  }
}




// Observed transform nodes -------------------------------------------------------

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
  for ( nodeIt = this->ObservedTransformNodes.begin(); nodeIt != this->ObservedTransformNodes.end(); nodeIt++ )
  {
    if ( strcmp( TransformNodeID, (*nodeIt)->GetID() ) == 0 )
    {
      (*nodeIt)->RemoveObserver( (vtkCommand*)this->MRMLCallbackCommand );
      vtkSetAndObserveMRMLObjectMacro( *nodeIt, NULL );
      this->ObservedTransformNodes.erase( nodeIt );
	  break; // Otherwise the iterator will run out of bounds
    }
  }
  
  
  // Remove node ID and reference.
  std::vector< char* >::iterator transformIt;
  for ( transformIt = this->ObservedTransformNodeIDs.begin(); transformIt != this->ObservedTransformNodeIDs.end(); transformIt ++ )
  {
    if ( strcmp( TransformNodeID, *transformIt ) == 0 )
    {
      if ( this->GetScene() )
      {
        this->GetScene()->RemoveReferencedNodeID( TransformNodeID, this );
      }
      
      this->ObservedTransformNodeIDs.erase( transformIt );
      break; // Otherwise the iterator will run out of bounds
    }
  }

}



void vtkMRMLTransformRecorderNode
::ClearObservedTranformNodes()
{
  // Observe that the elements keep sliding to fill the 0th slot, so just keep removing from the zeroth slot
  while ( this->ObservedTransformNodeIDs.size() > 0 )
  {
    this->RemoveObservedTransformNode( *this->ObservedTransformNodeIDs.begin() );
  }
}



vtkMRMLLinearTransformNode* vtkMRMLTransformRecorderNode
::GetObservedTransformNode( const char* TransformNodeID )
{
  vtkMRMLLinearTransformNode* node = NULL;
  if ( this->GetScene() && TransformNodeID != NULL )
  {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID( TransformNodeID );
    node = vtkMRMLLinearTransformNode::SafeDownCast( snode );
  }
  return node;
}


bool vtkMRMLTransformRecorderNode
::IsObservedTransformNode( const char* TransformNodeID )
{
  for ( int i = 0; i < this->ObservedTransformNodeIDs.size(); i++ )
  {
    if ( strcmp( this->ObservedTransformNodeIDs.at(i), TransformNodeID ) == 0 )
	{
	  return true;
	}
  }
  return false;
}

void vtkMRMLTransformRecorderNode
::AddObservedTransformNodesFromStoredNames()
{
  // Create a new node if none exists with this name
  // Then add the node to the list of observed transform nodes
  for ( int i = 0; i < this->StoredTransformNodeNames.size(); i++ )
  {
    vtkMRMLLinearTransformNode* node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetScene()->GetFirstNode( StoredTransformNodeNames.at(i).c_str(), "vtkMRMLLinearTransformNode" ) );
    if ( node == NULL )
    {
      node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetScene()->CreateNodeByClass( "vtkMRMLLinearTransformNode" ) );
	  this->GetScene()->AddNode( node );
	  node->SetName( StoredTransformNodeNames.at(i).c_str() );
    }
    const char* nodeID = node->GetID();
    this->AddObservedTransformNode( nodeID );

	// Remove
	StoredTransformNodeNames.erase( StoredTransformNodeNames.begin() + i );
	i--;
  }

}



// Clear --------------------------------------------------------------------

bool vtkMRMLTransformRecorderNode
::GetRecording()
{
  return this->Recording;
}


void vtkMRMLTransformRecorderNode
::SetRecording( bool newRecording )
{
  this->Recording = newRecording;
}


void vtkMRMLTransformRecorderNode
::Clear()
{
  this->TotalNeedlePath = 0.0;
  this->TotalNeedlePathInside = 0.0;

  if ( this->LastNeedleTransform != NULL )
  {
    this->LastNeedleTransform->Delete();
    this->LastNeedleTransform = NULL;
  }

  this->TransformBuffer->Clear();
}






// Time -----------------------------------------------------------------------------

void vtkMRMLTransformRecorderNode
::GetCurrentTimestamp( int &sec, int &nsec )
{
  clock_t clock1 = clock();
  double seconds = double( clock1 - this->Clock0 ) / CLOCKS_PER_SEC;
  sec = floor( seconds );
  nsec = ( seconds - sec ) * 1e9;    
}


double vtkMRMLTransformRecorderNode
::GetCurrentTimestamp()
{
  clock_t clock1 = clock();
  return double( clock1 - this->Clock0 ) / CLOCKS_PER_SEC;
}





// Saving to file ----------------------------------------------------------------------------

std::string vtkMRMLTransformRecorderNode
::GetFileName()
{
  return this->fileName;
}

void vtkMRMLTransformRecorderNode
::SetFileName( std::string newFileName )
{
  this->fileName = newFileName;
}



void vtkMRMLTransformRecorderNode::SaveToFile( std::string newFileName )
{
  this->fileName = newFileName;
  std::ofstream output( this->fileName.c_str() );
  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Record file could not be opened!" );
    return;
  }

  output << this->TransformBuffer->ToXMLString();

  output.close();
}





// Trajectory metrics ---------------------------------------------------------------------------------------------

double vtkMRMLTransformRecorderNode::GetTotalTime()
{
  int n = this->TransformBuffer->GetNumTransforms();
  
  if ( n < 2 )
  {
    return 0.0;
  }
  
  return ( this->TransformBuffer->GetCurrentTransform()->GetTime() - this->TransformBuffer->GetTransformAt(0)->GetTime() );

}




double vtkMRMLTransformRecorderNode::GetTotalPath()
{
  return this->TotalNeedlePath;
}



double vtkMRMLTransformRecorderNode::GetTotalPathInside()
{
  return this->TotalNeedlePathInside;
}





// Add transform or message -----------------------------------------------------------------------------------

void vtkMRMLTransformRecorderNode
::AddMessage( std::string name, double time )
{
  if ( time == -1 )
  {
    time = this->GetCurrentTimestamp();
  }

  vtkMessageRecord* messageRecord = vtkMessageRecord::New();
  messageRecord->SetName( name );
  messageRecord->SetTime( time );
  TransformBuffer->AddMessage( messageRecord ); 
  
  // This should probably be redesigned at some point.
  
  if ( name.compare( "IN" ) == 0 )
  {
    this->NeedleInside = true;
  }
  else if ( name.compare( "OUT" ) == 0 )
  {
    this->NeedleInside = false;
  }
}




void vtkMRMLTransformRecorderNode::AddTransform( const char* TransformNodeID )
{
  vtkSmartPointer< vtkMatrix4x4 > m = vtkSmartPointer< vtkMatrix4x4 >::New();
  
  // Get the new transform matrix 
  vtkMRMLLinearTransformNode* ltn = this->GetObservedTransformNode( TransformNodeID );  
  if ( ltn != NULL )
  {
    m->DeepCopy( ltn->GetMatrixTransformToParent() );
  }
  else
  {
    vtkErrorMacro( "Transform node not found." );
  }

  if ( this->Recording == false )
  {
    return;
  }
  
    
  // Get the device name and time for the new transform  
  std::string deviceName = std::string( ltn->GetName() );
  double time = this->GetCurrentTimestamp();
  
  
  
  // Compute path lengths for needle  
  if (    deviceName.compare( "Needle" ) == 0
       || deviceName.compare( "NeedleToReference" ) == 0
       || deviceName.compare( "Stylus" ) == 0
       || deviceName.compare( "StylusTipToRAS" ) == 0
       || deviceName.compare( "StylusTip" ) == 0
       || deviceName.compare( "StylusTipToReference" ) == 0 )
  {
    
    if ( this->LastNeedleTransform == NULL  ||  this->LastNeedleTime < 0.0 )  // This is the first needle transform.
    {
      this->LastNeedleTransform = vtkTransform::New();
    }
    else  // Add to length.
    {

	  double distVector[ 3 ];
	  distVector[0] = m->GetElement( 0, 3 ) - this->LastNeedleTransform->GetMatrix()->GetElement( 0, 3 );
	  distVector[1] = m->GetElement( 1, 3 ) - this->LastNeedleTransform->GetMatrix()->GetElement( 1, 3 );
	  distVector[2] = m->GetElement( 2, 3 ) - this->LastNeedleTransform->GetMatrix()->GetElement( 2, 3 );
      double dist = vtkMath::Norm( distVector );
      double dt = time - this->LastNeedleTime;
	  double vNeedle = dist / dt; // mm / s

      if ( dt > 0.0 && vNeedle < MAX_NEEDLE_SPEED_MMPERS )
      {
        this->TotalNeedlePath += dist;
        if ( this->NeedleInside )
        {
          this->TotalNeedlePathInside += dist;
        }
      }

    }

    this->LastNeedleTransform->SetMatrix( m );
    this->LastNeedleTime = time;

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
  
  vtkTransformRecord* transformRecord = vtkTransformRecord::New();
  transformRecord->SetTransform( mss.str() );
  transformRecord->SetDeviceName( deviceName );
  transformRecord->SetTime( time );
 
  
  // Look for the most recent value of this transform.
  // If the value hasn't changed, we don't record.
  bool duplicate = false;
  for ( int i = this->TransformBuffer->GetNumTransforms() - 1; i >= 0; i-- )
  {
    if ( this->TransformBuffer->GetTransformAt(i)->GetDeviceName().compare( transformRecord->GetDeviceName() ) == 0 )
	{
      if ( this->TransformBuffer->GetTransformAt(i)->GetTransform().compare( transformRecord->GetTransform() ) == 0 )
	  {
        duplicate = true;
	  }
	  break;
	}
  }

  if ( ! duplicate )
  {
    this->TransformBuffer->AddTransform( transformRecord );
  }

}
