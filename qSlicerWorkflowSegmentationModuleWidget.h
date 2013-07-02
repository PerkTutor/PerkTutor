
#ifndef __qSlicerWorkflowSegmentationModuleWidget_h
#define __qSlicerWorkflowSegmentationModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include <QtGui>
#include "qSlicerWorkflowSegmentationModuleExport.h"
#include "qSlicerMessagesWidget.h"
#include "qSlicerWorkflowSegmentationRecorderControlsWidget.h"

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
  virtual void exit();

protected slots:

  void onModuleNodeSelected();
 
  void onWorkflowProcedureButtonClicked();
  void onWorkflowInputButtonClicked();
  void onWorkflowTrainingButtonClicked();
  void onWorkflowTrainingFilesButtonClicked();
  void onTrainButtonClicked();

  void onSegmentTransformBufferButtonClicked();

  void setupInstructions();
  void enableButtons();  
  void updateWidget();

private:
  Q_DECLARE_PRIVATE(qSlicerWorkflowSegmentationModuleWidget);
  Q_DISABLE_COPY(qSlicerWorkflowSegmentationModuleWidget);

  QLabel* InstructionLabel;
};

#endif
