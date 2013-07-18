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

#ifndef __qSlicerPerkEvaluatorMessagesWidget_h
#define __qSlicerPerkEvaluatorMessagesWidget_h

// Qt includes
#include "qSlicerWidget.h"
#include "qSlicerPerkEvaluatorModuleWidgetsExport.h"

#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkSlicerPerkEvaluatorLogic.h"
#include "qSlicerMessagesWidget.h"

class qSlicerPerkEvaluatorMessagesWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_PERKEVALUATOR_WIDGETS_EXPORT 
qSlicerPerkEvaluatorMessagesWidget : public qSlicerMessagesWidget
{
  Q_OBJECT
public:
  typedef qSlicerMessagesWidget Superclass;
  qSlicerPerkEvaluatorMessagesWidget(QWidget *parent=0);
  virtual ~qSlicerPerkEvaluatorMessagesWidget();

  static qSlicerPerkEvaluatorMessagesWidget* New( qSlicerTransformBufferWidget* newBufferWidget, vtkSlicerPerkEvaluatorLogic* newPerkEvaluatorLogic );

  vtkSlicerPerkEvaluatorLogic* PerkEvaluatorLogic;

protected slots:

  void onAddMessageButtonClicked();

protected:
  QScopedPointer<qSlicerPerkEvaluatorMessagesWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerPerkEvaluatorMessagesWidget);
  Q_DISABLE_COPY(qSlicerPerkEvaluatorMessagesWidget);

  vtkSlicerPerkEvaluatorLogic* peLogic;

};

#endif
