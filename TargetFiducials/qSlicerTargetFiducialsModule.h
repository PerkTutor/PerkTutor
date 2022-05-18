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

#ifndef __qSlicerTargetFiducialsModule_h
#define __qSlicerTargetFiducialsModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "vtkSlicerConfigure.h" // For Slicer_HAVE_QT5

#include "qSlicerTargetFiducialsModuleExport.h"

class qSlicerTargetFiducialsModulePrivate;

/// \ingroup Slicer_QtModules_TargetFiducials
class Q_SLICER_QTMODULES_TARGETFIDUCIALS_EXPORT qSlicerTargetFiducialsModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerTargetFiducialsModule(QObject *parent=0);
  virtual ~qSlicerTargetFiducialsModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);
  
  /// Help to use the module
  QString helpText() const override;

  /// Return acknowledgements
  QString acknowledgementText() const override;

  /// Return the authors of the module
  QStringList  contributors() const override;

  /// Return a custom icon for the module
  QIcon icon() const override;

  /// Return the categories for the module
  QStringList categories() const override;

protected:

  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerTargetFiducialsModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTargetFiducialsModule);
  Q_DISABLE_COPY(qSlicerTargetFiducialsModule);

};

#endif
