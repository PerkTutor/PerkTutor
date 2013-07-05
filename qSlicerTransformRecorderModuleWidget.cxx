

/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/
#include <iostream>
// Qt includes

// SlicerQt includes
#include "qSlicerTransformRecorderModuleWidget.h"
#include "ui_qSlicerTransformRecorderModule.h"
#include "qSlicerIO.h"
#include "qSlicerIOManager.h"
#include "qSlicerApplication.h"
#include <QtGui>

// MRMLWidgets includes
#include <qMRMLUtils.h>

#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkMRMLTransformRecorderNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "qMRMLNodeComboBox.h"
#include "vtkMRMLViewNode.h"
#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkMRMLTransformRecorderNode.h"
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TransformRecorder
class qSlicerTransformRecorderModuleWidgetPrivate: public Ui_qSlicerTransformRecorderModule
{
  Q_DECLARE_PUBLIC( qSlicerTransformRecorderModuleWidget ); 

protected:
  qSlicerTransformRecorderModuleWidget* const q_ptr;
public:
  qSlicerTransformRecorderModuleWidgetPrivate( qSlicerTransformRecorderModuleWidget& object );
  ~qSlicerTransformRecorderModuleWidgetPrivate();

  vtkSlicerTransformRecorderLogic* logic() const;

  // Add embedded widgets here
  qSlicerTransformBufferWidget* BufferWidget;
};

//-----------------------------------------------------------------------------
// qSlicerTransformRecorderModuleWidgetPrivate methods



qSlicerTransformRecorderModuleWidgetPrivate::qSlicerTransformRecorderModuleWidgetPrivate( qSlicerTransformRecorderModuleWidget& object ) : q_ptr(&object)
{
  // Initialize embedded widgets here
  this->TransformBufferWidget = qSlicerTransformBufferWidget::New( this->logic() );
  this->RecorderControlsWidget = qSlicerRecorderControlsWidget::New( this->logic() );
  this->MessagesWidget = qSlicerMessagesWidget::New( this->logic() )
}

//-----------------------------------------------------------------------------

qSlicerTransformRecorderModuleWidgetPrivate::~qSlicerTransformRecorderModuleWidgetPrivate()
{
}


vtkSlicerTransformRecorderLogic* qSlicerTransformRecorderModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerTransformRecorderModuleWidget );
  return vtkSlicerTransformRecorderLogic::SafeDownCast( q->logic() );
}


//-----------------------------------------------------------------------------
// qSlicerTransformRecorderModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTransformRecorderModuleWidget::qSlicerTransformRecorderModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerTransformRecorderModuleWidgetPrivate( *this ) )
{
}



qSlicerTransformRecorderModuleWidget::~qSlicerTransformRecorderModuleWidget()
{
}



void qSlicerTransformRecorderModuleWidget::setup()
{
  Q_D(qSlicerTransformRecorderModuleWidget);

  d->TotalTimeLabel=NULL;
  d->NumTransformsLabel=NULL;
  d->NumTransformsLabel=NULL;

  d->setupUi(this);
  // Embed widgets here
  d->BufferGroupBox->layout()->addWidget( d->BufferWidget );
  d->ControlsGroupBox->layout()->addWidget( d->RecorderControlsWidget );
  d->MessagesGroupBox->layout()->addWidget( d->MessagesWidget ); 
  this->Superclass::setup();
  
  // GUI refresh: updates every 10ms
  QTimer *t = new QTimer( this );
  connect( t,  SIGNAL( timeout() ), this, SLOT( updateWidget() ) );
  t->start(10);

}


void qSlicerTransformRecorderModuleWidget::enter()
{
  this->Superclass::enter();
  this->updateWidget();
}



void qSlicerTransformRecorderModuleWidget
::updateWidget()
{
  Q_D( qSlicerTransformRecorderModuleWidget );
  
  ss.str( "" );
  ss.precision( 2 );
  ss << std::fixed << d->BufferWidget->GetBufferNode()->GetTotalTime();
  d->TotalTimeResultLabel->setText( ss.str().c_str() );
  
  ss.str( "" );
  ss.precision( 2 );
  ss << std::fixed << d->BufferWidget->GetBufferNode()->GetNumTransforms();;
  d->NumTransformsResultLabel->setText( ss.str().c_str() );
  
  ss.str( "" );
  ss.precision( 2 );
  ss << std::fixed << d->BufferWidget->GetBufferNode()->GetNumMessages();;
  d->NumMessagesResultLabel->setText( ss.str().c_str() );
   
}
