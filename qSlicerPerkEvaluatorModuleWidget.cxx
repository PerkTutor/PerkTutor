
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
  qSlicerTrackedSequenceBrowserWidget* BrowserWidget;
  qMRMLSequenceBrowserPlayWidget* PlayWidget;
  qMRMLSequenceBrowserSeekWidget* SeekWidget;
  qSlicerPerkEvaluatorMessagesWidget* MessagesWidget;
  qSlicerMetricsTableWidget* MetricsTableWidget;
  qSlicerPerkEvaluatorRecorderControlsWidget* RecorderControlsWidget;
  qSlicerPerkEvaluatorTransformRolesWidget* TransformRolesWidget;
  qSlicerPerkEvaluatorAnatomyRolesWidget* AnatomyRolesWidget;

  QProgressDialog* AnalysisStateDialog;
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
  // Nothing to do
}


//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorModuleWidget::~qSlicerPerkEvaluatorModuleWidget()
{
  delete this->PlaybackTimer;
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

  // Metrics table
  vtkMRMLTableNode* metricsTableNode = peNode->GetMetricsTableNode();
  if ( metricsTableNode == NULL )
  {
    return;
  }

  d->AnalysisStateDialog->setLabelText( "Please wait while analyzing procedure..." );
  d->AnalysisStateDialog->show();
  d->AnalysisStateDialog->setValue( 0 );
  this->qvtkConnect( peNode, vtkMRMLPerkEvaluatorNode::AnalysisStateUpdatedEvent, this, SLOT( OnAnalysisStateUpdated( vtkObject*, void* ) ) );

  d->logic()->ComputeMetrics( peNode ); // This will populate the metrics table node with computed metrics

  this->qvtkDisconnect( peNode, vtkMRMLPerkEvaluatorNode::AnalysisStateUpdatedEvent, this, SLOT( OnAnalysisStateUpdated( vtkObject*, void* ) ) );
  d->AnalysisStateDialog->close();
}


void qSlicerPerkEvaluatorModuleWidget
::OnAnalysisStateUpdated( vtkObject* caller, void* value )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  // Check we can downcast
  int* progressValue = reinterpret_cast< int* >( value );
  d->AnalysisStateDialog->setValue( *progressValue );
}


void qSlicerPerkEvaluatorModuleWidget
::OnAnalysisCanceled()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  // Cancel all analyses (even though only one should be going on at a given time)
  vtkCollection* nodes = d->logic()->GetMRMLScene()->GetNodesByClass( "vtkMRMLPerkEvaluatorNode" );
  for ( int i = 0; i < nodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( nodes->GetItemAsObject( i ) );
    if ( peNode == NULL )
    {
      return;
    }
    peNode->SetAnalysisState( -1 ); // This will halt the analysis
  }

}


void qSlicerPerkEvaluatorModuleWidget
::OnBatchProcessButtonClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  // Remember the original Perk Evaluator node
  vtkMRMLNode* originalPerkEvaluatorNode = d->PerkEvaluatorNodeComboBox->currentNode();

  // Iterate over all nodes and calculate
  QList< vtkMRMLNode* > peNodeBatch = d->BatchPerkEvaluatorNodeComboBox->checkedNodes();

  for ( int i = 0; i < peNodeBatch.size(); i++ )
  {
    vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( peNodeBatch.at( i ) );
    if ( peNode == NULL )
    {
      vtkMRMLTransformBufferNode* transformBuffer = vtkMRMLTransformBufferNode::SafeDownCast( peNodeBatch.at( i ) );
      if ( transformBuffer == NULL )
      {
        continue;
      }

      // Create relevant nodes automatically
      // TODO: Should this be done in the logic?
      peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->addNode() );
      peNode->Copy( originalPerkEvaluatorNode );
      d->MetricsTableWidget->addMetricsTableNode();
      // d->BrowserWidget->setTrackedSequenceBrowserNode( transformBuffer );
    }

    std::stringstream labelText;
    labelText << "Please wait while analyzing procedure (" << i + 1 << "/" << peNodeBatch.size() << ")..."; // Use i + 1 to be human-readable
    d->AnalysisStateDialog->setLabelText( labelText.str().c_str() );
    d->AnalysisStateDialog->setValue( 0 );
    d->AnalysisStateDialog->show();
    this->qvtkConnect( peNode, vtkMRMLPerkEvaluatorNode::AnalysisStateUpdatedEvent, this, SLOT( OnAnalysisStateUpdated( vtkObject*, void* ) ) );

    d->logic()->ComputeMetrics( peNode ); // This will populate the metrics table node with computed metrics

    this->qvtkDisconnect( peNode, vtkMRMLPerkEvaluatorNode::AnalysisStateUpdatedEvent, this, SLOT( OnAnalysisStateUpdated( vtkObject*, void* ) ) );
    d->AnalysisStateDialog->close();
  }

  d->PerkEvaluatorNodeComboBox->setCurrentNode( originalPerkEvaluatorNode );
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

  peNode->SetMarkBegin( d->logic()->GetSelectedTime( peNode->GetTrackedSequenceBrowserNode() ) );
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

  peNode->SetMarkEnd( d->logic()->GetSelectedTime( peNode->GetTrackedSequenceBrowserNode() ) );
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
    d->logic()->SetMetricInstancesRolesToID( peNode, mnode->GetID(), "Tissue", vtkMRMLMetricInstanceNode::AnatomyRole );
  }
  else
  {
    d->logic()->SetMetricInstancesRolesToID( peNode, "", "Tissue", vtkMRMLMetricInstanceNode::AnatomyRole );
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
    d->logic()->SetMetricInstancesRolesToID( peNode, tnode->GetID(), "Needle", vtkMRMLMetricInstanceNode::TransformRole );
  }
  else
  {
    d->logic()->SetMetricInstancesRolesToID( peNode, "", "Needle", vtkMRMLMetricInstanceNode::TransformRole );
  }
}


void qSlicerPerkEvaluatorModuleWidget
::OnEditMetricInstanceNodeCreated( vtkMRMLNode* node )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::SafeDownCast( node );
  vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::SafeDownCast( d->BaseMetricScriptComboBox->currentNode() );
  if ( miNode == NULL || msNode == NULL )
  {
    return;
  }

  miNode->SetAssociatedMetricScriptID( msNode->GetID() );
  miNode->SetName( msNode->GetName() );
}


void qSlicerPerkEvaluatorModuleWidget
::OnEditMetricInstanceNodeChanged()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  d->AnatomyRolesWidget->setMetricInstanceNode( d->EditMetricInstanceNodeComboBox->currentNode() );
  d->TransformRolesWidget->setMetricInstanceNode( d->EditMetricInstanceNodeComboBox->currentNode() );
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


void
qSlicerPerkEvaluatorModuleWidget
::OnDownloadAdditionalMetricsClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  d->logic()->DownloadAdditionalMetrics();
}


void qSlicerPerkEvaluatorModuleWidget
::OnMetricInstanceNodesChanged()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  // Accumluate all of the IDs, and then write them to the node
  // Assume that it was the current node that was changed
  // This will be much faster if we don't have to deal with all the nodes
  vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::SafeDownCast( d->MetricInstanceComboBox->currentNode() );
  if ( miNode == NULL )
  {
    return;
  }

  if ( d->MetricInstanceComboBox->checkState( miNode ) == Qt::Checked )
  {
    peNode->AddMetricInstanceID( miNode->GetID() );
  }
  else
  {
    peNode->RemoveMetricInstanceID( miNode->GetID() );
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
  d->BrowserWidget = new qSlicerTrackedSequenceBrowserWidget();
  d->BufferGroupBox->layout()->addWidget( d->BrowserWidget );
  d->BrowserWidget->setMRMLScene( NULL );
  d->BrowserWidget->setMRMLScene( d->logic()->GetMRMLScene() );
  d->BrowserWidget->setTrackedSequenceBrowserNode( NULL ); // Do not automatically select a node on entering the widget

  d->PlayWidget = new qMRMLSequenceBrowserPlayWidget();
  d->PlaybackGroupbox->layout()->addWidget( d->PlayWidget );
  d->PlayWidget->setMRMLScene( NULL ); 
  d->PlayWidget->setMRMLScene( d->logic()->GetMRMLScene() ); 

  d->SeekWidget = new qMRMLSequenceBrowserSeekWidget();
  d->PlaybackGroupbox->layout()->addWidget( d->SeekWidget );
  d->SeekWidget->setMRMLScene( NULL ); 
  d->SeekWidget->setMRMLScene( d->logic()->GetMRMLScene() ); 

  d->MessagesWidget = new qSlicerPerkEvaluatorMessagesWidget();
  d->MessagesGroupBox->layout()->addWidget( d->MessagesWidget );
  d->MessagesWidget->setMRMLScene( NULL ); 
  d->MessagesWidget->setMRMLScene( d->logic()->GetMRMLScene() ); 

  d->MetricsTableWidget = new qSlicerMetricsTableWidget();
  d->MetricsTableWidget->setExpandHeightToContents( true );
  d->ResultsGroupBox->layout()->addWidget( d->MetricsTableWidget );
  d->MetricsTableWidget->setMRMLScene( NULL ); 
  d->MetricsTableWidget->setMRMLScene( d->logic()->GetMRMLScene() );

  d->RecorderControlsWidget = new qSlicerPerkEvaluatorRecorderControlsWidget();
  d->RealTimeProcessingGroupBox->layout()->addWidget( d->RecorderControlsWidget );
  d->RecorderControlsWidget->setMRMLScene( NULL ); 
  d->RecorderControlsWidget->setMRMLScene( d->logic()->GetMRMLScene() );

  d->TransformRolesWidget = new qSlicerPerkEvaluatorTransformRolesWidget();
  d->TransformRolesGroupBox->layout()->addWidget( d->TransformRolesWidget );
  d->TransformRolesWidget->setMRMLScene( NULL ); 
  d->TransformRolesWidget->setMRMLScene( d->logic()->GetMRMLScene() );

  d->AnatomyRolesWidget = new qSlicerPerkEvaluatorAnatomyRolesWidget();
  d->AnatomyRolesGroupBox->layout()->addWidget( d->AnatomyRolesWidget );
  d->AnatomyRolesWidget->setMRMLScene( NULL ); 
  d->AnatomyRolesWidget->setMRMLScene( d->logic()->GetMRMLScene() );

  d->AnalysisStateDialog = new QProgressDialog();
  d->AnalysisStateDialog->setModal( true );
  d->AnalysisStateDialog->setValue( 0 );
  d->AnalysisStateDialog->close();

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
  connect( d->PerkEvaluatorNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( mrmlNodeChanged( vtkMRMLNode* ) ) );
  // NOTE: The roles widgets will be updated with the other components of the widget


  // Display tab

  // If the transform buffer node is changed, update everything
  connect( d->BrowserWidget, SIGNAL( trackedSequenceBrowserNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTrackedSequenceBrowserNodeChanged( vtkMRMLNode* ) ) );
  connect( d->MetricsTableWidget, SIGNAL( metricsTableNodeChanged( vtkMRMLNode* ) ), this, SLOT( onMetricsTableChanged( vtkMRMLNode* ) ) );


  // Analysis tab
  connect( d->BeginSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnMarkBeginChanged() ) );
  connect( d->MarkBeginButton, SIGNAL( clicked() ), this, SLOT( OnMarkBeginClicked() ) );
  connect( d->EndSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnMarkEndChanged() ) );
  connect( d->MarkEndButton, SIGNAL( clicked() ), this, SLOT( OnMarkEndClicked() ) );

  connect( d->BodyNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTissueModelChanged( vtkMRMLNode* ) ) );
  connect( d->NeedleReferenceComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onNeedleTransformChanged( vtkMRMLNode* ) ) );

  connect( d->AnalyzeButton, SIGNAL( clicked() ), this, SLOT( OnAnalyzeClicked() ) );

  connect( d->BatchProcessButton, SIGNAL( clicked() ), this, SLOT( OnBatchProcessButtonClicked() ) );
  d->BatchProcessButton->setIcon( QIcon( ":/Icons/Go.png" ) );

  connect( d->AnalysisStateDialog, SIGNAL( canceled() ), this, SLOT( OnAnalysisCanceled() ) );

  // Update the recorder controls widget when the transform buffer is changed
  connect( d->BrowserWidget, SIGNAL( transformBufferNodeChanged( vtkMRMLNode* ) ), d->RecorderControlsWidget, SLOT( setTransformBufferNode( vtkMRMLNode* ) ) );
  connect( d->PerkEvaluatorNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), d->RecorderControlsWidget, SLOT( setPerkEvaluatorNode( vtkMRMLNode* ) ) );


  // Advanced tab
  connect( d->EditMetricInstanceNodeComboBox, SIGNAL( nodeAddedByUser( vtkMRMLNode* ) ), this, SLOT( OnEditMetricInstanceNodeCreated( vtkMRMLNode* ) ) );
  connect( d->EditMetricInstanceNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( OnEditMetricInstanceNodeChanged() ) );
  connect( d->MetricInstanceComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( OnMetricInstanceNodesChanged() ) );
  connect( d->AutoUpdateMeasurementRangeCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( OnAutoUpdateMeasurementRangeToggled() ) );
  connect( d->NeedleOrientationButtonGroup, SIGNAL( buttonClicked( QAbstractButton* ) ), this, SLOT( onNeedleOrientationChanged( QAbstractButton* ) ) );
  connect( d->DownloadAdditionalMetricsButton, SIGNAL( clicked() ), this, SLOT( OnDownloadAdditionalMetricsClicked() ) );




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
::onTrackedSequenceBrowserNodeChanged( vtkMRMLNode* newTrackedSequenceBrowserNode )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );
  if ( peNode == NULL )
  {
    return;
  }

  if ( newTrackedSequenceBrowserNode == NULL )
  {
    peNode->SetTrackedSequenceBrowserNodeID( "" );
    return;
  }

  peNode->SetTrackedSequenceBrowserNodeID( newTrackedSequenceBrowserNode->GetID() );
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
  vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::SafeDownCast( d->EditMetricInstanceNodeComboBox->currentNode() );
  d->TransformRolesWidget->setMetricInstanceNode( miNode );
  d->AnatomyRolesWidget->setMetricInstanceNode( miNode );

  d->BrowserWidget->setTrackedSequenceBrowserNode( peNode->GetTrackedSequenceBrowserNode() );
  d->MessagesWidget->setTrackedSequenceBrowserNode( peNode->GetTrackedSequenceBrowserNode() );
  d->MetricsTableWidget->setMetricsTableNode( peNode->GetMetricsTableNode() );
  d->PlayWidget->setMRMLSequenceBrowserNode( peNode->GetTrackedSequenceBrowserNode() );
  d->SeekWidget->setMRMLSequenceBrowserNode( peNode->GetTrackedSequenceBrowserNode() );

  // Disconnect to the GUI from updating the MRML node with rounded values
  disconnect( d->BeginSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnMarkBeginChanged() ) );
  disconnect( d->EndSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnMarkEndChanged() ) );
  d->BeginSpinBox->setValue( peNode->GetMarkBegin() );
  d->EndSpinBox->setValue( peNode->GetMarkEnd() );
  connect( d->BeginSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnMarkBeginChanged() ) );
  connect( d->EndSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnMarkEndChanged() ) );

  // For the metric scripts
  // Disable to the onCheckedChanged listener when initializing the selections
  // We don't want to simultaneously update the observed nodes from selections and selections from observed nodes
  disconnect( d->MetricInstanceComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( OnMetricInstanceNodesChanged() ) );
  for ( int i = 0; i < d->MetricInstanceComboBox->nodeCount(); i++ )
  {
    if ( peNode->IsMetricInstanceID( d->MetricInstanceComboBox->nodeFromIndex( i )->GetID() ) )
    {
	    d->MetricInstanceComboBox->setCheckState( d->MetricInstanceComboBox->nodeFromIndex( i ), Qt::Checked );
    }
    else
    {
      d->MetricInstanceComboBox->setCheckState( d->MetricInstanceComboBox->nodeFromIndex( i ), Qt::Unchecked );
    }
  }
  connect( d->MetricInstanceComboBox, SIGNAL( checkedNodesChanged() ), this, SLOT( OnMetricInstanceNodesChanged() ) );


  d->AutoUpdateMeasurementRangeCheckBox->setChecked( peNode->GetAutoUpdateMeasurementRange() ); 

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

  d->logic()->UpdateSceneToPlaybackTime( peNode, d->logic()->GetSelectedTime( peNode->GetTrackedSequenceBrowserNode() ) );  
}