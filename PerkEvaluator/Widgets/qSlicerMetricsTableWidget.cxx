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
#include "qSlicerMetricsTableWidget.h"

#include <QtGui>
#include <QScrollBar>

#include "vtkSlicerConfigure.h" // For Slicer_HAVE_QT5

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerMetricsTableWidgetPrivate
  : public Ui_qSlicerMetricsTableWidget
{
  Q_DECLARE_PUBLIC(qSlicerMetricsTableWidget);
protected:
  qSlicerMetricsTableWidget* const q_ptr;

public:
  qSlicerMetricsTableWidgetPrivate( qSlicerMetricsTableWidget& object);
  ~qSlicerMetricsTableWidgetPrivate();
  virtual void setupUi(qSlicerMetricsTableWidget*);
};

// --------------------------------------------------------------------------
qSlicerMetricsTableWidgetPrivate
::qSlicerMetricsTableWidgetPrivate( qSlicerMetricsTableWidget& object) : q_ptr(&object)
{
}

qSlicerMetricsTableWidgetPrivate
::~qSlicerMetricsTableWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerMetricsTableWidgetPrivate
::setupUi(qSlicerMetricsTableWidget* widget)
{
  this->Ui_qSlicerMetricsTableWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerMetricsTableWidget methods

//-----------------------------------------------------------------------------
qSlicerMetricsTableWidget
::qSlicerMetricsTableWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerMetricsTableWidgetPrivate(*this) )
{
  this->MetricsTableNode = NULL;
  this->ExpandHeightToContents = true;
  this->ShowMetricRoles = true;
  this->setup();
}


qSlicerMetricsTableWidget
::~qSlicerMetricsTableWidget()
{
}


void qSlicerMetricsTableWidget
::setup()
{
  Q_D(qSlicerMetricsTableWidget);

  d->setupUi(this);

  connect( d->MetricsTableNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMetricsTableNodeChanged( vtkMRMLNode* ) ) );
  
  connect( d->ClipboardButton, SIGNAL( clicked() ), this, SLOT( onClipboardButtonClicked() ) );
  d->ClipboardButton->setIcon( QIcon( ":/Icons/Small/SlicerEditCopy.png" ) );

  connect( d->MetricsTable->horizontalHeader(), SIGNAL( sectionDoubleClicked( int ) ), this, SLOT( onHeaderDoubleClicked( int ) ) );

  d->MetricsTable->installEventFilter( this );

  this->updateWidget();  
}


vtkMRMLTableNode* qSlicerMetricsTableWidget
::getMetricsTableNode()
{
  Q_D(qSlicerMetricsTableWidget);

  return this->MetricsTableNode;
}


void qSlicerMetricsTableWidget
::setMetricsTableNode( vtkMRMLNode* newMetricsTableNode )
{
  Q_D(qSlicerMetricsTableWidget);

  d->MetricsTableNodeComboBox->setCurrentNode( newMetricsTableNode );
  // If it is a new table node, then the onTrackedSequenceBrowserNodeChanged will be called automatically
}


vtkMRMLTableNode* qSlicerMetricsTableWidget
::addMetricsTableNode()
{
  Q_D(qSlicerMetricsTableWidget);

  return vtkMRMLTableNode::SafeDownCast( d->MetricsTableNodeComboBox->addNode() ); // Automatically calls "onMetricsTableNodeChanged" function
}


bool qSlicerMetricsTableWidget
::getExpandHeightToContents()
{
  Q_D(qSlicerMetricsTableWidget);

  return this->ExpandHeightToContents;
}


void qSlicerMetricsTableWidget
::setExpandHeightToContents( bool expand )
{
  Q_D(qSlicerMetricsTableWidget);

  this->ExpandHeightToContents = expand;
  this->updateWidget();
}


int qSlicerMetricsTableWidget
::getContentHeight()
{
  Q_D(qSlicerMetricsTableWidget);

  int contentHeight = d->MetricsTable->horizontalHeader()->height() + 4; // This "magic" number makes it so there is no scroll bar
  for ( int i = 0; i < d->MetricsTable->rowCount(); i++ )
  {
    contentHeight += d->MetricsTable->rowHeight( i );
  }
  contentHeight += d->MetricsTable->horizontalScrollBar()->height();
  return contentHeight;
}


bool qSlicerMetricsTableWidget
::getShowMetricRoles()
{
  Q_D(qSlicerMetricsTableWidget);

  return this->ShowMetricRoles;
}


void qSlicerMetricsTableWidget
::setShowMetricRoles( bool show )
{
  Q_D(qSlicerMetricsTableWidget);

  this->ShowMetricRoles = show;
  this->updateWidget();
}


void qSlicerMetricsTableWidget
::setMetricsTableSelectionRowVisible( bool visible )
{
  Q_D(qSlicerMetricsTableWidget);

  d->MetricsTableNodeComboBox->setVisible( visible );
  d->ClipboardButton->setVisible( visible );
  this->updateWidget();
}



void qSlicerMetricsTableWidget
::onMetricsTableNodeChanged( vtkMRMLNode* newMetricsTableNode )
{
  Q_D(qSlicerMetricsTableWidget);

  this->qvtkDisconnectAll();

  this->MetricsTableNode = vtkMRMLTableNode::SafeDownCast( newMetricsTableNode );

  this->qvtkConnect( this->MetricsTableNode, vtkCommand::ModifiedEvent, this, SLOT( onMetricsTableNodeModified() ) );

  this->updateWidget();

  emit metricsTableNodeChanged( this->MetricsTableNode );
}


void qSlicerMetricsTableWidget
::onMetricsTableNodeModified()
{
  this->updateWidget();
  emit metricsTableNodeModified(); // This should allows parent widgets to update themselves
}


void qSlicerMetricsTableWidget
::onClipboardButtonClicked()
{
  Q_D( qSlicerMetricsTableWidget );
  
  // Add all rows to the clipboard vector
  std::vector<bool> copyRow = std::vector<bool>( d->MetricsTable->rowCount(), true );
  std::vector<bool> copyColumn = std::vector<bool>( d->MetricsTable->columnCount(), true );
  this->copyMetricsTableToClipboard( copyRow, copyColumn );
}


void qSlicerMetricsTableWidget
::copyMetricsTableToClipboard( std::vector<bool> copyRow, std::vector<bool> copyColumn )
{
  Q_D( qSlicerMetricsTableWidget );

  // Grab all of the contents of the selected rows from whatever is currently on the metrics table
  // Note that we copy from the table widget, not directly from the table node
  // This is because the user expects what they copied to be what was on the table widget
  // In the case of sorting the table widget, the underlying table node would have different order than the user, causing the copied text to be unexpectedly different from what is displayed on the table widget
  QString clipString = QString( "" );
  for ( int i = 0; i < d->MetricsTable->rowCount(); i++ )
  {
    if ( ! copyRow.at( i ) )
    {
      continue;
    }
    for ( int j = 0; j < d->MetricsTable->columnCount(); j++ )
    {
      if ( ! copyColumn.at( j ) )
      {
        continue;
      }

      clipString.append( d->MetricsTable->item( i, j )->text() );
      clipString.append( "\t" );
    }
    clipString.chop( 1 ); // Remove the last tab from the line
    clipString.append( "\n" );
  }

  QApplication::clipboard()->setText( clipString );
}


bool qSlicerMetricsTableWidget
::eventFilter( QObject * watched, QEvent * event )
{
  Q_D( qSlicerMetricsTableWidget );

  if ( watched != d->MetricsTable || event->type() != QEvent::KeyPress )
  {
    return false;
  }

  QKeyEvent* keyEvent = static_cast< QKeyEvent* >( event );
  if ( ! keyEvent->matches(QKeySequence::Copy) )
  {
    return false;
  }
  
  QModelIndexList modelIndexList = d->MetricsTable->selectionModel()->selectedIndexes();
  std::vector<bool> copyRow = std::vector<bool>( d->MetricsTable->rowCount(), false );
  std::vector<bool> copyColumn = std::vector<bool>( d->MetricsTable->columnCount(), false );
  for ( QModelIndexList::iterator index = modelIndexList.begin(); index != modelIndexList.end(); index++ )
  {
    copyRow.at( ( *index ).row() ) = true;
    copyColumn.at( ( *index ).column() ) = true;
  }
  // Add all rows to the clipboard vector
  this->copyMetricsTableToClipboard( copyRow, copyColumn );

  return true;
}


void qSlicerMetricsTableWidget
::onHeaderDoubleClicked( int column )
{
  Q_D( qSlicerMetricsTableWidget );

  // Sort the column, and then reset the table sizing
  d->MetricsTable->sortItems( column );
  d->MetricsTable->resizeRowsToContents();

  if ( this->ExpandHeightToContents )
  {
    d->MetricsTable->setMinimumHeight( this->getContentHeight() );
  }
}


void qSlicerMetricsTableWidget
::updateWidget()
{
  Q_D(qSlicerMetricsTableWidget);

  d->MetricsTableNodeComboBox->setCurrentNode( this->MetricsTableNode );

  // Check what the current row and column are
  int currentRow = d->MetricsTable->currentRow();
  int currentColumn = d->MetricsTable->currentColumn();
  int scrollPosition = d->MetricsTable->verticalScrollBar()->value();

  // Set up the table
  d->MetricsTable->clear();
  d->MetricsTable->setRowCount( 0 );

  if ( this->MetricsTableNode == NULL )
  {
    return;
  }
  vtkTable* metricsTableTable = this->MetricsTableNode->GetTable();
  if ( metricsTableTable == NULL )
  {
    return;
  }

  // Get the names of all tasks present on the table
  QStringList taskNames;
  for ( int i = 0; i < metricsTableTable->GetNumberOfColumns(); i++ )
  {
    std::string currentColumnName = metricsTableTable->GetColumnName( i );
    if ( currentColumnName.compare( "MetricName" ) == 0 || currentColumnName.compare( "MetricRoles" ) == 0 || currentColumnName.compare( "MetricUnit" ) == 0 || currentColumnName.compare( "MetricValue" ) == 0 )
    {
      continue;
    }
    taskNames << currentColumnName.c_str();
  }

  QStringList metricsTableHeaders;
  metricsTableHeaders << "Metric" << "Value" << taskNames;
  d->MetricsTable->setRowCount( metricsTableTable->GetNumberOfRows() );
  d->MetricsTable->setColumnCount( metricsTableHeaders.count() );
  d->MetricsTable->setHorizontalHeaderLabels( metricsTableHeaders );

#ifdef Slicer_HAVE_QT5
  d->MetricsTable->horizontalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents );
  d->MetricsTable->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Interactive );
#else
  d->MetricsTable->horizontalHeader()->setResizeMode( QHeaderView::ResizeToContents );
  d->MetricsTable->horizontalHeader()->setResizeMode( 0, QHeaderView::Interactive );
#endif
  
  // Add the computed values to the table
  for ( int i = 0; i < metricsTableTable->GetNumberOfRows(); i++ )
  {
    QString nameString;
    nameString.append( metricsTableTable->GetValueByName( i, "MetricName" ).ToString() );
    
    if ( this->ShowMetricRoles )
    {
      nameString.append( " [" );
      nameString.append( metricsTableTable->GetValueByName( i, "MetricRoles" ).ToString() );
      nameString.append( "]" );
    }

    nameString.append( " (" );
    nameString.append( metricsTableTable->GetValueByName( i, "MetricUnit" ).ToString() );
    nameString.append( ")" );
    QTableWidgetItem* nameItem = new QTableWidgetItem( nameString );
    d->MetricsTable->setItem( i, metricsTableHeaders.indexOf( "Metric" ), nameItem );

    QString valueString;
    valueString.append( metricsTableTable->GetValueByName( i, "MetricValue" ).ToString() );
    QTableWidgetItem* valueItem = new QTableWidgetItem( valueString );    
    d->MetricsTable->setItem( i, metricsTableHeaders.indexOf( "Value" ), valueItem );

    // Add the task-specific metrics
    for ( int j = 0; j < taskNames.count(); j++ )
    {
      QString taskValueString;
      taskValueString.append( metricsTableTable->GetValueByName( i, taskNames.at( j ).toAscii() ).ToString() );
      QTableWidgetItem* taskValueItem = new QTableWidgetItem( taskValueString );
      d->MetricsTable->setItem( i, metricsTableHeaders.indexOf( taskNames.at( j ) ), taskValueItem );
    }
  }

  d->MetricsTable->sortItems( 0 ); // Sort by metric name to ensure we always have consistent ordering (no matter what order the metrics were imported into the scene)
  d->MetricsTable->resizeRowsToContents();

  // Make sure the table widget is large enough so that no scroll bar is needed to see all of the data
  if ( this->ExpandHeightToContents )
  {
    d->MetricsTable->setMinimumHeight( this->getContentHeight() );
  }

  // Reset the current row and column to what they were
  d->MetricsTable->setCurrentCell( currentRow, currentColumn );
  d->MetricsTable->verticalScrollBar()->setValue( scrollPosition );
}
