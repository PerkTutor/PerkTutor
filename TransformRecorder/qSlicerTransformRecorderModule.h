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

#ifndef __qSlicerTransformRecorderModule_h
#define __qSlicerTransformRecorderModule_h

// SlicerQt includes
#include "qSlicerLoadableModule.h"

#include "vtkSlicerConfigure.h" // For Slicer_HAVE_QT5

#include "qSlicerTransformRecorderModuleExport.h"

class qSlicerTransformRecorderModulePrivate;

/// \ingroup Slicer_QtModules_TransformRecorder
class Q_SLICER_QTMODULES_TRANSFORMRECORDER_EXPORT qSlicerTransformRecorderModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
#ifdef Slicer_HAVE_QT5
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");
#endif
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerTransformRecorderModule(QObject *parent=0);
  virtual ~qSlicerTransformRecorderModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);
  
  /// Help to use the module
  QString helpText() const override;

  /// Return acknowledgements
  QString acknowledgementText() const override;
  QStringList contributors() const override;

  /// Dependencies on other Slicer modules
  QStringList dependencies() const override;
  
  /// Return a custom icon for the module
  QIcon icon() const override;

  /// Return the category for the module
  QStringList categories() const override;

protected:

  /// Initialize the module. Register the volumes reader/writer
  void setup() override;

  /// Create and return the widget representation associated to this module
  qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerTransformRecorderModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTransformRecorderModule);
  Q_DISABLE_COPY(qSlicerTransformRecorderModule);

};

#endif
