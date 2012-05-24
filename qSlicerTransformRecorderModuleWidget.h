/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerTransformRecorderModuleWidget_h
#define __qSlicerTransformRecorderModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include <QtGui>
#include "qSlicerTransformRecorderModuleExport.h"

class qSlicerTransformRecorderModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLTransformRecorderNode;


/// \ingroup Slicer_QtModules_TransformRecorder
class Q_SLICER_QTMODULES_TRANSFORMRECORDER_EXPORT qSlicerTransformRecorderModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTransformRecorderModuleWidget(QWidget *parent=0);
  virtual ~qSlicerTransformRecorderModuleWidget();
  


public slots:
  void loadLogFile();

protected:
  QScopedPointer<qSlicerTransformRecorderModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  virtual void enter();

protected slots:
  void onTransformsNodeSelected(vtkMRMLNode* node);
  void onMRMLTransformNodeModified(vtkObject* caller);
  void onStopButtonPressed();
  void onStartButtonPressed();
  void onClearBufferButtonPressed();
  void insertItem();
  void clearItems();
  void onConnectorSelected();
  void onModuleNodeSelected();
  void updateWidget();

private:
  Q_DECLARE_PRIVATE(qSlicerTransformRecorderModuleWidget);
  Q_DISABLE_COPY(qSlicerTransformRecorderModuleWidget);
};

#endif
