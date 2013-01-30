
#ifndef __qSlicerWorkflowSegmentationModuleWidget_h
#define __qSlicerWorkflowSegmentationModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include <QtGui>
#include "qSlicerWorkflowSegmentationModuleExport.h"

class qSlicerWorkflowSegmentationModuleWidgetPrivate;
class vtkMRMLNode;
class vtkMRMLWorkflowSegmentationNode;


/// \ingroup Slicer_QtModules_WorkflowSegmentation
class Q_SLICER_QTMODULES_WORKFLOWSEGMENTATION_EXPORT qSlicerWorkflowSegmentationModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerWorkflowSegmentationModuleWidget(QWidget *parent=0);
  virtual ~qSlicerWorkflowSegmentationModuleWidget();
  

protected:
  QScopedPointer<qSlicerWorkflowSegmentationModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  virtual void enter();

protected slots:

  void onTransformsNodeSelected(vtkMRMLNode* node);
  void onMRMLTransformNodeModified(vtkObject* caller);
  void onModuleNodeSelected();
 
  void onProcedureDefinitionButtonClicked();
  void onInputParameterButtonClicked();
  void onTrainingParameterButtonClicked();
  void onTrainingDataButtonClicked();
  void onTrainButtonClicked();

  void onSaveTrackingLogButtonClicked();
  void onSaveSegmentationButtonClicked();
  void onSaveTrainingButtonClicked();

  void onStopButtonClicked();
  void onStartButtonClicked();
  void onClearBufferButtonClicked();

  void onSegmentTrackingLogButtonClicked();

  void insertAnnotation();
  void clearAnnotations();

  void enableButtons();  
  void updateGUI();

private:
  Q_DECLARE_PRIVATE(qSlicerWorkflowSegmentationModuleWidget);
  Q_DISABLE_COPY(qSlicerWorkflowSegmentationModuleWidget);
};

#endif
