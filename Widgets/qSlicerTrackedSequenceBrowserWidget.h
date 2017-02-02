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

#ifndef __qSlicerTrackedSequenceBrowserWidget_h
#define __qSlicerTrackedSequenceBrowserWidget_h

// Qt includes
#include "qSlicerWidget.h"

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLTransformBufferNode.h"
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkSlicerTransformRecorderLogic.h"

// FooBar Widgets includes
#include "qSlicerTransformRecorderModuleWidgetsExport.h"
#include "ui_qSlicerTrackedSequenceBrowserWidget.h"

class qSlicerTrackedSequenceBrowserWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_TRANSFORMRECORDER_WIDGETS_EXPORT 
qSlicerTrackedSequenceBrowserWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerTrackedSequenceBrowserWidget(QWidget *parent=0);
  virtual ~qSlicerTrackedSequenceBrowserWidget();

  virtual void setTrackedSequenceBrowserNode( vtkMRMLNode* newTrackedSequenceBrowserNode );
  virtual vtkMRMLSequenceBrowserNode* getTrackedSequenceBrowserNode();

protected slots:

  void onOpenButtonClicked();
  void onSaveButtonClicked();

  virtual void onTrackedSequenceBrowserNodeChanged( vtkMRMLNode* newTrackedSequenceBrowserNode );
  void onTrackedSequenceBrowserNodeModified();

  void updateWidget();

signals:

  void trackedSequenceBrowserNodeChanged( vtkMRMLNode* newTransformBufferNode );
  void trackedSequenceBrowserNodeModified();

protected:

  QScopedPointer<qSlicerTrackedSequenceBrowserWidgetPrivate> d_ptr;

  vtkWeakPointer< vtkMRMLSequenceBrowserNode > TrackedSequenceBrowserNode;
  vtkWeakPointer< vtkSlicerTransformRecorderLogic > TransformRecorderLogic;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerTrackedSequenceBrowserWidget);
  Q_DISABLE_COPY(qSlicerTrackedSequenceBrowserWidget);

};

#endif
