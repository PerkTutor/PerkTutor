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

#ifndef __qSlicerTransformSelectionWidget_h
#define __qSlicerTransformSelectionWidget_h

// Qt includes
#include "qSlicerWidget.h"
#include <QtGui>

#include "qSlicerTransformBufferWidgetHelper.h"

// FooBar Widgets includes
#include "qSlicerPerkEvaluatorModuleWidgetsExport.h"
#include "ui_qSlicerTransformSelectionWidget.h"

#include "vtkSlicerPerkEvaluatorLogic.h"
#include "vtkMRMLLinearTransformNode.h"

class qSlicerTransformSelectionWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_PERKEVALUATOR_WIDGETS_EXPORT 
qSlicerTransformSelectionWidget : public virtual qSlicerWidget
{
  Q_OBJECT
public:
  qSlicerTransformSelectionWidget(QWidget *parent=0);
  virtual ~qSlicerTransformSelectionWidget();

  vtkSlicerPerkEvaluatorLogic* PerkEvaluatorLogic;
  qSlicerTransformBufferWidgetHelper* BufferHelper;

public slots:

  virtual void setMRMLScene( vtkMRMLScene* newScene );

protected slots:

  void onTransformSelectionChanged();
  void onSelectAllClicked();
  void onUnselectAllClicked();
  void updateWidget();

protected:
  QScopedPointer<qSlicerTransformSelectionWidgetPrivate> d_ptr;

  virtual void setup();
  
  // Have two maps to correspond transforms nodes <-> ComboBox widgets
  std::map< vtkMRMLLinearTransformNode*, QComboBox* > NodeToComboBoxMap;
  std::map< QComboBox*, vtkMRMLLinearTransformNode* > ComboBoxToNodeMap;

private:
  Q_DECLARE_PRIVATE(qSlicerTransformSelectionWidget);
  Q_DISABLE_COPY(qSlicerTransformSelectionWidget);

};

#endif
