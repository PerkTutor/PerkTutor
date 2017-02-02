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
#include "qSlicerTransformBufferReader.h"
#include "qSlicerTrackedSequenceBrowserReader.h"
#include "qSlicerTrackedSequenceBrowserWriter.h"

// Slicer includes
#include "qSlicerNodeWriter.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerCoreApplication.h"

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
  return QStringList() << "Perk Tutor";
}

 //-----------------------------------------------------------------------------
 QStringList qSlicerTransformRecorderModule::dependencies()const
 {
   return QStringList() << "Sequences" << "SequenceBrowser";
 }

//-----------------------------------------------------------------------------
qSlicerTransformRecorderModule::~qSlicerTransformRecorderModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTransformRecorderModule::helpText()const
{
  return "The purpose of the Transform Recorder module is to record and save tool trajectories associated with needle-based interventions. For help on how to use this module visit: <a href='http://www.perktutor.org/'>PerkTutor</a>.";
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
  moduleContributors << QString("Matthew S. Holden (Queen's University)");
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

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  vtkSlicerTransformRecorderLogic* TransformRecorderLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( this->logic() );
  
  // Register the IO
  app->coreIOManager()->registerIO( new qSlicerTransformBufferReader( TransformRecorderLogic, this ) );
  app->coreIOManager()->registerIO( new qSlicerTrackedSequenceBrowserReader( TransformRecorderLogic, this ) );
  app->coreIOManager()->registerIO( new qSlicerNodeWriter( "Transform Recorder", QString( "Transform Buffer" ), QStringList() << "vtkMRMLTransformBufferNode", true, this ) );
  app->coreIOManager()->registerIO( new qSlicerTrackedSequenceBrowserWriter( TransformRecorderLogic, this ) );
  
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
