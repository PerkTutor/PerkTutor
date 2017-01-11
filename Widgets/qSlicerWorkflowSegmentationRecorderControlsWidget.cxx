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
#include "qSlicerWorkflowSegmentationRecorderControlsWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate
  : public Ui_qSlicerTrackedSequenceRecorderControlsWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkflowSegmentationRecorderControlsWidget);
protected:
  qSlicerWorkflowSegmentationRecorderControlsWidget* const q_ptr;

public:
  qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate( qSlicerWorkflowSegmentationRecorderControlsWidget& object);
  ~qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate();
  virtual void setupUi(qSlicerWorkflowSegmentationRecorderControlsWidget*);
};

// --------------------------------------------------------------------------
qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate
::qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate( qSlicerWorkflowSegmentationRecorderControlsWidget& object) : q_ptr(&object)
{
}

qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate
::~qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate
::setupUi(qSlicerWorkflowSegmentationRecorderControlsWidget* widget)
{
  this->Ui_qSlicerTrackedSequenceRecorderControlsWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWorkflowSegmentationRecorderControlsWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkflowSegmentationRecorderControlsWidget
::qSlicerWorkflowSegmentationRecorderControlsWidget(QWidget* parentWidget) : qSlicerTrackedSequenceRecorderControlsWidget( parentWidget ) , d_ptr( new qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate(*this) )
{
  this->WorkflowSegmentationNode = NULL;
  this->WorkflowSegmentationLogic = vtkSlicerWorkflowSegmentationLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "WorkflowSegmentation" ) );
}


qSlicerWorkflowSegmentationRecorderControlsWidget
::~qSlicerWorkflowSegmentationRecorderControlsWidget()
{
}


void qSlicerWorkflowSegmentationRecorderControlsWidget
::setWorkflowSegmentationNode( vtkMRMLNode* newWorkflowSegmentationNode )
{
  Q_D(qSlicerWorkflowSegmentationRecorderControlsWidget);  

  this->qvtkDisconnectAll();

  this->WorkflowSegmentationNode = vtkMRMLWorkflowSegmentationNode::SafeDownCast( newWorkflowSegmentationNode );

  this->qvtkConnect( this->WorkflowSegmentationNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateWidget();
}


void qSlicerWorkflowSegmentationRecorderControlsWidget
::onStartStopButtonClicked( bool state )
{
  Q_D(qSlicerWorkflowSegmentationRecorderControlsWidget);

  this->qSlicerTrackedSequenceRecorderControlsWidget::onStartStopButtonClicked( state );

  if ( this->WorkflowSegmentationNode == NULL )
  {
    return;
  }

  this->WorkflowSegmentationNode->SetRealTimeProcessing( state );
}
