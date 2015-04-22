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

#include "qSlicerTransformBufferWidget.h" // TODO: Remove when GetSlicerModuleLogic made into helper function

#include <QtGui>


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
  this->PerkEvaluatorLogic = vtkSlicerPerkEvaluatorLogic::SafeDownCast( PerkTutorCommon::GetSlicerModuleLogic( "PerkEvaluator" ) );
  this->PerkEvaluatorNode = NULL;
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

  this->setMRMLScene( this->PerkEvaluatorLogic->GetMRMLScene() );

  // No connections to set...
}


void qSlicerPerkEvaluatorRolesWidget
::setMRMLScene( vtkMRMLScene* newScene )
{
  this->qvtkDisconnect( this->mrmlScene(), vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->qSlicerWidget::setMRMLScene( newScene );
  this->qvtkConnect( this->mrmlScene(), vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
}


void qSlicerPerkEvaluatorRolesWidget
::enter()
{
}


void qSlicerPerkEvaluatorRolesWidget
::setPerkEvaluatorNode( vtkMRMLNode* node )
{
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( node );

  // This is ok if the node is null
  this->qvtkDisconnect( this->PerkEvaluatorNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );
  this->PerkEvaluatorNode = peNode;
  this->qvtkConnect( this->PerkEvaluatorNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidget() ) );

  this->updateWidget();
}

std::string qSlicerPerkEvaluatorRolesWidget
::getFixedHeader()
{
  return "";
}


std::string qSlicerPerkEvaluatorRolesWidget
::getMovingHeader()
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
  d->RolesTable->setColumnCount( 0 );
  
  // This is where the moving and fixed labels are grabbed
  std::vector< std::string > fixed = this->getAllFixed();
  std::vector< std::string > moving = this->getAllMoving();
  moving.insert( moving.begin(), "" ); // Put it at the beginning so we have a "blank" option that is default

  d->RolesTable->setRowCount( fixed.size() );
  d->RolesTable->setColumnCount( 2 );
  QStringList RolesTableHeaders;
  RolesTableHeaders << this->getFixedHeader().c_str() << this->getMovingHeader().c_str();
  d->RolesTable->setHorizontalHeaderLabels( RolesTableHeaders ); 
  d->RolesTable->horizontalHeader()->setResizeMode( QHeaderView::Stretch );

  // Set the roles in the table
  for ( int i = 0; i < fixed.size(); i++ )
  {
    // Add the fixed item to the table
    QTableWidgetItem* nameItem = new QTableWidgetItem( QString::fromStdString( fixed.at( i ) ) );
    d->RolesTable->setItem( i, 0, nameItem );

    // Determine the current transform's role
    std::string currentMoving = this->getMovingFromFixed( fixed.at( i ) );
	
    // Create the combo box
    QComboBox* roleComboBox = new QComboBox();
    roleComboBox->setCurrentIndex( 0 );
    for ( int j = 0; j < moving.size(); j++ )
    {
      roleComboBox->addItem( QString::fromStdString( moving.at( j ) ) );
      if ( currentMoving.compare( moving.at( j ) ) == 0 )
      {
        roleComboBox->setCurrentIndex( j );
      }
    }
	
    connect( roleComboBox, SIGNAL( currentIndexChanged( int ) ), this, SLOT( onRolesChanged() ) );
	
    d->RolesTable->setCellWidget( i, 1, roleComboBox );

    // Populate the maps
    this->ComboBoxToFixedMap[ roleComboBox ] = fixed.at( i );
    this->FixedToComboBoxMap[ fixed.at( i ) ] = roleComboBox;
  }

  // Reset the current row and column to what they were
  d->RolesTable->setCurrentCell( currentRow, currentColumn );
  d->RolesTable->verticalScrollBar()->setValue( scrollPosition );
  d->RolesTable->resizeRowsToContents();

}
