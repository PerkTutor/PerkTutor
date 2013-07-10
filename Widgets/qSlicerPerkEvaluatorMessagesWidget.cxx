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
#include "qSlicerPerkEvaluatorMessagesWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerPerkEvaluatorMessagesWidgetPrivate
  : public Ui_qSlicerMessagesWidget
{
  Q_DECLARE_PUBLIC(qSlicerPerkEvaluatorMessagesWidget);
protected:
  qSlicerPerkEvaluatorMessagesWidget* const q_ptr;

public:
  qSlicerPerkEvaluatorMessagesWidgetPrivate( qSlicerPerkEvaluatorMessagesWidget& object);
  ~qSlicerPerkEvaluatorMessagesWidgetPrivate();
  virtual void setupUi(qSlicerPerkEvaluatorMessagesWidget*);
};

// --------------------------------------------------------------------------
qSlicerPerkEvaluatorMessagesWidgetPrivate
::qSlicerPerkEvaluatorMessagesWidgetPrivate( qSlicerPerkEvaluatorMessagesWidget& object) : q_ptr(&object)
{
}

qSlicerPerkEvaluatorMessagesWidgetPrivate
::~qSlicerPerkEvaluatorMessagesWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerPerkEvaluatorMessagesWidgetPrivate
::setupUi(qSlicerPerkEvaluatorMessagesWidget* widget)
{
  this->Ui_qSlicerMessagesWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorMessagesWidget methods

//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorMessagesWidget
::qSlicerPerkEvaluatorMessagesWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerPerkEvaluatorMessagesWidgetPrivate(*this) )
{
}


qSlicerPerkEvaluatorMessagesWidget
::~qSlicerPerkEvaluatorMessagesWidget()
{
}


qSlicerPerkEvaluatorMessagesWidget* qSlicerPerkEvaluatorMessagesWidget
::New( qSlicerTransformBufferWidget* newBufferWidget, vtkSlicerPerkEvaluatorLogic* newPerkEvaluatorLogic )
{
  qSlicerPerkEvaluatorMessagesWidget* newMessagesWidget = new qSlicerPerkEvaluatorMessagesWidget();
  newMessagesWidget->BufferWidget = newBufferWidget;
  newMessagesWidget->PerkEvaluatorLogic = newPerkEvaluatorLogic;
  newMessagesWidget->setup();
  return newMessagesWidget;
}


void qSlicerPerkEvaluatorMessagesWidget
::onAddMessageButtonClicked()
{
  Q_D(qSlicerPerkEvaluatorMessagesWidget);  

  QString messageName = QInputDialog::getText( this, tr("Add Message"), tr("Input text for the new message:") );

  if ( messageName.isNull() )
  {
    return;
  }

  // Record the timestamp
  double time = this->PerkEvaluatorLogic->GetPlaybackTime();
  this->BufferWidget->TransformRecorderLogic->AddMessage( this->BufferWidget->GetBufferNode(), messageName.toStdString(), time );
  
  this->updateWidget();
}
