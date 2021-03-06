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

#include "vtkSlicerWorkflowSegmentationLogic.h"
#include "qSlicerTrackedSequenceRecorderControlsWidget.h"

class qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_WORKFLOWSEGMENTATION_WIDGETS_EXPORT 
qSlicerWorkflowSegmentationRecorderControlsWidget : public qSlicerTrackedSequenceRecorderControlsWidget
{
  Q_OBJECT
public:
  qSlicerWorkflowSegmentationRecorderControlsWidget(QWidget *parent=0);
  virtual ~qSlicerWorkflowSegmentationRecorderControlsWidget();

public slots:

  virtual void setWorkflowSegmentationNode( vtkMRMLNode* wsNode );

protected slots:

  void onStartStopButtonClicked( bool state );

protected:
  QScopedPointer<qSlicerWorkflowSegmentationRecorderControlsWidgetPrivate> d_ptr;
  
  vtkWeakPointer< vtkMRMLWorkflowSegmentationNode > WorkflowSegmentationNode;

private:
  Q_DECLARE_PRIVATE(qSlicerWorkflowSegmentationRecorderControlsWidget);
  Q_DISABLE_COPY(qSlicerWorkflowSegmentationRecorderControlsWidget);

};

#endif
