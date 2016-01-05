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
::getRolesHeader()
{
  return "Role";
}


std::string qSlicerPerkEvaluatorAnatomyRolesWidget
::getCandidateHeader()
{
  return "Anatomy Node";
}



void qSlicerPerkEvaluatorAnatomyRolesWidget
::onRolesChanged()
{
  Q_D(qSlicerPerkEvaluatorAnatomyRolesWidget);

  if ( this->MetricInstanceNode == NULL )
  {
    return;
  }

  // Find who the sender is and the corresponding node
  qMRMLNodeComboBox* sender = ( qMRMLNodeComboBox* ) this->sender();
  std::string anatomyRole = this->ComboBoxToRolesMap[ sender ];
  
  this->MetricInstanceNode->SetRoleID( sender->currentNodeID().toStdString(), anatomyRole, vtkMRMLMetricInstanceNode::AnatomyRole );

  this->updateWidget();
}


std::string qSlicerPerkEvaluatorAnatomyRolesWidget
::getNodeTypeForRole( std::string role )
{
  Q_D(qSlicerPerkEvaluatorAnatomyRolesWidget);

  if ( this->MetricInstanceNode == NULL )
  {
    return "";
  }
  
  return this->PerkEvaluatorLogic->GetAnatomyRoleClassName( this->MetricInstanceNode, role );
}


std::string qSlicerPerkEvaluatorAnatomyRolesWidget
::getNodeIDFromRole( std::string role )
{
  Q_D(qSlicerPerkEvaluatorAnatomyRolesWidget);

  return this->MetricInstanceNode->GetRoleID( role, vtkMRMLMetricInstanceNode::AnatomyRole );
}


std::vector< std::string > qSlicerPerkEvaluatorAnatomyRolesWidget
::getAllRoles()
{
  Q_D(qSlicerPerkEvaluatorAnatomyRolesWidget);

  if ( this->MetricInstanceNode == NULL )
  {
    return std::vector< std::string >();
  }
  
  return this->PerkEvaluatorLogic->GetAllAnatomyRoles( this->MetricInstanceNode );
}