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

#ifndef __qSlicerTrackedSequenceRecorderControlsWidget_h
#define __qSlicerTrackedSequenceRecorderControlsWidget_h

// Qt includes
#include "qSlicerWidget.h"

#include "vtkMRMLTransformBufferNode.h"
#include "vtkSlicerTransformRecorderLogic.h"

// FooBar Widgets includes
#include "qSlicerTransformRecorderModuleWidgetsExport.h"
#include "ui_qSlicerTrackedSequenceRecorderControlsWidget.h"

class qSlicerTrackedSequenceRecorderControlsWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_TRANSFORMRECORDER_WIDGETS_EXPORT 
qSlicerTrackedSequenceRecorderControlsWidget : public qSlicerWidget
{
  Q_OBJECT
public:

  qSlicerTrackedSequenceRecorderControlsWidget(QWidget *parent=0);
  virtual ~qSlicerTrackedSequenceRecorderControlsWidget();

public slots:

  virtual void setTransformBufferNode( vtkMRMLNode* newTransformBufferNode );

protected slots:

  virtual void onTransformBufferActiveTransformsChanged();

  virtual void onStartStopButtonClicked( bool );
  virtual void onClearButtonClicked();

  void onCheckedTransformsChanged();

  void updateWidget();

protected:
  QScopedPointer<qSlicerTrackedSequenceRecorderControlsWidgetPrivate> d_ptr;

  vtkWeakPointer< vtkMRMLTransformBufferNode > TransformBufferNode;
  vtkWeakPointer< vtkSlicerTransformRecorderLogic > TransformRecorderLogic;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerTrackedSequenceRecorderControlsWidget);
  Q_DISABLE_COPY(qSlicerTrackedSequenceRecorderControlsWidget);

};

#endif
