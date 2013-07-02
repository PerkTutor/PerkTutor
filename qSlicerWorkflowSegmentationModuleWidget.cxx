

/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/
#include <iostream>
// Qt includes

// SlicerQt includes
#include "qSlicerWorkflowSegmentationModuleWidget.h"
#include "ui_qSlicerWorkflowSegmentationModule.h"
#include "qSlicerIO.h"
#include "qSlicerIOManager.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLThreeDWidget.h"
#include <QtGui>

// MRMLWidgets includes
#include <qMRMLUtils.h>


#include "vtkMRMLWorkflowSegmentationNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "qMRMLNodeComboBox.h"
#include "vtkMRMLViewNode.h"
#include "vtkSlicerWorkflowSegmentationLogic.h"
#include "vtkMRMLWorkflowSegmentationNode.h"
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_WorkflowSegmentation
class qSlicerWorkflowSegmentationModuleWidgetPrivate: public Ui_qSlicerWorkflowSegmentationModule
{
  Q_DECLARE_PUBLIC( qSlicerWorkflowSegmentationModuleWidget ); 

protected:
  qSlicerWorkflowSegmentationModuleWidget* const q_ptr;
public:
  qSlicerWorkflowSegmentationModuleWidgetPrivate( qSlicerWorkflowSegmentationModuleWidget& object );
  ~qSlicerWorkflowSegmentationModuleWidgetPrivate();

  vtkSlicerWorkflowSegmentationLogic* logic() const;
};

//-----------------------------------------------------------------------------
// qSlicerWorkflowSegmentationModuleWidgetPrivate methods



qSlicerWorkflowSegmentationModuleWidgetPrivate::qSlicerWorkflowSegmentationModuleWidgetPrivate( qSlicerWorkflowSegmentationModuleWidget& object ) : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------

qSlicerWorkflowSegmentationModuleWidgetPrivate::~qSlicerWorkflowSegmentationModuleWidgetPrivate()
{
}


vtkSlicerWorkflowSegmentationLogic* qSlicerWorkflowSegmentationModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerWorkflowSegmentationModuleWidget );
  return vtkSlicerWorkflowSegmentationLogic::SafeDownCast( q->logic() );
}


//-----------------------------------------------------------------------------
// qSlicerWorkflowSegmentationModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkflowSegmentationModuleWidget::qSlicerWorkflowSegmentationModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerWorkflowSegmentationModuleWidgetPrivate( *this ) )
{
}



qSlicerWorkflowSegmentationModuleWidget::~qSlicerWorkflowSegmentationModuleWidget()
{
}



void qSlicerWorkflowSegmentationModuleWidget::setup()
{
  Q_D(qSlicerWorkflowSegmentationModuleWidget);

  d->setupUi(this);
  d->ControlsGroupBox->layout()->addWidget( qSlicerWorkflowSegmentationRecorderControlsWidget::New( d->logic() ) );
  d->MessagesGroupBox->layout()->addWidget( qSlicerMessagesWidget::New( d->logic()->TransformRecorderLogic ) ); 
  this->Superclass::setup();

  // Module node selection
  d->ModuleComboBox->setNoneEnabled( true );
  connect( d->ModuleComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModuleNodeSelected() ) );

  // Import parameters and files
  connect( d->WorkflowProcedureButton, SIGNAL(clicked()), this, SLOT( onWorkflowProcedureButtonClicked() ) );
  connect( d->WorkflowInputButton, SIGNAL(clicked()), this, SLOT( onWorkflowInputButtonClicked() ) );
  connect( d->WorkflowTrainingButton, SIGNAL(clicked()), this, SLOT( onWorkflowTrainingButtonClicked() ) );
  connect( d->WorkflowTrainingFilesButton, SIGNAL(clicked()), this, SLOT( onWorkflowTrainingFilesButtonClicked() ) );

  // Train algorithm
  connect( d->TrainButton, SIGNAL(clicked()), this, SLOT( onTrainButtonClicked() ) );

  // Recording controls
  connect( d->SegmentTransformBufferButton, SIGNAL(clicked()), this, SLOT( onSegmentTransformBufferButtonClicked() ) );

  // Setup the display of instructions
  this->setupInstructions();

  // GUI refresh: updates every 10ms
  QTimer *t = new QTimer( this );
  connect( t,  SIGNAL( timeout() ), this, SLOT( updateWidget() ) );
  t->start(10); 

}



void qSlicerWorkflowSegmentationModuleWidget::enter()
{
  this->Superclass::enter();
  this->InstructionLabel->show();
  this->updateWidget();
}


void qSlicerWorkflowSegmentationModuleWidget::exit()
{
  this->InstructionLabel->hide();
  this->Superclass::exit();
}



void qSlicerWorkflowSegmentationModuleWidget::onModuleNodeSelected()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
 
  vtkMRMLNode* currentNode = d->ModuleComboBox->currentNode();
  std::cout << "Current node pointer: " << currentNode << std::endl;
  
  vtkMRMLWorkflowSegmentationNode* WSNode = vtkMRMLWorkflowSegmentationNode::SafeDownCast( currentNode );

  if ( WSNode != NULL )
  {
    d->logic()->SetModuleNode( WSNode );
	d->logic()->ResetWorkflowAlgorithms();
  }

  this->updateWidget();
}




void qSlicerWorkflowSegmentationModuleWidget
::onWorkflowProcedureButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  QString fileName = QFileDialog::getOpenFileName( this, tr("Open procedure definition"), "", tr("XML Files (*.xml)") );
  
  if ( fileName.isEmpty() == false )
  {
    d->logic()->ImportWorkflowProcedure( fileName.toStdString() );
  }
  
  this->updateWidget();
}





void qSlicerWorkflowSegmentationModuleWidget
::onWorkflowInputButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  QString fileName = QFileDialog::getOpenFileName( this, tr("Open input parameters"), "", tr("XML Files (*.xml)") );
  
  if ( fileName.isEmpty() == false )
  {
    d->logic()->ImportWorkflowInput( fileName.toStdString() );
  }
  
  this->updateWidget();
}




void qSlicerWorkflowSegmentationModuleWidget
::onWorkflowTrainingButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  QString fileName = QFileDialog::getOpenFileName( this, tr("Open training parameters"), "", tr("XML Files (*.xml)") );
  
  if ( fileName.isEmpty() == false )
  {
	d->logic()->ImportWorkflowTraining( fileName.toStdString() );
    d->logic()->ResetWorkflowAlgorithms();
  }
  
  this->updateWidget();
}





void qSlicerWorkflowSegmentationModuleWidget
::onWorkflowTrainingFilesButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  QStringList files = QFileDialog::getOpenFileNames( this, tr("Open training files"), "", "XML Files (*.xml)" );

  if ( ! files.isEmpty() )
  {
    QProgressDialog dialog;
    dialog.setModal( true );
    dialog.setLabelText( "Please wait while reading XML files..." );
    dialog.show();
    dialog.setValue( 10 );

    // Add the buffers from each of the files to the workflow algorithms.
	d->logic()->ResetWorkflowAlgorithms();
    for( int i = 0; i < files.size(); i++ )
    {    
      d->logic()->AddTrainingBuffer( files.at(i).toStdString() );
    }

    dialog.close();
  }
  
  this->updateWidget();
}





void qSlicerWorkflowSegmentationModuleWidget
::onSegmentTransformBufferButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  QString fileName = QFileDialog::getOpenFileName( this, tr("Open tracking log"), "", "XML Files (*.xml)" );

  if ( fileName.isEmpty() == false )
  {    
    QProgressDialog dialog;
    dialog.setModal( true );
    dialog.setLabelText( "Please wait while reading XML file..." );
    dialog.show();
    dialog.setValue( 10 );

    d->logic()->SegmentBuffer( fileName.toStdString() );

    dialog.close(); 
  }
  
  this->updateWidget();
}






void qSlicerWorkflowSegmentationModuleWidget
::onTrainButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );

  QString fileName = QFileDialog::getSaveFileName( this, tr("Save training"), "", tr("XML Files (*.xml)") );

  // TODO: Make this time estimate accurate
  QProgressDialog dialog;
  dialog.setModal( true );
  dialog.setLabelText( "Please wait while training algorithm... Expected Time: 10 mins" );
  dialog.show();
  dialog.setValue( 30 );
  
  d->logic()->Train();

  dialog.close();

  if ( fileName.isEmpty() == false )
  {
	d->logic()->SaveWorkflowTraining( fileName.toStdString() );
  }
  d->logic()->ResetWorkflowAlgorithms();

  this->updateWidget();
}



void qSlicerWorkflowSegmentationModuleWidget::setupInstructions()
{
  // Add the real time instructions to the 3D viewer widget
  qSlicerApplication::application()->layoutManager()->threeDWidget( 0 );
  this->InstructionLabel = new QLabel( "" );
  this->InstructionLabel->setAlignment( Qt::AlignCenter );
  this->InstructionLabel->setStyleSheet("QLabel { background-color : white; color : black; font-size : 24px }");
  qSlicerApplication::application()->layoutManager()->threeDWidget( 0 )->layout()->addWidget( this->InstructionLabel );
}


void qSlicerWorkflowSegmentationModuleWidget::enableButtons()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );

  // Disabling node selector widgets if there is no module node to reference input nodes.    
  if ( d->logic()->GetModuleNode() == NULL )
  {
	d->WorkflowProcedureButton->setEnabled( false );
	d->WorkflowInputButton->setEnabled( false );
	d->WorkflowTrainingButton->setEnabled( false );
	d->SegmentTransformBufferButton->setEnabled( false );

	d->WorkflowTrainingFilesButton->setEnabled( false );
	d->TrainButton->setEnabled( false );

	return;
  }
  else
  {
	d->WorkflowProcedureButton->setEnabled( true );

	d->SegmentTransformBufferButton->setEnabled( true );
  }
  
  // If the algorithms are procedure defined
  if ( ! d->logic()->GetWorkflowAlgorithmsDefined() )
  {    
    d->WorkflowInputButton->setEnabled( false );
  }
  else
  {
    d->WorkflowInputButton->setEnabled( true );
  }

  // If the algorithms are parameters inputted
  if ( ! d->logic()->GetWorkflowAlgorithmsInputted() )
  {
    d->WorkflowTrainingButton->setEnabled( false );
	d->WorkflowTrainingFilesButton->setEnabled( false );
	d->TrainButton->setEnabled( false );
  }
  else
  {
    d->WorkflowTrainingButton->setEnabled( true );
	d->WorkflowTrainingFilesButton->setEnabled( true );
	d->TrainButton->setEnabled( true );
  }

  // If the algorithms are trained
  if ( ! d->logic()->GetWorkflowAlgorithmsTrained() )
  {
	d->SegmentTransformBufferButton->setEnabled( false );
  }
  else
  {
	d->SegmentTransformBufferButton->setEnabled( true );
  }

}



void qSlicerWorkflowSegmentationModuleWidget::updateWidget()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );

  // All the other things are taken care of by the TransformRecord logic/widgets
  enableButtons();

  // This updates the tasks
  d->logic()->Update();

  if ( d->logic() == NULL )
  {
    return;
  }

  this->InstructionLabel->setText( d->logic()->GetToolInstructions().c_str() );

  QStringList TableHeaders;
  TableHeaders << "Tools Available";
  d->ToolsAvailableTableWidget->setRowCount( 0 );
  d->ToolsAvailableTableWidget->setColumnCount( 1 );
  d->ToolsAvailableTableWidget->setHorizontalHeaderLabels( TableHeaders ); 
  d->ToolsAvailableTableWidget->horizontalHeader()->setResizeMode( QHeaderView::Stretch );

  if ( d->logic()->GetModuleNode() == NULL )
  {
    return;
  }

  d->ToolsAvailableTableWidget->setRowCount( d->logic()->GetModuleNode()->ToolCollection->GetNumTools() );

  for ( int i = 0; i < d->logic()->GetModuleNode()->ToolCollection->GetNumTools(); i++ )
  {
    vtkWorkflowTool* currentTool = d->logic()->GetModuleNode()->ToolCollection->GetToolAt(i);
    vtkWorkflowTool* completionTool = d->logic()->GetModuleNode()->GetCompletionTool( currentTool );
    std::string currentString = "";

    currentString += currentTool->Name + " (";
	if ( ! currentTool->Inputted )
	{
      currentString += "Not ";
	}
    currentString += "Inputted, ";
	if ( ! currentTool->Trained )
	{
      currentString += "Not ";
	}
	currentString += "Trained)";

	QTableWidgetItem* toolWidget = new QTableWidgetItem( QString::fromStdString( currentString ) );
	d->ToolsAvailableTableWidget->setItem( i, 0, toolWidget );
  }

  d->ToolsAvailableTableWidget->horizontalHeader()->hide();
  d->ToolsAvailableTableWidget->resizeRowsToContents();
}
