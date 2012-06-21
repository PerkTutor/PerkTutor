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

// TargetFiducials includes
#include "vtkSlicerTargetFiducialsLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"
#include "vtkMRMLAnnotationFiducialNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkMatrix4x4.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTargetFiducialsLogic);

//----------------------------------------------------------------------------
vtkSlicerTargetFiducialsLogic::vtkSlicerTargetFiducialsLogic()
{
  this->TransformNode = NULL;
  this->AnnotationHierarchyNode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerTargetFiducialsLogic::~vtkSlicerTargetFiducialsLogic()
{
  if ( this->TransformNode != NULL )
  {
    this->TransformNode->Delete();
    this->TransformNode = NULL;
  }
  
  if ( this->AnnotationHierarchyNode != NULL )
  {
    this->AnnotationHierarchyNode->Delete();
    this->AnnotationHierarchyNode = NULL;
  }
}

//----------------------------------------------------------------------------
void vtkSlicerTargetFiducialsLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "TransformNode: "
     << ( this->TransformNode ? this->TransformNode->GetNodeTagName() : "(none)" ) << std::endl;
}

void vtkSlicerTargetFiducialsLogic::SetTransformNode( vtkMRMLLinearTransformNode *node )
{
  vtkSetMRMLNodeMacro( this->TransformNode, node );
  this->Modified();
}



void vtkSlicerTargetFiducialsLogic::SetAnnotationHierarchyNode( vtkMRMLAnnotationHierarchyNode *node )
{
  vtkSetMRMLNodeMacro( this->AnnotationHierarchyNode, node );
  this->Modified();
}


void vtkSlicerTargetFiducialsLogic::GetFiducialCoords()
{
  if (this->TransformNode == NULL ||  this->AnnotationHierarchyNode == NULL)
  {
    return;
  }
  
  vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
  this->TransformNode->GetMatrixTransformToWorld( transformToWorld );
  
  vtkMRMLAnnotationFiducialNode * fiducialNode = vtkMRMLAnnotationFiducialNode::New();
  
  double stylusCoord[ 3 ] = { transformToWorld->GetElement( 0, 3 ), transformToWorld->GetElement( 1, 3 ), transformToWorld->GetElement( 2, 3 ) };
  
  double fiducialCoord[ 3 ] = {0,0,0};
  bool gotFiducialCoord = fiducialNode->GetFiducialCoordinates(fiducialCoord);

  // fnode->SetControlPoint( 0, coord );
  
  //this->GetMRMLScene()->AddNode( fnode );

  fiducialNode->Delete();
  transformToWorld->Delete();
}



//---------------------------------------------------------------------------
void vtkSlicerTargetFiducialsLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerTargetFiducialsLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerTargetFiducialsLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerTargetFiducialsLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}

//---------------------------------------------------------------------------
void vtkSlicerTargetFiducialsLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}

