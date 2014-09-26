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
#include "qSlicerRecorderControlsWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerRecorderControlsWidgetPrivate
  : public Ui_qSlicerRecorderControlsWidget
{
  Q_DECLARE_PUBLIC(qSlicerRecorderControlsWidget);
protected:
  qSlicerRecorderControlsWidget* const q_ptr;

public:
  qSlicerRecorderControlsWidgetPrivate( qSlicerRecorderControlsWidget& object);
  ~qSlicerRecorderControlsWidgetPrivate();
  virtual void setupUi(qSlicerRecorderControlsWidget*);
};

// --------------------------------------------------------------------------
qSlicerRecorderControlsWidgetPrivate
::qSlicerRecorderControlsWidgetPrivate( qSlicerRecorderControlsWidget& object) : q_ptr(&object)
{
}

qSlicerRecorderControlsWidgetPrivate
::~qSlicerRecorderControlsWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerRecorderControlsWidgetPrivate
::setupUi(qSlicerRecorderControlsWidget* widget)
{
  this->Ui_qSlicerRecorderControlsWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerRecorderControlsWidget methods

//-----------------------------------------------------------------------------
qSlicerRecorderControlsWidget
::qSlicerRecorderControlsWidget(QWidget* parentWidget) : qSlicerWidget( parentWidget ) , d_ptr( new qSlicerRecorderControlsWidgetPrivate(*this) )
{
  this->BufferHelper = new qSlicerTransformBufferWidgetHelper();
  this->TransformRecorderLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( qSlicerTransformBufferWidgetHelper::GetSlicerModuleLogic( "TransformRecorder" ) );
  this->setup();
}


qSlicerRecorderControlsWidget
::~qSlicerRecorderControlsWidget()
{
}


void qSlicerRecorderControlsWidget
::setup()
{
  Q_D(qSlicerRecorderControlsWidget);

  d->setupUi(this);

  connect( d->TransformCheckableComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedTransformsChanged() ) );

  connect( d->StartButton, SIGNAL( clicked() ), this, SLOT( onStartButtonClicked() ) );
  connect( d->StopButton, SIGNAL( clicked() ), this, SLOT( onStopButtonClicked() ) );
  connect( d->ClearButton, SIGNAL( clicked() ), this, SLOT( onClearButtonClicked() ) );

  // Listen for updates from the helper
  connect( this->BufferHelper, SIGNAL( transformBufferNodeChanged( vtkMRMLTransformBufferNode* ) ), this, SLOT( onTransformBufferActiveTransformsChanged() ) );
  connect( this->BufferHelper, SIGNAL( transformBufferActiveTransformAdded() ), this, SLOT( onTransformBufferActiveTransformsChanged() ) );
  connect( this->BufferHelper, SIGNAL( transformBufferActiveTransformRemoved() ), this, SLOT( onTransformBufferActiveTransformsChanged() ) );

  this->updateWidget();
}


void qSlicerRecorderControlsWidget
::onTransformBufferActiveTransformsChanged()
{
  this->updateWidget();
  this->onCheckedTransformsChanged(); // TODO: This is just a hack until the observed transforms are synced with the active transforms
}


void qSlicerRecorderControlsWidget
::onActiveTransformsUpdated()
{
  Q_D(qSlicerRecorderControlsWidget);

  // Disable to the onCheckedChanged listener when initializing the selections
  // We don't want to simultaneously update the observed nodes from selections and selections from observed nodes
  disconnect( d->TransformCheckableComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onCheckedTransformsChanged() ) );

  // Assume the default is not checked, and check all those that are observed
  for ( int i = 0; i < d->TransformCheckableComboBox->nodeCount(); i++ )
  {
    if ( this->TransformRecorderLogic->IsObservedTransformNode( this->BufferHelper->GetTransformBufferNode(), d->TransformCheckableComboBox->nodeFromIndex(i) ) )
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


void qSlicerRecorderControlsWidget
::onCheckedTransformsChanged()
{
  Q_D(qSlicerRecorderControlsWidget);
    
  // Go through transform types (ie ProbeToReference, StylusTipToReference, etc)  
  for ( int i = 0; i < d->TransformCheckableComboBox->nodeCount(); i++ )
  {
    if( d->TransformCheckableComboBox->checkState( d->TransformCheckableComboBox->nodeFromIndex(i) ) == Qt::Checked  )
    {
      this->TransformRecorderLogic->AddObservedTransformNode( this->BufferHelper->GetTransformBufferNode(), d->TransformCheckableComboBox->nodeFromIndex(i) );
    }
    else
    {
      this->TransformRecorderLogic->RemoveObservedTransformNode( this->BufferHelper->GetTransformBufferNode(), d->TransformCheckableComboBox->nodeFromIndex(i) );
    }
  }

}



void qSlicerRecorderControlsWidget
::onStartButtonClicked()
{
  Q_D(qSlicerRecorderControlsWidget);  
  
  // The observed transforms should be dealt with in the TransformRecorder logic
  this->TransformRecorderLogic->SetRecording( this->BufferHelper->GetTransformBufferNode(), true );
  d->StatusResultLabel->setText( "Recording" );
  
  this->updateWidget();
}


void qSlicerRecorderControlsWidget
::onStopButtonClicked()
{
  Q_D(qSlicerRecorderControlsWidget);  

  this->TransformRecorderLogic->SetRecording( this->BufferHelper->GetTransformBufferNode(), false );
  d->StatusResultLabel->setText( "Waiting" );
  
  this->updateWidget();
}


void qSlicerRecorderControlsWidget
::onClearButtonClicked()
{
  Q_D(qSlicerRecorderControlsWidget);

  this->TransformRecorderLogic->ClearTransforms( this->BufferHelper->GetTransformBufferNode() );
  
  this->updateWidget();
}


void qSlicerRecorderControlsWidget
::updateWidget()
{
  Q_D(qSlicerRecorderControlsWidget);

  // Set the text indicating recording
  if ( this->TransformRecorderLogic->GetRecording( this->BufferHelper->GetTransformBufferNode() ) )
  {
    d->StatusResultLabel->setText( "Recording" );
  }
  else
  {
    d->StatusResultLabel->setText( "Waiting" );
  }

  this->onActiveTransformsUpdated();
}
