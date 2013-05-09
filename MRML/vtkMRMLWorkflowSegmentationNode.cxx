
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
#include "vtkXMLDataParser.h"

// WorkflowSegmentation MRML includes
#include "vtkMRMLWorkflowSegmentationNode.h"

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
  std::ofstream output( "_vtkMRMLWorkflowSegmentationNode.txt", std::ios_base::app );
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



vtkMRMLWorkflowSegmentationNode
::vtkMRMLWorkflowSegmentationNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  // this->SetModifiedSinceRead( true );
  
  this->Recording = false;
  
  this->TrackingLogFileName = "-";
  this->SegmentationLogFileName = "-";
  this->ProcedureDefinitionFileName = "-";
  this->InputParameterFileName = "-";
  this->TrainingParameterFileName = "-";
  
  // Initialize zero time point.
  this->Clock0 = clock();
  this->IGTLTimeOffsetSeconds = 0.0;
  this->IGTLTimeSynchronized = false;
  
  this->LastNeedleTransform = NULL;
  this->LastNeedleTime = -1.0;

}



vtkMRMLWorkflowSegmentationNode
::~vtkMRMLWorkflowSegmentationNode()
{
  this->ClearObservedTranformNodes();
  
  if ( this->LastNeedleTransform != NULL )
  {
    this->LastNeedleTransform->Delete();
    this->LastNeedleTransform = NULL;
  }
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

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  
  of << indent << " Recording=\"" << this->Recording << "\"";
  of << indent << " TrackingLogFileName=\"" << this->TrackingLogFileName << "\"";
  of << indent << " SegmentationLogFileName=\"" << this->SegmentationLogFileName << "\"";
  of << indent << " ProcedureDefinitionFileName=\"" << this->ProcedureDefinitionFileName << "\"";
  of << indent << " InputParameterFileName=\"" << this->InputParameterFileName << "\"";
  of << indent << " TrainingParameterFileName=\"" << this->TrainingParameterFileName << "\"";

}



void vtkMRMLWorkflowSegmentationNode::ReadXMLAttributes( const char** atts )
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
	if ( ! strcmp( attName, "TrackingLogFileName" ) )
    {
      this->TrackingLogFileName = std::string( attValue );
    }
    if ( ! strcmp( attName, "SegmentationLogFileName" ) )
    {
      this->SegmentationLogFileName = std::string( attValue );
    }
	if ( ! strcmp( attName, "ProcedureDefinitionFileName" ) )
    {
      this->ProcedureDefinitionFileName = std::string( attValue );
    }
	if ( ! strcmp( attName, "InputParameterFileName" ) )
    {
      this->InputParameterFileName = std::string( attValue );
    }
	if ( ! strcmp( attName, "TrainingParameterFileName" ) )
    {
      this->TrainingParameterFileName = std::string( attValue );
    }

  }

  // Now, read from file if the files are specified
  this->ImportAvailableData();

}



// File I/O: Saving and loading
// ----------------------------------------------------------------------------



void
vtkMRMLWorkflowSegmentationNode
::SaveTrackingLog()
{
  std::ofstream output( this->TrackingLogFileName.c_str() );
  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Record file could not be opened!" );
    return;
  }

  output << this->toolCollection.BuffersToXMLString();

  output.close();
  
}





void
vtkMRMLWorkflowSegmentationNode
::SaveTrainingParameters()
{
  std::ofstream output( this->TrainingParameterFileName.c_str() );
  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Record file could not be opened!" );
    return;
  }
  
  output << this->toolCollection.TrainingParameterToXMLString();

  output.close();
  
}



void
vtkMRMLWorkflowSegmentationNode
::ImportProcedureDefinition()
{

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( this->ProcedureDefinitionFileName.c_str() );
  parser->Parse();
  
  // Get the root element (and check it exists)
  vtkXMLDataElement* rootElement = parser->GetRootElement();

  this->toolCollection.PerkProcedureFromXMLElement( rootElement );

}




void
vtkMRMLWorkflowSegmentationNode
::ImportInputParameters()
{

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( this->InputParameterFileName.c_str() );
  parser->Parse();
  
  // Get the root element (and check it exists)
  vtkXMLDataElement* rootElement = parser->GetRootElement();

  this->toolCollection.InputParameterFromXMLElement( rootElement );

}


void
vtkMRMLWorkflowSegmentationNode
::ImportTrainingParameters()
{

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( this->TrainingParameterFileName.c_str() );
  parser->Parse();
  
  // Get the root element (and check it exists)
  vtkXMLDataElement* rootElement = parser->GetRootElement();

  this->toolCollection.TrainingParameterFromXMLElement( rootElement );

}


void vtkMRMLWorkflowSegmentationNode
::ImportAvailableData()
{
  this->ImportProcedureDefinition();
  this->ImportInputParameters();
  this->ImportTrainingParameters();
}


// Local variable getters and setters
// ----------------------------------------------------------------------------


bool vtkMRMLWorkflowSegmentationNode::GetRecording()
{
  return this->Recording;
}

void vtkMRMLWorkflowSegmentationNode::SetRecording( bool newState )
{
  this->Recording = newState;
  // TODO: I don't think we need anything below
  if ( this->Recording )
  {
    this->InvokeEvent( this->RecordingStartEvent, NULL );
  }
  else
  {
    this->InvokeEvent( this->RecordingStopEvent, NULL );
  }
}


std::string vtkMRMLWorkflowSegmentationNode::GetTrackingLogFileName()
{
  return this->TrackingLogFileName;
}

void vtkMRMLWorkflowSegmentationNode::SetTrackingLogFileName( std::string name )
{
  this->TrackingLogFileName = name;
}

std::string vtkMRMLWorkflowSegmentationNode::GetSegmentationLogFileName()
{
  return this->SegmentationLogFileName;
}

void vtkMRMLWorkflowSegmentationNode::SetSegmentationLogFileName( std::string name )
{
  this->SegmentationLogFileName = name;
}

std::string vtkMRMLWorkflowSegmentationNode::GetProcedureDefinitionFileName()
{
  return this->ProcedureDefinitionFileName;
}

void vtkMRMLWorkflowSegmentationNode::SetProcedureDefinitionFileName( std::string name )
{
  this->ProcedureDefinitionFileName = name;
}

std::string vtkMRMLWorkflowSegmentationNode::GetInputParameterFileName()
{
  return this->InputParameterFileName;
}

void vtkMRMLWorkflowSegmentationNode::SetInputParameterFileName( std::string name )
{
  this->InputParameterFileName = name;
}

std::string vtkMRMLWorkflowSegmentationNode::GetTrainingParameterFileName()
{
  return this->TrainingParameterFileName;
}

void vtkMRMLWorkflowSegmentationNode::SetTrainingParameterFileName( std::string name )
{
  this->TrainingParameterFileName = name;
}


// Slicer Scene
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode );
  vtkMRMLWorkflowSegmentationNode *node = ( vtkMRMLWorkflowSegmentationNode* ) anode;

    // Observers must be removed here, otherwise MRML updates would activate nodes on the undo stack
  
  for ( unsigned int i = 0; i < this->ObservedTransformNodes.size(); ++ i )
    {
    this->ObservedTransformNodes[ i ]->RemoveObservers( vtkMRMLLinearTransformNode::TransformModifiedEvent );
    }
  
  // Note: It seems that the WriteXML function copies the node then writes the copied node to file
  // So, anything we want in the MRML file we must copy here (I don't think we need to copy other things)
  this->SetRecording( node->GetRecording() );
  this->SetTrackingLogFileName( node->GetTrackingLogFileName() );
  this->SetSegmentationLogFileName( node->GetSegmentationLogFileName() );
  this->SetProcedureDefinitionFileName( node->GetProcedureDefinitionFileName() );
  this->SetInputParameterFileName( node->GetInputParameterFileName() );
  this->SetTrainingParameterFileName( node->GetTrainingParameterFileName() );
  this->ImportAvailableData();

}


void vtkMRMLWorkflowSegmentationNode::UpdateReferences()
{
  Superclass::UpdateReferences();
      // MRML node ID's should be checked. If Scene->GetNodeByID( id ) returns NULL,
    // the reference should be deleted (set to NULL).
  
}



void vtkMRMLWorkflowSegmentationNode::UpdateReferenceID( const char *oldID, const char *newID )
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



void vtkMRMLWorkflowSegmentationNode::UpdateScene( vtkMRMLScene *scene )
{
  Superclass::UpdateScene( scene );
  // this->SetAndObserveConnectorNodeID( this->ConnectorNodeID );
  // TODO: Deal with observed transforms.
}



void vtkMRMLWorkflowSegmentationNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "TrackingLogFileName: " << this->TrackingLogFileName << "\n";
}


// MRML Observers
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
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



void vtkMRMLWorkflowSegmentationNode::RemoveMRMLObservers()
{
  for ( unsigned int i = 0; i < this->ObservedTransformNodes.size(); ++ i )
  {
    this->ObservedTransformNodes[ i ]->RemoveObservers( vtkMRMLLinearTransformNode::TransformModifiedEvent );
  }
}





// Observed transforms
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode
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



void vtkMRMLWorkflowSegmentationNode
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



void vtkMRMLWorkflowSegmentationNode
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



vtkMRMLLinearTransformNode* vtkMRMLWorkflowSegmentationNode
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



// Time
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode
::GetTimestamp( int &sec, int &nsec )
{
  clock_t clock1 = clock();
  double seconds = double( clock1 - this->Clock0 ) / CLOCKS_PER_SEC;
  sec = floor( seconds );
  nsec = ( seconds - sec ) * 1e9;    
}


double vtkMRMLWorkflowSegmentationNode
::GetTimestamp()
{
  clock_t clock1 = clock();
  return double( clock1 - this->Clock0 ) / CLOCKS_PER_SEC;
}




// Buffers
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode::AddNewTransform( const char* TransformNodeID )
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
  

    // Record the transform.
  
  std::stringstream mss;
  for ( int row = 0; row < 4; ++ row )
  {
    for ( int col = 0; col < 4; ++ col )
    {
      mss << m->GetElement( row, col ) << " ";
    }
  }

  // May be this should go higher
  if ( this->Recording == false )
  {
    return;
  }

  // Check if the transform has changed from the last time stamp. If it hasn't, then don't record
  Tool currTool = this->toolCollection.GetTool( deviceName );
  TransformRecord prevTransform = currTool.transBuff.GetTransformLast();
  if ( strcmp( mss.str().c_str(), prevTransform.Transform.c_str() ) != 0 )
  {
    currTool.transBuff.AddTransform( mss.str(), sec, nsec );
  }


}






// TODO: I don't think this function is needed at all
/**
 * @param selections Should contain as many elements as the number of incoming
 *        transforms throught the active connector. Order follows the order in
 *        the connector node. 0 means transform is not tracked, 1 means it's tracked.
 */
void vtkMRMLWorkflowSegmentationNode::SetTransformSelections( std::vector< int > selections )
{
  this->TransformSelections.clear();
  
  for ( unsigned int i = 0; i < selections.size(); ++ i )
  {
    this->TransformSelections.push_back( selections[ i ] );
  }


}