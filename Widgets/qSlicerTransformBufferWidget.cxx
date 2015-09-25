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
#include "qSlicerTransformBufferWidget.h"

#include "qSlicerIOManager.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerTransformBufferWidgetPrivate
  : public Ui_qSlicerTransformBufferWidget
{
  Q_DECLARE_PUBLIC(qSlicerTransformBufferWidget);
protected:
  qSlicerTransformBufferWidget* const q_ptr;

public:
  qSlicerTransformBufferWidgetPrivate( qSlicerTransformBufferWidget& object);
  ~qSlicerTransformBufferWidgetPrivate();
  virtual void setupUi(qSlicerTransformBufferWidget*);
};

// --------------------------------------------------------------------------
qSlicerTransformBufferWidgetPrivate
::qSlicerTransformBufferWidgetPrivate( qSlicerTransformBufferWidget& object) : q_ptr(&object)
{
}

qSlicerTransformBufferWidgetPrivate
::~qSlicerTransformBufferWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerTransformBufferWidgetPrivate
::setupUi(qSlicerTransformBufferWidget* widget)
{
  this->Ui_qSlicerTransformBufferWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerTransformBufferWidget methods

//-----------------------------------------------------------------------------
qSlicerTransformBufferWidget
::qSlicerTransformBufferWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerTransformBufferWidgetPrivate(*this) )
{
  this->TransformBufferNode = NULL;
  this->TransformRecorderLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "TransformRecorder" ) );
  this->setup();
}


qSlicerTransformBufferWidget
::~qSlicerTransformBufferWidget()
{
}


void qSlicerTransformBufferWidget
::setup()
{
  Q_D(qSlicerTransformBufferWidget);

  d->setupUi(this);

  connect( d->BufferNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTransformBufferNodeChanged( vtkMRMLNode* ) ) );

  connect( d->ImportButton, SIGNAL( clicked() ), this, SLOT( onImportButtonClicked() ) );
  connect( d->ExportButton, SIGNAL( clicked() ), this, SLOT( onExportButtonClicked() ) );

  this->updateWidget();  
}

vtkMRMLTransformBufferNode* qSlicerTransformBufferWidget
::getTransformBufferNode()
{
  Q_D(qSlicerTransformBufferWidget);

  return this->TransformBufferNode;
}


void qSlicerTransformBufferWidget
::setTransformBufferNode( vtkMRMLNode* newTransformBufferNode )
{
  Q_D(qSlicerTransformBufferWidget);

  d->BufferNodeComboBox->setCurrentNode( newTransformBufferNode );
  // If it is a new transform buffer, then the onTransformBufferNodeChanged will be called automatically
}


void qSlicerTransformBufferWidget
::onTransformBufferNodeChanged( vtkMRMLNode* newTransformBufferNode )
{
  Q_D(qSlicerTransformBufferWidget);

  this->qvtkDisconnectAll();

  this->TransformBufferNode = vtkMRMLTransformBufferNode::SafeDownCast( newTransformBufferNode );

  this->qvtkConnect( this->TransformBufferNode, vtkCommand::ModifiedEvent, this, SLOT( onTransformBufferNodeModified() ) );

  this->updateWidget();

  emit transformBufferNodeChanged( this->TransformBufferNode );
}


void qSlicerTransformBufferWidget
::onTransformBufferNodeModified()
{
  this->updateWidget();
  emit transformBufferNodeModified(); // This should allows parent widgets to update themselves
}


void qSlicerTransformBufferWidget
::onImportButtonClicked()
{
  Q_D(qSlicerTransformBufferWidget);

  // Use the generic Slicer dialog  
  vtkSmartPointer< vtkCollection > loadedNodes = vtkSmartPointer< vtkCollection >::New();
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ioManager->openDialog( QString( "Transform Buffer" ), qSlicerFileDialog::Read, qSlicerIO::IOProperties(), loadedNodes );

  // Set one of the loaded nodes to be selected in the combo box
  if ( loadedNodes->GetNumberOfItems() > 0 )
  {
	  d->BufferNodeComboBox->setCurrentNode( vtkMRMLNode::SafeDownCast( loadedNodes->GetItemAsObject( 0 ) ) );
  }

  this->updateWidget();
  emit transformBufferNodeChanged( this->TransformBufferNode );
}


void qSlicerTransformBufferWidget
::onExportButtonClicked()
{
  Q_D(qSlicerTransformBufferWidget);  

  if ( this->TransformBufferNode == NULL )
  {
    return;
  }

  // Use the generic Slicer dialog
  qSlicerIO::IOProperties fileParameters;
  fileParameters[ "nodeID" ] = this->TransformBufferNode->GetID();
  
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ioManager->openDialog( QString( "Transform Buffer" ), qSlicerFileDialog::Write, fileParameters );

  // No need to update the buffer - it is not changed
  this->updateWidget();
}


void qSlicerTransformBufferWidget
::updateWidget()
{
  Q_D(qSlicerTransformBufferWidget);

  d->BufferNodeComboBox->setCurrentNode( this->TransformBufferNode );
}
