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

#include "qSlicerTransformRecorderModuleExport.h"

class qSlicerTransformRecorderModulePrivate;

/// \ingroup Slicer_QtModules_TransformRecorder
class Q_SLICER_QTMODULES_TRANSFORMRECORDER_EXPORT qSlicerTransformRecorderModule :
  public qSlicerLoadableModule
{
  Q_OBJECT
  Q_INTERFACES(qSlicerLoadableModule);

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerTransformRecorderModule(QObject *parent=0);
  virtual ~qSlicerTransformRecorderModule();

  qSlicerGetTitleMacro(QTMODULE_TITLE);
  
  /// Help to use the module
  virtual QString helpText()const;

  /// Return acknowledgements
  virtual QString acknowledgementText()const;
  virtual QStringList contributors()const;

  /// Dependencies on other Slicer modules
  virtual QStringList dependencies()const;
  
  /// Return a custom icon for the module
  virtual QIcon icon()const;

  /// Return the category for the module
  virtual QString category()const;
  virtual QStringList categories()const;

protected:

  /// Initialize the module. Register the volumes reader/writer
  virtual void setup();

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic();

protected:
  QScopedPointer<qSlicerTransformRecorderModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerTransformRecorderModule);
  Q_DISABLE_COPY(qSlicerTransformRecorderModule);

};

#endif
