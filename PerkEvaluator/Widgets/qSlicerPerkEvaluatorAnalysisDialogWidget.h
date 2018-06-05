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

#ifndef __qSlicerPerkEvaluatorAnalysisDialogWidget_h
#define __qSlicerPerkEvaluatorAnalysisDialogWidget_h

// Qt includes
#include <QtGui>
#include "qSlicerWidget.h"

#include "qSlicerPerkEvaluatorModuleWidgetsExport.h"
#include "ui_qSlicerPerkEvaluatorAnalysisDialogWidget.h"

#include "vtkMRMLNode.h"
#include "vtkSlicerPerkEvaluatorLogic.h"
#include "vtkMRMLPerkEvaluatorNode.h"

class qSlicerPerkEvaluatorAnalysisDialogWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_PERKEVALUATOR_WIDGETS_EXPORT 
qSlicerPerkEvaluatorAnalysisDialogWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerPerkEvaluatorAnalysisDialogWidget(QWidget *parent=0);
  virtual ~qSlicerPerkEvaluatorAnalysisDialogWidget();

public slots:

  void setPerkEvaluatorNode( vtkMRMLNode* node );

  void setLabelText( std::string newLabelText );

  void show(); // Re-implement
  void hide(); // Re-implement

protected slots:

  void onAnalysisStateUpdated( vtkObject* caller, void* value );
  void onAnalysisCanceled();

protected:
  QScopedPointer<qSlicerPerkEvaluatorAnalysisDialogWidgetPrivate> d_ptr;

  virtual void setup();
  virtual void enter();

  vtkWeakPointer< vtkSlicerPerkEvaluatorLogic > PerkEvaluatorLogic;
  vtkWeakPointer< vtkMRMLPerkEvaluatorNode > PerkEvaluatorNode;

private:
  Q_DECLARE_PRIVATE(qSlicerPerkEvaluatorAnalysisDialogWidget);
  Q_DISABLE_COPY(qSlicerPerkEvaluatorAnalysisDialogWidget);

};

#endif
