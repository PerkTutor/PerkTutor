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
#include "qSlicerPerkEvaluatorTransformRolesWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerPerkEvaluatorTransformRolesWidgetPrivate
  : public Ui_qSlicerPerkEvaluatorRolesWidget
{
  Q_DECLARE_PUBLIC(qSlicerPerkEvaluatorTransformRolesWidget);
protected:
  qSlicerPerkEvaluatorTransformRolesWidget* const q_ptr;

public:
  qSlicerPerkEvaluatorTransformRolesWidgetPrivate( qSlicerPerkEvaluatorTransformRolesWidget& object);
  ~qSlicerPerkEvaluatorTransformRolesWidgetPrivate();
  virtual void setupUi(qSlicerPerkEvaluatorTransformRolesWidget*);
};

// --------------------------------------------------------------------------
qSlicerPerkEvaluatorTransformRolesWidgetPrivate
::qSlicerPerkEvaluatorTransformRolesWidgetPrivate( qSlicerPerkEvaluatorTransformRolesWidget& object) : q_ptr(&object)
{
}

qSlicerPerkEvaluatorTransformRolesWidgetPrivate
::~qSlicerPerkEvaluatorTransformRolesWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerPerkEvaluatorTransformRolesWidgetPrivate
::setupUi(qSlicerPerkEvaluatorTransformRolesWidget* widget)
{
  this->Ui_qSlicerPerkEvaluatorRolesWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorTransformRolesWidget methods

//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorTransformRolesWidget
::qSlicerPerkEvaluatorTransformRolesWidget(QWidget* parentWidget) : qSlicerPerkEvaluatorRolesWidget( parentWidget ) , d_ptr( new qSlicerPerkEvaluatorTransformRolesWidgetPrivate(*this) )
{
}


qSlicerPerkEvaluatorTransformRolesWidget
::~qSlicerPerkEvaluatorTransformRolesWidget()
{
}


std::string qSlicerPerkEvaluatorTransformRolesWidget
::getRolesHeader()
{
  return "Role";
}


std::string qSlicerPerkEvaluatorTransformRolesWidget
::getCandidateHeader()
{
  return "Transform Node";
}



void qSlicerPerkEvaluatorTransformRolesWidget
::onRolesChanged()
{
  Q_D(qSlicerPerkEvaluatorTransformRolesWidget);

  if ( this->MetricInstanceNode == NULL )
  {
    return;
  }

  // Find who the sender is and the corresponding node
  qMRMLNodeComboBox* sender = ( qMRMLNodeComboBox* ) this->sender();
  std::string transformRole = this->ComboBoxToRolesMap[ sender ];
  
  this->MetricInstanceNode->SetRoleID( sender->currentNodeID().toStdString(), transformRole, vtkMRMLMetricInstanceNode::TransformRole );

  this->updateWidget();
}


std::string qSlicerPerkEvaluatorTransformRolesWidget
::getNodeTypeForRole( std::string role )
{
  Q_D(qSlicerPerkEvaluatorTransformRolesWidget);

  return "vtkMRMLLinearTransformNode";
}


std::string qSlicerPerkEvaluatorTransformRolesWidget
::getNodeIDFromRole( std::string role )
{
  Q_D(qSlicerPerkEvaluatorTransformRolesWidget);

  return this->MetricInstanceNode->GetRoleID( role, vtkMRMLMetricInstanceNode::TransformRole );
}


std::vector< std::string > qSlicerPerkEvaluatorTransformRolesWidget
::getAllRoles()
{
  Q_D(qSlicerPerkEvaluatorTransformRolesWidget);

  if ( this->MetricInstanceNode == NULL )
  {
    return std::vector< std::string >();
  }
  
  return this->PerkEvaluatorLogic->GetAllTransformRoles( this->MetricInstanceNode->GetAssociatedMetricScriptID() );
}