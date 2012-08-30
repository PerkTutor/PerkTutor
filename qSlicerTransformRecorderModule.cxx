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
  return QStringList() << "IGT";
}
//-----------------------------------------------------------------------------
qSlicerTransformRecorderModule::~qSlicerTransformRecorderModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTransformRecorderModule::helpText()const
{
  return "For help on how to use this module visit: <a href='https://www.assembla.com/spaces/slicerigt'>SlicerIGT</a>";
}

//-----------------------------------------------------------------------------
QString qSlicerTransformRecorderModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}


//-----------------------------------------------------------------------------
QStringList qSlicerTransformRecorderModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Tamas Ungi (Queen's University)");
  return moduleContributors;
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
