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
#include "qSlicerTrackedSequenceMessagesWidget.h"

#include <QtGui>
#include <QInputDialog>
#include <QScrollBar>

#include "vtkSlicerConfigure.h" // For Slicer_HAVE_QT


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerTrackedSequenceMessagesWidgetPrivate
  : public Ui_qSlicerTrackedSequenceMessagesWidget
{
  Q_DECLARE_PUBLIC(qSlicerTrackedSequenceMessagesWidget);
protected:
  qSlicerTrackedSequenceMessagesWidget* const q_ptr;

public:
  qSlicerTrackedSequenceMessagesWidgetPrivate( qSlicerTrackedSequenceMessagesWidget& object);
  ~qSlicerTrackedSequenceMessagesWidgetPrivate();
  virtual void setupUi(qSlicerTrackedSequenceMessagesWidget*);
};

// --------------------------------------------------------------------------
qSlicerTrackedSequenceMessagesWidgetPrivate
::qSlicerTrackedSequenceMessagesWidgetPrivate( qSlicerTrackedSequenceMessagesWidget& object) : q_ptr(&object)
{
}

qSlicerTrackedSequenceMessagesWidgetPrivate
::~qSlicerTrackedSequenceMessagesWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerTrackedSequenceMessagesWidgetPrivate
::setupUi(qSlicerTrackedSequenceMessagesWidget* widget)
{
  this->Ui_qSlicerTrackedSequenceMessagesWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerTrackedSequenceMessagesWidget methods

//-----------------------------------------------------------------------------
qSlicerTrackedSequenceMessagesWidget
::qSlicerTrackedSequenceMessagesWidget(QWidget* parentWidget) : qSlicerWidget( parentWidget ) , d_ptr( new qSlicerTrackedSequenceMessagesWidgetPrivate(*this) )
{
  this->TrackedSequenceBrowserNode = NULL;
  this->TransformRecorderLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "TransformRecorder" ) );
  this->setup();
}


qSlicerTrackedSequenceMessagesWidget
::~qSlicerTrackedSequenceMessagesWidget()
{
}


void qSlicerTrackedSequenceMessagesWidget
::setup()
{
  Q_D(qSlicerTrackedSequenceMessagesWidget);

  d->setupUi(this);

  connect( d->AddMessageButton, SIGNAL( clicked() ), this, SLOT( onAddMessageButtonClicked() ) );
  connect( d->RemoveMessageButton, SIGNAL( clicked() ), this, SLOT( onRemoveMessageButtonClicked() ) ); 
  connect( d->ClearMessagesButton, SIGNAL( clicked() ), this, SLOT( onClearMessagesButtonClicked() ) );

  d->AddMessageButton->setContextMenuPolicy( Qt::CustomContextMenu );
  connect( d->AddMessageButton, SIGNAL( customContextMenuRequested(const QPoint&) ), this, SLOT( onAddBlankMessageClicked() ) );

  connect( d->MessagesTableWidget, SIGNAL( cellChanged( int, int ) ), this, SLOT( onMessageEdited( int, int ) ) );
  connect( d->MessagesTableWidget, SIGNAL( cellDoubleClicked( int, int ) ), this, SLOT( onMessageDoubleClicked( int, int ) ) );

  this->updateWidget();  
}


void qSlicerTrackedSequenceMessagesWidget
::setTrackedSequenceBrowserNode( vtkMRMLNode* newTrackedSequenceBrowserNode )
{
  Q_D(qSlicerTrackedSequenceMessagesWidget);

  this->qvtkDisconnectAll();

  this->TrackedSequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast( newTrackedSequenceBrowserNode );

  this->qvtkConnect( this->TrackedSequenceBrowserNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateWidget();
}


void qSlicerTrackedSequenceMessagesWidget
::onAddMessageButtonClicked()
{
  Q_D(qSlicerTrackedSequenceMessagesWidget);

  if ( this->TrackedSequenceBrowserNode == NULL 
    || this->TrackedSequenceBrowserNode->GetMasterSequenceNode() == NULL )
  {
    return;
  }

  double time = this->TransformRecorderLogic->GetMaximumIndexValue( this->TrackedSequenceBrowserNode );

  QString messageName = QInputDialog::getText( this, tr( "Add Message" ), tr( "Input text for the new message:" ) );
  if ( messageName.isNull() )
  {
    return;
  }


  // Record the timestamp
  std::stringstream ss;
  ss << time;
  this->TransformRecorderLogic->AddMessage( this->TrackedSequenceBrowserNode, messageName.toStdString(), ss.str() );

  
  this->updateWidget();

}


void qSlicerTrackedSequenceMessagesWidget
::onRemoveMessageButtonClicked()
{
  Q_D(qSlicerTrackedSequenceMessagesWidget);

  
  if ( this->TrackedSequenceBrowserNode == NULL )
  {
    return;
  }

  this->TransformRecorderLogic->RemoveMessage( this->TrackedSequenceBrowserNode, d->MessagesTableWidget->currentRow() );
}


void qSlicerTrackedSequenceMessagesWidget
::onClearMessagesButtonClicked()
{
  Q_D(qSlicerTrackedSequenceMessagesWidget);

  if ( this->TrackedSequenceBrowserNode == NULL )
  {
    return;
  }

  this->TransformRecorderLogic->ClearMessages( this->TrackedSequenceBrowserNode );
  
  this->updateWidget();
}


void qSlicerTrackedSequenceMessagesWidget
::onAddBlankMessageClicked()
{
  Q_D(qSlicerTrackedSequenceMessagesWidget);

  if ( this->TrackedSequenceBrowserNode == NULL )
  {
    return;
  }

  double time = this->TransformRecorderLogic->GetMaximumIndexValue( this->TrackedSequenceBrowserNode );
  std::stringstream ss;
  ss << time;
  this->TransformRecorderLogic->AddMessage( this->TrackedSequenceBrowserNode, "", ss.str() );
  
  this->updateWidget();
}



void qSlicerTrackedSequenceMessagesWidget
::onMessageEdited( int row, int column )
{
  Q_D(qSlicerTrackedSequenceMessagesWidget);

  if ( this->TrackedSequenceBrowserNode == NULL || column != qSlicerTrackedSequenceMessagesWidget::MESSAGE_NAME_COLUMN )
  {
    return;
  }

  // Find the entry that we changed
  QTableWidgetItem* qItem = d->MessagesTableWidget->item( row, column );
  QString qText = qItem->text();

  this->TransformRecorderLogic->UpdateMessage( this->TrackedSequenceBrowserNode, qText.toStdString(), row );
  
  this->updateWidget();
}


void qSlicerTrackedSequenceMessagesWidget
::onMessageDoubleClicked( int row, int column )
{
  // Do nothing, with the understanding that sublasses can implement this slot
}


void qSlicerTrackedSequenceMessagesWidget
::updateWidget()
{
  Q_D(qSlicerTrackedSequenceMessagesWidget);

  // Check what the current row and column are
  int currentRow = d->MessagesTableWidget->currentRow();
  int currentColumn = d->MessagesTableWidget->currentColumn();
  int scrollPosition = d->MessagesTableWidget->verticalScrollBar()->value();
  
  // The only thing to do is update the table entries. Must ensure they are in sorted order (that's how they are stored in the buffer).
  d->MessagesTableWidget->clear();
  QStringList MessagesTableHeaders;
  MessagesTableHeaders << "Time" << "Message";
  d->MessagesTableWidget->setRowCount( 0 );
  d->MessagesTableWidget->setColumnCount( 2 );
  d->MessagesTableWidget->setHorizontalHeaderLabels( MessagesTableHeaders ); 
#ifdef Slicer_HAVE_QT5
  d->MessagesTableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
  d->MessagesTableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif

  if ( this->TrackedSequenceBrowserNode == NULL )
  {
    return;
  }

  // Iterate over all the messages in the message sequence
  vtkMRMLSequenceNode* messageSequenceNode = this->TransformRecorderLogic->GetMessageSequenceNode( this->TrackedSequenceBrowserNode );
  if ( messageSequenceNode == NULL )
  {
    return;
  }

  // Block signals while updating
  bool wasBlockedTableWidget = d->MessagesTableWidget->blockSignals( true );

  d->MessagesTableWidget->setRowCount( messageSequenceNode->GetNumberOfDataNodes() );
  for ( int i = 0; i < messageSequenceNode->GetNumberOfDataNodes(); i++ )
  {
    std::stringstream ss( messageSequenceNode->GetNthIndexValue( i ) );
    double messageTime; ss >> messageTime;
    QTableWidgetItem* timeItem = new QTableWidgetItem( QString::number( messageTime, 'f', 2 ) );
    timeItem->setFlags( timeItem->flags() & ~Qt::ItemIsEditable );

    vtkMRMLNode* messageNode = messageSequenceNode->GetNthDataNode( i );
    if ( messageNode == NULL )
    {
      return;
    }
    QTableWidgetItem* messageItem = new QTableWidgetItem( messageNode->GetAttribute( "Message" ) );
    d->MessagesTableWidget->setItem( i, qSlicerTrackedSequenceMessagesWidget::MESSAGE_TIME_COLUMN, timeItem );
    d->MessagesTableWidget->setItem( i, qSlicerTrackedSequenceMessagesWidget::MESSAGE_NAME_COLUMN, messageItem ); 
  }

  // Reset the current row and column to what they were
  d->MessagesTableWidget->setCurrentCell( currentRow, currentColumn );
  d->MessagesTableWidget->verticalScrollBar()->setValue( scrollPosition );

  d->MessagesTableWidget->blockSignals( wasBlockedTableWidget );
}
