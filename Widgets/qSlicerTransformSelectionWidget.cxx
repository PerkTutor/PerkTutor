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
#include "qSlicerTransformSelectionWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerTransformSelectionWidgetPrivate
  : public Ui_qSlicerTransformSelectionWidget
{
  Q_DECLARE_PUBLIC(qSlicerTransformSelectionWidget);
protected:
  qSlicerTransformSelectionWidget* const q_ptr;

public:
  qSlicerTransformSelectionWidgetPrivate( qSlicerTransformSelectionWidget& object);
  ~qSlicerTransformSelectionWidgetPrivate();
  virtual void setupUi(qSlicerTransformSelectionWidget*);
};

// --------------------------------------------------------------------------
qSlicerTransformSelectionWidgetPrivate
::qSlicerTransformSelectionWidgetPrivate( qSlicerTransformSelectionWidget& object) : q_ptr(&object)
{
}

qSlicerTransformSelectionWidgetPrivate
::~qSlicerTransformSelectionWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerTransformSelectionWidgetPrivate
::setupUi(qSlicerTransformSelectionWidget* widget)
{
  this->Ui_qSlicerTransformSelectionWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerTransformSelectionWidget methods

//-----------------------------------------------------------------------------
qSlicerTransformSelectionWidget
::qSlicerTransformSelectionWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerTransformSelectionWidgetPrivate(*this) )
{
}


qSlicerTransformSelectionWidget
::~qSlicerTransformSelectionWidget()
{
}


qSlicerTransformSelectionWidget* qSlicerTransformSelectionWidget
::New( qSlicerTransformBufferWidget* newBufferWidget, vtkSlicerPerkEvaluatorLogic* newPerkEvaluatorLogic )
{
  qSlicerTransformSelectionWidget* newtransformSelectionWidget = new qSlicerTransformSelectionWidget();
  newtransformSelectionWidget->BufferWidget = newBufferWidget;
  newtransformSelectionWidget->PerkEvaluatorLogic = newPerkEvaluatorLogic;
  newtransformSelectionWidget->BufferStatus = newBufferWidget->BufferStatus;
  newtransformSelectionWidget->BufferMessagesStatus = newBufferWidget->BufferMessagesStatus;
  newtransformSelectionWidget->setup();
  return newtransformSelectionWidget;
}


void qSlicerTransformSelectionWidget
::setup()
{
  Q_D(qSlicerTransformSelectionWidget);

  d->setupUi(this);
  this->setMRMLScene( this->BufferWidget->TransformRecorderLogic->GetMRMLScene() );

  connect( d->SelectAllButton, SIGNAL( clicked() ), this, SLOT( onSelectAllClicked() ) );
  connect( d->UnselectAllButton, SIGNAL( clicked() ), this, SLOT( onUnselectAllClicked() ) );
  
  // GUI refresh: updates every 10ms
  QTimer *t = new QTimer( this );
  connect( t,  SIGNAL( timeout() ), this, SLOT( updateWidget() ) );
  t->start(10); 

  this->updateWidget();  
}


void qSlicerTransformSelectionWidget
::enter()
{
}


void qSlicerTransformSelectionWidget
::onSelectAllClicked()
{
  Q_D(qSlicerTransformSelectionWidget);  

  vtkSmartPointer< vtkCollection > nodes = vtkSmartPointer< vtkCollection >::New();
  this->PerkEvaluatorLogic->GetSceneVisibleTransformNodes( nodes );
  
  for ( int i = 0; i < nodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( nodes->GetItemAsObject( i ) );
	  if ( transformNode != NULL )
	  {
	    this->PerkEvaluatorLogic->AddAnalyzeTransform( transformNode );
	  }
  }
  
  this->BufferStatus = this->BufferWidget->BufferStatus - 1; // Hack to cue update without buffer change
  this->updateWidget();
}


void qSlicerTransformSelectionWidget
::onUnselectAllClicked()
{
  Q_D(qSlicerTransformSelectionWidget);  

  vtkSmartPointer< vtkCollection > nodes = vtkSmartPointer< vtkCollection >::New();
  this->PerkEvaluatorLogic->GetSceneVisibleTransformNodes( nodes );
  
  for ( int i = 0; i < nodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( nodes->GetItemAsObject( i ) );
	  if ( transformNode != NULL )
	  {
	    this->PerkEvaluatorLogic->RemoveAnalyzeTransform( transformNode );
	  }
  }

  this->BufferStatus = this->BufferWidget->BufferStatus - 1; // Hack to cue update without buffer change
  this->updateWidget();
}


void qSlicerTransformSelectionWidget
::onTransformSelectionChanged()
{
  Q_D(qSlicerTransformSelectionWidget);

  // Find who the sender is and the corresponding node
  QComboBox* sender = (QComboBox*) this->sender();
  vtkMRMLLinearTransformNode* changedNode = this->ComboBoxToNodeMap[ sender ];
  
  if ( sender->currentText().toStdString().compare( "Selected" ) == 0 )
  {
    this->PerkEvaluatorLogic->AddAnalyzeTransform( changedNode );
  }
  else
  {
    this->PerkEvaluatorLogic->RemoveAnalyzeTransform( changedNode );
  }

  this->BufferStatus = this->BufferWidget->BufferStatus - 1; // Hack to cue update without buffer change
  this->updateWidget();
}


void qSlicerTransformSelectionWidget
::updateWidget()
{
  Q_D(qSlicerTransformSelectionWidget);

  if ( this->BufferWidget->TransformRecorderLogic == NULL )
  {
    return;
  }

  // Only update if the buffer has changed
  if ( this->BufferStatus == this->BufferWidget->BufferStatus && this->BufferMessagesStatus == this->BufferWidget->BufferMessagesStatus )
  {
    return;
  }
  this->BufferStatus = this->BufferWidget->BufferStatus;
  this->BufferMessagesStatus = this->BufferWidget->BufferMessagesStatus;

  
  // Check what the current row and column are
  int currentRow = d->TransformSelectionTable->currentRow();
  int currentColumn = d->TransformSelectionTable->currentColumn();
  int scrollPosition = d->TransformSelectionTable->verticalScrollBar()->value();
  
  // The only thing to do is update the table entries. Must ensure they are in sorted order (that's how they are stored in the buffer).
  d->TransformSelectionTable->clear();
  d->TransformSelectionTable->setRowCount( 0 );
  d->TransformSelectionTable->setColumnCount( 0 );

  if ( this->BufferWidget->GetBufferNode() == NULL )
  {
    return;
  }
  
  vtkSmartPointer< vtkCollection > nodes = vtkSmartPointer< vtkCollection >::New();
  this->PerkEvaluatorLogic->GetSceneVisibleTransformNodes( nodes ); 

  d->TransformSelectionTable->setRowCount( nodes->GetNumberOfItems() );
  d->TransformSelectionTable->setColumnCount( 2 );
  QStringList TransformSelectionTableHeaders;
  TransformSelectionTableHeaders << "Transform" << "Selection";
  d->TransformSelectionTable->setHorizontalHeaderLabels( TransformSelectionTableHeaders ); 
  d->TransformSelectionTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch );

  // Set the node selections in the table

  for ( int i = 0; i < nodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( nodes->GetItemAsObject( i ) );
    if ( transformNode == NULL )
    {
      continue;
    }
	
    // If it is a transform node, add it to the table
    QTableWidgetItem* nameItem = new QTableWidgetItem( QString::fromStdString( transformNode->GetName() ) );
    d->TransformSelectionTable->setItem( i, 0, nameItem );
	
    // Create the combo box
    QComboBox* selectionComboBox = new QComboBox();
    selectionComboBox->addItem( QString::fromStdString( "Selected" ) );
    selectionComboBox->addItem( QString::fromStdString( "Unselected" ) );
	
    if ( this->PerkEvaluatorLogic->IsAnalyzeTransform( transformNode ) )
    {
      selectionComboBox->setCurrentIndex( 0 );
    }
    else
    {
      selectionComboBox->setCurrentIndex( 1 );
    }
	
    connect( selectionComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onTransformSelectionChanged() ) );
	
    d->TransformSelectionTable->setCellWidget( i, 1, selectionComboBox );

    // Populate the maps
    this->ComboBoxToNodeMap[ selectionComboBox ] = transformNode;
    this->NodeToComboBoxMap[ transformNode ] = selectionComboBox;
  }

  // Reset the current row and column to what they were
  d->TransformSelectionTable->setCurrentCell( currentRow, currentColumn );
  d->TransformSelectionTable->verticalScrollBar()->setValue( scrollPosition );
  d->TransformSelectionTable->resizeRowsToContents();

}
