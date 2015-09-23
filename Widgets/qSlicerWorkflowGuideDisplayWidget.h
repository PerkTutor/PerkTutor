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

#ifndef __qSlicerWorkflowGuideDisplayWidget_h
#define __qSlicerWorkflowGuideDisplayWidget_h

// Qt includes
#include "qSlicerWidget.h"

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLWorkflowToolNode.h"

// FooBar Widgets includes
#include "qSlicerWorkflowSegmentationModuleWidgetsExport.h"
#include "ui_qSlicerWorkflowGuideDisplayWidget.h"

class qSlicerWorkflowGuideDisplayWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_WORKFLOWSEGMENTATION_WIDGETS_EXPORT 
qSlicerWorkflowGuideDisplayWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerWorkflowGuideDisplayWidget(QWidget *parent=0);
  virtual ~qSlicerWorkflowGuideDisplayWidget();

  virtual void setWorkflowToolNode( vtkMRMLNode* newWorkflowToolNode );
  virtual vtkMRMLWorkflowToolNode* getWorkflowToolNode();

protected slots:

  void onPopButtonClicked();

  void updateWidgetFromMRML();

protected:

  QScopedPointer<qSlicerWorkflowGuideDisplayWidgetPrivate> d_ptr;

  vtkMRMLWorkflowToolNode* WorkflowToolNode;
  QWidget* DefaultParentWidget;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerWorkflowGuideDisplayWidget);
  Q_DISABLE_COPY(qSlicerWorkflowGuideDisplayWidget);

};

#endif
