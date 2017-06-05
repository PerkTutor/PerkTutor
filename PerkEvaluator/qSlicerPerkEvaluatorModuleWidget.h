
#ifndef __qSlicerPerkEvaluatorModuleWidget_h
#define __qSlicerPerkEvaluatorModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include <QtGui>
#include "qSlicerTrackedSequenceBrowserWidget.h"
#include "qSlicerPerkEvaluatorMessagesWidget.h"
#include "qSlicerMetricsTableWidget.h"
#include "qSlicerPerkEvaluatorRecorderControlsWidget.h"
#include "qSlicerPerkEvaluatorTransformRolesWidget.h"
#include "qSlicerPerkEvaluatorAnatomyRolesWidget.h"
#include "qMRMLSequenceBrowserPlayWidget.h"
#include "qMRMLSequenceBrowserSeekWidget.h"

#include "qSlicerPerkEvaluatorModuleExport.h"

class qSlicerPerkEvaluatorModuleWidgetPrivate;
class vtkMRMLNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class Q_SLICER_QTMODULES_PERKEVALUATOR_EXPORT qSlicerPerkEvaluatorModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerPerkEvaluatorModuleWidget(QWidget *parent=0);
  virtual ~qSlicerPerkEvaluatorModuleWidget();

public slots:

  void OnAnalyzeClicked();

  void OnAnalysisStateUpdated( vtkObject* caller, void* value );
  void OnAnalysisCanceled();

  void OnBatchProcessButtonClicked();

  void OnMarkBeginChanged();
  void OnMarkBeginClicked();
  void OnMarkEndChanged();
  void OnMarkEndClicked();

  void OnMetricInstanceNodesChanged();

  void OnEditMetricInstanceNodeCreated( vtkMRMLNode* node );
  void OnEditMetricInstanceNodeChanged();

  void OnAutoUpdateMeasurementRangeToggled();
  void OnComputeTaskSpecificMetricsToggled();

  void OnDownloadAdditionalMetricsClicked();
  void OnRestoreDefaultMetricsClicked();

  void onTissueModelChanged( vtkMRMLNode* node );
  void onNeedleTransformChanged( vtkMRMLNode* node );
  void onNeedleOrientationChanged( QAbstractButton* newOrientationButton );
  
  void onTrackedSequenceBrowserNodeChanged( vtkMRMLNode* newTrackedSequenceBrowserNode );
  void onMetricsTableChanged( vtkMRMLNode* newMetricsTable );
  void mrmlNodeChanged( vtkMRMLNode* peNode );
  void updateWidgetFromMRMLNode();

protected:
  QScopedPointer<qSlicerPerkEvaluatorModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  virtual void setupEmbeddedWidgets();
  virtual void enter();

private:
  Q_DECLARE_PRIVATE(qSlicerPerkEvaluatorModuleWidget);
  Q_DISABLE_COPY(qSlicerPerkEvaluatorModuleWidget);
  
};

#endif
