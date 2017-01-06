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
  this->TransformBufferNode = NULL;
  this->TransformRecorderLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "TransformRecorder" ) );
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

  connect( d->StartStopButton, SIGNAL( clicked( bool ) ), this, SLOT( onStartStopButtonClicked( bool ) ) );
  connect( d->ClearButton, SIGNAL( clicked() ), this, SLOT( onClearButtonClicked() ) );

  this->updateWidget();
}


void qSlicerTrackedSequenceRecorderControlsWidget
::setTransformBufferNode( vtkMRMLNode* newTransformBufferNode )
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  this->qvtkDisconnectAll();

  this->TransformBufferNode = vtkMRMLTransformBufferNode::SafeDownCast( newTransformBufferNode );

  this->qvtkConnect( this->TransformBufferNode, vtkMRMLTransformBufferNode::ActiveTransformAddedEvent, this, SLOT( onTransformBufferActiveTransformsChanged() ) );
  this->qvtkConnect( this->TransformBufferNode, vtkMRMLTransformBufferNode::ActiveTransformRemovedEvent, this, SLOT( onTransformBufferActiveTransformsChanged() ) );
  this->qvtkConnect( this->TransformBufferNode, vtkMRMLTransformBufferNode::RecordingStateChangedEvent, this, SLOT( updateWidget() ) );

  this->updateWidget();
}


void qSlicerTrackedSequenceRecorderControlsWidget
::onTransformBufferActiveTransformsChanged()
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  // Disable to the onCheckedChanged listener when initializing the selections
  // We don't want to simultaneously update the observed nodes from selections and selections from observed nodes
  disconnect( d->TransformCheckableComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedTransformsChanged() ) );

  // Assume the default is not checked, and check all those that are observed
  for ( int i = 0; i < d->TransformCheckableComboBox->nodeCount(); i++ )
  {
    if ( this->TransformBufferNode != NULL && this->TransformBufferNode->IsActiveTransformID( d->TransformCheckableComboBox->nodeFromIndex(i)->GetID() ) )
    {
	    d->TransformCheckableComboBox->setCheckState( d->TransformCheckableComboBox->nodeFromIndex(i), Qt::Checked );
    }
    else
    {
      d->TransformCheckableComboBox->setCheckState( d->TransformCheckableComboBox->nodeFromIndex(i), Qt::Unchecked );
    }
  }

  connect( d->TransformCheckableComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedTransformsChanged() ) );
}


void qSlicerTrackedSequenceRecorderControlsWidget
::onCheckedTransformsChanged()
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  if ( this->TransformBufferNode == NULL )
  {
    return;
  }
    
  // Go through transform types (ie ProbeToReference, StylusTipToReference, etc)
  std::vector< std::string > activeTransformIDs;
  for ( int i = 0; i < d->TransformCheckableComboBox->nodeCount(); i++ )
  {
    if( d->TransformCheckableComboBox->checkState( d->TransformCheckableComboBox->nodeFromIndex(i) ) == Qt::Checked  )
    {
      activeTransformIDs.push_back( d->TransformCheckableComboBox->nodeFromIndex(i)->GetID() );
    }
  }

  this->TransformBufferNode->SetActiveTransformIDs( activeTransformIDs );
}



void qSlicerTrackedSequenceRecorderControlsWidget
::onStartStopButtonClicked( bool state )
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  if ( this->TransformBufferNode == NULL )
  {
    return;
  }
  
  // If recording is started
  if ( state == true )
  {
    this->TransformBufferNode->StartRecording();
    d->StatusResultLabel->setText( "Recording" );
    d->StartStopButton->setText( "Stop" );
  }
  else
  {
    this->TransformBufferNode->StopRecording();
    d->StatusResultLabel->setText( "Stopped" );
    d->StartStopButton->setText( "Start" );
  }
  
  this->updateWidget();
}



void qSlicerTrackedSequenceRecorderControlsWidget
::onClearButtonClicked()
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  if ( this->TransformBufferNode == NULL )
  {
    return;
  }

  this->TransformRecorderLogic->ClearTransforms( this->TransformBufferNode );
  
  this->updateWidget();
}


void qSlicerTrackedSequenceRecorderControlsWidget
::updateWidget()
{
  Q_D(qSlicerTrackedSequenceRecorderControlsWidget);

  if ( this->TransformBufferNode == NULL )
  {
    return;
  }

  // Set the text indicating recording
  if ( this->TransformBufferNode->GetRecording() )
  {
    d->StatusResultLabel->setText( "Recording" );
    d->StartStopButton->setChecked( true );
    d->StartStopButton->setText( "Stop" );
  }
  else
  {
    d->StatusResultLabel->setText( "Stopped" );
    d->StartStopButton->setChecked( false );
    d->StartStopButton->setText( "Start" );
  }

  this->onTransformBufferActiveTransformsChanged();
}
