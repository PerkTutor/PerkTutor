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

// Qt includes

// SlicerQt includes
#include "qSlicerTargetFiducialsModuleWidget.h"
#include "ui_qSlicerTargetFiducialsModule.h"

#include "vtkSlicerTargetFiducialsLogic.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLAnnotationHierarchyNode.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TargetFiducials
class qSlicerTargetFiducialsModuleWidgetPrivate: public Ui_qSlicerTargetFiducialsModule
{
  Q_DECLARE_PUBLIC( qSlicerTargetFiducialsModuleWidget ); 
protected:
  qSlicerTargetFiducialsModuleWidget* const q_ptr;
public:
  qSlicerTargetFiducialsModuleWidgetPrivate( qSlicerTargetFiducialsModuleWidget& object );
  vtkSlicerTargetFiducialsLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerTargetFiducialsModuleWidgetPrivate methods

//-----------------------------------------------------------------------------

qSlicerTargetFiducialsModuleWidgetPrivate::qSlicerTargetFiducialsModuleWidgetPrivate( qSlicerTargetFiducialsModuleWidget& object ) : q_ptr( &object )
{
}

vtkSlicerTargetFiducialsLogic* qSlicerTargetFiducialsModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerTargetFiducialsModuleWidget );
  return vtkSlicerTargetFiducialsLogic::SafeDownCast( q->logic() );
}

//-----------------------------------------------------------------------------
// qSlicerTargetFiducialsModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTargetFiducialsModuleWidget::qSlicerTargetFiducialsModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerTargetFiducialsModuleWidgetPrivate( *this ) )
{
}

//-----------------------------------------------------------------------------
qSlicerTargetFiducialsModuleWidget::~qSlicerTargetFiducialsModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTargetFiducialsModuleWidget::setup()
{
  Q_D(qSlicerTargetFiducialsModuleWidget);
  d->setupUi(this);
  //this->Superclass::setup();

  d->FiducialListComboBox->setNoneEnabled( true );
  d->TransformComboBox->setNoneEnabled( true );

  connect( d->FiducialListComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onFiducialListSelected() ) );
  connect( d->TransformComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTransformNodeSelected() ) );
}

/*
void qSlicerTargetFiducialsModuleWidget::enter()
{
  Q_D( qSlicerTargetFiducialsModuleWidget );

  this->Superclass::enter();

  this->updateWidget();
}
*/

/*
void qSlicerTargetFiducialsModuleWidget::onModuleNodeSelected()
{
  Q_D( qSlicerTargetFiducialsModuleWidget );
  
  vtkMRMLNode* currentNode = d->ModuleNodeComboBox->currentNode();
  
  vtkMRMLTargetFiducialsNode* node = vtkMRMLTargetFiducialsNode::SafeDownCast( currentNode );
  if ( node != NULL )
  {
      d->logic()->SetModuleNode( node );
  }
  this->updateWidget();
}
*/


void qSlicerTargetFiducialsModuleWidget::onFiducialListSelected()
{
  Q_D( qSlicerTargetFiducialsModuleWidget );
  
  vtkMRMLNode* node = d->FiducialListComboBox->currentNode();
  vtkMRMLAnnotationHierarchyNode* aNode = vtkMRMLAnnotationHierarchyNode::SafeDownCast( node );
  
  if( aNode != NULL )
  {
    d->logic()->SetAnnotationHierarchyNode( aNode );
  }
  
}

void qSlicerTargetFiducialsModuleWidget::onTransformNodeSelected()
{
  Q_D( qSlicerTargetFiducialsModuleWidget );
  
  vtkMRMLNode* node = d->TransformComboBox->currentNode();
  vtkMRMLLinearTransformNode* tNode = vtkMRMLLinearTransformNode::SafeDownCast( node );
  
  if( tNode != NULL )
  {
    d->logic()->SetTransformNode( tNode );
  }
}

/*
void qSlicerTargetFiducialsModuleWidget::updateWidget()
{
  Q_D( qSlicerTargetFiducialsModuleWidget );
  
  
    // Reference to the current module node.
  
  vtkMRMLTargetFiducialsNode* moduleNode = d->logic()->GetModuleNode();
  
  
    // Disableing node selector widgets if there is no module node to reference input nodes.
    
  if ( moduleNode == NULL )
  {
    d->FiducialListComboBox->setEnabled( false );
  }
  else
  {
    d->FiducialListComboBox->setEnabled( true );
  }
  
    // Update selector widgets if selected nodes have changed.
  
  if ( moduleNode == NULL )
  {
    return;
  }
 
  if ( moduleNode->GetConnectorNodeID() != NULL )
  {
    d->OpenIGTLinkConnectionSelector->setCurrentNode( QString( moduleNode->GetConnectorNodeID() ) );
  }
  
}
*/