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

// Qt includes
#include <QtPlugin>

// TransformRecorder Logic includes
#include <vtkSlicerTransformRecorderLogic.h>

// TransformRecorder includes
#include "qSlicerTransformRecorderModule.h"
#include "qSlicerTransformRecorderModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerTransformRecorderModule, qSlicerTransformRecorderModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TransformRecorder
class qSlicerTransformRecorderModulePrivate
{
public:
  qSlicerTransformRecorderModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTransformRecorderModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTransformRecorderModulePrivate::qSlicerTransformRecorderModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTransformRecorderModule methods

//-----------------------------------------------------------------------------
qSlicerTransformRecorderModule::qSlicerTransformRecorderModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTransformRecorderModulePrivate)
{
}

//-----------------------------------------------------------------------------
QString qSlicerTransformRecorderModule::category()const
{
  // return "Developer Tools";
  return "";
}

//-----------------------------------------------------------------------------

QStringList qSlicerTransformRecorderModule::categories() const
{
  return QStringList() << "";
}
//-----------------------------------------------------------------------------
qSlicerTransformRecorderModule::~qSlicerTransformRecorderModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTransformRecorderModule::helpText()const
{
  QString help = 
    "This template module is meant to be used with the"
    "with the ModuleWizard.py script distributed with the"
    "Slicer source code (starting with version 4)."
    "Developers can generate their own source code using the"
    "wizard and then customize it to fit their needs.";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerTransformRecorderModule::acknowledgementText()const
{
  return "This work was supported by NAMIC, NAC, and the Slicer Community...";
}

//-----------------------------------------------------------------------------
QIcon qSlicerTransformRecorderModule::icon()const
{
  return QIcon(":/Icons/TransformRecorder.png");
}

//-----------------------------------------------------------------------------
void qSlicerTransformRecorderModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerTransformRecorderModule::createWidgetRepresentation()
{
  return new qSlicerTransformRecorderModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTransformRecorderModule::createLogic()
{
  return vtkSlicerTransformRecorderLogic::New();
}
