/*=auto=========================================================================
Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.
See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.
Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformRecorderNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $
=========================================================================auto=*/
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
#include "vtkMRMLIGTLConnectorNode.h"
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



vtkMRMLTransformRecorderNode
::vtkMRMLTransformRecorderNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  this->SetModifiedSinceRead( true );
  
  this->TransformNodeID = NULL;
  this->TransformNode   = NULL;
  
  this->ConnectorNodeID = NULL;
  this->ConnectorNode   = NULL;
  
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

  
  this->SetAndObserveConnectorNodeID( NULL );
  
  for ( unsigned int i = 0; i < this->TransformNodes.size(); ++ i )
  {
    vtkSetAndObserveMRMLObjectMacro( this->TransformNodes[ i ], NULL );
  }
  
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
  
  if (this->TransformNodeID != NULL) 
    {
    of << indent << " TransformNodeRef=\""
       << this->TransformNodeID << "\"";
    }
  
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
  
  for ( unsigned int i = 0; i < this->TransformNodes.size(); ++ i )
    {
    this->TransformNodes[ i ]->RemoveObservers(
        vtkMRMLTransformNode::TransformModifiedEvent );
    }
  
  this->SetRecording( node->GetRecording() );
  this->SetLogFileName( node->GetLogFileName() );
  
  this->SetAndObserveConnectorNodeID( NULL );
  this->SetConnectorNodeID( node->ConnectorNodeID );
  

}


//----------------------------------------------------------------------------
void vtkMRMLTransformRecorderNode::UpdateReferences()
{
  Superclass::UpdateReferences();
      // MRML node ID's should be checked. If Scene->GetNodeByID( id ) returns NULL,
    // the reference should be deleted (set to NULL).
    if ( this->ConnectorNodeID != NULL && this->Scene->GetNodeByID( this->ConnectorNodeID ) == NULL )
    {
    this->SetConnectorNodeID( NULL );
    }
	
	  if ( this->TransformNodeID != NULL && this->Scene->GetNodeByID( this->TransformNodeID ) == NULL )
    {
    this->SetTransformNodeID( NULL );
    }

}



void vtkMRMLTransformRecorderNode::UpdateReferenceID( const char *oldID, const char *newID )
{
  Superclass::UpdateReferenceID( oldID, newID );
  
    
  if ( this->ConnectorNodeID && !strcmp( oldID, this->ConnectorNodeID ) )
    {
    this->SetAndObserveConnectorNodeID( newID );
    }
  
  
}



void vtkMRMLTransformRecorderNode::UpdateScene( vtkMRMLScene *scene )
{
   Superclass::UpdateScene( scene );
     this->SetAndObserveConnectorNodeID( this->ConnectorNodeID );

}



void vtkMRMLTransformRecorderNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "ConnectorNodeID: " << ( this->ConnectorNodeID ? this->ConnectorNodeID : "(none)" ) << "\n";
  os << indent << "TransformNodeID: " << (this->TransformNodeID? this->TransformNodeID: "(none)") << "\n";
  os << indent << "LogFileName: " << this->LogFileName << "\n";
  
}



void vtkMRMLTransformRecorderNode::SetAndObserveConnectorNodeID( const char* ConnectorNodeRef )
{
  vtkSetAndObserveMRMLObjectMacro( this->ConnectorNode, NULL );
  this->SetConnectorNodeID( ConnectorNodeRef );
  vtkMRMLIGTLConnectorNode* cnode = this->GetConnectorNode();
  vtkSetAndObserveMRMLObjectMacro( this->ConnectorNode, cnode );
  if ( cnode )
    {
    cnode->AddObserver( vtkMRMLIGTLConnectorNode::ReceiveEvent, (vtkCommand*)this->MRMLCallbackCommand );
    }
}



vtkMRMLIGTLConnectorNode* vtkMRMLTransformRecorderNode
::GetConnectorNode()
{
  vtkMRMLIGTLConnectorNode* node = NULL;
  if (    this->GetScene()
       && this->ConnectorNodeID != NULL )
    {
    vtkMRMLNode* snode = this->GetScene()->GetNodeByID( this->ConnectorNodeID );
    node = vtkMRMLIGTLConnectorNode::SafeDownCast( snode );
    }
  return node;
}



void vtkMRMLTransformRecorderNode::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{
    // Handle IGT Connector events.
  if ( ! this->Active ) return;
  
  if ( this->ConnectorNode == vtkMRMLIGTLConnectorNode::SafeDownCast( caller )
       && event == vtkMRMLIGTLConnectorNode::ReceiveEvent )
    {
    int numIncomingNodes = this->ConnectorNode->GetNumberOfIncomingMRMLNodes();
    std::vector< vtkMRMLLinearTransformNode* > transformNodes;
    for ( int nodeIndex = 0; nodeIndex < numIncomingNodes; ++ nodeIndex )
      {
      vtkMRMLNode* node = this->ConnectorNode->GetIncomingMRMLNode( nodeIndex );
      vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( node );
      if ( transformNode != NULL )
        {
        transformNodes.push_back( transformNode );
        }
      }
    
      // Update the vector of observed transform nodes.
    
    bool updateTransforms = false;
    if ( this->TransformNodes.size() != transformNodes.size() )
      {
      updateTransforms = true;
      }
    else
      {
      for ( unsigned int i = 0; i < transformNodes.size(); ++ i )
        {
        if ( strcmp( transformNodes[ i ]->GetID(), this->TransformNodes[ i ]->GetID() ) != 0 )
          {
          updateTransforms = true;
          }
        }
      }
    
        // Remove existing observers.
        // Add new observers.
      
    if ( updateTransforms )
      {
      
      for ( unsigned int i = 0; i < this->TransformNodes.size(); ++ i )
        {
        this->TransformNodes[ i ]->RemoveObservers( vtkMRMLTransformNode::TransformModifiedEvent );
        vtkSetAndObserveMRMLObjectMacro( this->TransformNodes[ i ], NULL );
        }
      
      this->TransformNodes.clear();
      
      for ( unsigned int i = 0; i < transformNodes.size(); ++ i )
        {
        this->TransformNodes.push_back( NULL );
        vtkSetAndObserveMRMLObjectMacro( this->TransformNodes[ i ], transformNodes[ i ] );
        this->TransformNodes[ i ]->AddObserver( vtkMRMLTransformNode::TransformModifiedEvent, (vtkCommand*)this->MRMLCallbackCommand );
        }
      }
    }
  
  
    // Handle modified event of any observed transform node.
  
  for ( unsigned int tIndex = 0; tIndex < this->TransformNodes.size(); ++ tIndex )
  {
    if (    this->TransformNodes[ tIndex ] == vtkMRMLTransformNode::SafeDownCast( caller )
         && event == vtkMRMLTransformNode::TransformModifiedEvent
         && this->Recording == true )
    {
      this->AddNewTransform( tIndex );
    }
  }
}



void vtkMRMLTransformRecorderNode::RemoveMRMLObservers()
{
  if ( this->TransformNode )
  {
    this->TransformNode->RemoveObservers( vtkMRMLTransformNode::TransformModifiedEvent );
  }
  
  if ( this->ConnectorNode )
  {
    this->ConnectorNode->RemoveObservers( vtkMRMLIGTLConnectorNode::ReceiveEvent );
  }
  
  
  for ( unsigned int i = 0; i < this->TransformNodes.size(); ++ i )
  {
    this->TransformNodes[ i ]->RemoveObservers( vtkMRMLTransformNode::TransformModifiedEvent );
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
  //this->UpdateFileFromBuffer();

}



std::string vtkMRMLTransformRecorderNode::GetLogFileName()
{
  return this->LogFileName;
}



void vtkMRMLTransformRecorderNode::CustomMessage( std::string message )
{
  clock_t clock1 = clock();
  double seconds = double( clock1 - this->Clock0 ) / CLOCKS_PER_SEC;
  
  long int sec = floor( seconds );
  int nsec = ( seconds - sec ) * 1e9;
  
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



void vtkMRMLTransformRecorderNode::AddNewTransform( int index )


{
  
    // Check if we have a valid list of which transform should be recorded.
    // If we don't, record every transform.
  
  bool recordAll = false;
  if ( this->TransformSelections.size() != this->TransformNodes.size() )
  {
    recordAll = true;
  }
  
  
  int sec = 0;
  int nsec = 0;
  vtkSmartPointer< vtkMatrix4x4 > m = vtkSmartPointer< vtkMatrix4x4 >::New();
  std::string deviceName;
  
  this->ConnectorNode->LockIncomingMRMLNode( this->TransformNodes[ index ] );
  
  
    // Compute the timestamp.
  
  this->ConnectorNode->GetIGTLTimeStamp( this->TransformNodes[ index ], sec, nsec );
  
  if ( sec == 0 && nsec == 0 )  // No IGTL time received. Use clock instead.
  {
    clock_t clock1 = clock();
    double seconds = double( clock1 - this->Clock0 ) / CLOCKS_PER_SEC;
    sec = floor( seconds );
    nsec = ( seconds - sec ) * 1e9;
  }
  else   // If the IGTL time was used.
  {
    if ( ! this->IGTLTimeSynchronized )  // First time to receive IGTL time. Need to synchronize.
    {
      clock_t clock1 = clock();
      double clockSeconds = double( clock1 - this->Clock0 ) / CLOCKS_PER_SEC;
      double igtlSeconds = sec + nsec * 1e-9;
      this->IGTLTimeOffsetSeconds = clockSeconds - igtlSeconds;
      this->IGTLTimeSynchronized = true;
    }
    
      // Apply IGTL time offset.
    
    double igtlSeconds = sec + nsec * 1e-9;
    double fixedSeconds = igtlSeconds + this->IGTLTimeOffsetSeconds;
    sec = floor( fixedSeconds );
    nsec = ( fixedSeconds - sec ) * 1e9;
  }
  
  
    // Get the new transform matrix.
  
  vtkMRMLLinearTransformNode* ltn = vtkMRMLLinearTransformNode::SafeDownCast( this->TransformNodes[ index ] );
  
  if ( ltn != NULL )
  {
    m->DeepCopy( ltn->GetMatrixTransformToParent() );
  }
  else
  {
    vtkErrorMacro( "Non linear transform received from IGT connector!" );
  }
  
    
    // Get the device name for the new transform.
  
  deviceName = std::string( this->TransformNodes[ index ]->GetName() );
  
  
  this->ConnectorNode->UnlockIncomingMRMLNode( this->TransformNodes[ index ] );
  
  
    // Determine if this device has to be recorded or not.
  
  int record = 1;
  if ( ! recordAll )
  {
    record = this->TransformSelections[ index ];
  }
  
  
    // Compute path lengths for needle.
  
  if (    deviceName.compare( "Needle" ) == 0
       || deviceName.compare( "NeedleToReference" ) == 0
       || deviceName.compare( "Stylus" ) == 0
       || deviceName.compare( "StylusToReference" ) == 0
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
        fileOutput( "Needle velocity", vNeedle );  // TODO: Should be removed to improve performance!
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
    
  
  if ( record )
  {
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
    
    if ( this->Recording )
    {

      this->TransformsBuffer.push_back( rec );
      this->InvokeEvent( this->TransformChangedEvent, NULL );
    }

    
  } // if ( record )
}