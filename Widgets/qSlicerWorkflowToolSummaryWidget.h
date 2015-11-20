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

#ifndef __qSlicerWorkflowToolSummaryWidget_h
#define __qSlicerWorkflowToolSummaryWidget_h

// Qt includes
#include "qSlicerWidget.h"

#include "vtkSlicerWorkflowSegmentationLogic.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLWorkflowToolNode.h"

// FooBar Widgets includes
#include "qSlicerWorkflowSegmentationModuleWidgetsExport.h"
#include "ui_qSlicerWorkflowToolSummaryWidget.h"

class qSlicerWorkflowToolSummaryWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_WORKFLOWSEGMENTATION_WIDGETS_EXPORT 
qSlicerWorkflowToolSummaryWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerWorkflowToolSummaryWidget(QWidget *parent=0);
  virtual ~qSlicerWorkflowToolSummaryWidget();
  
public slots:

  virtual void setWorkflowSegmentationNode( vtkMRMLNode* newWorkflowSegmentationNode );

protected slots:

  virtual void onToolSelectionsChanged();
  
  void onTrainButtonClicked();

  void updateWidgetFromMRML();


protected:

  QScopedPointer<qSlicerWorkflowToolSummaryWidgetPrivate> d_ptr;

  vtkWeakPointer< vtkSlicerWorkflowSegmentationLogic > WorkflowSegmentationLogic;
  vtkWeakPointer< vtkMRMLWorkflowSegmentationNode > WorkflowSegmentationNode;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerWorkflowToolSummaryWidget);
  Q_DISABLE_COPY(qSlicerWorkflowToolSummaryWidget);

};

#endif
