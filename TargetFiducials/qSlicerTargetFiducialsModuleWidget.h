/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#ifndef __qSlicerTargetFiducialsModuleWidget_h
#define __qSlicerTargetFiducialsModuleWidget_h

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerTargetFiducialsModuleExport.h"

class qSlicerTargetFiducialsModuleWidgetPrivate;
class vtkMRMLNode;
//class vtkMRMLTargetFiducialsNode;

/// \ingroup Slicer_QtModules_TargetFiducials
class Q_SLICER_QTMODULES_TARGETFIDUCIALS_EXPORT qSlicerTargetFiducialsModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerTargetFiducialsModuleWidget(QWidget *parent=0);
  virtual ~qSlicerTargetFiducialsModuleWidget();

public slots:


protected slots:

 // void updateWidget();
  void onFiducialListSelected();
  void onTransformNodeSelected();

protected:
  QScopedPointer<qSlicerTargetFiducialsModuleWidgetPrivate> d_ptr;
  
  void setup() override;

private:
  Q_DECLARE_PRIVATE(qSlicerTargetFiducialsModuleWidget);
  Q_DISABLE_COPY(qSlicerTargetFiducialsModuleWidget);
};

#endif
