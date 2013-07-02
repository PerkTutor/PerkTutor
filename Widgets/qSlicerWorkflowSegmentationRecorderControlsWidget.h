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

#ifndef __qSlicerWorkflowSegmentationRecorderControlsWidget_h
#define __qSlicerWorkflowSegmentationRecorderControlsWidget_h

// Qt includes
#include "qSlicerWidget.h"
#include "qSlicerWorkflowSegmentationModuleWidgetsExport.h"

#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkSlicerWorkflowSegmentationLogic.h"
#include "vtkMRMLTransformRecorderNode.h"
#include "qSlicerRecorderControlsWidget.h"

class qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_WORKFLOWSEGMENTATION_WIDGETS_EXPORT 
qSlicerWorkflowSegmentationRecorderControlsWidget : public qSlicerRecorderControlsWidget
{
  Q_OBJECT
public:
  typedef qSlicerRecorderControlsWidget Superclass;
  qSlicerWorkflowSegmentationRecorderControlsWidget(QWidget *parent=0);
  virtual ~qSlicerWorkflowSegmentationRecorderControlsWidget();

  static qSlicerWorkflowSegmentationRecorderControlsWidget* New( vtkSlicerWorkflowSegmentationLogic* newWSLogic );

  void SetLogic( vtkSlicerWorkflowSegmentationLogic* newWSLogic );

protected slots:

  void onClearButtonClicked();

protected:
  QScopedPointer<qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerWorkflowSegmentationRecorderControlsWidget);
  Q_DISABLE_COPY(qSlicerWorkflowSegmentationRecorderControlsWidget);

  vtkSlicerWorkflowSegmentationLogic* wsLogic;

};

#endif
