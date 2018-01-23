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
#include "qSlicerPerkEvaluatorRolesWidget.h"

#include <QtGui>
#include <QScrollBar>

#include "vtkSlicerConfigure.h" // For Slicer_HAVE_QT5

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerPerkEvaluatorRolesWidgetPrivate
  : public Ui_qSlicerPerkEvaluatorRolesWidget
{
  Q_DECLARE_PUBLIC(qSlicerPerkEvaluatorRolesWidget);
protected:
  qSlicerPerkEvaluatorRolesWidget* const q_ptr;

public:
  qSlicerPerkEvaluatorRolesWidgetPrivate( qSlicerPerkEvaluatorRolesWidget& object);
  ~qSlicerPerkEvaluatorRolesWidgetPrivate();
  virtual void setupUi(qSlicerPerkEvaluatorRolesWidget*);
};

// --------------------------------------------------------------------------
qSlicerPerkEvaluatorRolesWidgetPrivate
::qSlicerPerkEvaluatorRolesWidgetPrivate( qSlicerPerkEvaluatorRolesWidget& object) : q_ptr(&object)
{
}

qSlicerPerkEvaluatorRolesWidgetPrivate
::~qSlicerPerkEvaluatorRolesWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerPerkEvaluatorRolesWidgetPrivate
::setupUi(qSlicerPerkEvaluatorRolesWidget* widget)
{
  this->Ui_qSlicerPerkEvaluatorRolesWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorRolesWidget methods

//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorRolesWidget
::qSlicerPerkEvaluatorRolesWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerPerkEvaluatorRolesWidgetPrivate(*this) )
{
  this->PerkEvaluatorLogic = vtkSlicerPerkEvaluatorLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "PerkEvaluator" ) );
  this->MetricInstanceNode = NULL;
  this->setup();
}


qSlicerPerkEvaluatorRolesWidget
::~qSlicerPerkEvaluatorRolesWidget()
{
}



void qSlicerPerkEvaluatorRolesWidget
::setup()
{
  Q_D(qSlicerPerkEvaluatorRolesWidget);

  d->setupUi(this);

  // No connections to set...
}


void qSlicerPerkEvaluatorRolesWidget
::setMRMLScene( vtkMRMLScene* newScene )
{
  this->qvtkDisconnect( this->mrmlScene(), vtkMRMLScene::NodeAddedEvent, this, SLOT( updateWidget() ) );
  this->qvtkDisconnect( this->mrmlScene(), vtkMRMLScene::NodeRemovedEvent, this, SLOT( updateWidget() ) );
  this->qSlicerWidget::setMRMLScene( newScene );
  this->qvtkConnect( this->mrmlScene(), vtkMRMLScene::NodeAddedEvent, this, SLOT( updateWidget() ) );
  this->qvtkConnect( this->mrmlScene(), vtkMRMLScene::NodeRemovedEvent, this, SLOT( updateWidget() ) );
}


void qSlicerPerkEvaluatorRolesWidget
::enter()
{
}


void qSlicerPerkEvaluatorRolesWidget
::setMetricInstanceNode( vtkMRMLNode* node )
{
  vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::SafeDownCast( node );

  // This is ok if the node is null
  this->qvtkDisconnect( this->MetricInstanceNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->MetricInstanceNode = miNode;
  this->qvtkConnect( this->MetricInstanceNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateWidget();
}

std::string qSlicerPerkEvaluatorRolesWidget
::getRolesHeader()
{
  return "";
}


std::string qSlicerPerkEvaluatorRolesWidget
::getCandidateHeader()
{
  return "";
}


void qSlicerPerkEvaluatorRolesWidget
::updateWidget()
{
  Q_D(qSlicerPerkEvaluatorRolesWidget);

  // Check what the current row and column are
  int currentRow = d->RolesTable->currentRow();
  int currentColumn = d->RolesTable->currentColumn();
  int scrollPosition = d->RolesTable->verticalScrollBar()->value();
  
  // The only thing to do is update the table entries. Must ensure they are in sorted order (that's how they are stored in the buffer).
  d->RolesTable->clear();
  d->RolesTable->setRowCount( 0 );
  
  // This is where the roles are grabbed
  std::vector< std::string > roles = this->getAllRoles();

  d->RolesTable->setRowCount( roles.size() );
  d->RolesTable->setColumnCount( 2 );
  QStringList RolesTableHeaders;
  RolesTableHeaders << this->getRolesHeader().c_str() << this->getCandidateHeader().c_str();
  d->RolesTable->setHorizontalHeaderLabels( RolesTableHeaders ); 
#ifdef Slicer_HAVE_QT5
  d->RolesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
#else
  d->RolesTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
#endif

  // Set the roles in the table
  for ( int i = 0; i < roles.size(); i++ )
  {
    // Add the fixed item to the table
    QTableWidgetItem* nameItem = new QTableWidgetItem( QString::fromStdString( roles.at( i ) ) );
    d->RolesTable->setItem( i, 0, nameItem );

    // Determine the current transform's role
    std::string currentCandidateID = this->getNodeIDFromRole( roles.at( i ) );
	
    // Create the combo box
    qMRMLNodeComboBox* candidateComboBox = new qMRMLNodeComboBox();
    candidateComboBox->setNoneEnabled( true );
    candidateComboBox->setAddEnabled( false );
    candidateComboBox->setRemoveEnabled( false );
    candidateComboBox->setShowHidden( false );
    candidateComboBox->setShowChildNodeTypes( true );
    candidateComboBox->setMRMLScene( this->mrmlScene() );

    std::string roleNodeType = this->getNodeTypeForRole( roles.at( i ) );
    candidateComboBox->setNodeTypes( QStringList( QString::fromStdString( roleNodeType ) ) );

    candidateComboBox->setCurrentNodeID( QString::fromStdString( currentCandidateID ) );
	
    connect( candidateComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onRolesChanged() ) );
	
    d->RolesTable->setCellWidget( i, 1, candidateComboBox );

    // Populate the maps
    this->ComboBoxToRolesMap[ candidateComboBox ] = roles.at( i );
    this->RolesToComboBoxMap[ roles.at( i ) ] = candidateComboBox;
  }

  // Reset the current row and column to what they were
  d->RolesTable->setCurrentCell( currentRow, currentColumn );
  d->RolesTable->verticalScrollBar()->setValue( scrollPosition );
  d->RolesTable->resizeRowsToContents();

}
