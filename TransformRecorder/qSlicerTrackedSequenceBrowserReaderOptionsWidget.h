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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerTrackedSequenceBrowserReaderOptionsWidget_h
#define __qSlicerTrackedSequenceBrowserReaderOptionsWidget_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerIOOptionsWidget.h"

// TransformRecorder includes
#include "qSlicerTransformRecorderModuleExport.h"

class qSlicerTrackedSequenceBrowserReaderOptionsWidgetPrivate;

/// \ingroup Slicer_QtModules_TransformRecorder
class Q_SLICER_QTMODULES_TRANSFORMRECORDER_EXPORT qSlicerTrackedSequenceBrowserReaderOptionsWidget :
  public qSlicerIOOptionsWidget
{
  Q_OBJECT
public:
  qSlicerTrackedSequenceBrowserReaderOptionsWidget(QWidget *parent=0);
  virtual ~qSlicerTrackedSequenceBrowserReaderOptionsWidget();

protected slots:
  void updateProperties();


private:
  Q_DECLARE_PRIVATE_D(qGetPtrHelper(qSlicerIOOptions::d_ptr), qSlicerTrackedSequenceBrowserReaderOptionsWidget);
  Q_DISABLE_COPY(qSlicerTrackedSequenceBrowserReaderOptionsWidget);
};

#endif
