/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerWorkflowToolWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerWorkflowToolWidgetPrivate
  : public Ui_qSlicerWorkflowToolWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkflowToolWidget);
protected:
  qSlicerWorkflowToolWidget* const q_ptr;

public:
  qSlicerWorkflowToolWidgetPrivate( qSlicerWorkflowToolWidget& object);
  ~qSlicerWorkflowToolWidgetPrivate();
  virtual void setupUi(qSlicerWorkflowToolWidget*);
};

// --------------------------------------------------------------------------
qSlicerWorkflowToolWidgetPrivate
::qSlicerWorkflowToolWidgetPrivate( qSlicerWorkflowToolWidget& object) : q_ptr(&object)
{
}

qSlicerWorkflowToolWidgetPrivate
::~qSlicerWorkflowToolWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerWorkflowToolWidgetPrivate
::setupUi(qSlicerWorkflowToolWidget* widget)
{
  this->Ui_qSlicerWorkflowToolWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWorkflowToolWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkflowToolWidget
::qSlicerWorkflowToolWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerWorkflowToolWidgetPrivate(*this) )
{
  this->WorkflowToolNode = NULL;
  this->setup();
}


qSlicerWorkflowToolWidget
::~qSlicerWorkflowToolWidget()
{
}


void qSlicerWorkflowToolWidget
::setup()
{
  Q_D(qSlicerWorkflowToolWidget);

  d->setupUi(this);

  connect( d->WorkflowToolComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onWorkflowToolNodeChanged( vtkMRMLNode* ) ) );
  
  connect( d->WorkflowProcedureComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onWorkflowProcedureChanged( vtkMRMLNode* ) ) );
  connect( d->WorkflowInputComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onWorkflowInputChanged( vtkMRMLNode* ) ) );
  connect( d->WorkflowTrainingComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onWorkflowTrainingChanged( vtkMRMLNode* ) ) );

  this->updateWidget();  
}


vtkMRMLTableNode* qSlicerWorkflowToolWidget
::getWorkflowToolNode()
{
  Q_D(qSlicerWorkflowToolWidget);

  return this->WorkflowToolNode;
}


void qSlicerWorkflowToolWidget
::setWorkflowToolNode( vtkMRMLNode* newWorkflowToolNode )
{
  Q_D(qSlicerWorkflowToolWidget);

  d->WorkflowToolComboBox->setCurrentNode( newWorkflowToolNode );
  // If it is a new table node, then the onTransformBufferNodeChanged will be called automatically
}


void qSlicerWorkflowToolWidget
::onWorkflowToolNodeChanged( vtkMRMLNode* newWorkflowToolNode )
{
  Q_D(qSlicerWorkflowToolWidget);

  this->qvtkDisconnectAll();

  this->WorkflowToolNode = vtkMRMLWorkflowToolNode::SafeDownCast( newWorkflowToolNode );

  this->qvtkConnect( this->WorkflowToolNode, vtkCommand::ModifiedEvent, this, SLOT( onWorkflowToolNodeModified() ) );

  this->updateWidget();

  emit WorkflowToolNodeChanged( this->WorkflowToolNode );
}


void qSlicerWorkflowToolWidget
::onWorkflowToolNodeModified()
{
  this->updateWidget();
  emit WorkflowToolNodeModified(); // This should allows parent widgets to update themselves
}


void qSlicerWorkflowToolWidget
::onWorkflowProcedureChanged( vtkMRMLNode* newWorkflowProcedureNode )
{
  this->WorkflowToolNode->SetWorkflowProcedureID( newWorkflowProcedureNode->GetID() );
  this->updateWidget();
}


void qSlicerWorkflowToolWidget
::onWorkflowInputChanged( vtkMRMLNode* newWorkflowInputNode )
{
  this->WorkflowToolNode->SetWorkflowInputID( newWorkflowInputNode->GetID() );
  this->updateWidget();
}


void qSlicerWorkflowToolWidget
::onWorkflowTrainingChanged( vtkMRMLNode* newWorkflowTrainingNode )
{
  this->WorkflowToolNode->SetWorkflowTrainingID( newWorkflowTrainingNode->GetID() );
  this->updateWidget();
}


void qSlicerWorkflowToolWidget
::updateWidgetFromMRML()
{
  Q_D(qSlicerWorkflowToolWidget);

  d->WorkflowToolComboBox->setCurrentNode( this->WorkflowToolNode );
  
  if ( this->WorkflowToolNode == NULL )
  {
    return;
  }
  
  d->WorkflowProcedureComboxBox->setCurrentNode( this->WorkflowToolNode->GetWorkflowProcedureNode() );
  d->WorkflowInputComboxBox->setCurrentNode( this->WorkflowToolNode->GetWorkflowInputNode() );
  d->WorkflowTrainingComboxBox->setCurrentNode( this->WorkflowToolNode->GetWorkflowTrainingNode() );

}
