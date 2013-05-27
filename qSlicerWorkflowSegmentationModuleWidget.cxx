

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
  d->ControlsGroupBox->layout()->addWidget( qSlicerRecorderControlsWidget::New( d->logic()->TransformRecorderLogic ) );
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

  // Save tracking log and parameters
  connect( d->SaveWorkflowTrainingButton, SIGNAL(clicked()),this, SLOT ( onSaveWorkflowTrainingButtonClicked() ) );

  // Recording controls
  connect( d->SegmentTransformBufferButton, SIGNAL(clicked()), this, SLOT( onSegmentTransformBufferButtonClicked() ) );

  // GUI refresh: updates every 10ms
  QTimer *t = new QTimer( this );
  connect( t,  SIGNAL( timeout() ), this, SLOT( updateWidget() ) );
  t->start(10); 

}



void qSlicerWorkflowSegmentationModuleWidget::enter()
{
  this->Superclass::enter();
  this->updateWidget();
}




void qSlicerWorkflowSegmentationModuleWidget::onModuleNodeSelected()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
 
  vtkMRMLNode* currentNode = d->ModuleComboBox->currentNode();
  std::cout << "Current node pointer: " << currentNode << std::endl;
  
  vtkMRMLWorkflowSegmentationNode* WSNode = vtkMRMLWorkflowSegmentationNode::SafeDownCast( currentNode );

  if ( WSNode != NULL )
  {
    d->logic()->ModuleNode = WSNode;
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
	d->logic()->ResetWorkflowAlgorithms();
	d->logic()->ImportWorkflowTraining( fileName.toStdString() );
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

    d->logic()->ResetWorkflowAlgorithms();
    d->logic()->SegmentBuffer( fileName.toStdString() );

    dialog.close(); 
  }
  
  this->updateWidget();
}






void qSlicerWorkflowSegmentationModuleWidget
::onTrainButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );

  // TODO: Make this time estimate accurate
  QProgressDialog dialog;
  dialog.setModal( true );
  dialog.setLabelText( "Please wait while training algorithm... Expected Time: 10 mins" );
  dialog.show();
  dialog.setValue( 30 );
  
  d->logic()->Train();
  d->logic()->ResetWorkflowAlgorithms();

  dialog.close();

  this->updateWidget();
}



void qSlicerWorkflowSegmentationModuleWidget
::onSaveWorkflowTrainingButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  QString fileName = QFileDialog::getSaveFileName( this, tr("Save training"), "", tr("XML Files (*.xml)") );
  
  if ( fileName.isEmpty() == false )
  {
	d->logic()->SaveWorkflowTraining( fileName.toStdString() );
  }
  
  this->updateWidget();
}



void qSlicerWorkflowSegmentationModuleWidget::enableButtons()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );

  // Disabling node selector widgets if there is no module node to reference input nodes.    
  if ( d->logic()->ModuleNode == NULL )
  {
	d->WorkflowProcedureButton->setEnabled( false );
	d->WorkflowInputButton->setEnabled( false );
	d->WorkflowTrainingButton->setEnabled( false );
	d->SegmentTransformBufferButton->setEnabled( false );

	d->WorkflowTrainingFilesButton->setEnabled( false );
	d->TrainButton->setEnabled( false );

	d->SaveWorkflowTrainingButton->setEnabled( false );
  }
  else
  {
	d->WorkflowProcedureButton->setEnabled( true );

	d->SegmentTransformBufferButton->setEnabled( true );
  }
  
  // The following code requires a module node.  
  if ( d->logic()->ModuleNode == NULL )
  {
    return;
  } 
  
  // If the algorithms are procedure defined
  if ( ! d->logic()->ToolCollection->GetDefined() )
  {    
    d->WorkflowInputButton->setEnabled( false );
  }
  else
  {
    d->WorkflowInputButton->setEnabled( true );
  }

  // If the algorithms are parameters inputted
  if ( ! d->logic()->ToolCollection->GetInputted() )
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
  if ( ! d->logic()->ToolCollection->GetTrained() )
  {
    d->SaveWorkflowTrainingButton->setEnabled( false );
	d->SegmentTransformBufferButton->setEnabled( false );
  }
  else
  {
    d->SaveWorkflowTrainingButton->setEnabled( true );
	d->SegmentTransformBufferButton->setEnabled( true );
  }

}



void qSlicerWorkflowSegmentationModuleWidget::updateWidget()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );

  // TODO: Display the task name and instruction on screen
  // There is nothing to do here until we actually display the task on screen
  // For the moment, we display them just adding to the messages table
  // All the other things are taken care of by the TransformRecord logic/widgets
  enableButtons();

  // This updates the tasks
  d->logic()->Update();

}
