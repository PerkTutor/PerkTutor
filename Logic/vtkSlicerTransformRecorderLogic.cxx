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
  this->Clock0 = clock();
}

//----------------------------------------------------------------------------
vtkSlicerTransformRecorderLogic::~vtkSlicerTransformRecorderLogic()
{
  this->RecordingBuffers.clear(); // The objects themselves will be deleted elsewhere
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
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLTransformNode* callerNode = vtkMRMLTransformNode::SafeDownCast( caller );

  // For all buffers, iterate through all observed transforms and check if the name corresponds to the modified event
  for ( int i = 0; i < this->RecordingBuffers.size(); i++ )
  {
    std::vector<std::string> activeTransforms = this->RecordingBuffers.at(i)->GetActiveTransforms();
    for ( int j = 0; j < activeTransforms.size(); j++ )
    {
      if ( activeTransforms.at(j).compare( callerNode->GetName() ) == 0 )
      {
        this->AddTransform( this->RecordingBuffers.at(i), callerNode );
      }
    }
  }


}


double vtkSlicerTransformRecorderLogic
::GetCurrentTimestamp()
{
  clock_t clock1 = clock();  
  return double( clock1 - this->Clock0 ) / CLOCKS_PER_SEC;
}


void vtkSlicerTransformRecorderLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
  assert(this->GetMRMLScene() != 0);
}


void vtkSlicerTransformRecorderLogic
::AddObservedTransformNode( vtkMRMLTransformBufferNode* bufferNode, vtkMRMLNode* node )
{
  if ( bufferNode == NULL )
  {
    return;
  }

  // Make sure the node is observed
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast( node );
  node->AddObserver( vtkMRMLTransformNode::TransformModifiedEvent, (vtkCommand*) this->GetMRMLNodesCallbackCommand() );
  bufferNode->AddActiveTransform( node->GetName() );
}



void vtkSlicerTransformRecorderLogic
::RemoveObservedTransformNode( vtkMRMLTransformBufferNode* bufferNode, vtkMRMLNode* node )
{
  if ( bufferNode == NULL )
  {
    return;
  }

  // Unobserve the node
  vtkMRMLTransformNode* transformNode = vtkMRMLTransformNode::SafeDownCast( node );
  node->RemoveObservers( vtkMRMLTransformNode::TransformModifiedEvent, (vtkCommand*) this->GetMRMLNodesCallbackCommand() );
  bufferNode->RemoveActiveTransform( node->GetName() );
}



bool vtkSlicerTransformRecorderLogic
::IsObservedTransformNode( vtkMRMLTransformBufferNode* bufferNode, vtkMRMLNode* node )
{
  if ( bufferNode == NULL )
  {
    return false;
  }

  std::vector<std::string> activeTransforms = bufferNode->GetActiveTransforms();

  for ( int i = 0; i < activeTransforms.size(); i++ )
  {
    if ( activeTransforms.at(i).compare( node->GetName() ) == 0 )
    {
      return true;
    }
  }

  return false;
}



void vtkSlicerTransformRecorderLogic
::SetRecording( vtkMRMLTransformBufferNode* bufferNode, bool isRecording )
{
  if ( bufferNode == NULL )
  {
    return;
  }

  for ( int i = 0; i < this->RecordingBuffers.size(); i++ )
  {
    if ( bufferNode == this->RecordingBuffers.at(i) && ! isRecording )
    {
      this->RecordingBuffers.erase( this->RecordingBuffers.begin() + i ); // Erase all instances of buffer in list
      i--;
      continue;
    }
    if ( bufferNode == this->RecordingBuffers.at(i) && isRecording )
    {
      bufferNode->Modified(); // If the recoding is start, then the node has been modified
      return; // If we found that this buffer node is in the list already, then do nothing
    }
  }

  // Finally if the buffer was not found and we want to add
  if ( isRecording )
  {
    this->RecordingBuffers.push_back( bufferNode );
  }

}



bool vtkSlicerTransformRecorderLogic
::GetRecording( vtkMRMLTransformBufferNode* bufferNode )
{
  if ( bufferNode == NULL )
  {
    return false;
  }

  for ( int i = 0; i < this->RecordingBuffers.size(); i++ )
  {
    if ( bufferNode == this->RecordingBuffers.at(i) )
    {
      return true;
    }
  }

  return false;
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
::AddMessage( vtkMRMLTransformBufferNode* bufferNode, std::string messageName, double time )
{
  if ( bufferNode == NULL )
  {
    return;
  }

  vtkMessageRecord* newMessage = vtkMessageRecord::New();
  newMessage->SetName( messageName );
  newMessage->SetTime( time );
  bufferNode->AddMessage( newMessage );
}


void vtkSlicerTransformRecorderLogic
::RemoveMessage( vtkMRMLTransformBufferNode* bufferNode, int index )
{
  if ( bufferNode != NULL )
  {
	bufferNode->RemoveMessageAt( index );
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
::AddTransform( vtkMRMLTransformBufferNode* bufferNode, vtkMRMLTransformNode* transformNode )
{

  // Get the transform matrix from the node
  vtkMRMLLinearTransformNode* linearTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( transformNode );
  vtkSmartPointer< vtkMatrix4x4 > transformMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();

#ifdef TRANSFORM_NODE_MATRIX_COPY_REQUIRED
  linearTransformNode->GetMatrixTransformToParent( transformMatrix );
#else
  transformMatrix->DeepCopy( linearTransformNode->GetMatrixTransformToParent() );
#endif
  
  // Record the transform into a string  
  std::stringstream matrixsstring;
  for ( int row = 0; row < 4; ++ row )
  {
    for ( int col = 0; col < 4; ++ col )
    {
      matrixsstring << transformMatrix->GetElement( row, col ) << " ";
    }
  }
 
  
  // Look for the most recent value of this transform
  // If the value hasn't changed, we don't record
  for ( int i = bufferNode->GetNumTransforms() - 1; i >= 0; i-- )
  {
    if ( bufferNode->GetTransformAt(i)->GetDeviceName().compare( transformNode->GetName() ) == 0 )
	{
      if ( bufferNode->GetTransformAt(i)->GetTransform().compare( matrixsstring.str() ) == 0 )
	  {
        return; // If it is a duplicate then exit, we have nothing to record
	  }
      break;
	}
  }


  vtkTransformRecord* transformRecord = vtkTransformRecord::New();
  transformRecord->SetTransform( matrixsstring.str() );
  transformRecord->SetDeviceName( transformNode->GetName() );
  transformRecord->SetTime( this->GetCurrentTimestamp() );
  bufferNode->AddTransform( transformRecord );

}


void vtkSlicerTransformRecorderLogic
::AddTransformsToScene( vtkMRMLTransformBufferNode* bufferNode )
{
  // Add the active transform nodes to the scene
  std::vector<std::string> activeTransforms = bufferNode->GetActiveTransforms();
  for ( int i = 0; i < activeTransforms.size(); i++ )
  {	
    vtkSmartPointer< vtkMRMLLinearTransformNode > transformNode;
    transformNode = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetFirstNode( activeTransforms.at(i).c_str(), "vtkMRMLLinearTransformNode" ) );
    if ( transformNode == NULL )
    {
      transformNode.TakeReference( vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLLinearTransformNode" ) ) );
      transformNode->SetName( activeTransforms.at(i).c_str() );
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
      transformRecord->SetTransform( value );
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