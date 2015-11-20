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

#ifndef __qSlicerPerkEvaluatorRecorderControlsWidget_h
#define __qSlicerPerkEvaluatorRecorderControlsWidget_h

// Qt includes
#include "qSlicerWidget.h"
#include "qSlicerPerkEvaluatorModuleWidgetsExport.h"

#include "vtkSlicerPerkEvaluatorLogic.h"
#include "qSlicerRecorderControlsWidget.h"

class qSlicerPerkEvaluatorRecorderControlsWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_PERKEVALUATOR_WIDGETS_EXPORT 
qSlicerPerkEvaluatorRecorderControlsWidget : public qSlicerRecorderControlsWidget
{
  Q_OBJECT
public:
  qSlicerPerkEvaluatorRecorderControlsWidget(QWidget *parent=0);
  virtual ~qSlicerPerkEvaluatorRecorderControlsWidget();

protected slots:

  virtual void setPerkEvaluatorNode( vtkMRMLNode* newPerkEvaluatorNode );

  void onStartStopButtonClicked( bool state );
  
protected:
  QScopedPointer<qSlicerPerkEvaluatorRecorderControlsWidgetPrivate> d_ptr;

  vtkWeakPointer< vtkMRMLPerkEvaluatorNode > PerkEvaluatorNode;
  vtkWeakPointer< vtkSlicerPerkEvaluatorLogic > PerkEvaluatorLogic;
  
private:
  Q_DECLARE_PRIVATE(qSlicerPerkEvaluatorRecorderControlsWidget);
  Q_DISABLE_COPY(qSlicerPerkEvaluatorRecorderControlsWidget);

};

#endif
