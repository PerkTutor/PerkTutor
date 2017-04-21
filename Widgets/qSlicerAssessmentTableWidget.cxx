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
#include "qSlicerAssessmentTableWidget.h"

#include "qSlicerMetricWeightWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerAssessmentTableWidgetPrivate
  : public Ui_qSlicerAssessmentTableWidget
{
  Q_DECLARE_PUBLIC(qSlicerAssessmentTableWidget);
protected:
  qSlicerAssessmentTableWidget* const q_ptr;

public:
  qSlicerAssessmentTableWidgetPrivate( qSlicerAssessmentTableWidget& object);
  ~qSlicerAssessmentTableWidgetPrivate();
  virtual void setupUi(qSlicerAssessmentTableWidget*);
};

// --------------------------------------------------------------------------
qSlicerAssessmentTableWidgetPrivate
::qSlicerAssessmentTableWidgetPrivate( qSlicerAssessmentTableWidget& object) : q_ptr(&object)
{
}

qSlicerAssessmentTableWidgetPrivate
::~qSlicerAssessmentTableWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerAssessmentTableWidgetPrivate
::setupUi(qSlicerAssessmentTableWidget* widget)
{
  this->Ui_qSlicerAssessmentTableWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerAssessmentTableWidget methods

//-----------------------------------------------------------------------------
qSlicerAssessmentTableWidget
::qSlicerAssessmentTableWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerAssessmentTableWidgetPrivate(*this) )
{
  this->MetricsNode = NULL;
  this->MetricsWeightNode = NULL;
  this->MetricScoreNode = NULL;
  this->TaskScoreNode = NULL;
  
  this->OverallScore = 0;
  
  this->setup();
}


qSlicerAssessmentTableWidget
::~qSlicerAssessmentTableWidget()
{
}


void qSlicerAssessmentTableWidget
::setup()
{
  Q_D(qSlicerAssessmentTableWidget);

  d->setupUi(this);

  this->updateWidget();  
}


void qSlicerAssessmentTableWidget
::setMetricsNode( vtkMRMLNode* table )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->qvtkDisconnect( this->MetricsNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->MetricsNode = vtkMRMLTableNode::SafeDownCast( table );
  this->qvtkConnect( this->MetricsNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateWidget();
}


void qSlicerAssessmentTableWidget
::setMetricsWeightNode( vtkMRMLNode* table )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->qvtkDisconnect( this->MetricsWeightNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->MetricsWeightNode = vtkMRMLTableNode::SafeDownCast( table );
  this->qvtkConnect( this->MetricsWeightNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateWidget();
}


void qSlicerAssessmentTableWidget
::setMetricScoreNode( vtkMRMLNode* table )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->qvtkDisconnect( this->MetricScoreNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->MetricScoreNode = vtkMRMLTableNode::SafeDownCast( table );
  this->qvtkConnect( this->MetricScoreNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateWidget();
}


void qSlicerAssessmentTableWidget
::setTaskScoreNode( vtkMRMLNode* table )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->qvtkDisconnect( this->TaskScoreNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->TaskScoreNode = vtkMRMLTableNode::SafeDownCast( table );
  this->qvtkConnect( this->TaskScoreNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateWidget();
}


void qSlicerAssessmentTableWidget
::setOverallScore( double score )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->OverallScore = score;

  this->updateWidget();
}


void qSlicerAssessmentTableWidget
::onWeightSliderChanged( double weight )
{
  Q_D(qSlicerAssessmentTableWidget);
  
  if ( this->MetricsWeightNode == NULL || this->MetricsWeightNode->GetTable() == NULL )
  {
    return;
  }

  QObject* slider = sender();
  int rowIndex = slider->property( "row" ).toInt();
  int columnIndex = slider->property( "column" ).toInt();
  
  this->MetricsWeightNode->GetTable()->SetValue( rowIndex, columnIndex, weight );
}


void qSlicerAssessmentTableWidget
::updateWidget()
{
  Q_D(qSlicerAssessmentTableWidget);

  // Set up the table
  d->AssessmentTable->clear();
  d->AssessmentTable->setRowCount( 0 );
  
  // Get a table that has the header information in it  
  vtkTable* metricsTable = NULL;
  if ( this->MetricsNode != NULL )
  {
    metricsTable = this->MetricsNode->GetTable();
  }
  
  vtkTable* weightTable = NULL;
  if ( this->MetricsWeightNode != NULL )
  {
    weightTable = this->MetricsWeightNode->GetTable();
  }  
  
  vtkTable* headerTable = NULL;
  if ( metricsTable != NULL )
  {
    headerTable = metricsTable;
  }
  if ( weightTable != NULL )
  {
    headerTable = weightTable;
  }
  
  if ( headerTable == NULL ) // That is there is no metrics node and no weight node
  {
    return;
  }
  
  // The grid positions
  int numMetrics = headerTable->GetNumberOfRows();
  int numTasks = headerTable->GetNumberOfColumns() - 3; // Ignore "MetricName", "MetricRoles", "MetricUnit"
  
  int taskHeaderRowOffset = 0; int taskHeaderColumnOffset = 2;
  int metricHeaderRowOffset = 2; int metricHeaderColumnOffset = 0;
  int metricValuesRowOffset = 2; int metricValuesColumnOffset = 2;
  int taskScoresRowOffset = numMetrics + 3; int taskScoresColumnOffset = 2;
  int metricScoresRowOffset = 2; int metricScoresColumnOffset = numTasks + 3;
  int overallScoreRowOffset = numMetrics + 3; int overallScoreColumnOffset = numTasks + 3;
  
  d->AssessmentTable->setRowCount( numMetrics + 4 );
  d->AssessmentTable->setColumnCount( numTasks + 4 );
  
  // Note: This assumes that all tables have the same order
  // This needs to be enforced by the functions using this widget
  
  // Task headers
  int taskColumnCount = 0;
  for ( int columnIndex = 0; columnIndex < headerTable->GetNumberOfColumns(); columnIndex++ )
  {
    std::string columnName = headerTable->GetColumnName( columnIndex );
    if ( columnName.compare( "MetricName" ) == 0 || columnName.compare( "MetricRoles" ) == 0 || columnName.compare( "MetricUnit" ) == 0 )
    {
      continue;
    }
    QTableWidgetItem* taskHeaderItem = new QTableWidgetItem( columnName.c_str() );
    taskHeaderItem->setTextAlignment( Qt::AlignCenter );
    d->AssessmentTable->setItem( taskHeaderRowOffset, taskHeaderColumnOffset + taskColumnCount, taskHeaderItem );

    taskColumnCount++;
  }
  
  // Metric headers
  for ( int rowIndex = 0; rowIndex < headerTable->GetNumberOfRows(); rowIndex++ )
  {
    QString metricHeaderString;
    metricHeaderString.append( this->MetricsNode->GetTable()->GetValueByName( rowIndex, "MetricName" ).ToString() );

    metricHeaderString.append( " [" );
    metricHeaderString.append( this->MetricsNode->GetTable()->GetValueByName( rowIndex, "MetricRoles" ).ToString() );
    metricHeaderString.append( "]" );

    metricHeaderString.append( " (" );
    metricHeaderString.append( this->MetricsNode->GetTable()->GetValueByName( rowIndex, "MetricUnit" ).ToString() );
    metricHeaderString.append( ")" );
    
    QTableWidgetItem* metricHeaderItem = new QTableWidgetItem( metricHeaderString );
    d->AssessmentTable->setItem( rowIndex + metricHeaderRowOffset, metricHeaderColumnOffset, metricHeaderItem );
  }

  // Metric values
  taskColumnCount = 0;
  for ( int columnIndex = 0; columnIndex < headerTable->GetNumberOfColumns(); columnIndex++ )
  {
    std::string columnName = headerTable->GetColumnName( columnIndex );
    if ( columnName.compare( "MetricName" ) == 0 || columnName.compare( "MetricRoles" ) == 0 || columnName.compare( "MetricUnit" ) == 0 )
    {
      continue;
    }
    for ( int rowIndex = 0; rowIndex < headerTable->GetNumberOfRows(); rowIndex++ )
    {
      double value = 0;
      if ( metricsTable != NULL && columnIndex < metricsTable->GetNumberOfColumns() && rowIndex < metricsTable->GetNumberOfRows() )
      {
        value = metricsTable->GetValue( rowIndex, columnIndex ).ToDouble();
      }
      double weight = 0;
      if ( weightTable != NULL && columnIndex < weightTable->GetNumberOfColumns() && rowIndex < weightTable->GetNumberOfRows() )
      {
        weight = weightTable->GetValue( rowIndex, columnIndex ).ToDouble();
      }
      
      qSlicerMetricWeightWidget* metricWeightWidget = new qSlicerMetricWeightWidget();
      metricWeightWidget->setMetricValue( value );
      metricWeightWidget->setMetricWeight( weight );
      d->AssessmentTable->setCellWidget( rowIndex + metricValuesRowOffset, taskColumnCount + metricValuesColumnOffset, metricWeightWidget );
      
      connect( metricWeightWidget, SIGNAL( weightSliderChanged( double ) ), this, SLOT( onWeightSliderChanged( double ) ) );
      metricWeightWidget->setProperty( "row", QVariant( rowIndex ) );
      metricWeightWidget->setProperty( "column", QVariant( columnIndex ) );
    }
    taskColumnCount++;
  }
  
  // Metric scores
  vtkTable* metricScoreTable = NULL;
  if ( this->MetricScoreNode != NULL )
  {
    metricScoreTable = this->MetricScoreNode->GetTable();
  }
  if ( metricScoreTable != NULL )
  {
    int metricScoreColumn = metricScoreTable->GetNumberOfColumns() - 1;
    for ( int rowIndex = 0; rowIndex < metricScoreTable->GetNumberOfRows(); rowIndex++ )
    {      
      double value = metricScoreTable->GetValue( rowIndex, metricScoreColumn ).ToDouble();
      
      qSlicerMetricWeightWidget* metricWeightWidget = new qSlicerMetricWeightWidget();
      metricWeightWidget->setMetricValue( value );
      d->AssessmentTable->setCellWidget( rowIndex + metricScoresRowOffset, metricScoresColumnOffset, metricWeightWidget );
    }
  }
  
  // Task scores
  taskColumnCount = 0;
  vtkTable* taskScoreTable = NULL;
  if ( this->TaskScoreNode != NULL )
  {
    taskScoreTable = this->TaskScoreNode->GetTable();
  }
  if ( taskScoreTable != NULL )
  {
    int taskScoreRow = 0;
    for ( int columnIndex = 0; columnIndex < taskScoreTable->GetNumberOfColumns(); columnIndex++ )
    {
      std::string columnName = headerTable->GetColumnName( columnIndex );
      if ( columnName.compare( "MetricName" ) == 0 || columnName.compare( "MetricRoles" ) == 0 || columnName.compare( "MetricUnit" ) == 0 )
      {
        continue;
      }
      
      double value = taskScoreTable->GetValue( taskScoreRow, columnIndex ).ToDouble();
      
      qSlicerMetricWeightWidget* metricWeightWidget = new qSlicerMetricWeightWidget();
      metricWeightWidget->setMetricValue( value );
      d->AssessmentTable->setCellWidget( taskScoresRowOffset, taskColumnCount + taskScoresColumnOffset, metricWeightWidget );

      taskColumnCount++;
    }
  }
  
  // Overall score
  QTableWidgetItem* overallScoreItem = new QTableWidgetItem( QString::number( this->OverallScore ) );
  overallScoreItem->setTextAlignment( Qt::AlignCenter );
  d->AssessmentTable->setItem( overallScoreRowOffset, overallScoreColumnOffset, overallScoreItem );
  
  // Some stretching to make the table look nicer
  d->AssessmentTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  d->AssessmentTable->verticalHeader()->setResizeMode( QHeaderView::Stretch );

  d->AssessmentTable->horizontalHeader()->setResizeMode( 1, QHeaderView::ResizeToContents );
  d->AssessmentTable->horizontalHeader()->setResizeMode( d->AssessmentTable->columnCount() - 2, QHeaderView::ResizeToContents );

  d->AssessmentTable->verticalHeader()->setResizeMode( 1, QHeaderView::ResizeToContents );
  d->AssessmentTable->verticalHeader()->setResizeMode( d->AssessmentTable->rowCount() - 2, QHeaderView::ResizeToContents );
}
