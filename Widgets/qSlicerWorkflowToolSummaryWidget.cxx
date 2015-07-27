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
#include "qSlicerWorkflowToolSummaryWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerWorkflowToolSummaryWidgetPrivate
  : public Ui_qSlicerWorkflowToolSummaryWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkflowToolSummaryWidget);
protected:
  qSlicerWorkflowToolSummaryWidget* const q_ptr;

public:
  qSlicerWorkflowToolSummaryWidgetPrivate( qSlicerWorkflowToolSummaryWidget& object);
  ~qSlicerWorkflowToolSummaryWidgetPrivate();
  virtual void setupUi(qSlicerWorkflowToolSummaryWidget*);
};

// --------------------------------------------------------------------------
qSlicerWorkflowToolSummaryWidgetPrivate
::qSlicerWorkflowToolSummaryWidgetPrivate( qSlicerWorkflowToolSummaryWidget& object) : q_ptr(&object)
{
}

qSlicerWorkflowToolSummaryWidgetPrivate
::~qSlicerWorkflowToolSummaryWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerWorkflowToolSummaryWidgetPrivate
::setupUi(qSlicerWorkflowToolSummaryWidget* widget)
{
  this->Ui_qSlicerWorkflowToolSummaryWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWorkflowToolSummaryWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkflowToolSummaryWidget
::qSlicerWorkflowToolSummaryWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerWorkflowToolSummaryWidgetPrivate(*this) )
{
  this->WorkflowSegmentationNode = NULL;
  this->WorkflowSegmentationLogic = vtkSlicerWorkflowSegmentationLogic::SafeDownCast( PerkTutorCommon::GetSlicerModuleLogic( "WorkflowSegmentation" ) );
  this->setup();
}


qSlicerWorkflowToolSummaryWidget
::~qSlicerWorkflowToolSummaryWidget()
{
}


void qSlicerWorkflowToolSummaryWidget
::setup()
{
  Q_D(qSlicerWorkflowToolSummaryWidget);

  d->setupUi(this);

  connect( d->WorkflowToolsComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( onToolSelectionsChanged() ) );
  
  connect( d->TrainButton, SIGNAL( clicked() ), this, SLOT( onTrainButtonClicked() ) );

  this->updateWidgetFromMRML();  
}


void qSlicerWorkflowToolSummaryWidget
::setWorkflowSegmentationNode( vtkMRMLNode* newWorkflowSegmentationNode )
{
  Q_D(qSlicerWorkflowToolSummaryWidget);

  this->qvtkDisconnectAll();

  this->WorkflowSegmentationNode = vtkMRMLWorkflowSegmentationNode::SafeDownCast( newWorkflowSegmentationNode );

  this->qvtkConnect( this->WorkflowSegmentationNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidgetFromMRML() ) );

  this->updateWidgetFromMRML();
}



void qSlicerWorkflowToolSummaryWidget
::onToolSelectionsChanged()
{
  Q_D(qSlicerWorkflowToolSummaryWidget);

  if ( this->WorkflowSegmentationNode == NULL )
  {
    return;
  }
  
  std::vector< std::string > selectedToolIDs;
  
  QList< vtkMRMLNode* > selectedTools = d->WorkflowToolsComboBox->checkedNodes();
  QList< vtkMRMLNode* >::iterator itr;
  for ( itr = selectedTools.begin(); itr != selectedTools.end(); itr++ )
  {
    selectedToolIDs.push_back( (*itr)->GetID() );
  }
  
  this->WorkflowSegmentationNode->SetToolIDs( selectedToolIDs );

}


void qSlicerWorkflowToolSummaryWidget
::onTrainButtonClicked()
{
  Q_D(qSlicerWorkflowToolSummaryWidget);

  if ( this->WorkflowSegmentationNode == NULL )
  {
    return;
  }

  std::vector< std::string > trainingBufferIDs;
  
  QList< vtkMRMLNode* > trainingBuffers = d->TrainingBufferComboBox->checkedNodes();
  QList< vtkMRMLNode* >::iterator itr;
  for ( itr = trainingBuffers.begin(); itr != trainingBuffers.end(); itr++ )
  {
    trainingBufferIDs.push_back( (*itr)->GetID() );
  }

  this->WorkflowSegmentationLogic->TrainAllTools( this->WorkflowSegmentationNode, trainingBufferIDs );
}


void qSlicerWorkflowToolSummaryWidget
::updateWidgetFromMRML()
{
  Q_D(qSlicerWorkflowToolSummaryWidget);
  
  // Set up the table
  d->WorkflowToolsTable->clear();
  d->WorkflowToolsTable->setRowCount( 0 );
  d->WorkflowToolsTable->setColumnCount( 0 );

  if ( this->WorkflowSegmentationNode == NULL )
  {
    return;
  }
  
  std::vector< std::string > toolStatusStrings = this->WorkflowSegmentationLogic->GetToolStatusStrings( this->WorkflowSegmentationNode );
  
  QStringList WorkflowToolsTableHeaders;
  WorkflowToolsTableHeaders << "Tool";
  d->WorkflowToolsTable->setRowCount( toolStatusStrings.size() );
  d->WorkflowToolsTable->setColumnCount( 1 );
  d->WorkflowToolsTable->setHorizontalHeaderLabels( WorkflowToolsTableHeaders );
  d->WorkflowToolsTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  
  // Add the computed values to the table
  for ( int i = 0; i < toolStatusStrings.size(); i++ )
  {
    QTableWidgetItem* statusItem = new QTableWidgetItem( QString::fromStdString( toolStatusStrings.at( i ) ) );
    d->WorkflowToolsTable->setItem( i, 0, statusItem );
  }

}
