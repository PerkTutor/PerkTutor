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

#ifndef __qSlicerAssessmentTableWidget_h
#define __qSlicerAssessmentTableWidget_h

// Qt includes
#include "qSlicerWidget.h"

// VTK includes
#include <vtkTable.h>
#include "vtkMRMLTableNode.h"

// FooBar Widgets includes
#include "qSlicerSkillAssessmentModuleWidgetsExport.h"
#include "ui_qSlicerAssessmentTableWidget.h"

class qSlicerAssessmentTableWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_SKILLASSESSMENT_WIDGETS_EXPORT 
qSlicerAssessmentTableWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerAssessmentTableWidget(QWidget *parent=0);
  virtual ~qSlicerAssessmentTableWidget();

  Q_INVOKABLE virtual void setMetricNode( vtkMRMLNode* table ); // Note that this is could be the raw metrics or the regularized metrics
  Q_INVOKABLE virtual void setWeightNode( vtkMRMLNode* table );
  
  Q_INVOKABLE virtual void setMetricScoreNode( vtkMRMLNode* table );
  Q_INVOKABLE virtual void setTaskScoreNode( vtkMRMLNode* table );
  
  Q_INVOKABLE virtual void setOverallScore( double score );

  Q_INVOKABLE virtual void setMetricWeightsVisible( bool visible );
  Q_INVOKABLE virtual void setScoreWeightsVisible( bool visible );

protected slots:
  
  void onWeightSliderChanged( double weight );

  void updateWidget();

  void updateHeaders();
  void updateMetrics();
  void updateMetricScores();
  void updateTaskScores();
  void updateOverallScore();

protected:

  QScopedPointer<qSlicerAssessmentTableWidgetPrivate> d_ptr;

  vtkWeakPointer< vtkMRMLTableNode > MetricNode;
  vtkWeakPointer< vtkMRMLTableNode > WeightNode;
  vtkWeakPointer< vtkMRMLTableNode > MetricScoreNode;
  vtkWeakPointer< vtkMRMLTableNode > TaskScoreNode;
  
  double OverallScore;

  bool ShowMetricWeights;
  bool ShowScoreWeights;

  virtual void setup();

  void getHeaderTable( vtkTable* headerTable );

private:
  Q_DECLARE_PRIVATE(qSlicerAssessmentTableWidget);
  Q_DISABLE_COPY(qSlicerAssessmentTableWidget);

};

#endif
