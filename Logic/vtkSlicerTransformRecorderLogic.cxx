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
#include "vtkMRMLTransformRecorderNode.h"

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
::ProcessMRMLEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLTransformNode* callerNode = vtkMRMLTransformNode::SafeDownCast( caller );

  // For all buffers, iterate through all observed transforms and check if the name corresponds to the modified event
  for ( int i = 0; i < this->RecordingBuffers.size(); i++ )
  {
    std::vector<std::string> activeTransforms = this->RecordingBuffers.at(i)->GetActiveTransforms();
    for ( int j = 0; j < activeTransforms.size(); j++ )
    {
      if ( activeTransforms.at(j).compare( callerNode->GetName() ) )
      {
        //this->AddTransform( this->RecordingBuffer.at(i), callerNode );
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
  // Make sure the node is observed
  node->AddObserver( vtkMRMLTransformNode::TransformModifiedEvent, (vtkCommand*) this->MRMLLogicsCallback );

  if ( bufferNode != NULL )
  {
    bufferNode->AddActiveTransform( node->GetName() );
  }
}



void vtkSlicerTransformRecorderLogic
::RemoveObservedTransformNode( vtkMRMLTransformBufferNode* bufferNode, vtkMRMLNode* node )
{
  if ( bufferNode != NULL )
  {
    bufferNode->RemoveActiveTransform( node->GetName() );
  }
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
    if ( activeTransforms.at(i).compare( node->GetName() ) )
    {
      return true;
    }
  }

  return false;
}



void vtkSlicerTransformRecorderLogic
::SetRecording( vtkMRMLTransformBufferNode* bufferNode, bool isRecording )
{
  if ( bufferNode != NULL )
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