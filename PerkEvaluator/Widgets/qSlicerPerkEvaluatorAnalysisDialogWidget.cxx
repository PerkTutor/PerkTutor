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
#include "qSlicerPerkEvaluatorAnalysisDialogWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerPerkEvaluatorAnalysisDialogWidgetPrivate
  : public Ui_qSlicerPerkEvaluatorAnalysisDialogWidget
{
  Q_DECLARE_PUBLIC(qSlicerPerkEvaluatorAnalysisDialogWidget);
protected:
  qSlicerPerkEvaluatorAnalysisDialogWidget* const q_ptr;

public:
  qSlicerPerkEvaluatorAnalysisDialogWidgetPrivate( qSlicerPerkEvaluatorAnalysisDialogWidget& object);
  ~qSlicerPerkEvaluatorAnalysisDialogWidgetPrivate();
  virtual void setupUi(qSlicerPerkEvaluatorAnalysisDialogWidget*);
};

// --------------------------------------------------------------------------
qSlicerPerkEvaluatorAnalysisDialogWidgetPrivate
::qSlicerPerkEvaluatorAnalysisDialogWidgetPrivate( qSlicerPerkEvaluatorAnalysisDialogWidget& object) : q_ptr(&object)
{
}

qSlicerPerkEvaluatorAnalysisDialogWidgetPrivate
::~qSlicerPerkEvaluatorAnalysisDialogWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerPerkEvaluatorAnalysisDialogWidgetPrivate
::setupUi(qSlicerPerkEvaluatorAnalysisDialogWidget* widget)
{
  this->Ui_qSlicerPerkEvaluatorAnalysisDialogWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorAnalysisDialogWidget methods

//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorAnalysisDialogWidget
::qSlicerPerkEvaluatorAnalysisDialogWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerPerkEvaluatorAnalysisDialogWidgetPrivate(*this) )
{
  this->PerkEvaluatorLogic = vtkSlicerPerkEvaluatorLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "PerkEvaluator" ) );
  this->PerkEvaluatorNode = NULL;
  this->setup();
}


qSlicerPerkEvaluatorAnalysisDialogWidget
::~qSlicerPerkEvaluatorAnalysisDialogWidget()
{
}


void qSlicerPerkEvaluatorAnalysisDialogWidget
::setup()
{
  Q_D(qSlicerPerkEvaluatorAnalysisDialogWidget);

  d->setupUi(this);

  // Some defaults
  this->setWindowModality( Qt::ApplicationModal );
  this->hide();

  // Connect the canceled signal
  connect( d->CancelButton, SIGNAL( clicked() ), this, SLOT( onAnalysisCanceled() ) );
}


void qSlicerPerkEvaluatorAnalysisDialogWidget
::enter()
{
}


void qSlicerPerkEvaluatorAnalysisDialogWidget
::setPerkEvaluatorNode( vtkMRMLNode* node )
{
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( node );
  this->PerkEvaluatorNode = peNode;
}


void qSlicerPerkEvaluatorAnalysisDialogWidget
::setLabelText( std::string newLabelText )
{
  Q_D(qSlicerPerkEvaluatorAnalysisDialogWidget);  
  
  d->AnalysisLabel->setText( newLabelText.c_str() );
}


void qSlicerPerkEvaluatorAnalysisDialogWidget
::show()
{
  Q_D(qSlicerPerkEvaluatorAnalysisDialogWidget);  
  
  if ( this->PerkEvaluatorNode == NULL )
  {
    return;
  }
  if ( this->PerkEvaluatorNode->GetMetricsTableNode() == NULL )
  {
    return;
  }
  
  this->Superclass::show();
  d->ProgressBar->setValue( 0 );
  this->qvtkConnect( this->PerkEvaluatorNode, vtkMRMLPerkEvaluatorNode::AnalysisStateUpdatedEvent, this, SLOT( onAnalysisStateUpdated( vtkObject*, void* ) ) );
}


void qSlicerPerkEvaluatorAnalysisDialogWidget
::hide()
{
  Q_D(qSlicerPerkEvaluatorAnalysisDialogWidget);

  this->qvtkDisconnect( this->PerkEvaluatorNode, vtkMRMLPerkEvaluatorNode::AnalysisStateUpdatedEvent, this, SLOT( onAnalysisStateUpdated( vtkObject*, void* ) ) );
  this->Superclass::hide();
}


void qSlicerPerkEvaluatorAnalysisDialogWidget
::onAnalysisStateUpdated( vtkObject* caller, void* value )
{
  Q_D(qSlicerPerkEvaluatorAnalysisDialogWidget);

  // Check we can downcast
  int* progressValue = reinterpret_cast< int* >( value );
  d->ProgressBar->setValue( *progressValue );
  qSlicerCoreApplication::processEvents(); // TODO: This will allow any button clicks in the analysis dialog to be processed
}


void qSlicerPerkEvaluatorAnalysisDialogWidget
::onAnalysisCanceled()
{
  Q_D(qSlicerPerkEvaluatorAnalysisDialogWidget);
  
  if ( this->PerkEvaluatorNode == NULL )
  {
    return;
  }
  
  this->PerkEvaluatorNode->SetAnalysisState( -1 ); // This will halt the analysis
}