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

#ifndef __qSlicerTransformBufferWidget_h
#define __qSlicerTransformBufferWidget_h

// Qt includes
#include "qSlicerWidget.h"

// FooBar Widgets includes
#include "qSlicerTransformRecorderModuleWidgetsExport.h"
#include "ui_qSlicerTransformBufferWidget.h"

#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkMRMLTransformBufferNode.h"

class qSlicerTransformBufferWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_TRANSFORMRECORDER_WIDGETS_EXPORT 
qSlicerTransformBufferWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerTransformBufferWidget(QWidget *parent=0);
  virtual ~qSlicerTransformBufferWidget();

  
  static qSlicerTransformBufferWidget* New( vtkSlicerTransformRecorderLogic* newTransformRecorderLogic );
  
  vtkMRMLTransformBufferNode* GetBufferNode();

  vtkSlicerTransformRecorderLogic* TransformRecorderLogic;
  unsigned long int BufferModifiedTime;

protected slots:

  void onImportButtonClicked();
  void onExportButtonClicked();
  void onCurrentBufferNodeChanged();

  void updateWidget();

protected:
  QScopedPointer<qSlicerTransformBufferWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();

private:
  Q_DECLARE_PRIVATE(qSlicerTransformBufferWidget);
  Q_DISABLE_COPY(qSlicerTransformBufferWidget);

};

#endif
