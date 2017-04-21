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

// FooBar Widgets includes
#include "qSlicerMetricWeightWidget.h"

#include <QtGui>


//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CreateModels
class qSlicerMetricWeightWidgetPrivate
  : public Ui_qSlicerMetricWeightWidget
{
  Q_DECLARE_PUBLIC(qSlicerMetricWeightWidget);
protected:
  qSlicerMetricWeightWidget* const q_ptr;

public:
  qSlicerMetricWeightWidgetPrivate( qSlicerMetricWeightWidget& object);
  ~qSlicerMetricWeightWidgetPrivate();
  virtual void setupUi(qSlicerMetricWeightWidget*);
};

// --------------------------------------------------------------------------
qSlicerMetricWeightWidgetPrivate
::qSlicerMetricWeightWidgetPrivate( qSlicerMetricWeightWidget& object) : q_ptr(&object)
{
}

qSlicerMetricWeightWidgetPrivate
::~qSlicerMetricWeightWidgetPrivate()
{
}


// --------------------------------------------------------------------------
void qSlicerMetricWeightWidgetPrivate
::setupUi(qSlicerMetricWeightWidget* widget)
{
  this->Ui_qSlicerMetricWeightWidget::setupUi(widget);
}

//-----------------------------------------------------------------------------
// qSlicerMetricWeightWidget methods

//-----------------------------------------------------------------------------
qSlicerMetricWeightWidget
::qSlicerMetricWeightWidget(QWidget* parentWidget) : Superclass( parentWidget ) , d_ptr( new qSlicerMetricWeightWidgetPrivate(*this) )
{
  this->setup();
}


qSlicerMetricWeightWidget
::~qSlicerMetricWeightWidget()
{
}


void qSlicerMetricWeightWidget
::setup()
{
  Q_D(qSlicerMetricWeightWidget);

  d->setupUi(this);

  connect( d->WeightSlider, SIGNAL( valueChanged( double ) ), this, SLOT( onWeightSliderChanged( double ) ) );
}


void qSlicerMetricWeightWidget
::onWeightSliderChanged( double weight )
{
  Q_D(qSlicerMetricWeightWidget);

  emit weightSliderChanged( weight );
}


void qSlicerMetricWeightWidget
::setMetricValue( double value )
{
  Q_D(qSlicerMetricWeightWidget);
  
  d->MetricValueLabel->setText( QString( "(%1)" ).arg( value ) );
}


void qSlicerMetricWeightWidget
::setMetricWeight( double weight )
{
  Q_D(qSlicerMetricWeightWidget);
  
  d->WeightSlider->setValue( weight );
}


void qSlicerMetricWeightWidget
::setMetricValueVisible( bool visible )
{
  Q_D(qSlicerMetricWeightWidget);
  
  d->MetricValueLabel->setVisible( visible );
}


void qSlicerMetricWeightWidget
::setWeightSliderVisible( bool visible )
{
  Q_D(qSlicerMetricWeightWidget);
  
  d->WeightSlider->setVisible( visible );
}
