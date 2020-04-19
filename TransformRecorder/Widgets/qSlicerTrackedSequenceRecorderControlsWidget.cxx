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
#include "qSlicerTrackedSequenceRecorderControlsWidget.h"
#include "vtkMRMLSequenceBrowserNode.h"

#include <QtGui>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerTrackedSequenceRecorderControlsWidgetPrivate
  : public Ui_qSlicerTrackedSequenceRecorderControlsWidget
{
  Q_DECLARE_PUBLIC(qSlicerTrackedSequenceRecorderControlsWidget);
protected:
  qSlicerTrackedSequenceRecorderControlsWidget* const q_ptr;

public:
  qSlicerTrackedSequenceRecorderControlsWidgetPrivate( qSlicerTrackedSequenceRecorderControlsWidget& object);
  ~qSlicerTrackedSequenceRecorderControlsWidgetPrivate();
  virtual void setupUi(qSlicerTrackedSequenceRecorderControlsWidget*);
};

// --------------------------------------------------------------------------
qSlicerTrackedSequenceRecorderControlsWidgetPrivate
::qSlicerTrackedSequenceRecorderControlsWidgetPrivate( qSlicerTrackedSequenceRecorderControlsWidget& object) : q_ptr(&object)
{
}

qSlicerTrackedSequenceRecorderControlsWidgetPrivate
::~qSlicerTrackedSequenceRecorderControlsWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerTrackedSequenceRecorderControlsWidgetPrivate
::setupUi(qSlicerTrackedSequenceRecorderControlsWidget* widget)
{
  this->Ui_qSlicerTrackedSequenceRecorderControlsWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerTrackedSequenceRecorderControlsWidget methods

//-----------------------------------------------------------------------------
qSlicerTrackedSequenceRecorderControlsWidget
::qSlicerTrackedSequenceRecorderControlsWidget(QWidget* parentWidget) : qSlicerWidget( parentWidget ) , d_ptr( new qSlicerTrackedSequenceRecorderControlsWidgetPrivate(*this) )
{
  this->TrackedSequenceBrowserNode = NULL;
  this->SequencesLogic = vtkSlicerSequencesLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "Sequences" ) );
  this->setup();
}


qSlicerTrackedSequenceRecorderControlsWidget
::~qSlicerTrackedSequenceRecorderControlsWidget()
{
}


void qSlicerTrackedSequenceRecorderControlsWidget
::setup()
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  d->setupUi(this);

  connect( d->TransformCheckableComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedTransformsChanged() ) );
  connect( d->ImageCheckableComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedImagesChanged() ) );

  connect( d->StartStopButton, SIGNAL( clicked( bool ) ), this, SLOT( onStartStopButtonClicked( bool ) ) );
  connect( d->ClearButton, SIGNAL( clicked() ), this, SLOT( onClearButtonClicked() ) );

  this->updateWidget();
}


void qSlicerTrackedSequenceRecorderControlsWidget
::setTrackedSequenceBrowserNode( vtkMRMLNode* newTrackedSequenceBrowserNode )
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  this->qvtkDisconnectAll();

  this->TrackedSequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast( newTrackedSequenceBrowserNode );

  this->qvtkConnect( this->TrackedSequenceBrowserNode, vtkCommand::ModifiedEvent, this, SLOT( onTrackedSequenceBrowserProxyNodesChanged() ) );

  this->updateWidget();
}


void qSlicerTrackedSequenceRecorderControlsWidget
::onTrackedSequenceBrowserProxyNodesChanged()
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  if ( this->TrackedSequenceBrowserNode == NULL )
  {
    return;
  }

  // Disable to the onCheckedChanged listener when initializing the selections
  // We don't want to simultaneously update the observed nodes from selections and selections from observed nodes
  disconnect( d->TransformCheckableComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedTransformsChanged() ) );
  disconnect( d->ImageCheckableComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedImagesChanged() ) );

  // Assume the default is not checked, and check all those that are observed
  for ( int i = 0; i < d->TransformCheckableComboBox->nodeCount(); i++ )
  {
    vtkMRMLNode* currProxyNode = d->TransformCheckableComboBox->nodeFromIndex( i );
    vtkMRMLSequenceNode* currSequenceNode = this->TrackedSequenceBrowserNode->GetSequenceNode( currProxyNode );

    if ( currSequenceNode != NULL && this->TrackedSequenceBrowserNode->GetRecording( currSequenceNode ) ) // If the proxy node isn't synced, then this will certainly be false
    {
	    d->TransformCheckableComboBox->setCheckState( currProxyNode, Qt::Checked );
    }
    else
    {
      d->TransformCheckableComboBox->setCheckState( currProxyNode, Qt::Unchecked );
    }
  }

  // Assume the default is not checked, and check all those that are observed
  for ( int i = 0; i < d->ImageCheckableComboBox->nodeCount(); i++ )
  {
    vtkMRMLNode* currProxyNode = d->ImageCheckableComboBox->nodeFromIndex( i );
    vtkMRMLSequenceNode* currSequenceNode = this->TrackedSequenceBrowserNode->GetSequenceNode( currProxyNode );

    if ( currSequenceNode != NULL && this->TrackedSequenceBrowserNode->GetRecording( currSequenceNode ) ) // If the proxy node isn't synced, then this will certainly be false
    {
	    d->ImageCheckableComboBox->setCheckState( currProxyNode, Qt::Checked );
    }
    else
    {
      d->ImageCheckableComboBox->setCheckState( currProxyNode, Qt::Unchecked );
    }
  }

  connect( d->TransformCheckableComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedTransformsChanged() ) );
  connect( d->ImageCheckableComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedImagesChanged() ) );
}


void qSlicerTrackedSequenceRecorderControlsWidget
::onCheckedTransformsChanged()
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  if ( this->TrackedSequenceBrowserNode == NULL )
  {
    return;
  }
  

  // Go through transform types (ie ProbeToReference, StylusTipToReference, etc)
  int modifyFlag = this->TrackedSequenceBrowserNode->StartModify();

  for ( int i = 0; i < d->TransformCheckableComboBox->nodeCount(); i++ )
  {
    vtkMRMLNode* currProxyNode = d->TransformCheckableComboBox->nodeFromIndex( i );
    vtkMRMLSequenceNode* currSequenceNode = this->TrackedSequenceBrowserNode->GetSequenceNode( currProxyNode );
    bool currProxyNodeChecked = d->TransformCheckableComboBox->checkState( currProxyNode ) == Qt::Checked;

    if( currProxyNodeChecked && currSequenceNode == NULL )
    {
      currSequenceNode = this->SequencesLogic->AddSynchronizedNode( NULL, currProxyNode, this->TrackedSequenceBrowserNode );
      this->TrackedSequenceBrowserNode->SetOverwriteProxyName( currSequenceNode, false );
      this->TrackedSequenceBrowserNode->SetSaveChanges( currSequenceNode, false );
    }

    if ( currSequenceNode != NULL )
    {
      this->TrackedSequenceBrowserNode->SetRecording( currSequenceNode, currProxyNodeChecked );
    }
  }

  this->TrackedSequenceBrowserNode->EndModify( modifyFlag );
}


void qSlicerTrackedSequenceRecorderControlsWidget
::onCheckedImagesChanged()
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  if ( this->TrackedSequenceBrowserNode == NULL )
  {
    return;
  }
  

  // Go through images (ie Image_Image, etc)
  int modifyFlag = this->TrackedSequenceBrowserNode->StartModify();

  for ( int i = 0; i < d->ImageCheckableComboBox->nodeCount(); i++ )
  {
    vtkMRMLNode* currProxyNode = d->ImageCheckableComboBox->nodeFromIndex( i );
    vtkMRMLSequenceNode* currSequenceNode = this->TrackedSequenceBrowserNode->GetSequenceNode( currProxyNode );
    bool currProxyNodeChecked = d->ImageCheckableComboBox->checkState( currProxyNode ) == Qt::Checked;

    if( currProxyNodeChecked && currSequenceNode == NULL )
    {
      currSequenceNode = this->SequencesLogic->AddSynchronizedNode( NULL, currProxyNode, this->TrackedSequenceBrowserNode );
      this->TrackedSequenceBrowserNode->SetOverwriteProxyName( currSequenceNode, false );
      this->TrackedSequenceBrowserNode->SetSaveChanges( currSequenceNode, false );
    }

    if ( currSequenceNode != NULL )
    {
      this->TrackedSequenceBrowserNode->SetRecording( currSequenceNode, currProxyNodeChecked );
    }
  }


  this->TrackedSequenceBrowserNode->EndModify( modifyFlag );
}



void qSlicerTrackedSequenceRecorderControlsWidget
::onStartStopButtonClicked( bool state )
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  if ( this->TrackedSequenceBrowserNode == NULL )
  {
    return;
  }
  
  // If recording is started
  this->TrackedSequenceBrowserNode->SetRecordingActive( state );
  if ( state == true )
  {
    d->StatusResultLabel->setText( "Recording" );
    d->StartStopButton->setText( "Stop" );
  }
  else
  {
    d->StatusResultLabel->setText( "Stopped" );
    d->StartStopButton->setText( "Start" );
  }
  
  this->updateWidget();
}



void qSlicerTrackedSequenceRecorderControlsWidget
::onClearButtonClicked()
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  if ( this->TrackedSequenceBrowserNode == NULL )
  {
    return;
  }

  this->TrackedSequenceBrowserNode->RemoveAllSequenceNodes();
  
  this->updateWidget();
}


void qSlicerTrackedSequenceRecorderControlsWidget
::updateWidget()
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  if ( this->TrackedSequenceBrowserNode == NULL )
  {
    return;
  }

  // Set the text indicating recording
  d->StartStopButton->setChecked( this->TrackedSequenceBrowserNode->GetRecordingActive() );
  if ( this->TrackedSequenceBrowserNode->GetRecordingActive())
  {
    d->StatusResultLabel->setText( "Recording" );
    d->StartStopButton->setText( "Stop" );
  }
  else
  {
    d->StatusResultLabel->setText( "Stopped" );
    d->StartStopButton->setText( "Start" );
  }

  this->onTrackedSequenceBrowserProxyNodesChanged();
}
