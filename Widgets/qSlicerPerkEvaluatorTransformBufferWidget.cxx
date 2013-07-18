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
#include "qSlicerPerkEvaluatorTransformBufferWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerPerkEvaluatorTransformBufferWidgetPrivate
  : public Ui_qSlicerTransformBufferWidget
{
  Q_DECLARE_PUBLIC(qSlicerPerkEvaluatorTransformBufferWidget);
protected:
  qSlicerPerkEvaluatorTransformBufferWidget* const q_ptr;

public:
  qSlicerPerkEvaluatorTransformBufferWidgetPrivate( qSlicerPerkEvaluatorTransformBufferWidget& object);
  ~qSlicerPerkEvaluatorTransformBufferWidgetPrivate();
  virtual void setupUi(qSlicerPerkEvaluatorTransformBufferWidget*);
};

// --------------------------------------------------------------------------
qSlicerPerkEvaluatorTransformBufferWidgetPrivate
::qSlicerPerkEvaluatorTransformBufferWidgetPrivate( qSlicerPerkEvaluatorTransformBufferWidget& object) : q_ptr(&object)
{
}

qSlicerPerkEvaluatorTransformBufferWidgetPrivate
::~qSlicerPerkEvaluatorTransformBufferWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerPerkEvaluatorTransformBufferWidgetPrivate
::setupUi(qSlicerPerkEvaluatorTransformBufferWidget* widget)
{
  this->Ui_qSlicerTransformBufferWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorTransformBufferWidget methods

//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorTransformBufferWidget
::qSlicerPerkEvaluatorTransformBufferWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerPerkEvaluatorTransformBufferWidgetPrivate(*this) )
{
}


qSlicerPerkEvaluatorTransformBufferWidget
::~qSlicerPerkEvaluatorTransformBufferWidget()
{
}


qSlicerPerkEvaluatorTransformBufferWidget* qSlicerPerkEvaluatorTransformBufferWidget
::New( vtkSlicerPerkEvaluatorLogic* newPerkEvaluatorLogic )
{
  qSlicerPerkEvaluatorTransformBufferWidget* newTransformBufferWidget = new qSlicerPerkEvaluatorTransformBufferWidget();
  newTransformBufferWidget->PerkEvaluatorLogic = newPerkEvaluatorLogic;
  newTransformBufferWidget->TransformRecorderLogic = newPerkEvaluatorLogic->TransformRecorderLogic;
  newTransformBufferWidget->UpdateStatus = 0;
  newTransformBufferWidget->setup();
  return newTransformBufferWidget;
}

void qSlicerPerkEvaluatorTransformBufferWidget
::onCurrentBufferNodeChanged()
{
  Q_D(qSlicerPerkEvaluatorTransformBufferWidget);
  
  this->PerkEvaluatorLogic->UpdateToolTrajectories( this->GetBufferNode() );
  this->PerkEvaluatorLogic->SetPlaybackTime( this->PerkEvaluatorLogic->GetMinTime() );

  this->UpdateStatus++;
  this->updateWidget();
}