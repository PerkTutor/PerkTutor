
#ifndef __qSlicerWorkflowSegmentationModuleWidget_h
#define __qSlicerWorkflowSegmentationModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include <QtGui>
#include "qSlicerWorkflowSegmentationModuleExport.h"

#include "qSlicerTrackedSequenceMessagesWidget.h"
#include "qSlicerTrackedSequenceBrowserWidget.h"
#include "qSlicerWorkflowSegmentationRecorderControlsWidget.h"
#include "qSlicerWorkflowToolSummaryWidget.h"
#include "qSlicerWorkflowToolWidget.h"
#include "qSlicerWorkflowGuideDisplayWidget.h"


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
  
  void setup() override;
  virtual void setupEmbeddedWidgets();


protected slots:

  void onTrackedSequenceBrowserNodeChanged( vtkMRMLNode* newTrackedSequenceBrowserNode );
  void mrmlNodeChanged( vtkMRMLNode* wsNode );

  void onNodeAdded( vtkObject* caller, vtkObject* node );
  void onNodeRemoved( vtkObject* caller, vtkObject* node );
  void createWorkflowDisplaysForExistingNodes();
  
  void updateWidgetFromMRML();

private:
  Q_DECLARE_PRIVATE(qSlicerWorkflowSegmentationModuleWidget);
  Q_DISABLE_COPY(qSlicerWorkflowSegmentationModuleWidget);

};

#endif
