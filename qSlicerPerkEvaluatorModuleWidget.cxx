
// Qt includes
#include <QDebug>
#include <QtCore>
#include <QtGui>
#include <QTimer>

#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

// SlicerQt includes
#include "qSlicerPerkEvaluatorModuleWidget.h"
#include "ui_qSlicerPerkEvaluatorModule.h"

#include <qSlicerApplication.h>
#include <qSlicerCoreApplication.h>

#include "vtkSlicerPerkEvaluatorLogic.h"

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
::OnAnalyzeClicked()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );  
  
  double begin = d->BeginSpinBox->value() + d->logic()->GetMinTime();
  double end = d->EndSpinBox->value() + d->logic()->GetMinTime();
  d->logic()->SetMarkBegin( begin );
  d->logic()->SetMarkEnd( end );

  // Metrics table  
  std::vector<vtkSlicerPerkEvaluatorLogic::MetricType> metrics = d->logic()->GetMetrics();
  
  d->MetricsTable->clear();
  QStringList MetricsTableHeaders;
  MetricsTableHeaders << "Metric" << "Value";
  d->MetricsTable->setRowCount( metrics.size() );
  d->MetricsTable->setColumnCount( 2 );
  d->MetricsTable->setHorizontalHeaderLabels( MetricsTableHeaders );
  d->MetricsTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch );
  
  for ( int i = 0; i < metrics.size(); ++ i )
  {
    QTableWidgetItem* nameItem = new QTableWidgetItem( QString::fromStdString( metrics.at(i).first ) );
    QTableWidgetItem* valueItem = new QTableWidgetItem( QString::number( metrics.at(i).second, 'f', 2 ) );
    d->MetricsTable->setItem( i, 0, nameItem );
    d->MetricsTable->setItem( i, 1, valueItem );
  }
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
::OnNeedleOrientationChanged( QAbstractButton* newOrientationButton )
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  if ( newOrientationButton == d->PlusXRadioButton )
  {
    d->logic()->SetNeedleBase( 1.0, 0.0, 0.0 );
  }
  if ( newOrientationButton == d->MinusXRadioButton )
  {
    d->logic()->SetNeedleBase( -1.0, 0.0, 0.0 );
  }
  if ( newOrientationButton == d->PlusYRadioButton )
  {
    d->logic()->SetNeedleBase( 0.0, 1.0, 0.0 );
  }
  if ( newOrientationButton == d->MinusYRadioButton )
  {
    d->logic()->SetNeedleBase( 0.0, -1.0, 0.0 );
  }
  if ( newOrientationButton == d->PlusZRadioButton )
  {
    d->logic()->SetNeedleBase( 0.0, 0.0, 1.0 );
  }
  if ( newOrientationButton == d->MinusZRadioButton )
  {
    d->logic()->SetNeedleBase( 0.0, 0.0, -1.0 );
  }

}

void
qSlicerPerkEvaluatorModuleWidget
::setup()
{
  Q_D(qSlicerPerkEvaluatorModuleWidget);

  d->setupUi(this);
  // Embed widgets here
  d->TransformBufferWidget = qSlicerPerkEvaluatorTransformBufferWidget::New( d->logic() );
  d->BufferGroupBox->layout()->addWidget( d->TransformBufferWidget );
  d->MessagesWidget = qSlicerPerkEvaluatorMessagesWidget::New( d->TransformBufferWidget, d->logic() );
  d->MessagesGroupBox->layout()->addWidget( d->MessagesWidget ); 
  this->Superclass::setup();

  this->UpdateStatus = d->TransformBufferWidget->UpdateStatus;
  
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

  connect( d->AnalyzeButton, SIGNAL( clicked() ), this, SLOT( OnAnalyzeClicked() ) );

  connect( d->BodyNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( OnBodyModelNodeSelected() ) );
  connect( d->NeedleReferenceComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( OnNeedleReferenceSelected() ) );
  connect( d->NeedleOrientationButtonGroup, SIGNAL( buttonClicked( QAbstractButton* ) ), this, SLOT( OnNeedleOrientationChanged( QAbstractButton* ) ) );

  // GUI refresh: updates every 10ms
  QTimer *t = new QTimer( this );
  connect( t,  SIGNAL( timeout() ), this, SLOT( UpdateGUI() ) );
  t->start(10);

  this->UpdateGUI();
}



void qSlicerPerkEvaluatorModuleWidget
::UpdateGUI()
{
  Q_D( qSlicerPerkEvaluatorModuleWidget );

  if ( this->UpdateStatus != d->TransformBufferWidget->UpdateStatus )
  {
    d->PlaybackSlider->setMinimum( 0.0 );
    d->PlaybackSlider->setMaximum( d->logic()->GetTotalTime() );
    d->BeginSpinBox->setValue( 0.0 );
    d->EndSpinBox->setValue( d->logic()->GetTotalTime() );
    this->UpdateStatus = d->TransformBufferWidget->UpdateStatus;
  }
  
  // Playback slider
  d->PlaybackSlider->setValue( d->logic()->GetPlaybackTime() - d->logic()->GetMinTime() );  
}

