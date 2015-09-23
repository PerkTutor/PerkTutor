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

#ifndef __qSlicerWorkflowToolWidget_h
#define __qSlicerWorkflowToolWidget_h

// Qt includes
#include "qSlicerWidget.h"

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLWorkflowToolNode.h"

// FooBar Widgets includes
#include "qSlicerWorkflowSegmentationModuleWidgetsExport.h"
#include "ui_qSlicerWorkflowToolWidget.h"


class qSlicerWorkflowToolWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_WORKFLOWSEGMENTATION_WIDGETS_EXPORT 
qSlicerWorkflowToolWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerWorkflowToolWidget(QWidget *parent=0);
  virtual ~qSlicerWorkflowToolWidget();

  virtual void setWorkflowToolNode( vtkMRMLNode* newWorkflowToolNode );
  virtual vtkMRMLWorkflowToolNode* getWorkflowToolNode();

protected slots:

  virtual void onWorkflowToolNodeChanged( vtkMRMLNode* newWorkflowToolNode );
  void onWorkflowToolNodeModified();
  
  void onWorkflowProcedureChanged( vtkMRMLNode* newWorkflowProcedureNode );
  void onWorkflowInputChanged( vtkMRMLNode* newWorkflowInputNode );
  void onWorkflowTrainingChanged( vtkMRMLNode* newWorkflowTrainingNode );
  // This widget doesn't care if the procedure/input/training are modified, because it doesn't chow that information

  void updateWidgetFromMRML();

signals:

  void WorkflowToolNodeChanged( vtkMRMLNode* newWorkflowToolNode );
  void WorkflowToolNodeModified();

protected:

  QScopedPointer<qSlicerWorkflowToolWidgetPrivate> d_ptr;

  vtkMRMLWorkflowToolNode* WorkflowToolNode;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerWorkflowToolWidget);
  Q_DISABLE_COPY(qSlicerWorkflowToolWidget);

};

#endif
