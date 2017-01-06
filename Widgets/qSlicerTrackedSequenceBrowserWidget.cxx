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
#include "qSlicerTrackedSequenceBrowserWidget.h"

#include "qSlicerIOManager.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerTrackedSequenceBrowserWidgetPrivate
  : public Ui_qSlicerTrackedSequenceBrowserWidget
{
  Q_DECLARE_PUBLIC(qSlicerTrackedSequenceBrowserWidget);
protected:
  qSlicerTrackedSequenceBrowserWidget* const q_ptr;

public:
  qSlicerTrackedSequenceBrowserWidgetPrivate( qSlicerTrackedSequenceBrowserWidget& object);
  ~qSlicerTrackedSequenceBrowserWidgetPrivate();
  virtual void setupUi(qSlicerTrackedSequenceBrowserWidget*);
};

// --------------------------------------------------------------------------
qSlicerTrackedSequenceBrowserWidgetPrivate
::qSlicerTrackedSequenceBrowserWidgetPrivate( qSlicerTrackedSequenceBrowserWidget& object) : q_ptr(&object)
{
}

qSlicerTrackedSequenceBrowserWidgetPrivate
::~qSlicerTrackedSequenceBrowserWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerTrackedSequenceBrowserWidgetPrivate
::setupUi(qSlicerTrackedSequenceBrowserWidget* widget)
{
  this->Ui_qSlicerTrackedSequenceBrowserWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerTrackedSequenceBrowserWidget methods

//-----------------------------------------------------------------------------
qSlicerTrackedSequenceBrowserWidget
::qSlicerTrackedSequenceBrowserWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerTrackedSequenceBrowserWidgetPrivate(*this) )
{
  this->TransformBufferNode = NULL;
  this->TransformRecorderLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "TransformRecorder" ) );
  this->setup();
}


qSlicerTrackedSequenceBrowserWidget
::~qSlicerTrackedSequenceBrowserWidget()
{
}


void qSlicerTrackedSequenceBrowserWidget
::setup()
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);

  d->setupUi(this);

  connect( d->BufferNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTransformBufferNodeChanged( vtkMRMLNode* ) ) );

  connect( d->ImportButton, SIGNAL( clicked() ), this, SLOT( onImportButtonClicked() ) );
  d->ImportButton->setIcon( QApplication::style()->standardIcon( QStyle::SP_DialogOpenButton ) );
  connect( d->ExportButton, SIGNAL( clicked() ), this, SLOT( onExportButtonClicked() ) );
  d->ExportButton->setIcon( QApplication::style()->standardIcon( QStyle::SP_DialogSaveButton ) );

  this->updateWidget();  
}

vtkMRMLTransformBufferNode* qSlicerTrackedSequenceBrowserWidget
::getTransformBufferNode()
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);

  return this->TransformBufferNode;
}


void qSlicerTrackedSequenceBrowserWidget
::setTransformBufferNode( vtkMRMLNode* newTransformBufferNode )
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);

  d->BufferNodeComboBox->setCurrentNode( newTransformBufferNode );
  // If it is a new transform buffer, then the onTransformBufferNodeChanged will be called automatically
}


void qSlicerTrackedSequenceBrowserWidget
::onTransformBufferNodeChanged( vtkMRMLNode* newTransformBufferNode )
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);

  this->qvtkDisconnectAll();

  this->TransformBufferNode = vtkMRMLTransformBufferNode::SafeDownCast( newTransformBufferNode );

  this->qvtkConnect( this->TransformBufferNode, vtkCommand::ModifiedEvent, this, SLOT( onTransformBufferNodeModified() ) );

  this->updateWidget();

  emit transformBufferNodeChanged( this->TransformBufferNode );
}


void qSlicerTrackedSequenceBrowserWidget
::onTransformBufferNodeModified()
{
  this->updateWidget();
  emit transformBufferNodeModified(); // This should allows parent widgets to update themselves
}


void qSlicerTrackedSequenceBrowserWidget
::onImportButtonClicked()
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);

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


void qSlicerTrackedSequenceBrowserWidget
::onExportButtonClicked()
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);  

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


void qSlicerTrackedSequenceBrowserWidget
::updateWidget()
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);

  d->BufferNodeComboBox->setCurrentNode( this->TransformBufferNode );
}
