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
::qSlicerTransformSelectionWidget(QWidget* parentWidget) : qSlicerWidget( parentWidget ) , d_ptr( new qSlicerTransformSelectionWidgetPrivate(*this) )
{
  this->BufferHelper = new qSlicerTransformBufferWidgetHelper();
  this->PerkEvaluatorLogic = vtkSlicerPerkEvaluatorLogic::SafeDownCast( qSlicerTransformBufferWidgetHelper::GetSlicerModuleLogic( "PerkEvaluator" ) );
  this->setup();
}


qSlicerTransformSelectionWidget
::~qSlicerTransformSelectionWidget()
{
}


void qSlicerTransformSelectionWidget
::setup()
{
  Q_D(qSlicerTransformSelectionWidget);

  d->setupUi(this);

  connect( d->SelectAllButton, SIGNAL( clicked() ), this, SLOT( onSelectAllClicked() ) );
  connect( d->UnselectAllButton, SIGNAL( clicked() ), this, SLOT( onUnselectAllClicked() ) );

  // Listen for updates from the helper
  connect( this->BufferHelper, SIGNAL( transformBufferNodeChanged( vtkMRMLTransformBufferNode* ) ), this, SLOT( updateWidget() ) );
  connect( this->BufferHelper, SIGNAL( transformBufferTransformAdded( int ) ), this, SLOT( updateWidget() ) );
  connect( this->BufferHelper, SIGNAL( transformBufferTransformRemoved( int ) ), this, SLOT( updateWidget() ) ); // In case a new transform name has been added/removed

  this->updateWidget();  
}


void qSlicerTransformSelectionWidget
::setMRMLScene( vtkMRMLScene* newScene )
{
  this->qvtkDisconnect( this->mrmlScene(), vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->qSlicerWidget::setMRMLScene( newScene );
  this->qvtkConnect( this->mrmlScene(), vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
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

  this->updateWidget();
}


void qSlicerTransformSelectionWidget
::updateWidget()
{
  Q_D(qSlicerTransformSelectionWidget);

  if ( this->PerkEvaluatorLogic == NULL )
  {
    return;
  }
  
  // Check what the current row and column are
  int currentRow = d->TransformSelectionTable->currentRow();
  int currentColumn = d->TransformSelectionTable->currentColumn();
  int scrollPosition = d->TransformSelectionTable->verticalScrollBar()->value();
  
  // The only thing to do is update the table entries. Must ensure they are in sorted order (that's how they are stored in the buffer).
  d->TransformSelectionTable->clear();
  d->TransformSelectionTable->setRowCount( 0 );
  d->TransformSelectionTable->setColumnCount( 0 );
  
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
