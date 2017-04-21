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

#include <vtkNew.h>


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
  this->MetricNode = NULL;
  this->WeightNode = NULL;
  this->MetricScoreNode = NULL;
  this->TaskScoreNode = NULL;
  
  this->OverallScore = 0;

  this->ShowMetricWeights = true;
  this->ShowScoreWeights = true;
  
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
::getHeaderTable( vtkTable* headerTable )
{
  Q_D(qSlicerAssessmentTableWidget);

  if ( this->WeightNode != NULL && this->WeightNode->GetTable() != NULL )
  {
    headerTable->DeepCopy( this->WeightNode->GetTable() );
  }
  if ( this->MetricNode != NULL && this->MetricNode->GetTable() != NULL )
  {
    headerTable->DeepCopy( this->MetricNode->GetTable() );
  }
}


void qSlicerAssessmentTableWidget
::setMetricNode( vtkMRMLNode* table )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->qvtkDisconnect( this->MetricNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->MetricNode = vtkMRMLTableNode::SafeDownCast( table );
  this->qvtkConnect( this->MetricNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateWidget();
}


void qSlicerAssessmentTableWidget
::setWeightNode( vtkMRMLNode* table )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->qvtkDisconnect( this->WeightNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->WeightNode = vtkMRMLTableNode::SafeDownCast( table );
  this->qvtkConnect( this->WeightNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateWidget();
}


void qSlicerAssessmentTableWidget
::setMetricScoreNode( vtkMRMLNode* table )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->qvtkDisconnect( this->MetricScoreNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->MetricScoreNode = vtkMRMLTableNode::SafeDownCast( table );
  this->qvtkConnect( this->MetricScoreNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateMetricScores();
}


void qSlicerAssessmentTableWidget
::setTaskScoreNode( vtkMRMLNode* table )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->qvtkDisconnect( this->TaskScoreNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->TaskScoreNode = vtkMRMLTableNode::SafeDownCast( table );
  this->qvtkConnect( this->TaskScoreNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateTaskScores();
}


void qSlicerAssessmentTableWidget
::setOverallScore( double score )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->OverallScore = score;

  this->updateOverallScore();
}


void qSlicerAssessmentTableWidget
::setMetricWeightsVisible( bool visible )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->ShowMetricWeights = visible;
}


void qSlicerAssessmentTableWidget
::setScoreWeightsVisible( bool visible )
{
  Q_D(qSlicerAssessmentTableWidget);

  this->ShowScoreWeights = visible;
}


void qSlicerAssessmentTableWidget
::onWeightSliderChanged( double weight )
{
  Q_D(qSlicerAssessmentTableWidget);
  
  if ( this->WeightNode == NULL || this->WeightNode->GetTable() == NULL )
  {
    return;
  }

  QObject* slider = sender();
  int rowIndex = slider->property( "row" ).toInt();
  int columnIndex = slider->property( "column" ).toInt();
  
  this->WeightNode->GetTable()->SetValue( rowIndex, columnIndex, weight );
}


void qSlicerAssessmentTableWidget
::updateHeaders()
{
  Q_D(qSlicerAssessmentTableWidget);

  // Get the header table
  vtkNew< vtkTable > headerTable;
  this->getHeaderTable( headerTable.GetPointer() );
  if ( headerTable->GetNumberOfRows() == 0 || headerTable->GetNumberOfColumns() == 0 )
  {
    qWarning( "qSlicerAssessmentTableWidget::updateTableHeaders: Metrics and weight tables improperly specified." );
    return;
  }

  // The grid positions
  int numMetrics = headerTable->GetNumberOfRows();
  int numTasks = headerTable->GetNumberOfColumns() - 3; // Ignore "MetricName", "MetricRoles", "MetricUnit"
  
  d->AssessmentTable->setRowCount( numMetrics + 4 );
  d->AssessmentTable->setColumnCount( numTasks + 4 );

  int taskHeaderColumnOffset = 2;
  int metricHeaderRowOffset = 2;

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
    d->AssessmentTable->setItem( 0, taskHeaderColumnOffset + taskColumnCount, taskHeaderItem );

    taskColumnCount++;
  }
  
  // Metric headers
  for ( int rowIndex = 0; rowIndex < headerTable->GetNumberOfRows(); rowIndex++ )
  {
    QString metricHeaderString;
    metricHeaderString.append( headerTable->GetValueByName( rowIndex, "MetricName" ).ToString() );

    metricHeaderString.append( " [" );
    metricHeaderString.append( headerTable->GetValueByName( rowIndex, "MetricRoles" ).ToString() );
    metricHeaderString.append( "]" );

    metricHeaderString.append( " (" );
    metricHeaderString.append( headerTable->GetValueByName( rowIndex, "MetricUnit" ).ToString() );
    metricHeaderString.append( ")" );
    
    QTableWidgetItem* metricHeaderItem = new QTableWidgetItem( metricHeaderString );
    d->AssessmentTable->setItem( rowIndex + metricHeaderRowOffset, 0, metricHeaderItem );
  }

}


void qSlicerAssessmentTableWidget
::updateMetrics()
{
  Q_D(qSlicerAssessmentTableWidget);

  // Ensure this table and the header table are compatible
  vtkTable* metricTable = NULL;
  if ( this->MetricNode != NULL )
  {
    metricTable = this->MetricNode->GetTable();
  }
  vtkTable* weightTable = NULL;
  if ( this->WeightNode != NULL )
  {
    weightTable = this->WeightNode->GetTable();
  }

  vtkNew< vtkTable > headerTable;
  this->getHeaderTable( headerTable.GetPointer() );

  bool metricTableCompatible = metricTable != NULL
    && metricTable->GetNumberOfRows() == headerTable->GetNumberOfRows()
    && metricTable->GetNumberOfColumns() == headerTable->GetNumberOfColumns();
  bool weightTableCompatible = weightTable != NULL
    && weightTable->GetNumberOfRows() == headerTable->GetNumberOfRows()
    && weightTable->GetNumberOfColumns() == headerTable->GetNumberOfColumns();

  int metricRowOffset = 2;
  int metricColumnOffset = 2;

  // Put the metric values and weights into the table
  int taskColumnCount = 0;
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
      if ( metricTableCompatible )
      {
        value = metricTable->GetValue( rowIndex, columnIndex ).ToDouble();
      }
      double weight = 0;
      if ( weightTableCompatible )
      {
        weight = weightTable->GetValue( rowIndex, columnIndex ).ToDouble();
      }
      
      qSlicerMetricWeightWidget* metricWeightWidget = new qSlicerMetricWeightWidget();
      metricWeightWidget->setMetric( value );
      metricWeightWidget->setWeight( weight );
      metricWeightWidget->setWeightVisible( this->ShowMetricWeights );
      d->AssessmentTable->setCellWidget( rowIndex + metricRowOffset, taskColumnCount + metricColumnOffset, metricWeightWidget );
      
      connect( metricWeightWidget, SIGNAL( weightSliderChanged( double ) ), this, SLOT( onWeightSliderChanged( double ) ) );
      metricWeightWidget->setProperty( "row", QVariant( rowIndex ) );
      metricWeightWidget->setProperty( "column", QVariant( columnIndex ) );
    }
    taskColumnCount++;
  }

}


void qSlicerAssessmentTableWidget
::updateMetricScores()
{
  Q_D(qSlicerAssessmentTableWidget);

  // Ensure this table and the header table are compatible
  vtkTable* metricScoreTable = NULL;
  if ( this->MetricScoreNode != NULL )
  {
    metricScoreTable = this->MetricScoreNode->GetTable();
  }

  vtkNew< vtkTable > headerTable;
  this->getHeaderTable( headerTable.GetPointer() );

  if ( metricScoreTable == NULL
    || metricScoreTable->GetNumberOfRows() != headerTable->GetNumberOfRows()
    || metricScoreTable->GetNumberOfColumns() != 4 )
  {
    qWarning( "qSlicerAssessmentTableWidget::updateMetricScores: Metric score table improperly specified." );
    return;
  }

  int metricScoreRowOffset = 2;
  int metricScoreColumn = metricScoreTable->GetNumberOfColumns() - 1;

  for ( int rowIndex = 0; rowIndex < metricScoreTable->GetNumberOfRows(); rowIndex++ )
  {      
    double value = metricScoreTable->GetValue( rowIndex, metricScoreColumn ).ToDouble();
      
    qSlicerMetricWeightWidget* metricWeightWidget = new qSlicerMetricWeightWidget();
    metricWeightWidget->setMetric( value );
    metricWeightWidget->setWeightVisible( this->ShowScoreWeights );
    d->AssessmentTable->setCellWidget( rowIndex + metricScoreRowOffset, d->AssessmentTable->columnCount() - 1, metricWeightWidget );
  }

}


void qSlicerAssessmentTableWidget
::updateTaskScores()
{
  Q_D(qSlicerAssessmentTableWidget);

  // Ensure this table and the header table are compatible
  vtkTable* taskScoreTable = NULL;
  if ( this->TaskScoreNode != NULL )
  {
    taskScoreTable = this->TaskScoreNode->GetTable();
  }

  vtkNew< vtkTable > headerTable;
  this->getHeaderTable( headerTable.GetPointer() );

  if ( taskScoreTable == NULL
    || taskScoreTable->GetNumberOfColumns() != headerTable->GetNumberOfColumns()
    || taskScoreTable->GetNumberOfRows() != 1 )
  {
    qWarning( "qSlicerAssessmentTableWidget::updateTaskScores: Task score table improperly specified." );
    return;
  }

  int taskScoreColumnOffset = 2;
  int taskScoreRow = 0;
  int taskColumnCount = 0;

  for ( int columnIndex = 0; columnIndex < taskScoreTable->GetNumberOfColumns(); columnIndex++ )
  {
    std::string columnName = headerTable->GetColumnName( columnIndex );
    if ( columnName.compare( "MetricName" ) == 0 || columnName.compare( "MetricRoles" ) == 0 || columnName.compare( "MetricUnit" ) == 0 )
    {
      continue;
    }
      
    double value = taskScoreTable->GetValue( taskScoreRow, columnIndex ).ToDouble();
  
    qSlicerMetricWeightWidget* metricWeightWidget = new qSlicerMetricWeightWidget();
    metricWeightWidget->setMetric( value );
    metricWeightWidget->setWeightVisible( this->ShowScoreWeights );
    d->AssessmentTable->setCellWidget( d->AssessmentTable->rowCount() - 1, taskColumnCount + taskScoreColumnOffset, metricWeightWidget );

    taskColumnCount++;
  }

}


void qSlicerAssessmentTableWidget
::updateOverallScore()
{
  Q_D(qSlicerAssessmentTableWidget);

  QTableWidgetItem* overallScoreItem = new QTableWidgetItem( QString::number( this->OverallScore ) );
  overallScoreItem->setTextAlignment( Qt::AlignCenter );
  d->AssessmentTable->setItem( d->AssessmentTable->rowCount() - 1, d->AssessmentTable->columnCount() - 1, overallScoreItem );
}


void qSlicerAssessmentTableWidget
::updateWidget()
{
  Q_D(qSlicerAssessmentTableWidget);

  // Set up the table
  d->AssessmentTable->clear();
  d->AssessmentTable->setRowCount( 0 );

  this->updateHeaders();
  this->updateMetrics();
  this->updateMetricScores();
  this->updateTaskScores();
  this->updateOverallScore();

  // Some stretching to make the table look nicer
  d->AssessmentTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  d->AssessmentTable->verticalHeader()->setResizeMode( QHeaderView::Stretch );

  if ( d->AssessmentTable->columnCount() > 1 )
  {  
    d->AssessmentTable->horizontalHeader()->setResizeMode( 1, QHeaderView::ResizeToContents );
    d->AssessmentTable->horizontalHeader()->setResizeMode( d->AssessmentTable->columnCount() - 2, QHeaderView::ResizeToContents );
  }

  if ( d->AssessmentTable->rowCount() > 1 )
  {
    d->AssessmentTable->verticalHeader()->setResizeMode( 1, QHeaderView::ResizeToContents );
    d->AssessmentTable->verticalHeader()->setResizeMode( d->AssessmentTable->rowCount() - 2, QHeaderView::ResizeToContents );
  }
}
