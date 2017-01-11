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

#ifndef __qSlicerTrackedSequenceMessagesWidget_h
#define __qSlicerTrackedSequenceMessagesWidget_h

// Qt includes
#include "qSlicerWidget.h"

// FooBar Widgets includes
#include "qSlicerTransformRecorderModuleWidgetsExport.h"
#include "ui_qSlicerTrackedSequenceMessagesWidget.h"

#include "vtkMRMLTransformBufferNode.h"
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkSlicerTransformRecorderLogic.h"

class qSlicerTrackedSequenceMessagesWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_TRANSFORMRECORDER_WIDGETS_EXPORT 
qSlicerTrackedSequenceMessagesWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  qSlicerTrackedSequenceMessagesWidget(QWidget *parent=0);
  virtual ~qSlicerTrackedSequenceMessagesWidget();

public slots:

  virtual void setTrackedSequenceBrowserNode( vtkMRMLNode* newTrackedSequenceBrowserNode );

protected slots:

  virtual void onAddMessageButtonClicked();
  virtual void onRemoveMessageButtonClicked();
  virtual void onClearMessagesButtonClicked();
  virtual void onAddBlankMessageClicked();

  virtual void onMessageEdited( int row, int column );
  virtual void onMessageDoubleClicked( int row, int column );
  
  virtual void updateWidget();

protected:
  QScopedPointer<qSlicerTrackedSequenceMessagesWidgetPrivate> d_ptr;

  vtkWeakPointer< vtkMRMLSequenceBrowserNode > TrackedSequenceBrowserNode;
  vtkWeakPointer< vtkSlicerTransformRecorderLogic > TransformRecorderLogic;

  enum MessagesColumnsEnum{ MESSAGE_TIME_COLUMN, MESSAGE_NAME_COLUMN };

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerTrackedSequenceMessagesWidget);
  Q_DISABLE_COPY(qSlicerTrackedSequenceMessagesWidget);

};

#endif
