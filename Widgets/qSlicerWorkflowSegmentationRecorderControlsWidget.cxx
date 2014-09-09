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
  : public Ui_qSlicerRecorderControlsWidget
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
  this->Ui_qSlicerRecorderControlsWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWorkflowSegmentationRecorderControlsWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkflowSegmentationRecorderControlsWidget
::qSlicerWorkflowSegmentationRecorderControlsWidget(QWidget* parentWidget) : qSlicerRecorderControlsWidget( parentWidget ) , d_ptr( new qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate(*this) )
{
  this->WorkflowSegmentationLogic = vtkSlicerWorkflowSegmentationLogic::SafeDownCast( qSlicerTransformBufferWidgetHelper::GetSlicerModuleLogic( "WorkflowSegmentation" ) );
}


qSlicerWorkflowSegmentationRecorderControlsWidget
::~qSlicerWorkflowSegmentationRecorderControlsWidget()
{
}

void qSlicerWorkflowSegmentationRecorderControlsWidget
::onClearButtonClicked()
{
  Q_D(qSlicerWorkflowSegmentationRecorderControlsWidget);  

  //this->WorkflowSegmentationLogic->ResetWorkflowAlgorithms();
  this->TransformRecorderLogic->ClearTransforms( this->BufferHelper->GetTransformBufferNode() );
  this->WorkflowSegmentationLogic->ResetWorkflowAlgorithms();
  
  this->updateWidget();
}
