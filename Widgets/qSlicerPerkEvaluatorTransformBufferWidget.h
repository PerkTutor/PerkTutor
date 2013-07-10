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

#ifndef __qSlicerPerkEvaluatorTransformBufferWidget_h
#define __qSlicerPerkEvaluatorTransformBufferWidget_h

// Qt includes
#include "qSlicerWidget.h"

// FooBar Widgets includes
#include "qSlicerPerkEvaluatorModuleWidgetsExport.h"

#include "vtkSlicerPerkEvaluatorLogic.h"
#include "vtkMRMLTransformBufferNode.h"
#include "qSlicerTransformBufferWidget.h"

class qSlicerPerkEvaluatorTransformBufferWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_PERKEVALUATOR_WIDGETS_EXPORT 
qSlicerPerkEvaluatorTransformBufferWidget : public qSlicerTransformBufferWidget
{
  Q_OBJECT
public:
  typedef qSlicerTransformBufferWidget Superclass;
  qSlicerPerkEvaluatorTransformBufferWidget(QWidget *parent=0);
  virtual ~qSlicerPerkEvaluatorTransformBufferWidget();

  
  static qSlicerPerkEvaluatorTransformBufferWidget* New( vtkSlicerPerkEvaluatorLogic* newPerkEvaluatorLogic );
  
  vtkSlicerPerkEvaluatorLogic* PerkEvaluatorLogic;

protected slots:

  void onCurrentBufferNodeChanged();

protected:
  QScopedPointer<qSlicerPerkEvaluatorTransformBufferWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPerkEvaluatorTransformBufferWidget);
  Q_DISABLE_COPY(qSlicerPerkEvaluatorTransformBufferWidget);

};

#endif
