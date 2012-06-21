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

// TargetFiducials Logic includes
#include <vtkSlicerTargetFiducialsLogic.h>

// TargetFiducials includes
#include "qSlicerTargetFiducialsModule.h"
#include "qSlicerTargetFiducialsModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerTargetFiducialsModule, qSlicerTargetFiducialsModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TargetFiducials
class qSlicerTargetFiducialsModulePrivate
{
public:
  qSlicerTargetFiducialsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTargetFiducialsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTargetFiducialsModulePrivate::qSlicerTargetFiducialsModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTargetFiducialsModule methods

//-----------------------------------------------------------------------------
qSlicerTargetFiducialsModule::qSlicerTargetFiducialsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTargetFiducialsModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerTargetFiducialsModule::categories()const
{
  return QStringList() << "Developer Tools";
}

//-----------------------------------------------------------------------------
qSlicerTargetFiducialsModule::~qSlicerTargetFiducialsModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTargetFiducialsModule::helpText()const
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
QString qSlicerTargetFiducialsModule::acknowledgementText()const
{
  return "This work was supported by NAMIC, NAC, and the Slicer Community...";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTargetFiducialsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (Organization)");
  // moduleContributors << QString("Richard Roe (Organization2)");
  // ...
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerTargetFiducialsModule::icon()const
{
  return QIcon(":/Icons/TargetFiducials.png");
}

//-----------------------------------------------------------------------------
void qSlicerTargetFiducialsModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerTargetFiducialsModule::createWidgetRepresentation()
{
  return new qSlicerTargetFiducialsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTargetFiducialsModule::createLogic()
{
  return vtkSlicerTargetFiducialsLogic::New();
}
