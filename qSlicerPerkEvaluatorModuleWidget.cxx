
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
  qSlicerPerkEvaluatorTransformBufferWidget* TransformBufferWidget;
  qSlicerPerkEvaluatorMessagesWidget* MessagesWidget;
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
  
  d->logic()->SetPlaybackTime( value + d->logic()->GetMinTime() );
  this->updateWidget();
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackNextClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  d->logic()->SetPlaybackTime( d->logic()->GetPlaybackTime() + 0.2 );
  this->updateWidget();
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackPrevClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  d->logic()->SetPlaybackTime( d->logic()->GetPlaybackTime() - 0.2 );
  this->updateWidget();
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackBeginClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  d->logic()->SetPlaybackTime( d->logic()->GetMinTime() );
  this->updateWidget();
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackEndClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  d->logic()->SetPlaybackTime( d->logic()->GetMaxTime() );
  this->updateWidget();
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackPlayClicked()
{
  this->PlaybackTimer->start( int( this->PlaybackTimerIntervalSec * 1000 ) );
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
  
  double newPlaybackTime = d->logic()->GetPlaybackTime() + this->PlaybackTimerIntervalSec;
  if ( newPlaybackTime >= d->logic()->GetMaxTime() )
  {
    if ( d->PlaybackRepeatCheckBox->checkState() == Qt::Checked )
    {
      d->logic()->SetPlaybackTime( d->logic()->GetMinTime() );
    }
    else
    {
      d->logic()->SetPlaybackTime( d->logic()->GetMaxTime() );
      this->PlaybackTimer->stop();
    }
  }
  else
  {
    d->logic()->SetPlaybackTime( d->logic()->GetPlaybackTime() + this->PlaybackTimerIntervalSec );
  }
  
  this->updateWidget();
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
  vtkMRMLTableNode* metricsNode = d->logic()->GetMetrics( peNode );
  if ( metricsNode == NULL )
  {
    return;
  }

  dialog.setValue( 80 );
  
  d->MetricsTable->clear();
  d->MetricsTable->setRowCount( 0 );
  d->MetricsTable->setColumnCount( 0 ); 

  QStringList MetricsTableHeaders;
  MetricsTableHeaders << "Metric" << "Value";
  d->MetricsTable->setRowCount( metricsNode->GetTable()->GetNumberOfRows() );
  d->MetricsTable->setColumnCount( 2 );
  d->MetricsTable->setHorizontalHeaderLabels( MetricsTableHeaders );
  d->MetricsTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  
  for ( int i = 0; i < metricsNode->GetTable()->GetNumberOfRows(); i++ )
  {
    QString nameString;
    nameString = nameString + QString( metricsNode->GetTable()->GetValueByName( i, "TransformName" ).ToString() );
    nameString = nameString + QString( " " );
    nameString = nameString + QString( metricsNode->GetTable()->GetValueByName( i, "MetricName" ).ToString() );
    QTableWidgetItem* nameItem = new QTableWidgetItem( nameString );
    d->MetricsTable->setItem( i, 0, nameItem );

    QString valueString;
    valueString = valueString + QString( metricsNode->GetTable()->GetValueByName( i, "MetricValue" ).ToString() );
    valueString = valueString + QString( " " );
    valueString = valueString + QString( metricsNode->GetTable()->GetValueByName( i, "MetricUnit" ).ToString() );
    QTableWidgetItem* valueItem = new QTableWidgetItem( valueString );    
    d->MetricsTable->setItem( i, 1, valueItem );
  }

  dialog.close();
}


void qSlicerPerkEvaluatorModuleWidget
::OnClipboardClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  // Grab all of the contents from whatever is currently on the metrics table
  QString clipString = QString( "" );

  for ( int i = 0; i < d->MetricsTable->rowCount(); i++ )
  {
    for ( int j = 0; j < d->MetricsTable->columnCount(); j++ )
    {
      QTableWidgetItem* currentItem = d->MetricsTable->item( i, j );
      clipString.append( currentItem->text() );
      clipString.append( QString( "\t" ) );
    }
    clipString.append( QString( "\n" ) );
  }

  QApplication::clipboard()->setText( clipString );
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

  d->BeginSpinBox->setValue( d->logic()->GetPlaybackTime() - d->logic()->GetMinTime() );
  peNode->SetMarkBegin( d->logic()->GetPlaybackTime() - d->logic()->GetMinTime() ); // Do this, otherwise the value will be rounded
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

  d->EndSpinBox->setValue( d->logic()->GetPlaybackTime() - d->logic()->GetMinTime() );
  peNode->SetMarkEnd( d->logic()->GetPlaybackTime() - d->logic()->GetMinTime() ); // Do this, otherwise the value will be rounded
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
  d->TransformBufferWidget = new qSlicerPerkEvaluatorTransformBufferWidget();
  d->BufferGroupBox->layout()->addWidget( d->TransformBufferWidget );
  d->TransformBufferWidget->setMRMLScene( NULL );
  d->TransformBufferWidget->setMRMLScene( d->logic()->GetMRMLScene() );
  d->TransformBufferWidget->setTransformBufferNode( NULL ); // Do not automatically select a node on entering the widget

  d->MessagesWidget = new qSlicerPerkEvaluatorMessagesWidget();
  d->MessagesGroupBox->layout()->addWidget( d->MessagesWidget );
  d->MessagesWidget->setMRMLScene( NULL ); 
  d->MessagesWidget->setMRMLScene( d->logic()->GetMRMLScene() ); 

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
  connect( d->TransformBufferWidget, SIGNAL( transformBufferNodeChanged( vtkMRMLTransformBufferNode* ) ), d->MessagesWidget, SLOT( setTransformBufferNode( vtkMRMLTransformBufferNode* ) ) );
}


void
qSlicerPerkEvaluatorModuleWidget
::setup()
{
  Q_D(qSlicerPerkEvaluatorModuleWidget);

  d->setupUi(this);

  // Embed widgets here
  this->setupEmbeddedWidgets();

  // Perk Evaluator node independent
  connect( d->PlaybackSlider, SIGNAL( valueChanged( double ) ), this, SLOT( OnPlaybackSliderChanged( double ) ) );
  connect( d->NextButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackNextClicked() ) );
  connect( d->PrevButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackPrevClicked() ) );
  connect( d->BeginButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackBeginClicked() ) );
  connect( d->EndButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackEndClicked() ) );
  connect( d->PlayButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackPlayClicked() ) );
  connect( d->StopButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackStopClicked() ) );

  connect( this->PlaybackTimer, SIGNAL( timeout() ), this, SLOT( OnTimeout() ) );

  connect( d->AnalyzeButton, SIGNAL( clicked() ), this, SLOT( OnAnalyzeClicked() ) );
  connect( d->ClipboardButton, SIGNAL( clicked() ), this, SLOT( OnClipboardClicked() ) );
  d->ClipboardButton->setIcon( QIcon( ":/Icons/Small/SlicerEditCopy.png" ) );

  // If the transform buffer node is changed, update everything
  connect( d->TransformBufferWidget, SIGNAL( transformBufferNodeChanged( vtkMRMLTransformBufferNode* ) ), this, SLOT( resetWidget() ) );
 
  // TODO: If the transform buffer is updated, then we want to clear the widget, and reset the tool trajectories
  // But resetting the tool trajectories is computationally expensive
  // It might be better to update the trajectories only when necessary parts of the GUI are interacted with (i.e. Analyze, or playback controls)



  // Perk Evaluator node dependent
  connect( d->BeginSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnMarkBeginChanged() ) );
  connect( d->MarkBeginButton, SIGNAL( clicked() ), this, SLOT( OnMarkBeginClicked() ) );
  connect( d->EndSpinBox, SIGNAL( valueChanged( double ) ), this, SLOT( OnMarkEndChanged() ) );
  connect( d->MarkEndButton, SIGNAL( clicked() ), this, SLOT( OnMarkEndClicked() ) );

  connect( d->BodyNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTissueModelChanged( vtkMRMLNode* ) ) );
  connect( d->NeedleReferenceComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onNeedleTransformChanged( vtkMRMLNode* ) ) );

  connect( d->MetricsDirectoryButton, SIGNAL( clicked() ), this, SLOT( OnMetricsDirectoryClicked() ) );
  connect( d->AutoUpdateMeasurementRangeCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( OnAutoUpdateMeasurementRangeToggled() ) );
  connect( d->AutoUpdateTransformRolesCheckBox, SIGNAL( toggled( bool ) ), this, SLOT( OnAutoUpdateTransformRolesToggled() ) );
  connect( d->NeedleOrientationButtonGroup, SIGNAL( buttonClicked( QAbstractButton* ) ), this, SLOT( onNeedleOrientationChanged( QAbstractButton* ) ) );


  // Connect the Perk Evaluator node to the update
  connect( d->PerkEvaluatorNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( mrmlNodeChanged( vtkMRMLNode* ) ) ); // If the node is changed connect it to update 
  // NOTE: The roles widgets will be updated with the other components of the widget

  this->clearWidget();
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
    vtkSmartPointer< vtkMRMLNode > peNode;
    peNode.TakeReference( this->mrmlScene()->CreateNodeByClass( "vtkMRMLPerkEvaluatorNode" ) );
    peNode->SetScene( this->mrmlScene() );
    this->mrmlScene()->AddNode( peNode );
    d->PerkEvaluatorNodeComboBox->setCurrentNode( peNode );
  }
  else
  {
    this->updateWidgetFromMRMLNode();
  }
}


void qSlicerPerkEvaluatorModuleWidget
::updateWidget()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  // Playback slider
  d->PlaybackSlider->setValue( d->logic()->GetPlaybackTime() - d->logic()->GetMinTime() );
}


void qSlicerPerkEvaluatorModuleWidget
::resetWidget()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  d->logic()->UpdateToolTrajectories( d->TransformBufferWidget->getTransformBufferNode() );
  d->logic()->SetPlaybackTime( d->logic()->GetMinTime() );

  // This is where we need to update parameters on buffer node changed
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( d->PerkEvaluatorNodeComboBox->currentNode() );

  if ( peNode != NULL && peNode->GetAutoUpdateMeasurementRange() )
  {
    peNode->SetMarkBegin( 0.0 );
    peNode->SetMarkEnd( d->logic()->GetTotalTime() );
  }

  if ( peNode != NULL && peNode->GetAutoUpdateTransformRoles() )
  {
    std::vector< std::string > anyRoleTransforms = d->logic()->GetAllBufferToolNames();
    for ( int i = 0; i < anyRoleTransforms.size(); i++ )
    {
      // If it already has a non-generic role, let it maintain the more specific role (since the generic metrics will be computed regardless)
      if ( peNode->GetTransformRole( anyRoleTransforms.at( i ) ).compare( "" ) == 0 )
      {
        peNode->SetTransformRole( anyRoleTransforms.at( i ), "Any" );
      }
    }
  }

  // Should also clear everything to default
  this->clearWidget();
}


void qSlicerPerkEvaluatorModuleWidget
::clearWidget()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  // Do not update trajectories
  this->updateWidget();
  d->PlaybackSlider->setMinimum( 0.0 );
  d->PlaybackSlider->setMaximum( d->logic()->GetTotalTime() );

  d->MetricsTable->clear();
  d->MetricsTable->setRowCount( 0 );
  d->MetricsTable->setColumnCount( 0 );
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

  // Update the roles widget every time the Perk Evaluator node is modified
  d->TransformRolesWidget->setPerkEvaluatorNode( peNode );
  d->AnatomyRolesWidget->setPerkEvaluatorNode( peNode );

  d->BeginSpinBox->setValue( peNode->GetMarkBegin() );
  d->EndSpinBox->setValue( peNode->GetMarkEnd() );

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

  
}