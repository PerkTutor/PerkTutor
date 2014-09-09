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
  this->BufferHelper = new qSlicerTransformBufferWidgetHelper();
  this->TransformRecorderLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( qSlicerTransformBufferWidgetHelper::GetSlicerModuleLogic( "TransformRecorder" ) );
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

  // Listen for updates from the helper
  connect( this->BufferHelper, SIGNAL( transformBufferNodeChanged( vtkMRMLTransformBufferNode* ) ), this, SLOT( updateWidget() ) );
  connect( this->BufferHelper, SIGNAL( transformBufferNodeModified() ), this, SLOT( onTransformBufferNodeModified() ) );

  this->updateWidget();  
}


void qSlicerTransformBufferWidget
::onTransformBufferNodeChanged( vtkMRMLNode* newTransformBufferNode )
{
  Q_D(qSlicerTransformBufferWidget);

  this->BufferHelper->SetTransformBufferNode( vtkMRMLTransformBufferNode::SafeDownCast( newTransformBufferNode ) ); // Update widget taken care of already
  emit transformBufferNodeChanged( this->BufferHelper->GetTransformBufferNode() );
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
    vtkSmartPointer< vtkMRMLTransformBufferNode > importBufferNode = this->BufferHelper->GetTransformBufferNode();
    if ( importBufferNode == NULL )
    {
      importBufferNode.TakeReference( vtkMRMLTransformBufferNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLTransformBufferNode" ) ) );
      importBufferNode->SetScene( this->mrmlScene() );
      this->mrmlScene()->AddNode( importBufferNode );
    }
    
    dialog.setValue( 10 );
    this->TransformRecorderLogic->ImportFromFile( importBufferNode, filename.toStdString() );

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
    this->TransformRecorderLogic->ExportToFile( this->BufferHelper->GetTransformBufferNode(), filename.toStdString() );
  }

  // No need to update the buffer - it is not changed
  this->updateWidget();
}


void qSlicerTransformBufferWidget
::updateWidget()
{
  Q_D(qSlicerTransformBufferWidget);

  d->BufferNodeComboBox->setCurrentNode( this->BufferHelper->GetTransformBufferNode() );
}
