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

#include "qSlicerTransformBufferWidgetHelper.h"
#include "vtkSlicerTransformRecorderLogic.h"

// FooBar Widgets includes
#include "qSlicerTransformRecorderModuleWidgetsExport.h"
#include "ui_qSlicerTransformBufferWidget.h"

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

  qSlicerTransformBufferWidgetHelper* BufferHelper;
  vtkSlicerTransformRecorderLogic* TransformRecorderLogic;

protected slots:

  void onImportButtonClicked();
  void onExportButtonClicked();

  virtual void onTransformBufferNodeChanged( vtkMRMLNode* );
  void onTransformBufferNodeModified();

  void updateWidget();

signals:

  void transformBufferNodeChanged( vtkMRMLTransformBufferNode* );
  void transformBufferNodeModified();

protected:

  QScopedPointer<qSlicerTransformBufferWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerTransformBufferWidget);
  Q_DISABLE_COPY(qSlicerTransformBufferWidget);

};

#endif
