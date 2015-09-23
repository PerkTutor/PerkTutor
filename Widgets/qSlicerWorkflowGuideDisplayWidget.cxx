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
#include "qSlicerWorkflowGuideDisplayWidget.h"

#include "vtkSlicerWorkflowSegmentationLogic.h"

#include <QtGui>
#include <QFont>

#include <sstream>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerWorkflowGuideDisplayWidgetPrivate
  : public Ui_qSlicerWorkflowGuideDisplayWidget
{
  Q_DECLARE_PUBLIC(qSlicerWorkflowGuideDisplayWidget);
protected:
  qSlicerWorkflowGuideDisplayWidget* const q_ptr;

public:
  qSlicerWorkflowGuideDisplayWidgetPrivate( qSlicerWorkflowGuideDisplayWidget& object);
  ~qSlicerWorkflowGuideDisplayWidgetPrivate();
  virtual void setupUi(qSlicerWorkflowGuideDisplayWidget*);
};

// --------------------------------------------------------------------------
qSlicerWorkflowGuideDisplayWidgetPrivate
::qSlicerWorkflowGuideDisplayWidgetPrivate( qSlicerWorkflowGuideDisplayWidget& object) : q_ptr(&object)
{
}

qSlicerWorkflowGuideDisplayWidgetPrivate
::~qSlicerWorkflowGuideDisplayWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerWorkflowGuideDisplayWidgetPrivate
::setupUi(qSlicerWorkflowGuideDisplayWidget* widget)
{
  this->Ui_qSlicerWorkflowGuideDisplayWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerWorkflowGuideDisplayWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkflowGuideDisplayWidget
::qSlicerWorkflowGuideDisplayWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerWorkflowGuideDisplayWidgetPrivate(*this) )
{
  this->WorkflowToolNode = NULL;
  this->DefaultParentWidget = parentWidget;
  
  this->setup();
}


qSlicerWorkflowGuideDisplayWidget
::~qSlicerWorkflowGuideDisplayWidget()
{
}


void qSlicerWorkflowGuideDisplayWidget
::setup()
{
  Q_D(qSlicerWorkflowGuideDisplayWidget);

  d->setupUi(this);
  
  connect( d->PopButton, SIGNAL( clicked() ), this, SLOT( onPopButtonClicked() ) );
  d->PopButton->setIcon( QApplication::style()->standardIcon( QStyle::SP_TitleBarNormalButton ) );

  this->updateWidgetFromMRML();  
}


vtkMRMLWorkflowToolNode* qSlicerWorkflowGuideDisplayWidget
::getWorkflowToolNode()
{
  Q_D(qSlicerWorkflowGuideDisplayWidget);

  return this->WorkflowToolNode;
}


void qSlicerWorkflowGuideDisplayWidget
::setWorkflowToolNode( vtkMRMLNode* newWorkflowToolNode )
{
  Q_D(qSlicerWorkflowGuideDisplayWidget);

  this->qvtkDisconnectAll();

  this->WorkflowToolNode = vtkMRMLWorkflowToolNode::SafeDownCast( newWorkflowToolNode );

  this->qvtkConnect( this->WorkflowToolNode, vtkMRMLWorkflowToolNode::CurrentTaskChangedEvent, this, SLOT( updateWidgetFromMRML() ) );
  this->qvtkConnect( this->WorkflowToolNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidgetFromMRML() ) );

  this->updateWidgetFromMRML();
}


void qSlicerWorkflowGuideDisplayWidget
::onPopButtonClicked()
{
  Q_D(qSlicerWorkflowGuideDisplayWidget);

  if ( this->parentWidget() == this->DefaultParentWidget )
  {
    this->setParent( NULL );
    this->show();
  }
  else
  {
    this->setParent( this->DefaultParentWidget );
    this->DefaultParentWidget->layout()->addWidget( this );
    this->show();
  }

}


void qSlicerWorkflowGuideDisplayWidget
::updateWidgetFromMRML()
{
  Q_D(qSlicerWorkflowGuideDisplayWidget);

  // Clear the table
  d->WorkflowDisplayTable->clear();
  d->WorkflowDisplayTable->setRowCount( 0 );
  d->WorkflowDisplayTable->setColumnCount( 0 );  
  
  if ( this->WorkflowToolNode == NULL )
  {
    return;
  }

  std::stringstream columnText;
  columnText << this->WorkflowToolNode->GetName() << " (" << this->WorkflowToolNode->GetToolName() << ")";
  d->ToolName->setText( QString::fromStdString( columnText.str() ) );

  // Grab the tasks in order
  vtkSlicerWorkflowSegmentationLogic* wsLogic = vtkSlicerWorkflowSegmentationLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "WorkflowSegmentation" ) );
  std::vector< std::string > orderedTaskStrings = wsLogic->GetOrderedWorkflowTaskStrings( this->WorkflowToolNode );

  QStringList WorkflowDisplayTableHeaders;
  WorkflowDisplayTableHeaders << "Workflow";
  d->WorkflowDisplayTable->setRowCount( orderedTaskStrings.size() );
  d->WorkflowDisplayTable->setColumnCount( 1 );
  d->WorkflowDisplayTable->setHorizontalHeaderLabels( WorkflowDisplayTableHeaders );
  d->WorkflowDisplayTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  
  // Add the computed values to the table
  for ( int i = 0; i < orderedTaskStrings.size(); i++ )
  {
    vtkWorkflowTask* checkTask = this->WorkflowToolNode->GetWorkflowProcedureNode()->GetTask( orderedTaskStrings.at( i ) );

    std::stringstream taskString;
    taskString << checkTask->GetName() << ": " << checkTask->GetInstruction() << "."; 
    QTableWidgetItem* taskItem = new QTableWidgetItem( QString::fromStdString( taskString.str() ) );

    QFont taskFont;
    if ( checkTask == this->WorkflowToolNode->GetCurrentTask() )
    {
      taskFont.setBold( true );
      d->WorkflowDisplayTable->selectRow( i );
    }

    taskItem->setFont( taskFont );
    d->WorkflowDisplayTable->setItem( i, 0, taskItem );
  }

}
