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
#include "qSlicerPerkEvaluatorRecorderControlsWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerPerkEvaluatorRecorderControlsWidgetPrivate
  : public Ui_qSlicerRecorderControlsWidget
{
  Q_DECLARE_PUBLIC(qSlicerPerkEvaluatorRecorderControlsWidget);
protected:
  qSlicerPerkEvaluatorRecorderControlsWidget* const q_ptr;

public:
  qSlicerPerkEvaluatorRecorderControlsWidgetPrivate( qSlicerPerkEvaluatorRecorderControlsWidget& object);
  ~qSlicerPerkEvaluatorRecorderControlsWidgetPrivate();
  virtual void setupUi(qSlicerPerkEvaluatorRecorderControlsWidget*);
};

// --------------------------------------------------------------------------
qSlicerPerkEvaluatorRecorderControlsWidgetPrivate
::qSlicerPerkEvaluatorRecorderControlsWidgetPrivate( qSlicerPerkEvaluatorRecorderControlsWidget& object) : q_ptr(&object)
{
}

qSlicerPerkEvaluatorRecorderControlsWidgetPrivate
::~qSlicerPerkEvaluatorRecorderControlsWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerPerkEvaluatorRecorderControlsWidgetPrivate
::setupUi(qSlicerPerkEvaluatorRecorderControlsWidget* widget)
{
  this->Ui_qSlicerRecorderControlsWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorRecorderControlsWidget methods

//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorRecorderControlsWidget
::qSlicerPerkEvaluatorRecorderControlsWidget(QWidget* parentWidget) : qSlicerRecorderControlsWidget( parentWidget ) , d_ptr( new qSlicerPerkEvaluatorRecorderControlsWidgetPrivate(*this) )
{
  this->PerkEvaluatorNode = NULL;
  this->PerkEvaluatorLogic = vtkSlicerPerkEvaluatorLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "PerkEvaluator" ) );
}


qSlicerPerkEvaluatorRecorderControlsWidget
::~qSlicerPerkEvaluatorRecorderControlsWidget()
{
}


void qSlicerPerkEvaluatorRecorderControlsWidget
::setPerkEvaluatorNode( vtkMRMLNode* newPerkEvaluatorNode )
{
  Q_D(qSlicerPerkEvaluatorRecorderControlsWidget);

  this->PerkEvaluatorNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( newPerkEvaluatorNode );

  this->updateWidget();
}


void qSlicerPerkEvaluatorRecorderControlsWidget
::onStartButtonClicked()
{
  Q_D(qSlicerPerkEvaluatorRecorderControlsWidget);

  this->qSlicerRecorderControlsWidget::onStartButtonClicked();

  if ( this->PerkEvaluatorNode == NULL )
  {
    return;
  }

  this->PerkEvaluatorNode->SetRealTimeProcessing( true );  
}


void qSlicerPerkEvaluatorRecorderControlsWidget
::onStopButtonClicked()
{
  Q_D(qSlicerPerkEvaluatorRecorderControlsWidget);

  this->qSlicerRecorderControlsWidget::onStopButtonClicked();

  if ( this->PerkEvaluatorNode == NULL )
  {
    return;
  }

  this->PerkEvaluatorNode->SetRealTimeProcessing( false );  
}