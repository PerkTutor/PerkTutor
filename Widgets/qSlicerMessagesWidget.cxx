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
#include "qSlicerMessagesWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerMessagesWidgetPrivate
  : public Ui_qSlicerMessagesWidget
{
  Q_DECLARE_PUBLIC(qSlicerMessagesWidget);
protected:
  qSlicerMessagesWidget* const q_ptr;

public:
  qSlicerMessagesWidgetPrivate( qSlicerMessagesWidget& object);
  ~qSlicerMessagesWidgetPrivate();
  virtual void setupUi(qSlicerMessagesWidget*);
};

// --------------------------------------------------------------------------
qSlicerMessagesWidgetPrivate
::qSlicerMessagesWidgetPrivate( qSlicerMessagesWidget& object) : q_ptr(&object)
{
}

qSlicerMessagesWidgetPrivate
::~qSlicerMessagesWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerMessagesWidgetPrivate
::setupUi(qSlicerMessagesWidget* widget)
{
  this->Ui_qSlicerMessagesWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerMessagesWidget methods

//-----------------------------------------------------------------------------
qSlicerMessagesWidget
::qSlicerMessagesWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerMessagesWidgetPrivate(*this) )
{
}


qSlicerMessagesWidget
::~qSlicerMessagesWidget()
{
}


qSlicerMessagesWidget* qSlicerMessagesWidget
::New( vtkSlicerTransformRecorderLogic* newTRLogic )
{
  qSlicerMessagesWidget* newMessagesWidget = new qSlicerMessagesWidget();
  newMessagesWidget->SetLogic( newTRLogic );
  newMessagesWidget->setup();
  return newMessagesWidget;
}


void qSlicerMessagesWidget
::SetLogic( vtkSlicerTransformRecorderLogic* newTRLogic )
{
  this->trLogic = newTRLogic;
}


void qSlicerMessagesWidget
::setup()
{
  Q_D(qSlicerMessagesWidget);

  d->setupUi(this);

  connect( d->AddMessageButton, SIGNAL( clicked() ), this, SLOT( onAddMessageButtonClicked() ) );
  connect( d->RemoveMessageButton, SIGNAL( clicked() ), this, SLOT( onRemoveMessageButtonClicked() ) ); 
  connect( d->ClearMessagesButton, SIGNAL( clicked() ), this, SLOT( onClearMessagesButtonClicked() ) );

  this->updateWidget();  
}


void qSlicerMessagesWidget
::enter()
{
}


// TODO: Implement these properly
void qSlicerMessagesWidget
::onAddMessageButtonClicked()
{
  Q_D(qSlicerMessagesWidget);  

  QString messageName = QInputDialog::getText( this, tr("Add Message"), tr("Input text for the new message:") );

  if ( messageName.isNull() )
  {
    return;
  }

  // Record the timestamp
  double time = this->trLogic->GetCurrentTimestamp();
  this->trLogic->AddMessage( messageName.toStdString(), time );
  
  this->updateWidget();
}


void qSlicerMessagesWidget
::onRemoveMessageButtonClicked()
{
  Q_D(qSlicerMessagesWidget);

  this->trLogic->RemoveMessage( d->MessagesTableWidget->currentRow() );

  this->updateWidget();
}


void qSlicerMessagesWidget
::onClearMessagesButtonClicked()
{
  Q_D(qSlicerMessagesWidget);

  this->trLogic->ClearMessages();
  
  this->updateWidget();
}


void qSlicerMessagesWidget
::updateWidget()
{
  Q_D(qSlicerMessagesWidget);
  
  // The only thing to do is update the table entries. Must ensure they are in sorted order (that's how they are stored in the buffer).
  d->MessagesTableWidget->clear();
  QStringList MessagesTableHeaders;
  MessagesTableHeaders << "Time" << "Message";
  d->MessagesTableWidget->setRowCount( 0 );
  d->MessagesTableWidget->setColumnCount( 2 );
  d->MessagesTableWidget->setHorizontalHeaderLabels( MessagesTableHeaders ); 
  d->MessagesTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );

  if ( this->trLogic->GetModuleNode() == NULL )
  {
    return;
  }

  // Iterate over all the messages in the buffer and add them in order
  d->MessagesTableWidget->setRowCount( this->trLogic->GetBuffer()->GetNumMessages() );
  for ( int i = 0; i < this->trLogic->GetBuffer()->GetNumMessages(); i++ )
  {
    QTableWidgetItem* timeItem = new QTableWidgetItem( QString::number( this->trLogic->GetBuffer()->GetMessageAt(i)->GetTime() ) );
	QTableWidgetItem* messageItem = new QTableWidgetItem( QString::fromStdString( this->trLogic->GetBuffer()->GetMessageAt(i)->GetName() ) );
    d->MessagesTableWidget->setItem( i, 0, timeItem );
    d->MessagesTableWidget->setItem( i, 1, messageItem ); 
  }

}
