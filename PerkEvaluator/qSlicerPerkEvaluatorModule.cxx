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

// ExtensionTemplate Logic includes
#include "vtkSlicerPerkEvaluatorLogic.h"
#include "vtkSlicerTransformRecorderLogic.h"

// ExtensionTemplate includes
#include "qSlicerPerkEvaluatorModule.h"
#include "qSlicerPerkEvaluatorModuleWidget.h"
#include "qSlicerMetricScriptReader.h"

#include "qSlicerNodeWriter.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerCoreApplication.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerPerkEvaluatorModule, qSlicerPerkEvaluatorModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPerkEvaluatorModulePrivate
{
public:
  qSlicerPerkEvaluatorModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorModulePrivate::qSlicerPerkEvaluatorModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerPerkEvaluatorModule methods

//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorModule::qSlicerPerkEvaluatorModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPerkEvaluatorModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerPerkEvaluatorModule::~qSlicerPerkEvaluatorModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerPerkEvaluatorModule::helpText()const
{

  return "The purpose of the Perk Evaluator module is to review previously recorded procedures, and calculate motion efficiency metrics. For help on how to use this module visit: <a href='http://www.perktutor.org/'>PerkTutor</a>.";
}

//-----------------------------------------------------------------------------
QString qSlicerPerkEvaluatorModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPerkEvaluatorModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Tamas Ungi (Queen's University)");
  moduleContributors << QString("Matthew S. Holden (Queen's University)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerPerkEvaluatorModule::icon()const
{
  return QIcon(":/Icons/PerkEvaluator.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerPerkEvaluatorModule::categories() const
{
  return QStringList() << "Perk Tutor";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPerkEvaluatorModule::dependencies() const
{
  return QStringList() << "Sequences" << "SequenceBrowser" << "TransformRecorder";
}

//-----------------------------------------------------------------------------
void qSlicerPerkEvaluatorModule::setup()
{
  this->Superclass::setup();

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  vtkSlicerPerkEvaluatorLogic* PerkEvaluatorLogic = vtkSlicerPerkEvaluatorLogic::SafeDownCast( this->logic() );
  
  // Register the IO
  app->coreIOManager()->registerIO( new qSlicerMetricScriptReader( PerkEvaluatorLogic, this ) );
  app->coreIOManager()->registerIO( new qSlicerNodeWriter( "Python Metric Script", QString( "Python Metric Script" ), QStringList() << "vtkMRMLMetricScriptNode", true, this ) );

}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerPerkEvaluatorModule::createWidgetRepresentation()
{
  return new qSlicerPerkEvaluatorModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerPerkEvaluatorModule::createLogic()
{
	return vtkSlicerPerkEvaluatorLogic::New();
}
