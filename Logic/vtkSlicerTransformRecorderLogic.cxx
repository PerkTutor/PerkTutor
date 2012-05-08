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
#include "vtkMRMLIGTLConnectorNode.h"
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
  this->ModuleNode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerTransformRecorderLogic::~vtkSlicerTransformRecorderLogic()
{
  if ( this->ModuleNode != NULL )
  {
    this->ModuleNode->Delete();
    this->ModuleNode = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerTransformRecorderLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerTransformRecorderLogic::InitializeEventListeners()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(this->GetMRMLScene(), events.GetPointer());
}

//---------------------------------------------------------------------------
void vtkSlicerTransformRecorderLogic::SetOpenIGTLConnectorNode( vtkMRMLIGTLConnectorNode* node )
{
  if ( this->GetModuleNode() != NULL )
  {
    this->GetModuleNode()->SetAndObserveConnectorNodeID( node->GetID() );
  }
}


//-----------------------------------------------------------------------------
void vtkSlicerTransformRecorderLogic::RegisterNodes()
{
  //assert(this->GetMRMLScene() != 0);
  
  if( ! this->GetMRMLScene() )
  {
    return;
  }
  vtkMRMLTransformRecorderNode* pNode = vtkMRMLTransformRecorderNode::New();
  this->GetMRMLScene()->RegisterNodeClass( pNode );
  pNode->Delete(); 
  
}

//---------------------------------------------------------------------------
void vtkSlicerTransformRecorderLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerTransformRecorderLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{

}

//---------------------------------------------------------------------------
void vtkSlicerTransformRecorderLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
	assert(this->GetMRMLScene() != 0);
}

void vtkSlicerTransformRecorderLogic
::SetModuleNode( vtkMRMLTransformRecorderNode* node )
{
  vtkSetMRMLNodeMacro( this->ModuleNode, node );
  this->Modified();
}


