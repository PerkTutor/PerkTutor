
#ifndef __qSlicerTransformRecorderModuleWidget_h
#define __qSlicerTransformRecorderModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include <QtGui>
#include "qSlicerTransformRecorderModuleExport.h"
#include "qSlicerMessagesWidget.h"
#include "qSlicerRecorderControlsWidget.h"

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
  
protected:
  QScopedPointer<qSlicerTransformRecorderModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  virtual void enter();

protected slots:
  void onModuleNodeSelected();
  void updateSelectionsFromObservedNodes();
  void updateObservedNodesFromSelections();
  void updateWidget();

private:
  Q_DECLARE_PRIVATE(qSlicerTransformRecorderModuleWidget);
  Q_DISABLE_COPY(qSlicerTransformRecorderModuleWidget);

  bool selectionsInitialized;
};

#endif
