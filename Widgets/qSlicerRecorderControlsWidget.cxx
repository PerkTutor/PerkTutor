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
::qSlicerRecorderControlsWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerRecorderControlsWidgetPrivate(*this) )
{
}


qSlicerRecorderControlsWidget
::~qSlicerRecorderControlsWidget()
{
}


qSlicerRecorderControlsWidget* qSlicerRecorderControlsWidget
::New( vtkSlicerTransformRecorderLogic* newTRLogic )
{
  qSlicerRecorderControlsWidget* newRecorderControlsWidget = new qSlicerRecorderControlsWidget();
  newRecorderControlsWidget->SetLogic( newTRLogic );
  newRecorderControlsWidget->setup();
  return newRecorderControlsWidget;
}


void qSlicerRecorderControlsWidget
::SetLogic( vtkSlicerTransformRecorderLogic* newTRLogic )
{
  this->trLogic = newTRLogic;
}


void qSlicerRecorderControlsWidget
::setup()
{
  Q_D(qSlicerRecorderControlsWidget);

  d->setupUi(this);

  connect( d->StartButton, SIGNAL( clicked() ), this, SLOT( onStartButtonClicked() ) );
  connect( d->StopButton, SIGNAL( clicked() ), this, SLOT( onStopButtonClicked() ) ); 
  connect( d->SaveButton, SIGNAL( clicked() ), this, SLOT( onSaveButtonClicked() ) );
  connect( d->ClearButton, SIGNAL( clicked() ), this, SLOT( onClearButtonClicked() ) );

  // GUI refresh: updates every 10ms
  QTimer *t = new QTimer( this );
  connect( t,  SIGNAL( timeout() ), this, SLOT( updateWidget() ) );
  t->start(10); 

  this->updateWidget();  
}


void qSlicerRecorderControlsWidget
::enter()
{
}


void qSlicerRecorderControlsWidget
::onStartButtonClicked()
{
  Q_D(qSlicerRecorderControlsWidget);  
  
  // The observed transforms should be dealt with in the TransformRecorder widget
  this->trLogic->SetRecording( true );
  
  this->updateWidget();
}


void qSlicerRecorderControlsWidget
::onStopButtonClicked()
{
  Q_D(qSlicerRecorderControlsWidget);  

  this->trLogic->SetRecording( false );
  
  this->updateWidget();
}


void qSlicerRecorderControlsWidget
::onSaveButtonClicked()
{
  Q_D(qSlicerRecorderControlsWidget);  

  QString filename = QFileDialog::getSaveFileName( this, tr("Save buffer"), "", tr("XML Files (*.xml)") );
  
  if ( ! filename.isEmpty() )
  {
    trLogic->SaveToFile( filename.toStdString() );
  }
  
  this->updateWidget();
}



void qSlicerRecorderControlsWidget
::onClearButtonClicked()
{
  Q_D(qSlicerRecorderControlsWidget);

  this->trLogic->ClearBuffer();
  
  this->updateWidget();
}


void qSlicerRecorderControlsWidget
::updateWidget()
{
  Q_D(qSlicerRecorderControlsWidget);

  if ( trLogic->GetRecording() )
  {
    d->StatusResultLabel->setText( "Recording" );
  }
  else
  {
    d->StatusResultLabel->setText( "Waiting" );
  }

}
