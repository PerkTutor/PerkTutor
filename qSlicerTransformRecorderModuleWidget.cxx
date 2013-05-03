

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
#include "qSlicerTransformRecorderModuleWidget.h"
#include "ui_qSlicerTransformRecorderModule.h"
#include "qSlicerIO.h"
#include "qSlicerIOManager.h"
#include "qSlicerApplication.h"
#include <QtGui>

// MRMLWidgets includes
#include <qMRMLUtils.h>

#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkMRMLTransformRecorderNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "qMRMLNodeComboBox.h"
#include "vtkMRMLViewNode.h"
#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkMRMLTransformRecorderNode.h"
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TransformRecorder
class qSlicerTransformRecorderModuleWidgetPrivate: public Ui_qSlicerTransformRecorderModule
{
  Q_DECLARE_PUBLIC( qSlicerTransformRecorderModuleWidget ); 

protected:
  qSlicerTransformRecorderModuleWidget* const q_ptr;
public:
  qSlicerTransformRecorderModuleWidgetPrivate( qSlicerTransformRecorderModuleWidget& object );
  ~qSlicerTransformRecorderModuleWidgetPrivate();

  vtkSlicerTransformRecorderLogic* logic() const;
  vtkMRMLLinearTransformNode*   MRMLTransformNode;
};

//-----------------------------------------------------------------------------
// qSlicerTransformRecorderModuleWidgetPrivate methods



qSlicerTransformRecorderModuleWidgetPrivate::qSlicerTransformRecorderModuleWidgetPrivate( qSlicerTransformRecorderModuleWidget& object ) : q_ptr(&object)
{
  this->MRMLTransformNode = 0;
}

//-----------------------------------------------------------------------------

qSlicerTransformRecorderModuleWidgetPrivate::~qSlicerTransformRecorderModuleWidgetPrivate()
{
}


vtkSlicerTransformRecorderLogic* qSlicerTransformRecorderModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerTransformRecorderModuleWidget );
  return vtkSlicerTransformRecorderLogic::SafeDownCast( q->logic() );
}


//-----------------------------------------------------------------------------
// qSlicerTransformRecorderModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTransformRecorderModuleWidget::qSlicerTransformRecorderModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerTransformRecorderModuleWidgetPrivate( *this ) )
{
}



qSlicerTransformRecorderModuleWidget::~qSlicerTransformRecorderModuleWidget()
{
}



void qSlicerTransformRecorderModuleWidget::setup()
{
  Q_D(qSlicerTransformRecorderModuleWidget);

  d->StatusResultLabel= NULL;
  d->NumRecordsResultLabel=NULL;
  d->TotalTimeResultsLabel=NULL;
  d->TotaNeedlelPathResultsLabel=NULL;
  d->InsideNeedlePathResultsLabel=NULL;

  d->setupUi(this);
  d->verticalLayout->addWidget( qSlicerMessagesWidget::New( d->logic() ) );  
  this->Superclass::setup();


  d->ModuleComboBox->setNoneEnabled( true );

  connect( d->ModuleComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModuleNodeSelected() ) );
  connect( d->LoadLogButton, SIGNAL( clicked() ), this, SLOT ( saveToFile() ) );
  connect( d->StartButton, SIGNAL( pressed() ), this, SLOT( onStartButtonPressed() ) );
  connect( d->StopButton, SIGNAL( pressed() ), this, SLOT( onStopButtonPressed() ) );
  connect( d->ClearBufferButton, SIGNAL( pressed() ), this, SLOT( onClearBufferButtonPressed() ) );
  connect( d->TransformCheckableComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ),
           this, SLOT( onTransformsNodeSelected( vtkMRMLNode* ) ) );
  
  
  // GUI refresh: updates every 10ms
  QTimer *t = new QTimer( this );
  connect( t,  SIGNAL( timeout() ), this, SLOT( updateWidget() ) );
  t->start(10); 

}



void qSlicerTransformRecorderModuleWidget
::onTransformsNodeSelected( vtkMRMLNode* node )
{
  Q_D(qSlicerTransformRecorderModuleWidget);
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



void qSlicerTransformRecorderModuleWidget::onMRMLTransformNodeModified( vtkObject* caller )
{
  Q_D( qSlicerTransformRecorderModuleWidget );
  
  // TODO: I'm not sure this function is needed at all.
  /*
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( caller );
  if (!transformNode) { return; }
  
  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
  */
}



void qSlicerTransformRecorderModuleWidget::saveToFile()
{
  Q_D( qSlicerTransformRecorderModuleWidget );
  
  QString filename = QFileDialog::getSaveFileName( this, tr("Save record"), "", tr("XML Files (*.xml)") );
  
  if ( filename.isEmpty() == false )
  {
    d->logic()->SaveToFile( filename.toStdString() );
  }
  
  this->updateWidget();
}



void qSlicerTransformRecorderModuleWidget::enter()
{
  this->Superclass::enter();
  this->updateWidget();
}



void qSlicerTransformRecorderModuleWidget::onModuleNodeSelected()
{
  Q_D( qSlicerTransformRecorderModuleWidget );
 
  vtkMRMLNode* currentNode = d->ModuleComboBox->currentNode();
  std::cout << "Current node pointer: " << currentNode << std::endl;
  
  vtkMRMLTransformRecorderNode* TRNode = vtkMRMLTransformRecorderNode::SafeDownCast( currentNode );
  if ( TRNode != NULL )
  {
    
  }
  
  d->logic()->SetModuleNode( TRNode );
  this->updateSelectionsFromObservedNodes();
  this->updateWidget();
}



void qSlicerTransformRecorderModuleWidget
::onStartButtonPressed()
{
  Q_D( qSlicerTransformRecorderModuleWidget );
    
  // Go through transform types (ie ProbeToReference, StylusTipToReference, etc)  
  for ( int i = 0; i < d->TransformCheckableComboBox->nodeCount(); i++ )
  {
    if( d->TransformCheckableComboBox->checkState( d->TransformCheckableComboBox->nodeFromIndex( i ) ) == Qt::Checked  )
    {
      d->logic()->AddObservedTransformNode( d->TransformCheckableComboBox->nodeFromIndex( i )->GetID() );
    }
    else
    {
      d->logic()->RemoveObservedTransformNode( d->TransformCheckableComboBox->nodeFromIndex( i )->GetID() );
    }
  }
    
  d->logic()->SetRecording( true );
  
  this->updateWidget();
  
}



void qSlicerTransformRecorderModuleWidget
::onStopButtonPressed()
{
  Q_D( qSlicerTransformRecorderModuleWidget );

  d->logic()->SetRecording( false );
   
  this->updateWidget();
}



void qSlicerTransformRecorderModuleWidget
::onClearBufferButtonPressed() 
{
  Q_D( qSlicerTransformRecorderModuleWidget );
  
  d->logic()->ClearBuffer();

  this->updateWidget();
}


void qSlicerTransformRecorderModuleWidget
::updateSelectionsFromObservedNodes()
{
  Q_D( qSlicerTransformRecorderModuleWidget );

  // Assume the default is not checked, and check all those that are observed
  for ( int i = 0; i < d->TransformCheckableComboBox->nodeCount(); i++ )
  {
    if( d->logic()->IsObservedTransformNode( d->TransformCheckableComboBox->nodeFromIndex( i )->GetID() ) )
    {
	  d->TransformCheckableComboBox->setCheckState( d->TransformCheckableComboBox->nodeFromIndex( i ), Qt::Checked );
    }
  }

}



void qSlicerTransformRecorderModuleWidget::updateWidget()
{
  Q_D( qSlicerTransformRecorderModuleWidget );
  
  
  // Disableing node selector widgets if there is no module node to reference input nodes.    
  if ( d->logic()->GetModuleNode() == NULL )
  {
    d->TransformCheckableComboBox->setEnabled( false );
	return;
  }
  else
  {
    d->TransformCheckableComboBox->setEnabled( true );
  }
 
  
  // Update status descriptor labels.
  
  if ( d->logic()->GetRecording() )
  {
    d->StatusResultLabel->setText( "Recording" );
    d->TransformCheckableComboBox->setEnabled( false );
  }
  else
  {
    d->StatusResultLabel->setText( "Waiting" );
    d->TransformCheckableComboBox->setEnabled( true );
  }
  
  
  int numRec = d->logic()->GetBufferSize();
  std::stringstream ss;
  ss << numRec;
  d->NumRecordsResultLabel->setText( QString::fromStdString( ss.str() ) );
  
  ss.str( "" );
  ss.precision( 2 );
  ss << std::fixed << d->logic()->GetTotalTime();
  d->TotalTimeResultsLabel->setText( ss.str().c_str() );
  
  ss.str( "" );
  ss.precision( 2 );
  ss << std::fixed << d->logic()->GetTotalPath();
  d->TotaNeedlelPathResultsLabel->setText( ss.str().c_str() );
  
  ss.str( "" );
  ss.precision( 2 );
  ss << std::fixed << d->logic()->GetTotalPathInside();
  d->InsideNeedlePathResultsLabel->setText( ss.str().c_str() );
   
}
