

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
  vtkMRMLLinearTransformNode*   MRMLTransformNode;
};

//-----------------------------------------------------------------------------
// qSlicerWorkflowSegmentationModuleWidgetPrivate methods



qSlicerWorkflowSegmentationModuleWidgetPrivate::qSlicerWorkflowSegmentationModuleWidgetPrivate( qSlicerWorkflowSegmentationModuleWidget& object ) : q_ptr(&object)
{
  this->MRMLTransformNode = 0;
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

  // Deal with the labels
  d->StatusResultLabel= NULL;
  d->NumRecordsResultLabel=NULL;
  d->TotalTimeResultLabel=NULL;
  d->CurrentTaskResultLabel=NULL;
  d->CurrentInstructionResultLabel=NULL;
  d->NextTaskResultLabel=NULL;
  d->NextInstructionResultLabel=NULL;

  d->setupUi(this);
  this->Superclass::setup();

  // Module node selection
  d->ModuleComboBox->setNoneEnabled( true );
  connect( d->ModuleComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModuleNodeSelected() ) );

  // Transform Selection
  connect( d->TransformCheckableComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTransformsNodeSelected(vtkMRMLNode*) ) );

  // Import parameters and files
  connect( d->InputParameterButton, SIGNAL(clicked()), this, SLOT( onInputParameterButtonClicked() ) );
  connect( d->TrainingParameterButton, SIGNAL(clicked()), this, SLOT( onTrainingParameterButtonClicked() ) );
  connect( d->TrainingDataButton, SIGNAL(clicked()), this, SLOT( onTrainingDataButtonClicked() ) );

  // Train algorithm
  connect( d->TrainButton, SIGNAL(clicked()), this, SLOT( onTrainButtonClicked() ) );

  // Save tracking log and parameters
  connect( d->SaveTrackingLogButton, SIGNAL(clicked()),this, SLOT ( onSaveTrackingLogButtonClicked() ) );
  connect( d->SaveTrainingButton, SIGNAL(clicked()),this, SLOT ( onSaveTrainingButtonClicked() ) );

  // Recording controls
  connect( d->StartButton, SIGNAL(clicked()), this, SLOT(onStartButtonClicked() ) );
  connect( d->StopButton, SIGNAL(clicked()), this, SLOT(onStopButtonClicked() ) );
  connect( d->ClearBufferButton, SIGNAL(clicked()), this, SLOT(onClearBufferButtonClicked() ) );

  //Annotations
  d->AnnotationListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  connect( d->InsertAnnotationButton, SIGNAL(clicked()), this, SLOT(insertAnnotation()));
  connect( d->ClearAnnotationButton, SIGNAL(clicked()), this, SLOT(clearAnnotations()));


  // GUI refresh: updates every 10ms
  QTimer *t = new QTimer( this );
  connect( t,  SIGNAL( timeout() ), this, SLOT( updateGUI() ) );
  t->start(10); 

}



void qSlicerWorkflowSegmentationModuleWidget
::onTransformsNodeSelected( vtkMRMLNode* node )
{
  Q_D(qSlicerWorkflowSegmentationModuleWidget);
  /*
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( node );
  
  if ( transformNode != NULL
       && d->logic()->GetModuleNode() != NULL )
  {
    d->logic()->GetModuleNode()->AddObservedTransformNode( node->GetID() );
  }
  */
  
    // TODO: Why was this here?
    // Listen for Transform node changes
  /*
  this->qvtkReconnect( d->MRMLTransformNode, transformNode, vtkMRMLTransformableNode::TransformModifiedEvent,
                       this, SLOT( onMRMLTransformNodeModified( vtkObject* ) ) ); 
  */
}



void qSlicerWorkflowSegmentationModuleWidget::onMRMLTransformNodeModified( vtkObject* caller )
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  // TODO: I'm not sure this function is needed at all.
  /*
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( caller );
  if (!transformNode) { return; }
  
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  */
}



void qSlicerWorkflowSegmentationModuleWidget::enter()
{
  this->Superclass::enter();
  this->updateGUI();
}





void qSlicerWorkflowSegmentationModuleWidget::onModuleNodeSelected()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
 
  vtkMRMLNode* currentNode = d->ModuleComboBox->currentNode();
  std::cout << "Current node pointer: " << currentNode << std::endl;
  
  vtkMRMLWorkflowSegmentationNode* TRNode = vtkMRMLWorkflowSegmentationNode::SafeDownCast( currentNode );
  if ( TRNode != NULL )
  {
    
  }
  
  d->logic()->SetModuleNode( TRNode );
  d->logic()->GetWorkflowAlgorithm()->setMRMLNode( d->logic()->GetModuleNode() );

  this->updateGUI();

}





void qSlicerWorkflowSegmentationModuleWidget
::onInputParameterButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  QString fileName = QFileDialog::getOpenFileName( this, tr("Open parameters"), "", tr("XML Files (*.xml)") );
  
  if ( fileName.isEmpty() == false )
  {
	d->logic()->GetModuleNode()->ImportInputParameters( fileName.toStdString() );
	d->logic()->GetWorkflowAlgorithm()->GetInputParamtersFromMRMLNode();
	d->logic()->GetModuleNode()->SetHasInput( true );
  }
  
  this->updateGUI();
}




void qSlicerWorkflowSegmentationModuleWidget
::onTrainingParameterButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  QString fileName = QFileDialog::getOpenFileName( this, tr("Open parameters"), "", tr("XML Files (*.xml)") );
  
  if ( fileName.isEmpty() == false )
  {
	d->logic()->GetModuleNode()->ImportTrainingParameters( fileName.toStdString() );
	d->logic()->GetWorkflowAlgorithm()->GetTrainingParametersFromMRMLNode();
	d->logic()->GetWorkflowAlgorithm()->InitializeSegmentationRT();
	d->logic()->GetModuleNode()->SetIsTrained( true );
  }
  
  this->updateGUI();
}





void qSlicerWorkflowSegmentationModuleWidget
::onTrainingDataButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  QStringList files = QFileDialog::getOpenFileNames( this, tr("Open training files"), "", "XML Files (*.xml)" );
  
  // Vector of QStrings
  std::vector<std::string> trainingFileVector;
  for( int i = 0; i < files.size(); i++ )
  {
    trainingFileVector.push_back( files.at(i).toStdString() );
  }

  if ( files.isEmpty() == false )
  {
    
    QProgressDialog dialog;
    dialog.setModal( true );
    dialog.setLabelText( "Please wait while reading XML files..." );
    dialog.show();
    dialog.setValue( 10 );
    
	d->logic()->GetWorkflowAlgorithm()->ReadAllProcedures( trainingFileVector );

    dialog.close();
    
  }
  
  this->updateGUI();
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
  dialog.setValue( 10 );
  
  d->logic()->GetWorkflowAlgorithm()->setMRMLNode( d->logic()->GetModuleNode() );
  dialog.setValue( 20 );
  d->logic()->GetWorkflowAlgorithm()->GetInputParamtersFromMRMLNode();
  dialog.setValue( 30 );
  d->logic()->GetModuleNode()->SetIsTrained( d->logic()->GetWorkflowAlgorithm()->train() );
  dialog.setValue( 80 );
  if ( d->logic()->GetModuleNode()->GetIsTrained() )
  {
    d->logic()->GetWorkflowAlgorithm()->InitializeSegmentationRT();
  }

  dialog.close();


  this->updateGUI();
}




void qSlicerWorkflowSegmentationModuleWidget
::onSaveTrackingLogButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  QString filename = QFileDialog::getSaveFileName( this, tr("Save log"), "", tr("XML Files (*.xml)") );
  
  if ( filename.isEmpty() == false )
  {
    d->logic()->GetModuleNode()->SaveTrackingLog( filename.toStdString() );
    d->AnnotationListWidget->clear();
  }
  
  this->updateGUI();
}



void qSlicerWorkflowSegmentationModuleWidget
::onSaveTrainingButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  QString filename = QFileDialog::getSaveFileName( this, tr("Save training"), "", tr("XML Files (*.xml)") );
  
  if ( filename.isEmpty() == false )
  {
    d->logic()->GetModuleNode()->SaveTrainingParameters( filename.toStdString() );
  }
  
  this->updateGUI();
}





void qSlicerWorkflowSegmentationModuleWidget
::onStartButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  
  if ( d->logic()->GetModuleNode() == NULL )
  {
    return;
  }
  
  
    // Go through transform types (ie ProbeToReference, StylusTipToReference, etc) 
    // Save selected state as 1 and unselected state as 0 of each transform type in transformSelections vector.
  
  std::vector< int > transformSelections;
  const int unselected = 0;
  const int selected = 1;
  
  for ( int i = 0; i < d->TransformCheckableComboBox->nodeCount(); i++ )
  {
	  if( d->TransformCheckableComboBox->checkState( d->TransformCheckableComboBox->nodeFromIndex( i ) ) == Qt::Checked  )
	  {
	    d->logic()->GetModuleNode()->AddObservedTransformNode( d->TransformCheckableComboBox->nodeFromIndex( i )->GetID() );
	  }
	  else
	  {
	    d->logic()->GetModuleNode()->RemoveObservedTransformNode( d->TransformCheckableComboBox->nodeFromIndex( i )->GetID() );
	  }
	}
  
  
  d->logic()->GetModuleNode()->SetRecording( true );
  
  this->updateGUI();
  
}



void qSlicerWorkflowSegmentationModuleWidget
::onStopButtonClicked()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );

  if( d->logic()->GetModuleNode() == NULL )
  {
    return;
  }

  d->logic()->GetModuleNode()->SetRecording( false );
   
  this->updateGUI();
}



void qSlicerWorkflowSegmentationModuleWidget
::onClearBufferButtonClicked() 
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );

  if( d->logic()->GetModuleNode() == NULL )
  {
    return;
  }

  d->logic()->GetModuleNode()->ClearBuffer();

  if ( d->logic()->GetModuleNode()->GetIsTrained() )
  {
  d->logic()->GetWorkflowAlgorithm()->InitializeSegmentationRT();
  }

  this->updateGUI();
}





void qSlicerWorkflowSegmentationModuleWidget
::insertAnnotation()
{
	Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  vtkMRMLWorkflowSegmentationNode* moduleNode = d->logic()->GetModuleNode();
  if ( moduleNode == NULL )
  {
    return;
  }
  
  
  int sec = 0;
  int nsec = 0;
  
  moduleNode->GetTimestamp( sec, nsec );
  
  
    // Get the timestamp for this annotation.
  
  
  QString itemText = QInputDialog::getText(this, tr("Insert Annotation"),
      tr("Input text for the new annotation:"));

  if (itemText.isNull())
      return;
  
  
  QListWidgetItem *newItem = new QListWidgetItem;
  newItem->setText(itemText);
  
  QString toolTipText = tr("Tooltip:") + itemText;
  QString statusTipText = tr("Status tip:") + itemText;
  QString whatsThisText = tr("What's This?:") + itemText;

  newItem->setToolTip(toolTipText);
  newItem->setStatusTip(toolTipText);
  newItem->setWhatsThis(whatsThisText);

	d->AnnotationListWidget->addItem(newItem);
	std::string annotation = itemText.toStdString();
	moduleNode->CustomMessage( annotation, sec, nsec );

}




void qSlicerWorkflowSegmentationModuleWidget
::clearAnnotations()
{
	Q_D( qSlicerWorkflowSegmentationModuleWidget );

	d->AnnotationListWidget->clear();
}





void qSlicerWorkflowSegmentationModuleWidget::updateGUI()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  
  // Disableing node selector widgets if there is no module node to reference input nodes.
    
  if ( d->logic()->GetModuleNode() == NULL )
  {
    d->TransformCheckableComboBox->setEnabled( false );

	d->InputParameterButton->setEnabled( false );
	d->TrainingParameterButton->setEnabled( false );
	d->TrainingDataButton->setEnabled( false );
	d->TrainButton->setEnabled( false );

	d->SaveTrackingLogButton->setEnabled( false );
    d->SaveSegmentationButton->setEnabled( false );
	d->SaveTrainingButton->setEnabled( false );
  }
  else
  {
    d->TransformCheckableComboBox->setEnabled( true );

	d->InputParameterButton->setEnabled( true );

	d->SaveTrackingLogButton->setEnabled( true );
    d->SaveSegmentationButton->setEnabled( true );
  }
  
    // The following code requires a module node.
  
  if ( d->logic()->GetModuleNode() == NULL )
  {
    return;
  }
  
  
  
  // Update status descriptor labels.

  if ( ! d->logic()->GetModuleNode()->GetHasInput() )
  {
    d->TrainingParameterButton->setEnabled( false );
	d->TrainingDataButton->setEnabled( false );
	d->TrainButton->setEnabled( false );
  }
  else
  {
    d->TrainingParameterButton->setEnabled( true );
	d->TrainingDataButton->setEnabled( true );
	d->TrainButton->setEnabled( true );
  }

  if ( ! d->logic()->GetModuleNode()->GetIsTrained() )
  {
    d->SaveTrainingButton->setEnabled( false );
  }
  else
  {
    d->SaveTrainingButton->setEnabled( true );
  }
  
  if ( d->logic()->GetModuleNode()->GetRecording() )
  {
    d->StatusResultLabel->setText( "Recording" );
    d->TransformCheckableComboBox->setEnabled( false );
  }
  else
  {
    d->StatusResultLabel->setText( "Waiting" );
    d->TransformCheckableComboBox->setEnabled( true );
  }
  
  
  int numRec = d->logic()->GetModuleNode()->GetTransformsBufferSize() + d->logic()->GetModuleNode()->GetMessagesBufferSize();
  std::stringstream ss;
  ss << numRec;
  d->NumRecordsResultLabel->setText( QString::fromStdString( ss.str() ) );
  
  ss.str( "" );
  ss.precision( 2 );
  ss << std::fixed << d->logic()->GetModuleNode()->GetTotalTime();
  d->TotalTimeResultLabel->setText( ss.str().c_str() );

  // TODO: Record the workflow segmentations in MRML (like annotations)
  ss.str( "" );
  ss << d->logic()->GetWorkflowAlgorithm()->getCurrentTask();
  d->CurrentTaskResultLabel->setText( ss.str().c_str() );

  ss.str( "" );
  ss << d->logic()->GetModuleNode()->GetCurrentInstruction();
  d->CurrentInstructionResultLabel->setText( ss.str().c_str() );

  ss.str( "" );
  ss << d->logic()->GetModuleNode()->GetNextTask();
  d->NextTaskResultLabel->setText( ss.str().c_str() );

  ss.str( "" );
  ss << d->logic()->GetModuleNode()->GetNextInstruction();
  d->NextInstructionResultLabel->setText( ss.str().c_str() );

}
