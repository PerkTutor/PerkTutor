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

#ifndef __qSlicerPerkEvaluatorRolesWidget_h
#define __qSlicerPerkEvaluatorRolesWidget_h

// Qt includes
#include <QtGui>
#include "qSlicerWidget.h"
#include "qMRMLNodeComboBox.h"

// FooBar Widgets includes
#include "qSlicerPerkEvaluatorModuleWidgetsExport.h"
#include "ui_qSlicerPerkEvaluatorRolesWidget.h"

#include "vtkMRMLNode.h"
#include "vtkSlicerPerkEvaluatorLogic.h"
#include "vtkMRMLPerkEvaluatorNode.h"

class qSlicerPerkEvaluatorRolesWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_PERKEVALUATOR_WIDGETS_EXPORT 
qSlicerPerkEvaluatorRolesWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerPerkEvaluatorRolesWidget(QWidget *parent=0);
  virtual ~qSlicerPerkEvaluatorRolesWidget();

public slots:

  void setMRMLScene( vtkMRMLScene* newScene ) override;
  void setMetricInstanceNode( vtkMRMLNode* node );

protected slots:

  virtual void onRolesChanged() = 0;
  void updateWidget();

protected:
  QScopedPointer<qSlicerPerkEvaluatorRolesWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();

  virtual std::string getRolesHeader();
  virtual std::string getCandidateHeader();

  virtual std::vector< std::string > getAllRoles() = 0; // Just a list of all roles
  virtual std::string getNodeTypeForRole( std::string role ) = 0; // A list of the node types for that role
  virtual std::string getNodeIDFromRole( std::string role ) = 0; // Get the ID of the node fulfilling the role
  
  // Have two maps to correspond transforms nodes <-> ComboBox widgets
  std::map< std::string, qMRMLNodeComboBox* > RolesToComboBoxMap;
  std::map< qMRMLNodeComboBox*, std::string > ComboBoxToRolesMap;

  vtkWeakPointer< vtkSlicerPerkEvaluatorLogic > PerkEvaluatorLogic;
  vtkWeakPointer< vtkMRMLMetricInstanceNode > MetricInstanceNode;

private:
  Q_DECLARE_PRIVATE(qSlicerPerkEvaluatorRolesWidget);
  Q_DISABLE_COPY(qSlicerPerkEvaluatorRolesWidget);

};

#endif
