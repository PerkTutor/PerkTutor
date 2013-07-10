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

#ifndef __qSlicerRecorderControlsWidget_h
#define __qSlicerRecorderControlsWidget_h

// Qt includes
#include "qSlicerWidget.h"

// FooBar Widgets includes
#include "qSlicerTransformRecorderModuleWidgetsExport.h"
#include "ui_qSlicerRecorderControlsWidget.h"

#include "qSlicerTransformBufferWidget.h"
#include "vtkMRMLTransformBufferNode.h"

class qSlicerRecorderControlsWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_TRANSFORMRECORDER_WIDGETS_EXPORT 
qSlicerRecorderControlsWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerRecorderControlsWidget(QWidget *parent=0);
  virtual ~qSlicerRecorderControlsWidget();

  static qSlicerRecorderControlsWidget* New( qSlicerTransformBufferWidget* newBufferWidget );

protected slots:

  void onStartButtonClicked();
  void onStopButtonClicked();
  void onClearButtonClicked();

  void onCheckedTransformsChanged();
  void onActiveTransformsUpdated();

  void updateWidget();

protected:
  QScopedPointer<qSlicerRecorderControlsWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();

  qSlicerTransformBufferWidget* BufferWidget;
  unsigned int UpdateStatus;
  bool updatingCheckedTransforms;

private:
  Q_DECLARE_PRIVATE(qSlicerRecorderControlsWidget);
  Q_DISABLE_COPY(qSlicerRecorderControlsWidget);

};

#endif
