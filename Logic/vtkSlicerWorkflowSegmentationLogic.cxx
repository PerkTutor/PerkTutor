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

// WorkflowSegmentation includes
#include "vtkSlicerWorkflowSegmentationLogic.h"
#include "vtkMRMLWorkflowSegmentationNode.h"

// MRML includes
// #include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerWorkflowSegmentationLogic);

//----------------------------------------------------------------------------
vtkSlicerWorkflowSegmentationLogic::vtkSlicerWorkflowSegmentationLogic()
{
  this->ModuleNode = NULL;
  this->workflowAlgorithm = vtkWorkflowAlgorithm::New();
}

//----------------------------------------------------------------------------
vtkSlicerWorkflowSegmentationLogic::~vtkSlicerWorkflowSegmentationLogic()
{
  if ( this->ModuleNode != NULL )
  {
    this->ModuleNode->Delete();
    this->ModuleNode = NULL;
  }

  this->workflowAlgorithm->Delete();
  this->workflowAlgorithm = NULL;
}



void vtkSlicerWorkflowSegmentationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}



void vtkSlicerWorkflowSegmentationLogic::InitializeEventListeners()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(this->GetMRMLScene(), events.GetPointer());
}



void vtkSlicerWorkflowSegmentationLogic::RegisterNodes()
{
  //assert(this->GetMRMLScene() != 0);
  
  if( ! this->GetMRMLScene() )
  {
    return;
  }
  vtkMRMLWorkflowSegmentationNode* pNode = vtkMRMLWorkflowSegmentationNode::New();
  this->GetMRMLScene()->RegisterNodeClass( pNode );
  pNode->Delete(); 
  
}



void vtkSlicerWorkflowSegmentationLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}



void vtkSlicerWorkflowSegmentationLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{

}

//---------------------------------------------------------------------------
void vtkSlicerWorkflowSegmentationLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
	assert(this->GetMRMLScene() != 0);
}

void vtkSlicerWorkflowSegmentationLogic
::SetModuleNode( vtkMRMLWorkflowSegmentationNode* node )
{
  vtkSetMRMLNodeMacro( this->ModuleNode, node );
  this->Modified();
}

vtkWorkflowAlgorithm* vtkSlicerWorkflowSegmentationLogic
::GetWorkflowAlgorithm()
{
  return this->workflowAlgorithm;
}

