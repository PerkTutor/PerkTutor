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
  this->PerkEvaluatorLogic = vtkSlicerPerkEvaluatorLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "PerkEvaluator" ) );
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
  // If it is a new table node, then the onTransformBufferNodeChanged will be called automatically
}


vtkMRMLTableNode* qSlicerMetricsTableWidget
::addMetricsTableNode()
{
  Q_D(qSlicerMetricsTableWidget);

  return vtkMRMLTableNode::SafeDownCast( d->MetricsTableNodeComboBox->addNode() ); // Automatically calls "onMetricsTableNodeChanged" function
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

  // Grab all of the contents from whatever is currently on the metrics table
  QString clipString = QString( "" );

  for ( int i = 0; i < this->MetricsTableNode->GetTable()->GetNumberOfRows(); i++ )
  {
    clipString.append( this->MetricsTableNode->GetTable()->GetValueByName( i, "TransformName" ).ToString() );
    clipString.append( " " );
    clipString.append( this->MetricsTableNode->GetTable()->GetValueByName( i, "MetricName" ).ToString() );
    clipString.append( " (" );
    clipString.append( this->MetricsTableNode->GetTable()->GetValueByName( i, "MetricUnit" ).ToString() );
    clipString.append( ") " );

    clipString.append( "\t" );
    clipString.append( this->MetricsTableNode->GetTable()->GetValueByName( i, "MetricValue" ).ToString() );
    clipString.append( "\n" );
  }

  QApplication::clipboard()->setText( clipString );
}


void qSlicerMetricsTableWidget
::updateWidget()
{
  Q_D(qSlicerMetricsTableWidget);

  d->MetricsTableNodeComboBox->setCurrentNode( this->MetricsTableNode );

  // Set up the table
  d->MetricsTable->clear();
  d->MetricsTable->setRowCount( 0 );
  d->MetricsTable->setColumnCount( 0 );

  if ( this->MetricsTableNode == NULL )
  {
    return;
  }

  QStringList MetricsTableHeaders;
  MetricsTableHeaders << "Metric" << "Value";
  d->MetricsTable->setRowCount( this->MetricsTableNode->GetTable()->GetNumberOfRows() );
  d->MetricsTable->setColumnCount( 2 );
  d->MetricsTable->setHorizontalHeaderLabels( MetricsTableHeaders );
  d->MetricsTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  
  // Add the computed values to the table
  for ( int i = 0; i < this->MetricsTableNode->GetTable()->GetNumberOfRows(); i++ )
  {
    QString nameString;
    nameString.append( this->MetricsTableNode->GetTable()->GetValueByName( i, "TransformName" ).ToString() );
    nameString.append( " " );
    nameString.append( this->MetricsTableNode->GetTable()->GetValueByName( i, "MetricName" ).ToString() );
    QTableWidgetItem* nameItem = new QTableWidgetItem( nameString );
    d->MetricsTable->setItem( i, 0, nameItem );

    QString valueString;
    valueString.append( this->MetricsTableNode->GetTable()->GetValueByName( i, "MetricValue" ).ToString() );
    valueString.append( " " );
    valueString.append( this->MetricsTableNode->GetTable()->GetValueByName( i, "MetricUnit" ).ToString() );
    QTableWidgetItem* valueItem = new QTableWidgetItem( valueString );    
    d->MetricsTable->setItem( i, 1, valueItem );
  }
}
