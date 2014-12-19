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
#include "qSlicerPerkEvaluatorAnatomyRolesWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerPerkEvaluatorAnatomyRolesWidgetPrivate
  : public Ui_qSlicerPerkEvaluatorRolesWidget
{
  Q_DECLARE_PUBLIC(qSlicerPerkEvaluatorAnatomyRolesWidget);
protected:
  qSlicerPerkEvaluatorAnatomyRolesWidget* const q_ptr;

public:
  qSlicerPerkEvaluatorAnatomyRolesWidgetPrivate( qSlicerPerkEvaluatorAnatomyRolesWidget& object);
  ~qSlicerPerkEvaluatorAnatomyRolesWidgetPrivate();
  virtual void setupUi(qSlicerPerkEvaluatorAnatomyRolesWidget*);
};

// --------------------------------------------------------------------------
qSlicerPerkEvaluatorAnatomyRolesWidgetPrivate
::qSlicerPerkEvaluatorAnatomyRolesWidgetPrivate( qSlicerPerkEvaluatorAnatomyRolesWidget& object) : q_ptr(&object)
{
}

qSlicerPerkEvaluatorAnatomyRolesWidgetPrivate
::~qSlicerPerkEvaluatorAnatomyRolesWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerPerkEvaluatorAnatomyRolesWidgetPrivate
::setupUi(qSlicerPerkEvaluatorAnatomyRolesWidget* widget)
{
  this->Ui_qSlicerPerkEvaluatorRolesWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorAnatomyRolesWidget methods

//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorAnatomyRolesWidget
::qSlicerPerkEvaluatorAnatomyRolesWidget(QWidget* parentWidget) : qSlicerPerkEvaluatorRolesWidget( parentWidget ) , d_ptr( new qSlicerPerkEvaluatorAnatomyRolesWidgetPrivate(*this) )
{
}


qSlicerPerkEvaluatorAnatomyRolesWidget
::~qSlicerPerkEvaluatorAnatomyRolesWidget()
{
}


std::string qSlicerPerkEvaluatorAnatomyRolesWidget
::getFixedHeader()
{
  return "Role";
}


std::string qSlicerPerkEvaluatorAnatomyRolesWidget
::getMovingHeader()
{
  return "Anatomy Node";
}



void qSlicerPerkEvaluatorAnatomyRolesWidget
::onRolesChanged()
{
  Q_D(qSlicerPerkEvaluatorAnatomyRolesWidget);

  if ( this->PerkEvaluatorNode == NULL )
  {
    return;
  }

  // Find who the sender is and the corresponding node
  QComboBox* sender = (QComboBox*) this->sender();
  std::string anatomyRole = this->ComboBoxToFixedMap[ sender ];
  
  this->PerkEvaluatorNode->SetAnatomyNodeName( anatomyRole, sender->currentText().toStdString() );

  this->updateWidget();
}


std::vector< std::string > qSlicerPerkEvaluatorAnatomyRolesWidget
::getAllMoving()
{
  Q_D(qSlicerPerkEvaluatorAnatomyRolesWidget);
  
  vtkSmartPointer< vtkCollection > anatomyNodes = vtkSmartPointer< vtkCollection >::New();
  this->PerkEvaluatorLogic->GetSceneVisibleAnatomyNodes( anatomyNodes );
  
  std::vector< std::string > anatomyNodeNames( anatomyNodes->GetNumberOfItems(), "" );
  
  for ( int i = 0; i < anatomyNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLNode* currentAnatomyNode = vtkMRMLNode::SafeDownCast( anatomyNodes->GetItemAsObject( i ) );
    anatomyNodeNames.at( i ) = currentAnatomyNode->GetName();
  }

  return anatomyNodeNames;
}


std::vector< std::string > qSlicerPerkEvaluatorAnatomyRolesWidget
::getAllFixed()
{
  Q_D(qSlicerPerkEvaluatorAnatomyRolesWidget);
  
  return this->PerkEvaluatorLogic->GetAllAnatomyRoles();
}


std::string qSlicerPerkEvaluatorAnatomyRolesWidget
::getMovingFromFixed( std::string fixed )
{
  Q_D(qSlicerPerkEvaluatorAnatomyRolesWidget);

  if ( this->PerkEvaluatorNode == NULL )
  {
    return "";
  }
  
  return this->PerkEvaluatorNode->GetAnatomyNodeName( fixed );
}