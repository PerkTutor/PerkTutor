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

  vtkMRMLTransformBufferNode* pNode = vtkMRMLTransformBufferNode::New();
  this->GetMRMLScene()->RegisterNodeClass( pNode );
  pNode->Delete();   
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
::AddTransformBuffer( const char* fileName, const char* nodeName )
{
  
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
  bufferNode->Modified();
}



void vtkSlicerTransformRecorderLogic
::RemoveObservedTransformNode( vtkMRMLTransformBufferNode* bufferNode, vtkMRMLNode* node )
{
  if ( bufferNode == NULL )
  {
    return;
  }

  // Unobserve the node
  node->RemoveObservers( vtkMRMLTransformNode::TransformModifiedEvent, (vtkCommand*) this->GetMRMLNodesCallbackCommand() );
  bufferNode->RemoveActiveTransform( node->GetName() );
  bufferNode->Modified();
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
::ImportFromFile( vtkMRMLTransformBufferNode* bufferNode, std::string fileName )
{
  // Clear the current buffer prior to importing
  bufferNode->Clear();

  vtkXMLDataParser* parser = vtkXMLDataParser::New();
  parser->SetFileName( fileName.c_str() );
  parser->Parse();

  bufferNode->FromXMLElement( parser->GetRootElement() );
  bufferNode->SetActiveTransformsFromBuffer();

  // Change the name to reflect the file it was read from
  int dotFound = fileName.find_last_of( "." );
  int slashFound = fileName.find_last_of( "/" );
  std::stringstream rootName;
  rootName << bufferNode->GetName() << "_" << fileName.substr( slashFound + 1, dotFound - slashFound - 1 );
  bufferNode->SetName( rootName.str().c_str() );

  // Add the active transform nodes to the scene
  // This will be particularly useful for the PerkEvaluator
  std::vector<std::string> activeTransforms = bufferNode->GetActiveTransforms();
  for ( int i = 0; i < activeTransforms.size(); i++ )
  {	
    vtkMRMLLinearTransformNode* node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetFirstNode( activeTransforms.at(i).c_str(), "vtkMRMLLinearTransformNode" ) );
    if ( node == NULL )
    {
      node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLLinearTransformNode" ) );
	  this->GetMRMLScene()->AddNode( node );
	  node->SetScene( this->GetMRMLScene() );
	  node->SetName( activeTransforms.at(i).c_str() );
    }
  }

  bufferNode->Modified();

}


void vtkSlicerTransformRecorderLogic
::SaveToFile( vtkMRMLTransformBufferNode* bufferNode, std::string fileName )
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
::AddTransform( vtkMRMLTransformBufferNode* bufferNode, vtkMRMLTransformNode* transformNode )
{

  // Get the transform matrix from the node
  vtkMRMLLinearTransformNode* linearTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( transformNode );
  vtkSmartPointer< vtkMatrix4x4 > transformMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  transformMatrix->DeepCopy( linearTransformNode->GetMatrixTransformToParent() );
  
  
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