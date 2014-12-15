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
::getFixedHeader()
{
  return "Transform Node";
}


std::string qSlicerPerkEvaluatorTransformRolesWidget
::getMovingHeader()
{
  return "Role";
}



void qSlicerPerkEvaluatorTransformRolesWidget
::onRolesChanged()
{
  Q_D(qSlicerPerkEvaluatorTransformRolesWidget);

  // Find who the sender is and the corresponding node
  QComboBox* sender = (QComboBox*) this->sender();
  std::string transformName = this->ComboBoxToFixedMap[ sender ];
  
  this->PerkEvaluatorLogic->SetTransformRole( transformName, sender->currentText().toStdString() );

  this->updateWidget();
}


std::vector< std::string > qSlicerPerkEvaluatorTransformRolesWidget
::getAllMoving()
{
  Q_D(qSlicerPerkEvaluatorTransformRolesWidget);
  
  return this->PerkEvaluatorLogic->GetAllTransformRoles();
}


std::vector< std::string > qSlicerPerkEvaluatorTransformRolesWidget
::getAllFixed()
{
  Q_D(qSlicerPerkEvaluatorTransformRolesWidget);
  
  vtkSmartPointer< vtkCollection > transformNodes = vtkSmartPointer< vtkCollection >::New();
  this->PerkEvaluatorLogic->GetSceneVisibleTransformNodes( transformNodes );
  
  std::vector< std::string > transformNodeNames( transformNodes->GetNumberOfItems(), "" );
  
  for ( int i = 0; i < transformNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLNode* currentTransformNode = vtkMRMLNode::SafeDownCast( transformNodes->GetItemAsObject( i ) );
    transformNodeNames.at( i ) = currentTransformNode->GetName();
  }

  return transformNodeNames;
}


std::string qSlicerPerkEvaluatorTransformRolesWidget
::getMovingFromFixed( std::string fixed )
{
  Q_D(qSlicerPerkEvaluatorTransformRolesWidget);
  
  return this->PerkEvaluatorLogic->GetTransformRole( fixed );
}