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
  : public Ui_qSlicerTrackedSequenceMessagesWidget
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
  this->Ui_qSlicerTrackedSequenceMessagesWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorMessagesWidget methods

//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorMessagesWidget
::qSlicerPerkEvaluatorMessagesWidget(QWidget* parentWidget) : qSlicerTrackedSequenceMessagesWidget( parentWidget ) , d_ptr( new qSlicerPerkEvaluatorMessagesWidgetPrivate(*this) )
{
  this->PerkEvaluatorNode = NULL;
  this->PerkEvaluatorLogic = vtkSlicerPerkEvaluatorLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "PerkEvaluator" ) );
}


qSlicerPerkEvaluatorMessagesWidget
::~qSlicerPerkEvaluatorMessagesWidget()
{
}



void qSlicerPerkEvaluatorMessagesWidget
::setPerkEvaluatorNode( vtkMRMLNode* newPerkEvaluatorNode )
{
  Q_D(qSlicerPerkEvaluatorMessagesWidget);

  this->PerkEvaluatorNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( newPerkEvaluatorNode );

  this->updateWidget();
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

  double time = this->PerkEvaluatorLogic->GetSelectedTime( this->TrackedSequenceBrowserNode );
  
  // Record the timestamp
  std::stringstream ss;
  ss << time;
  this->TransformRecorderLogic->AddMessage( this->TrackedSequenceBrowserNode, messageName.toStdString(), ss.str() );
  
  this->updateWidget();  // Force this update widget
}


void qSlicerPerkEvaluatorMessagesWidget
::onAddBlankMessageClicked()
{
  Q_D(qSlicerPerkEvaluatorMessagesWidget);  

  double time = this->PerkEvaluatorLogic->GetSelectedTime( this->TrackedSequenceBrowserNode );
  
  // Record the timestamp
  std::stringstream ss;
  ss << time;
  this->TransformRecorderLogic->AddMessage( this->TrackedSequenceBrowserNode, "", ss.str() );
  
  this->updateWidget();  // Force this update widget
}


void qSlicerPerkEvaluatorMessagesWidget
::onMessageDoubleClicked( int row, int column )
{
  Q_D(qSlicerPerkEvaluatorMessagesWidget);

  if ( this->TrackedSequenceBrowserNode == NULL || column == qSlicerTrackedSequenceMessagesWidget::MESSAGE_NAME_COLUMN )
  {
    return;
  }

  vtkMRMLSequenceNode* messagesSequenceNode = this->TransformRecorderLogic->GetMessageSequenceNode( this->TrackedSequenceBrowserNode );
  vtkMRMLSequenceNode* masterSequenceNode = this->TrackedSequenceBrowserNode->GetMasterSequenceNode();
  if ( messagesSequenceNode == NULL || masterSequenceNode == NULL )
  {
    return;
  }

  std::string messageTimeString = messagesSequenceNode->GetNthIndexValue( row );
  int masterItemNumber = masterSequenceNode->GetItemNumberFromIndexValue( messageTimeString );
  this->TrackedSequenceBrowserNode->SetSelectedItemNumber( masterItemNumber );

  this->updateWidget();  // Force this update widget
}