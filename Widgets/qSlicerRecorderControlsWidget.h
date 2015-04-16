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

#include "vtkMRMLTransformBufferNode.h"
#include "vtkSlicerTransformRecorderLogic.h"

// FooBar Widgets includes
#include "qSlicerTransformRecorderModuleWidgetsExport.h"
#include "ui_qSlicerRecorderControlsWidget.h"

class qSlicerRecorderControlsWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_TRANSFORMRECORDER_WIDGETS_EXPORT 
qSlicerRecorderControlsWidget : public qSlicerWidget
{
  Q_OBJECT
public:

  qSlicerRecorderControlsWidget(QWidget *parent=0);
  virtual ~qSlicerRecorderControlsWidget();

protected slots:

  virtual void setTransformBufferNode( vtkMRMLTransformBufferNode* newTransformBufferNode );

  virtual void onTransformBufferActiveTransformsChanged();

  void onStartButtonClicked();
  void onStopButtonClicked();
  void onClearButtonClicked();

  void onCheckedTransformsChanged();

  void updateWidget();

protected:
  QScopedPointer<qSlicerRecorderControlsWidgetPrivate> d_ptr;

  vtkMRMLTransformBufferNode* TransformBufferNode;
  vtkSlicerTransformRecorderLogic* TransformRecorderLogic;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerRecorderControlsWidget);
  Q_DISABLE_COPY(qSlicerRecorderControlsWidget);

};

#endif
