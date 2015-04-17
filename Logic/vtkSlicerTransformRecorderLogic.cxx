/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// TransformRecorder includes
#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkMRMLTransformBufferNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLNode.h"

// MRML includes
// #include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>


// Helper Methods ------------------------------------------------------------

void Trim(std::string &str)
{
  str.erase(str.find_last_not_of(" \t\r\n")+1);
  str.erase(0,str.find_first_not_of(" \t\r\n"));
}

// Constants for reading transforms
static const int MAX_LINE_LENGTH = 1000;

static std::string SEQMETA_FIELD_FRAME_FIELD_PREFIX = "Seq_Frame";
static std::string SEQMETA_FIELD_IMG_STATUS = "ImageStatus";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTransformRecorderLogic);

//----------------------------------------------------------------------------
vtkSlicerTransformRecorderLogic::vtkSlicerTransformRecorderLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerTransformRecorderLogic::~vtkSlicerTransformRecorderLogic()
{
}



void vtkSlicerTransformRecorderLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}



void vtkSlicerTransformRecorderLogic::InitializeEventListeners()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(this->GetMRMLScene(), events.GetPointer());
}



void vtkSlicerTransformRecorderLogic::RegisterNodes()
{
  if( ! this->GetMRMLScene() )
  {
    return;
  }

  vtkMRMLTransformBufferNode* tbNode = vtkMRMLTransformBufferNode::New();
  this->GetMRMLScene()->RegisterNodeClass( tbNode );
  tbNode->Delete();

  vtkMRMLTransformBufferStorageNode* tbsNode = vtkMRMLTransformBufferStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass( tbsNode );
  tbsNode->Delete();   
}



void vtkSlicerTransformRecorderLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}



void vtkSlicerTransformRecorderLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
  assert(this->GetMRMLScene() != 0);
}


void vtkSlicerTransformRecorderLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
  assert(this->GetMRMLScene() != 0);
}


void vtkSlicerTransformRecorderLogic
::ClearTransforms( vtkMRMLTransformBufferNode* bufferNode )
{
  if ( bufferNode != NULL )
  {
    bufferNode->ClearTransforms();
  }
}


void vtkSlicerTransformRecorderLogic
::AddMessage( vtkMRMLTransformBufferNode* bufferNode, std::string messageString, double time )
{
  if ( bufferNode == NULL )
  {
    return;
  }

  vtkSmartPointer< vtkMessageRecord > newMessageRecord = vtkSmartPointer< vtkMessageRecord >::New();
  newMessageRecord->SetMessageString( messageString );
  newMessageRecord->SetTime( time );
  bufferNode->AddMessage( newMessageRecord );
}


void vtkSlicerTransformRecorderLogic
::RemoveMessage( vtkMRMLTransformBufferNode* bufferNode, int index )
{
  if ( bufferNode != NULL )
  {
	  bufferNode->RemoveMessage( index );
  }
}


void vtkSlicerTransformRecorderLogic
::ClearMessages( vtkMRMLTransformBufferNode* bufferNode )
{
  if ( bufferNode != NULL )
  {
	  bufferNode->ClearMessages();
  }
}


void vtkSlicerTransformRecorderLogic
::ObserveAllRecordedTransforms( vtkMRMLTransformBufferNode* bufferNode )
{
  // First add all of the recorded transforms to the scene
  this->AddAllRecordedTransformsToScene( bufferNode );

  // Now, observe all of them
  std::vector< std::string > recordedTransforms = bufferNode->GetAllRecordedTransformNames();
  for ( int i = 0; i < recordedTransforms.size(); i++ )
  {	
    vtkSmartPointer< vtkMRMLLinearTransformNode > transformNode;
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetFirstNode( recordedTransforms.at(i).c_str(), "vtkMRMLLinearTransformNode" ) );
    if ( transformNode != NULL ) // We just added them all to the scene, so they should all be non-null
    {
      bufferNode->AddActiveTransformID( transformNode->GetID() );
    }
  }

}


void vtkSlicerTransformRecorderLogic
::AddAllRecordedTransformsToScene( vtkMRMLTransformBufferNode* bufferNode )
{
  // This adds all the recorded transforms to the scene
  // Note: The active transforms should already be in the scene anyway

  std::vector< std::string > recordedTransforms = bufferNode->GetAllRecordedTransformNames();
  for ( int i = 0; i < recordedTransforms.size(); i++ )
  {	
    vtkSmartPointer< vtkMRMLLinearTransformNode > transformNode;
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetFirstNode( recordedTransforms.at(i).c_str(), "vtkMRMLLinearTransformNode" ) );
    if ( transformNode == NULL )
    {
      transformNode.TakeReference( vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLLinearTransformNode" ) ) );
      transformNode->SetName( recordedTransforms.at(i).c_str() );
      transformNode->SetScene( this->GetMRMLScene() );
	    this->GetMRMLScene()->AddNode( transformNode );
    }
  }

}


void vtkSlicerTransformRecorderLogic
::ImportFromXMLFile( vtkMRMLTransformBufferNode* bufferNode, std::string fileName )
{
  // Clear the current buffer prior to importing
  bufferNode->Clear();

  vtkXMLDataParser* parser = vtkXMLDataParser::New();
  parser->SetFileName( fileName.c_str() );
  parser->Parse();

  bufferNode->FromXMLElement( parser->GetRootElement() );

  parser->Delete();
  bufferNode->Modified();
}


void vtkSlicerTransformRecorderLogic
::ExportToFile( vtkMRMLTransformBufferNode* bufferNode, std::string fileName )
{
  if ( bufferNode == NULL )
  {
    return;
  }

  std::ofstream output( fileName.c_str() );
  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Record file could not be opened!" );
    return;
  }

  output << bufferNode->ToXMLString();

  output.close();

}


void vtkSlicerTransformRecorderLogic
::ImportFromMHAFile( vtkMRMLTransformBufferNode* bufferNode, std::string fileName )
{
  // Clear the current buffer prior to importing
  bufferNode->Clear();
  
  // Open in binary mode because we determine the start of the image buffer also during this read
  const char* flags = "rb";
  FILE* stream = fopen( fileName.c_str(), flags ); // TODO: Removed error

  char line[ MAX_LINE_LENGTH + 1 ] = { 0 };

  // It contains the largest frame number. It will be used to iterate through all the frame numbers from 0 to lastFrameNumber
  int lastFrameNumber = -1;
  std::map< int, std::string > FrameNumberToTimestamp;
  std::map< int, std::vector< vtkTransformRecord* > > FrameNumberToTransformRecords;
  std::map< int, std::map< std::string, bool > > FrameNumberToStatuses;

  while ( fgets( line, MAX_LINE_LENGTH, stream ) )
  {
    std::string lineStr = line;

    // Split line into name and value
    size_t equalSignFound = 0;
    equalSignFound = lineStr.find_first_of( "=" );
    if ( equalSignFound == std::string::npos )
    {
      vtkWarningMacro( "Parsing line failed, equal sign is missing (" << lineStr << ")" );
      continue;
    }
    std::string name = lineStr.substr( 0, equalSignFound );
    std::string value = lineStr.substr( equalSignFound + 1 );

    Trim( name );
    Trim( value );

    if ( name.compare( "ElementDataFile" ) == NULL )
    {
      // This is the last field of the header
      break;
    }

    // Only consider the Seq_Frame
    if ( name.compare( 0, SEQMETA_FIELD_FRAME_FIELD_PREFIX.size(), SEQMETA_FIELD_FRAME_FIELD_PREFIX ) != 0 )
    {
      // nNt a frame field, ignore it
      continue;
    }

    // frame field
    // name: Seq_Frame0000_CustomTransform
    name.erase( 0, SEQMETA_FIELD_FRAME_FIELD_PREFIX.size() ); // 0000_CustomTransform

    // Split line into name and value
    size_t underscoreFound;
    underscoreFound = name.find_first_of( "_" );
    if ( underscoreFound == std::string::npos )
    {
      vtkWarningMacro( "Parsing line failed, underscore is missing from frame field name (" << lineStr << ")" );
      continue;
    }

    std::string frameNumberStr = name.substr( 0, underscoreFound ); // 0000
    std::string frameFieldName = name.substr( underscoreFound + 1 ); // CustomTransform

    int frameNumber = atoi( frameNumberStr.c_str() );
    if ( frameNumber > lastFrameNumber )
    {
      lastFrameNumber = frameNumber;
    }

    // Convert the string to transform and add transform to hierarchy
    if ( frameFieldName.find( "Transform" ) != std::string::npos && frameFieldName.find( "Status" ) == std::string::npos )
    {
      // Find the transform name (i.e. remove the "Transform" from the end)
      size_t transformFound;
      transformFound = frameFieldName.find( "Transform" );
      std::string transformName = frameFieldName.substr( 0, transformFound );

      // Create the transform record
      vtkTransformRecord* transformRecord =  vtkTransformRecord::New();
      transformRecord->SetTransformString( value );
      transformRecord->SetDeviceName( transformName );
      transformRecord->SetTime( frameNumber );

      FrameNumberToTransformRecords[ frameNumber ].push_back( transformRecord );
    }

    // Find the transform status
    if ( frameFieldName.find( "Transform" ) != std::string::npos && frameFieldName.find( "Status" ) != std::string::npos )
    {
      // Find the transform name (i.e. remove the "Transform" from the end)
      size_t transformFound;
      transformFound = frameFieldName.find( "Transform" );
      std::string transformName = frameFieldName.substr( 0, transformFound );

      if ( value.compare( "INVALID" ) == 0 )
      {
        FrameNumberToStatuses[ frameNumber ][ transformName ] = false;
      }
      else
      {
        FrameNumberToStatuses[ frameNumber ][ transformName ] = true;
      }
    }

    if ( frameFieldName.find( "Timestamp" ) != std::string::npos )
    {
      FrameNumberToTimestamp[ frameNumber ] = value;
    }

    if ( ferror( stream ) )
    {
      vtkErrorMacro( "Error reading the file " << fileName.c_str() );
      break;
    }
    
    if ( feof( stream ) )
    {
      break;
    }

  }
  fclose( stream );

  // Now put all the transform records into the transform buffer  
  for ( int i = 0; i < lastFrameNumber; i++ )
  {
    std::stringstream ss( FrameNumberToTimestamp[ i ] );
    double currentTime; ss >> currentTime;

    for ( int j = 0; j < FrameNumberToTransformRecords[ i ].size(); j++ )
    {
      // Skip if the transform is invalid at the time
      if ( ! FrameNumberToStatuses[ i ][ FrameNumberToTransformRecords[ i ].at( j )->GetDeviceName() ] )
      {
        continue;
      }

      FrameNumberToTransformRecords[ i ].at( j )->SetTime( currentTime );
      bufferNode->AddTransform( FrameNumberToTransformRecords[ i ].at( j ) );
    }    
  }

  FrameNumberToTimestamp.clear();
  FrameNumberToTransformRecords.clear();
  FrameNumberToStatuses.clear();
}