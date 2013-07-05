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
}


qSlicerTransformBufferWidget
::~qSlicerTransformBufferWidget()
{
}


qSlicerTransformBufferWidget* qSlicerTransformBufferWidget
::New( vtkSlicerTransformRecorderLogic* newTransformRecorderLogic )
{
  qSlicerTransformBufferWidget* newTransformBufferWidget = new qSlicerTransformBufferWidget();
  newTransformBufferWidget->SetLogic( newTransformRecorderLogic );
  newTransformBufferWidget->setup();
  return newTransformBufferWidget;
}


void qSlicerTransformBufferWidget
::SetLogic( vtkSlicerTransformRecorderLogic* newTransformRecorderLogic )
{
  this->TransformRecorderLogic = newTransformRecorderLogic;
}


vtkSlicerTransformRecorderLogic* qSlicerTransformBufferWidget
::GetLogic()
{
  return this->TransformRecorderLogic;
}


vtkMRMLTransformBufferNode* qSlicerTransformBufferWidget
::GetBufferNode()
{
  return d->BufferComboBox->currentNode();
}


void qSlicerTransformBufferWidget
::setup()
{
  Q_D(qSlicerTransformBufferWidget);

  d->setupUi(this);

  connect( d->ImportButton, SIGNAL( clicked() ), this, SLOT( onImportButtonClicked() ) );
  connect( d->SaveButton, SIGNAL( clicked() ), this, SLOT( onSaveButtonClicked() ) );

  // GUI refresh: updates every 10ms
  QTimer *t = new QTimer( this );
  connect( t,  SIGNAL( timeout() ), this, SLOT( updateWidget() ) );
  t->start(10); 

  this->updateWidget();  
}


void qSlicerTransformBufferWidget
::enter()
{
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
    
    dialog.setValue( 10 );
    this->TransformRecorderLogic->ImportFile( this->GetBufferNode(), filename.toStdString() );

    dialog.close();
  }
  
  this->updateWidget();
}


void qSlicerTransformBufferWidget
::onSaveButtonClicked()
{
  Q_D(qSlicerTransformBufferWidget);  

  QString filename = QFileDialog::getSaveFileName( this, tr("Save buffer"), "", tr("XML Files (*.xml)") );
  
  if ( ! filename.isEmpty() )
  {
    this->TransformRecorderLogic->SaveToFile( this->GetBufferNode(), filename.toStdString() );
  }
  
  this->updateWidget();
}


void qSlicerTransformBufferWidget
::updateWidget()
{
  Q_D(qSlicerTransformBufferWidget);

  if ( this->TransformRecorderLogic->GetRecording( this->GetBufferNode() ) )
  {
    d->StatusResultLabel->setText( "Recording" );
  }
  else
  {
    d->StatusResultLabel->setText( "Waiting" );
  }

}
