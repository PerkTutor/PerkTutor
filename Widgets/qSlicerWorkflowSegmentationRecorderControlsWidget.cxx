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
::qSlicerWorkflowSegmentationRecorderControlsWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate(*this) )
{
}


qSlicerWorkflowSegmentationRecorderControlsWidget
::~qSlicerWorkflowSegmentationRecorderControlsWidget()
{
}


qSlicerWorkflowSegmentationRecorderControlsWidget* qSlicerWorkflowSegmentationRecorderControlsWidget
::New( vtkSlicerWorkflowSegmentationLogic* newWSLogic )
{
  qSlicerWorkflowSegmentationRecorderControlsWidget* newRecorderControlsWidget = new qSlicerWorkflowSegmentationRecorderControlsWidget();
  newRecorderControlsWidget->SetLogic( newWSLogic );
  newRecorderControlsWidget->setup();
  return newRecorderControlsWidget;
}


void qSlicerWorkflowSegmentationRecorderControlsWidget
::SetLogic( vtkSlicerWorkflowSegmentationLogic* newWSLogic )
{
  this->wsLogic = newWSLogic;
  this->qSlicerRecorderControlsWidget::SetLogic( newWSLogic->TransformRecorderLogic );
}


void qSlicerWorkflowSegmentationRecorderControlsWidget
::onClearButtonClicked()
{
  Q_D(qSlicerWorkflowSegmentationRecorderControlsWidget);  

  this->trLogic->ClearBuffer();
  this->wsLogic->ResetWorkflowAlgorithms();
  
  this->updateWidget();
}
