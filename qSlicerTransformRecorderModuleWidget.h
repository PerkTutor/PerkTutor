
#ifndef __qSlicerTransformRecorderModuleWidget_h
#define __qSlicerTransformRecorderModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include <QtGui>
#include "qSlicerTransformRecorderModuleExport.h"
#include "qSlicerTransformBufferWidget.h"
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

  // This widget will keep track if the buffer is changed
  unsigned long BufferStatus;
  // These quantities might be repeated by different buffers, so we still need the above
  unsigned long BufferTransformsStatus;
  unsigned long BufferMessagesStatus;
  
protected:
  QScopedPointer<qSlicerTransformRecorderModuleWidgetPrivate> d_ptr;
  
  virtual void setup();
  virtual void enter();

protected slots:

  void updateWidget();

private:
  Q_DECLARE_PRIVATE(qSlicerTransformRecorderModuleWidget);
  Q_DISABLE_COPY(qSlicerTransformRecorderModuleWidget);

  bool selectionsInitialized;
};

#endif
