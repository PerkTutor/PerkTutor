
// Qt includes
#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QTimer>

// SlicerQt includes
#include "qSlicerPerkEvaluatorModuleWidget.h"
#include "ui_qSlicerPerkEvaluatorModule.h"

#include "vtkSlicerPerkEvaluatorLogic.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLNode.h"



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
  this->Timer = new QTimer( this );
  this->TimerIntervalSec = 0.1;
}


//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorModuleWidget::~qSlicerPerkEvaluatorModuleWidget()
{
  delete this->Timer;
}



void qSlicerPerkEvaluatorModuleWidget
::OnImportClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  QString filename = QFileDialog::getOpenFileName( this, tr("Open record"), "", tr("XML Files (*.xml)") );
  
  if ( filename.isEmpty() == false )
  {
    /*
    QMessageBox mb( this );
    mb.setModal( true );
    mb.setStandardButtons( 0 );
    mb.setText( QString( "Please wait while analysing data..." ) );
    mb.update();
    mb.show();
    */
    
    QProgressDialog dialog;
    dialog.setModal( true );
    dialog.setLabelText( "Please wait while reading XML file..." );
    dialog.show();
    dialog.setValue( 10 );
    
    d->logic()->ImportFile( filename.toStdString() );
    dialog.setValue( 60 );
    QFileInfo pathInfo( filename );
    d->FileNameLabel->setText( pathInfo.fileName() );
    dialog.setValue( 70 );
    d->logic()->SetPlaybackTime( d->logic()->GetMinTime() );
    dialog.close();
    
    // mb.hide();
  }
  
  this->UpdateGUI();
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackSliderChanged( double value )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  d->logic()->SetPlaybackTime( value + d->logic()->GetMinTime() );
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackNextClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  d->logic()->SetPlaybackTime( d->logic()->GetPlaybackTime() + 0.2 );
  this->UpdateGUI();
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackPrevClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  d->logic()->SetPlaybackTime( d->logic()->GetPlaybackTime() - 0.2 );
  this->UpdateGUI();
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackBeginClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  d->logic()->SetPlaybackTime( d->logic()->GetMinTime() );
  this->UpdateGUI();
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackEndClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  d->logic()->SetPlaybackTime( d->logic()->GetMaxTime() );
  this->UpdateGUI();
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackPlayClicked()
{
  this->Timer->start( int( this->TimerIntervalSec * 1000 ) );
}



void qSlicerPerkEvaluatorModuleWidget
::OnPlaybackStopClicked()
{
  this->Timer->stop();
}



void qSlicerPerkEvaluatorModuleWidget
::OnTimeout()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  double newPlaybackTime = d->logic()->GetPlaybackTime() + this->TimerIntervalSec;
  if ( newPlaybackTime >= d->logic()->GetMaxTime() )
  {
    if ( d->PlaybackRepeatCheckBox->checkState() == Qt::Checked )
    {
      d->logic()->SetPlaybackTime( d->logic()->GetMinTime() );
    }
    else
    {
      d->logic()->SetPlaybackTime( d->logic()->GetMaxTime() );
      this->Timer->stop();
    }
  }
  else
  {
    d->logic()->SetPlaybackTime( d->logic()->GetPlaybackTime() + this->TimerIntervalSec );
  }
  
  this->UpdateGUI();
}



void qSlicerPerkEvaluatorModuleWidget
::OnMarkBeginClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  d->BeginSpinBox->setValue( d->logic()->GetPlaybackTime() - d->logic()->GetMinTime() );
}



void qSlicerPerkEvaluatorModuleWidget
::OnMarkEndClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  d->EndSpinBox->setValue( d->logic()->GetPlaybackTime() - d->logic()->GetMinTime() );
}



void qSlicerPerkEvaluatorModuleWidget
::OnAnalyseClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  
  
  double begin = d->BeginSpinBox->value() + d->logic()->GetMinTime();
  double end = d->EndSpinBox->value() + d->logic()->GetMinTime();
  d->logic()->SetMarkBegin( begin );
  d->logic()->SetMarkEnd( end ); 
  
  d->logic()->Analyse();
  
  
  
  this->UpdateGUI();
}



void
qSlicerPerkEvaluatorModuleWidget
::OnBodyModelNodeSelected()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  vtkMRMLModelNode* mnode = vtkMRMLModelNode::SafeDownCast( d->BodyNodeComboBox->currentNode() );
  d->logic()->SetBodyModelNode( mnode );
}



void
qSlicerPerkEvaluatorModuleWidget
::OnNeedleReferenceSelected()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  vtkMRMLLinearTransformNode* tnode = vtkMRMLLinearTransformNode::SafeDownCast( d->NeedleReferenceComboBox->currentNode() );
  d->logic()->SetNeedleTransformNode( tnode );
}



void
qSlicerPerkEvaluatorModuleWidget
::setup()
{
  Q_D(qSlicerPerkEvaluatorModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
  
  connect( d->ImportButton, SIGNAL( clicked() ), this, SLOT( OnImportClicked() ) );
  connect( d->PlaybackSlider, SIGNAL( valueChanged( double ) ), this, SLOT( OnPlaybackSliderChanged( double ) ) );
  connect( d->NextButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackNextClicked() ) );
  connect( d->PrevButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackPrevClicked() ) );
  connect( d->BeginButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackBeginClicked() ) );
  connect( d->EndButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackEndClicked() ) );
  connect( d->PlayButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackPlayClicked() ) );
  connect( d->StopButton, SIGNAL( clicked() ), this, SLOT( OnPlaybackStopClicked() ) );
  connect( this->Timer, SIGNAL( timeout() ), this, SLOT( OnTimeout() ) );
  connect( d->MarkBeginButton, SIGNAL( clicked() ), this, SLOT( OnMarkBeginClicked() ) );
  connect( d->MarkEndButton, SIGNAL( clicked() ), this, SLOT( OnMarkEndClicked() ) );
  connect( d->AnalyseButton, SIGNAL( clicked() ), this, SLOT( OnAnalyseClicked() ) );
  connect( d->BodyNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( OnBodyModelNodeSelected() ) );
  connect( d->NeedleReferenceComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( OnNeedleReferenceSelected() ) );
}



void qSlicerPerkEvaluatorModuleWidget
::UpdateGUI()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );
  
  d->PlaybackSlider->setMinimum( 0 );
  d->PlaybackSlider->setMaximum( d->logic()->GetMaxTime() - d->logic()->GetMinTime() );
  d->PlaybackSlider->setValue( d->logic()->GetPlaybackTime() - d->logic()->GetMinTime() );
  
    // Annotations table
  
  vtkSlicerPerkEvaluatorLogic::AnnotationVectorType annotations = d->logic()->GetAnnotations();
  
  d->AnnotationsTable->setRowCount( annotations.size() );
  d->AnnotationsTable->setColumnCount( 2 );
  QStringList AnnotationHeaders;
  AnnotationHeaders << "Time" << "Annotation";
  d->AnnotationsTable->setHorizontalHeaderLabels( AnnotationHeaders );
  d->AnnotationsTable->setColumnWidth( 1, 250 );
  
  for ( int i = 0; i < annotations.size(); ++ i )
  {
    QTableWidgetItem* TimeItem = new QTableWidgetItem( QString::number( annotations[ i ].first - d->logic()->GetMinTime(), 'f', 1 ) );
    QTableWidgetItem* NoteItem = new QTableWidgetItem( QString::fromStdString( annotations[ i ].second ) );
    d->AnnotationsTable->setItem( i, 0, TimeItem );
    d->AnnotationsTable->setItem( i, 1, NoteItem );
  }
  
  
    // Metrics table
  
  vtkSlicerPerkEvaluatorLogic::MetricVectorType metrics = d->logic()->GetMetrics();
  
  d->MetricsTable->setRowCount( metrics.size() );
  d->MetricsTable->setColumnCount( 2 );
  QStringList headerLabels;
  headerLabels << "Metric name" << "Value";
  d->MetricsTable->setHorizontalHeaderLabels( headerLabels );
  d->MetricsTable->setColumnWidth( 0, 250 );
  
  for ( int i = 0; i < metrics.size(); ++ i )
  {
    QTableWidgetItem* nameItem = new QTableWidgetItem( QString::fromStdString( metrics[ i ].first ) );
    QTableWidgetItem* valueItem = new QTableWidgetItem( QString::number( metrics[ i ].second, 'f', 2 ) );
    d->MetricsTable->setItem( i, 0, nameItem );
    d->MetricsTable->setItem( i, 1, valueItem );
  }
  
}

