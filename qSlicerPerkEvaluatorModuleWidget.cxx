
// Standard includes 
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

// Qt includes
#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QTimer>

// VTK includes
#include "vtkCommand.h"
#include "vtkTable.h"

// SlicerQt includes
#include "qSlicerPerkEvaluatorModuleWidget.h"
#include "ui_qSlicerPerkEvaluatorModule.h"

#include <qSlicerApplication.h>
#include <qSlicerCoreApplication.h>

#include "vtkSlicerPerkEvaluatorLogic.h"
#include "vtkMRMLPerkEvaluatorNode.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLNode.h"


// Debug
void PrintToFile( std::string str )
{
  ofstream o( "PerkEvaluatorLog.txt", std::ios_base::app );
  int c = clock();
  o << std::fixed << setprecision( 2 ) << ( c / (double)CLOCKS_PER_SEC ) << " : " << str << std::endl;
  o.close();
}



//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPerkEvaluatorModuleWidgetPrivate: public Ui_qSlicerPerkEvaluatorModule
{
  Q_DECLARE_PUBLIC( qSlicerPerkEvaluatorModuleWidget );
protected:
  qSlicerPerkEvaluatorModuleWidget* const q_ptr;
public:
  qSlicerPerkEvaluatorModuleWidgetPrivate( qSlicerPerkEvaluatorModuleWidget& object );

  vtkSlicerPerkEvaluatorLogic* logic() const;

  // Add embedded widgets here
  qSlicerTransformBufferWidget* TransformBufferWidget;
  qSlicerPerkEvaluatorMessagesWidget* MessagesWidget;
  qSlicerMetricsTableWidget* MetricsTableWidget;
  qSlicerPerkEvaluatorTransformRolesWidget* TransformRolesWidget;
  qSlicerPerkEvaluatorAnatomyRolesWidget* AnatomyRolesWidget;
};




//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorModuleWidgetPrivate methods



qSlicerPerkEvaluatorModuleWidgetPrivate
::qSlicerPerkEvaluatorModuleWidgetPrivate( qSlicerPerkEvaluatorModuleWidget& object )
 : q_ptr( &object )
{
}


//-----------------------------------------------------------------------------
vtkSlicerPerkEvaluatorLogic*
qSlicerPerkEvaluatorModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerPerkEvaluatorModuleWidget );
  return vtkSlicerPerkEvaluatorLogic::SafeDownCast( q->logic() );
}





//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorModuleWidget methods


//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorModuleWidget::qSlicerPerkEvaluatorModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerPerkEvaluatorModuleWidgetPrivate( *this ) )
{
  this->PlaybackTimer = new QTimer( this );
  this->PlaybackTimerIntervalSec = 0.1; // seconds
  this->FrameStepSec = 0.1; // seconds
}


//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorModuleWidget::~qSlicerPerkEvaluatorModuleWidget()
{
  delete this->PlaybackTimer;
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackSliderChanged( double value )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );

  d->logic()->SetRelativePlaybackTime( peNode, value );
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackNextClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );

  d->logic()->SetRelativePlaybackTime( peNode, d->logic()->GetRelativePlaybackTime( peNode ) + this->FrameStepSec );
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackPrevClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );

  d->logic()->SetRelativePlaybackTime( peNode, d->logic()->GetRelativePlaybackTime( peNode ) - this->FrameStepSec );
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackBeginClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );

  d->logic()->SetRelativePlaybackTime( peNode, 0 );
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackEndClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );

  d->logic()->SetRelativePlaybackTime( peNode, d->logic()->GetMaximumRelativePlaybackTime( peNode ) );
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackPlayClicked()
{
  this->PlaybackTimer->start( int( this->PlaybackTimerIntervalSec * 1000 ) ); // convert to milliseconds
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackStopClicked()
{
  this->PlaybackTimer->stop();
}



void qSlicerPerkEvaluatorModuleWidget
::OnTimeout()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );

  double newRelativePlaybackTime = d->logic()->GetRelativePlaybackTime( peNode ) + this->PlaybackTimerIntervalSec;

  if ( newRelativePlaybackTime >= d->logic()->GetMaximumRelativePlaybackTime( peNode ) )
  {
    if ( d->PlaybackRepeatCheckBox->checkState() == Qt::Checked )
    {
      d->logic()->SetRelativePlaybackTime( peNode, 0 );
    }
    else
    {
      d->logic()->SetRelativePlaybackTime( peNode, d->logic()->GetMaximumRelativePlaybackTime( peNode ) );
      this->PlaybackTimer->stop();
    }
  }
  else
  {
    d->logic()->SetRelativePlaybackTime( peNode, newRelativePlaybackTime );
  }

}

void qSlicerPerkEvaluatorModuleWidget
::OnAnalyzeClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  QProgressDialog dialog;
  dialog.setModal( true );
  dialog.setLabelText( "Please wait while analyzing procedure..." );
  dialog.show();
  dialog.setValue( 10 );

  // Metrics table
  vtkMRMLTableNode* metricsTableNode = peNode->GetMetricsTableNode();
  if ( metricsTableNode == NULL )
  {
    return;
  }

  dialog.setValue( 20 );

  d->logic()->ComputeMetrics( peNode ); // This will populate the metrics table node with computed metrics

  dialog.setValue( 80 );

  dialog.close();
}


void qSlicerPerkEvaluatorModuleWidget
::OnBatchPerkEvaluatorNodeClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  QProgressDialog dialog;
  dialog.setModal( true );
  dialog.setLabelText( "Please wait while analyzing procedures..." );
  dialog.show();
  dialog.setValue( 0 );

  // Iterate over all nodes and calculate
  QList< vtkMRMLNode* > peNodeBatch = d->BatchPerkEvaluatorNodeComboBox->checkedNodes();

  for ( int i = 0; i < peNodeBatch.size(); i++ )
  {
    int progress = 100 * i / peNodeBatch.size();
    dialog.setValue( progress );

    vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( peNodeBatch.at( i ) );
    if ( peNode == NULL )
    {
      continue;
    }

    d->logic()->ComputeMetrics( peNode );
  }

  dialog.setValue( 100 );
  dialog.close();
}


void qSlicerPerkEvaluatorModuleWidget
::OnBatchTransformBufferClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  QProgressDialog dialog;
  dialog.setModal( true );
  dialog.setLabelText( "Please wait while analyzing procedures..." );
  dialog.show();
  dialog.setValue( 0 );

  // Remember the original Perk Evaluator node
  vtkMRMLNode* originalPerkEvaluatorNode = d->PerkEvaluatorNodeComboBox->currentNode();

  // Iterate over all nodes and calculate
  QList< vtkMRMLNode* > transformBufferBatch = d->BatchTransformBufferComboBox->checkedNodes();

  for ( int i = 0; i < transformBufferBatch.size(); i++ )
  {
    int progress = 100 * i / transformBufferBatch.size();
    dialog.setValue( progress );

    vtkMRMLTransformBufferNode* transformBuffer = vtkMRMLTransformBufferNode::SafeDownCast( transformBufferBatch.at( i ) );
    if ( transformBuffer == NULL )
    {
      continue;
    }

    // Create relevant nodes automatically
    // TODO: Should this be done in the logic?
    vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->addNode() );
    peNode->Copy( originalPerkEvaluatorNode );
    d->MetricsTableWidget->addMetricsTableNode();
    d->TransformBufferWidget->setTransformBufferNode( transformBuffer );

    d->logic()->ComputeMetrics( peNode );
  }

  d->PerkEvaluatorNodeComboBox->setCurrentNode( originalPerkEvaluatorNode );

  dialog.setValue( 100 );
  dialog.close();
}


void qSlicerPerkEvaluatorModuleWidget
::OnMarkBeginChanged()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  peNode->SetMarkBegin( d->BeginSpinBox->value() );
}


void qSlicerPerkEvaluatorModuleWidget
::OnMarkBeginClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  peNode->SetMarkBegin( d->logic()->GetRelativePlaybackTime( peNode ) );
}


void qSlicerPerkEvaluatorModuleWidget
::OnMarkEndChanged()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  peNode->SetMarkEnd( d->EndSpinBox->value() );
}


void qSlicerPerkEvaluatorModuleWidget
::OnMarkEndClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  peNode->SetMarkEnd( d->logic()->GetRelativePlaybackTime( peNode ) );
}


void
qSlicerPerkEvaluatorModuleWidget
::onTissueModelChanged( vtkMRMLNode* node )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }
  
  vtkMRMLModelNode* mnode = vtkMRMLModelNode::SafeDownCast( node );
  if ( mnode != NULL )
  {
    peNode->SetAnatomyNodeName( "Tissue", mnode->GetName() );
  }
  else
  {
    peNode->SetAnatomyNodeName( "Tissue", "" );
  }
}



void
qSlicerPerkEvaluatorModuleWidget
::onNeedleTransformChanged( vtkMRMLNode* node )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }
  
  vtkMRMLLinearTransformNode* tnode = vtkMRMLLinearTransformNode::SafeDownCast( node );
  if ( tnode != NULL )
  {
    peNode->SetTransformRole( tnode->GetName(), "Needle" );
  }
  else
  {
    while( peNode->GetFirstTransformNodeName( "Needle" ).compare( "" ) != 0 )
    {
      peNode->SetTransformRole( peNode->GetFirstTransformNodeName( "Needle" ), "" );
    }
  }
}


void qSlicerPerkEvaluatorModuleWidget
::OnAutoUpdateMeasurementRangeToggled()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  peNode->SetAutoUpdateMeasurementRange( d->AutoUpdateMeasurementRangeCheckBox->isChecked() );
}


void qSlicerPerkEvaluatorModuleWidget
::OnAutoUpdateTransformRolesToggled()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  peNode->SetAutoUpdateTransformRoles( d->AutoUpdateTransformRolesCheckBox->isChecked() );
}


void qSlicerPerkEvaluatorModuleWidget
::OnMetricsDirectoryClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  QString fileName = QFileDialog::getExistingDirectory( this, tr("Open metrics directory"), "", QFileDialog::ShowDirsOnly );  
  if ( fileName.isEmpty() == false )
  {
    peNode->SetMetricsDirectory( fileName.toStdString() );
  }

}


void
qSlicerPerkEvaluatorModuleWidget
::onNeedleOrientationChanged( QAbstractButton* newOrientationButton )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  if ( newOrientationButton == d->PlusXRadioButton )
  {
    peNode->SetNeedleOrientation( vtkMRMLPerkEvaluatorNode::PlusX );
  }
  if ( newOrientationButton == d->MinusXRadioButton )
  {
    peNode->SetNeedleOrientation( vtkMRMLPerkEvaluatorNode::MinusX );
  }
  if ( newOrientationButton == d->PlusYRadioButton )
  {
    peNode->SetNeedleOrientation( vtkMRMLPerkEvaluatorNode::PlusY );
  }
  if ( newOrientationButton == d->MinusYRadioButton )
  {
    peNode->SetNeedleOrientation( vtkMRMLPerkEvaluatorNode::MinusY );
  }
  if ( newOrientationButton == d->PlusZRadioButton )
  {
    peNode->SetNeedleOrientation( vtkMRMLPerkEvaluatorNode::PlusZ );
  }
  if ( newOrientationButton == d->MinusZRadioButton )
  {
    peNode->SetNeedleOrientation( vtkMRMLPerkEvaluatorNode::MinusZ );
  }

}


void
qSlicerPerkEvaluatorModuleWidget
::setupEmbeddedWidgets()
{
  Q_D(qSlicerPerkEvaluatorModuleWidget);

  // Adding the embedded widgets
  d->TransformBufferWidget = new qSlicerTransformBufferWidget();
  d->BufferGroupBox->layout()->addWidget( d->TransformBufferWidget );
  d->TransformBufferWidget->setMRMLScene( NULL );
  d->TransformBufferWidget->setMRMLScene( d->logic()->GetMRMLScene() );
  d->TransformBufferWidget->setTransformBufferNode( NULL ); // Do not automatically select a node on entering the widget

  d->MessagesWidget = new qSlicerPerkEvaluatorMessagesWidget();
  d->MessagesGroupBox->layout()->addWidget( d->MessagesWidget );
  d->MessagesWidget->setMRMLScene( NULL ); 
  d->MessagesWidget->setMRMLScene( d->logic()->GetMRMLScene() ); 

  d->MetricsTableWidget = new qSlicerMetricsTableWidget();
  d->ResultsGroupBox->layout()->addWidget( d->MetricsTableWidget );
  d->MetricsTableWidget->setMRMLScene( NULL ); 
  d->MetricsTableWidget->setMRMLScene( d->logic()->GetMRMLScene() );

  d->TransformRolesWidget = new qSlicerPerkEvaluatorTransformRolesWidget();
  d->TransformRolesGroupBox->layout()->addWidget( d->TransformRolesWidget );
  d->TransformRolesWidget->setMRMLScene( NULL ); 
  d->TransformRolesWidget->setMRMLScene( d->logic()->GetMRMLScene() );

  d->AnatomyRolesWidget = new qSlicerPerkEvaluatorAnatomyRolesWidget();
  d->AnatomyRolesGroupBox->layout()->addWidget( d->AnatomyRolesWidget );
  d->AnatomyRolesWidget->setMRMLScene( NULL ); 
  d->AnatomyRolesWidget->setMRMLScene( d->logic()->GetMRMLScene() );

  // Setting up connections for embedded widgets
  // Connect the child widget to the transform buffer node change event (they already observe the modified event)

}


void
qSlicerPerkEvaluatorModuleWidget
::setup()
{
  Q_D(qSlicerPerkEvaluatorModuleWidget);

  d->setupUi(this);

  // Embed widgets here
  this->setupEmbeddedWidgets();

  // Perk Evaluator Node

  // Connect the Perk Evaluator node to the update
  connect( d->PerkEvaluatorNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( mrmlNodeChanged( vtkMRMLNode* ) ) ); // If the node is changed connect it to update 

  //connect( d->PerkEvaluatorNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), d->TransformBufferWidget, SLOT( setPerkEvaluatorNode( vtkMRMLNode* ) ) );
  connect( d->PerkEvaluatorNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), d->MessagesWidget, SLOT( setPerkEvaluatorNode( vtkMRMLNode* ) ) );
  // NOTE: The roles widgets will be updated with the other components of the widget


  // Display tab

  connect( d->PlaybackSlider, SIGNAL( valueChanged( double ) ), this, SLOT( OnPlaybackSliderChanged( double ) ) );
  connect( d->NextButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackNextClicked() ) );
  connect( d->PrevButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackPrevClicked() ) );
  connect( d->BeginButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackBeginClicked() ) );
  connect( d->EndButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackEndClicked() ) );
  connect( d->PlayButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackPlayClicked() ) );
  connect( d->StopButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackStopClicked() ) );

  connect( this->PlaybackTimer, SIGNAL( timeout() ), this, SLOT( OnTimeout() ) );

  // If the transform buffer node is changed, update everything
  connect( d->TransformBufferWidget, SIGNAL( transformBufferNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTransformBufferChanged( vtkMRMLNode* ) ) );
  connect( d->MetricsTableWidget, SIGNAL( metricsTableNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMetricsTableChanged( vtkMRMLNode* ) ) );
 
  // TODO: If the transform buffer is updated, then we want to clear the widget, and reset the tool trajectories
  // But resetting the tool trajectories is computationally expensive
  // It might be better to update the trajectories only when necessary parts of the GUI are interacted with (i.e. Analyze, or playback controls)
  connect( d->TransformBufferWidget, SIGNAL( transformBufferNodeChanged( vtkMRMLNode* ) ), d->MessagesWidget, SLOT( setTransformBufferNode( vtkMRMLNode* ) ) );


  // Analysis tab

  connect( d->BeginSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnMarkBeginChanged() ) );
  connect( d->MarkBeginButton, SIGNAL( clicked() ), this, SLOT( OnMarkBeginClicked() ) );
  connect( d->EndSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnMarkEndChanged() ) );
  connect( d->MarkEndButton, SIGNAL( clicked() ), this, SLOT( OnMarkEndClicked() ) );

  connect( d->BodyNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTissueModelChanged( vtkMRMLNode* ) ) );
  connect( d->NeedleReferenceComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onNeedleTransformChanged( vtkMRMLNode* ) ) );

  connect( d->AnalyzeButton, SIGNAL( clicked() ), this, SLOT( OnAnalyzeClicked() ) );

  connect( d->BatchPerkEvaluatorNodeButton, SIGNAL( clicked() ), this, SLOT( OnBatchPerkEvaluatorNodeClicked() ) );
  connect( d->BatchTransformBufferButton, SIGNAL( clicked() ), this, SLOT( OnBatchTransformBufferClicked() ) );


  // Advanced tab

  connect( d->MetricsDirectoryButton, SIGNAL( clicked() ), this, SLOT( OnMetricsDirectoryClicked() ) );
  connect( d->AutoUpdateMeasurementRangeCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( OnAutoUpdateMeasurementRangeToggled() ) );
  connect( d->AutoUpdateTransformRolesCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( OnAutoUpdateTransformRolesToggled() ) );
  connect( d->NeedleOrientationButtonGroup, SIGNAL( buttonClicked( QAbstractButton* ) ), this, SLOT( onNeedleOrientationChanged( QAbstractButton* ) ) );




  this->updateWidgetFromMRMLNode();
}


void
qSlicerPerkEvaluatorModuleWidget
::enter()
{
  Q_D(qSlicerPerkEvaluatorModuleWidget);

  this->qSlicerAbstractModuleWidget::enter();

  // Create a node by default if none already exists
  int numPENodes = this->mrmlScene()->GetNumberOfNodesByClass( "vtkMRMLPerkEvaluatorNode" );
  if ( numPENodes == 0 )
  {
    d->PerkEvaluatorNodeComboBox->addNode();
    d->MetricsTableWidget->addMetricsTableNode();
  }
  else
  {
    this->updateWidgetFromMRMLNode();
  }


}


void qSlicerPerkEvaluatorModuleWidget
::onTransformBufferChanged( vtkMRMLNode* newTransformBuffer )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  peNode->SetTransformBufferID( newTransformBuffer->GetID() );
  // The Perk Evaluator node automatically call the "updateWidgetFromMRML" function to deal with the widget
}


void qSlicerPerkEvaluatorModuleWidget
::onMetricsTableChanged( vtkMRMLNode* newMetricsTable )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  // This is where we need to update parameters on buffer node changed
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL || newMetricsTable == NULL )
  {
    return;
  }

  peNode->SetMetricsTableID( newMetricsTable->GetID() );
  // The Perk Evaluator node automatically call the "updateWidgetFromMRML" function to deal with the widget
}


void qSlicerPerkEvaluatorModuleWidget
::mrmlNodeChanged( vtkMRMLNode* peNode )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  this->qvtkDisconnectAll(); // Remove connections to previous node
  if ( peNode != NULL )
  {
    this->qvtkConnect( peNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidgetFromMRMLNode() ) );
  }

  this->updateWidgetFromMRMLNode();
}


void qSlicerPerkEvaluatorModuleWidget
::updateWidgetFromMRMLNode()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  // Grab the MRML node
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  // Update the roles widgets and metrics table widget every time the Perk Evaluator node is modified
  d->TransformRolesWidget->setPerkEvaluatorNode( peNode );
  d->AnatomyRolesWidget->setPerkEvaluatorNode( peNode );
  d->MetricsTableWidget->setMetricsTableNode( peNode->GetMetricsTableNode() );

  d->BeginSpinBox->setValue( peNode->GetMarkBegin() );
  d->EndSpinBox->setValue( peNode->GetMarkEnd() );

  d->PlaybackSlider->setMinimum( 0 );
  d->PlaybackSlider->setMaximum( d->logic()->GetMaximumRelativePlaybackTime( peNode ) );
  d->PlaybackSlider->setValue( d->logic()->GetRelativePlaybackTime( peNode ) );

  vtkMRMLNode* needleNode = this->mrmlScene()->GetFirstNodeByName( peNode->GetFirstTransformNodeName( "Needle" ).c_str() );
  d->NeedleReferenceComboBox->setCurrentNode( needleNode );

  vtkMRMLNode* tissueNode = this->mrmlScene()->GetFirstNodeByName( peNode->GetAnatomyNodeName( "Tissue" ).c_str() );
  d->BodyNodeComboBox->setCurrentNode( tissueNode );

  // Get the name of the base directory
  QDir metricsDirectory( peNode->GetMetricsDirectory().c_str() );
  d->MetricsDirectoryButton->setText( metricsDirectory.dirName() );
  d->MetricsDirectoryButton->setToolTip( metricsDirectory.absolutePath() );

  d->AutoUpdateMeasurementRangeCheckBox->setChecked( peNode->GetAutoUpdateMeasurementRange() ); 
  d->AutoUpdateTransformRolesCheckBox->setChecked( peNode->GetAutoUpdateTransformRoles() ); 

  if ( peNode->GetNeedleOrientation() == vtkMRMLPerkEvaluatorNode::PlusX )
  {
    d->PlusXRadioButton->setChecked( Qt::Checked );
  }
  if ( peNode->GetNeedleOrientation() == vtkMRMLPerkEvaluatorNode::MinusX )
  {
    d->MinusXRadioButton->setChecked( Qt::Checked );
  }
  if ( peNode->GetNeedleOrientation() == vtkMRMLPerkEvaluatorNode::PlusY )
  {
    d->PlusYRadioButton->setChecked( Qt::Checked );
  }
  if ( peNode->GetNeedleOrientation() == vtkMRMLPerkEvaluatorNode::MinusY )
  {
    d->MinusYRadioButton->setChecked( Qt::Checked );
  }
  if ( peNode->GetNeedleOrientation() == vtkMRMLPerkEvaluatorNode::PlusZ )
  {
    d->PlusZRadioButton->setChecked( Qt::Checked );
  }
  if ( peNode->GetNeedleOrientation() == vtkMRMLPerkEvaluatorNode::MinusZ )
  {
    d->MinusZRadioButton->setChecked( Qt::Checked );
  }

  d->logic()->UpdateSceneToPlaybackTime( peNode );  
}