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
  this->TrackedSequenceBrowserNode = NULL;
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

  connect( d->TrackedSequenceBrowserNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTrackedSequenceBrowserNodeChanged( vtkMRMLNode* ) ) );

  connect( d->ImportButton, SIGNAL( clicked() ), this, SLOT( onImportButtonClicked() ) );
  d->ImportButton->setIcon( QApplication::style()->standardIcon( QStyle::SP_DialogOpenButton ) );
  connect( d->ExportButton, SIGNAL( clicked() ), this, SLOT( onExportButtonClicked() ) );
  d->ExportButton->setIcon( QApplication::style()->standardIcon( QStyle::SP_DialogSaveButton ) );

  this->updateWidget();  
}

vtkMRMLSequenceBrowserNode* qSlicerTrackedSequenceBrowserWidget
::getTrackedSequenceBrowserNode()
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);

  return this->TrackedSequenceBrowserNode;
}


void qSlicerTrackedSequenceBrowserWidget
::setTrackedSequenceBrowserNode( vtkMRMLNode* newTrackedSequenceBrowserNode )
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);

  d->TrackedSequenceBrowserNodeComboBox->setCurrentNode( newTrackedSequenceBrowserNode );
  // If it is a new sequence browser node, then the onTrackedSequenceBrowserNodeChanged will be called automatically
}


void qSlicerTrackedSequenceBrowserWidget
::onTrackedSequenceBrowserNodeChanged( vtkMRMLNode* newTrackedSequenceBrowserNode )
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);

  this->qvtkDisconnectAll();

  this->TrackedSequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast( newTrackedSequenceBrowserNode );

  this->qvtkConnect( this->TrackedSequenceBrowserNode, vtkCommand::ModifiedEvent, this, SLOT( onTrackedSequenceBrowserNodeModified() ) );

  this->updateWidget();

  emit trackedSequenceBrowserNodeChanged( this->TrackedSequenceBrowserNode );
}


void qSlicerTrackedSequenceBrowserWidget
::onTrackedSequenceBrowserNodeModified()
{
  this->updateWidget();
  emit trackedSequenceBrowserNodeModified(); // This should allows parent widgets to update themselves
}


void qSlicerTrackedSequenceBrowserWidget
::onImportButtonClicked()
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);

  // Use the generic Slicer dialog  
  vtkSmartPointer< vtkCollection > loadedNodes = vtkSmartPointer< vtkCollection >::New();
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ioManager->openDialog( QString( "Sequence Metafile" ), qSlicerFileDialog::Read, qSlicerIO::IOProperties(), loadedNodes );

  // Set one of the loaded nodes to be selected in the combo box
  if ( loadedNodes->GetNumberOfItems() > 0 )
  {
	  d->TrackedSequenceBrowserNodeComboBox->setCurrentNode( vtkMRMLNode::SafeDownCast( loadedNodes->GetItemAsObject( 0 ) ) );
  }

  this->updateWidget();
  emit trackedSequenceBrowserNodeChanged( this->TrackedSequenceBrowserNode );
}


void qSlicerTrackedSequenceBrowserWidget
::onExportButtonClicked()
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);  

  if ( this->TrackedSequenceBrowserNode == NULL )
  {
    return;
  }

  // Use the generic Slicer dialog
  qSlicerIO::IOProperties fileParameters;
  fileParameters[ "nodeID" ] = this->TrackedSequenceBrowserNode->GetID();
  
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ioManager->openDialog( QString( "Sequence Metafile" ), qSlicerFileDialog::Write, fileParameters );

  // No need to update the buffer - it is not changed
  this->updateWidget();
}


void qSlicerTrackedSequenceBrowserWidget
::updateWidget()
{
  Q_D(qSlicerTrackedSequenceBrowserWidget);

  d->TrackedSequenceBrowserNodeComboBox->setCurrentNode( this->TrackedSequenceBrowserNode );
}
