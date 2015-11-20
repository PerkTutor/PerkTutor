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

  virtual void setMRMLScene( vtkMRMLScene* newScene );
  void setPerkEvaluatorNode( vtkMRMLNode* node );

protected slots:

  virtual void onRolesChanged() = 0;
  void updateWidget();

protected:
  QScopedPointer<qSlicerPerkEvaluatorRolesWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();

  virtual std::string getFixedHeader();
  virtual std::string getMovingHeader();

  virtual std::vector< std::string > getAllFixed() = 0; // Just a list of all fixed options
  virtual std::vector< std::string > getAllMoving() = 0; // Just a list of all moving option
  virtual std::string getMovingFromFixed( std::string fixed ) = 0;
  
  // Have two maps to correspond transforms nodes <-> ComboBox widgets
  std::map< std::string, QComboBox* > FixedToComboBoxMap;
  std::map< QComboBox*, std::string > ComboBoxToFixedMap;

  vtkWeakPointer< vtkSlicerPerkEvaluatorLogic > PerkEvaluatorLogic;
  vtkWeakPointer< vtkMRMLPerkEvaluatorNode > PerkEvaluatorNode;

private:
  Q_DECLARE_PRIVATE(qSlicerPerkEvaluatorRolesWidget);
  Q_DISABLE_COPY(qSlicerPerkEvaluatorRolesWidget);

};

#endif
