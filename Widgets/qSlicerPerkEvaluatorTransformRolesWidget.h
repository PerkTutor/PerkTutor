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

#ifndef __qSlicerPerkEvaluatorTransformRolesWidget_h
#define __qSlicerPerkEvaluatorTransformRolesWidget_h

// Qt includes
#include "qSlicerWidget.h"
#include "qSlicerPerkEvaluatorModuleWidgetsExport.h"

#include "qSlicerPerkEvaluatorRolesWidget.h"

class qSlicerPerkEvaluatorTransformRolesWidgetPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_PERKEVALUATOR_WIDGETS_EXPORT 
qSlicerPerkEvaluatorTransformRolesWidget : public qSlicerPerkEvaluatorRolesWidget
{
  Q_OBJECT
public:
  qSlicerPerkEvaluatorTransformRolesWidget(QWidget *parent=0);
  virtual ~qSlicerPerkEvaluatorTransformRolesWidget();

protected slots:

  void onRolesChanged();

protected:
  QScopedPointer<qSlicerPerkEvaluatorTransformRolesWidgetPrivate> d_ptr;

  std::string getFixedHeader();
  std::string getMovingHeader();
  
  std::vector< std::string > getAllFixed();
  std::vector< std::string > getAllMoving();
  std::string getMovingFromFixed( std::string fixed );

private:
  Q_DECLARE_PRIVATE(qSlicerPerkEvaluatorTransformRolesWidget);
  Q_DISABLE_COPY(qSlicerPerkEvaluatorTransformRolesWidget);

};

#endif
