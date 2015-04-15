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

#include <QtGui>


// Helper functions -------------------------------------------------------------

#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"

// TODO: This should really be a helper function
vtkMRMLAbstractLogic* qSlicerTransformBufferWidget
::GetSlicerModuleLogic( std::string moduleName )
{
  qSlicerAbstractCoreModule* Module = qSlicerApplication::application()->moduleManager()->module( moduleName.c_str() );
  if ( Module != NULL )
  {
    return Module->logic();
  }
  return NULL;
}


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
  this->TransformRecorderLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( qSlicerTransformBufferWidget::GetSlicerModuleLogic( "TransformRecorder" ) );
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
::setTransformBufferNode( vtkMRMLTransformBufferNode* newTransformBufferNode )
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
  
  QString filename = QFileDialog::getOpenFileName( this, tr("Open record"), "", tr("XML Files (*.xml)") );
  
  if ( filename.isEmpty() == false )
  {
    QProgressDialog dialog;
    dialog.setModal( true );
    dialog.setLabelText( "Please wait while reading XML file..." );
    dialog.show();

    // We should create a new buffer node if there isn't one already selected
    vtkSmartPointer< vtkMRMLTransformBufferNode > importBufferNode = this->TransformBufferNode;
    if ( importBufferNode == NULL )
    {
      importBufferNode.TakeReference( vtkMRMLTransformBufferNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLTransformBufferNode" ) ) );
      importBufferNode->SetScene( this->mrmlScene() );
      this->mrmlScene()->AddNode( importBufferNode );
    }
    
    dialog.setValue( 10 );
    this->TransformRecorderLogic->ImportFromXMLFile( importBufferNode, filename.toStdString() );

    // Triggers the buffer node changed signal
    d->BufferNodeComboBox->setCurrentNode( NULL );
    d->BufferNodeComboBox->setCurrentNode( importBufferNode );

    dialog.close();
  }
  
  this->updateWidget();
}


void qSlicerTransformBufferWidget
::onExportButtonClicked()
{
  Q_D(qSlicerTransformBufferWidget);  

  QString filename = QFileDialog::getSaveFileName( this, tr("Save buffer"), "", tr("XML Files (*.xml)") );
  
  if ( ! filename.isEmpty() )
  {
    this->TransformRecorderLogic->ExportToFile( this->TransformBufferNode, filename.toStdString() );
  }

  // No need to update the buffer - it is not changed
  this->updateWidget();
}


void qSlicerTransformBufferWidget
::updateWidget()
{
  Q_D(qSlicerTransformBufferWidget);

  d->BufferNodeComboBox->setCurrentNode( this->TransformBufferNode );
}
