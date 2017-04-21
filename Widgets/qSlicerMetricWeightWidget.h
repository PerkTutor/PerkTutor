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

#ifndef __qSlicerMetricWeightWidget_h
#define __qSlicerMetricWeightWidget_h

// Qt includes
#include "qSlicerWidget.h"

// VTK includes


// FooBar Widgets includes
#include "qSlicerSkillAssessmentModuleWidgetsExport.h"
#include "ui_qSlicerMetricWeightWidget.h"

class qSlicerMetricWeightWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_SKILLASSESSMENT_WIDGETS_EXPORT 
qSlicerMetricWeightWidget : public qSlicerWidget
{
  Q_OBJECT
public:
  typedef qSlicerWidget Superclass;
  qSlicerMetricWeightWidget(QWidget *parent=0);
  virtual ~qSlicerMetricWeightWidget();

  Q_INVOKABLE virtual void setMetricValue( double value );
  Q_INVOKABLE virtual void setMetricWeight( double weight );

  Q_INVOKABLE void setMetricValueVisible( bool visible );
  Q_INVOKABLE void setWeightSliderVisible( bool visible );

protected slots:
  
  void onWeightSliderChanged( double weight );

signals:

  void weightSliderChanged( double weight );

protected:

  QScopedPointer<qSlicerMetricWeightWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerMetricWeightWidget);
  Q_DISABLE_COPY(qSlicerMetricWeightWidget);

};

#endif
