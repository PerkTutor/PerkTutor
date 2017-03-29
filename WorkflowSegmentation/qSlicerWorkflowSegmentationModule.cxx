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

// WorkflowSegmentation Logic includes
#include "vtkSlicerWorkflowSegmentationLogic.h"
#include "vtkSlicerTransformRecorderLogic.h"

// WorkflowSegmentation includes
#include "qSlicerWorkflowSegmentationModule.h"
#include "qSlicerWorkflowSegmentationModuleWidget.h"
#include "qSlicerWorkflowProcedureReader.h"
#include "qSlicerWorkflowInputReader.h"
#include "qSlicerWorkflowTrainingReader.h"

// Slicer includes
#include "qSlicerNodeWriter.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerCoreApplication.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerWorkflowSegmentationModule, qSlicerWorkflowSegmentationModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_WorkflowSegmentation
class qSlicerWorkflowSegmentationModulePrivate
{
public:
  qSlicerWorkflowSegmentationModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerWorkflowSegmentationModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerWorkflowSegmentationModulePrivate::qSlicerWorkflowSegmentationModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerWorkflowSegmentationModule methods

//-----------------------------------------------------------------------------
qSlicerWorkflowSegmentationModule::qSlicerWorkflowSegmentationModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWorkflowSegmentationModulePrivate)
{
}

//-----------------------------------------------------------------------------
QString qSlicerWorkflowSegmentationModule::category()const
{
  // return "Developer Tools";
  return "";
}

//-----------------------------------------------------------------------------

QStringList qSlicerWorkflowSegmentationModule::categories() const
{
  return QStringList() << "Perk Tutor";
}

//-----------------------------------------------------------------------------
QStringList qSlicerWorkflowSegmentationModule::dependencies()const
{
  return QStringList() << "TransformRecorder";
}

//-----------------------------------------------------------------------------
qSlicerWorkflowSegmentationModule::~qSlicerWorkflowSegmentationModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerWorkflowSegmentationModule::helpText()const
{
  return "The purpose of the Workflow Segmentation module is to provide real-time instruction to medical trainees performing needle-based interventions by analyzing the procedural workflow. For help on how to use this module visit: <a href='http://www.perktutor.org/'>PerkTutor</a>.";
}

//-----------------------------------------------------------------------------
QString qSlicerWorkflowSegmentationModule::acknowledgementText()const
{
  return "This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO)";
}


//-----------------------------------------------------------------------------
QStringList qSlicerWorkflowSegmentationModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Tamas Ungi (Queen's University)");
  moduleContributors << QString("Matthew S. Holden (Queen's University)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerWorkflowSegmentationModule::icon()const
{
  return QIcon(":/Icons/WorkflowSegmentation.png");
}

//-----------------------------------------------------------------------------
void qSlicerWorkflowSegmentationModule::setup()
{
  this->Superclass::setup();

  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  vtkSlicerWorkflowSegmentationLogic* WorkflowSegmentationLogic = vtkSlicerWorkflowSegmentationLogic::SafeDownCast( this->logic() );
  
  // Register the IO
  app->coreIOManager()->registerIO( new qSlicerWorkflowProcedureReader( WorkflowSegmentationLogic, this ) );
  app->coreIOManager()->registerIO( new qSlicerNodeWriter( "Workflow Procedure", QString( "Workflow Procedure" ), QStringList() << "vtkMRMLWorkflowProcedureNode", true, this ) );
  app->coreIOManager()->registerIO( new qSlicerWorkflowInputReader( WorkflowSegmentationLogic, this ) );
  app->coreIOManager()->registerIO( new qSlicerNodeWriter( "Workflow Input", QString( "Workflow Input" ), QStringList() << "vtkMRMLWorkflowInputNode", true, this ) );
  app->coreIOManager()->registerIO( new qSlicerWorkflowTrainingReader( WorkflowSegmentationLogic, this ) );
  app->coreIOManager()->registerIO( new qSlicerNodeWriter( "Workflow Training", QString( "Workflow Training" ), QStringList() << "vtkMRMLWorkflowTrainingNode", true, this ) );

}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerWorkflowSegmentationModule::createWidgetRepresentation()
{
  return new qSlicerWorkflowSegmentationModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerWorkflowSegmentationModule::createLogic()
{
  vtkSlicerWorkflowSegmentationLogic* WorkflowSegmentationLogic = vtkSlicerWorkflowSegmentationLogic::New();
  return WorkflowSegmentationLogic;
}
