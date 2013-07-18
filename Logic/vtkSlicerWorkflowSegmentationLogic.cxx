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

// WorkflowSegmentation includes
#include "vtkSlicerWorkflowSegmentationLogic.h"
#include "vtkMRMLWorkflowSegmentationNode.h"

// MRML includes
// #include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerWorkflowSegmentationLogic);

//----------------------------------------------------------------------------
vtkSlicerWorkflowSegmentationLogic::vtkSlicerWorkflowSegmentationLogic()
{
  this->ModuleNode = NULL;
  this->TransformRecorderLogic = NULL;
  this->Parser = vtkXMLDataParser::New();
  this->IndexToProcess = 0;
}

//----------------------------------------------------------------------------
vtkSlicerWorkflowSegmentationLogic::~vtkSlicerWorkflowSegmentationLogic()
{
  if ( this->ModuleNode != NULL )
  {
    this->ModuleNode->Delete();
    this->ModuleNode = NULL;
  }
  // The TransformRecorderLogic is already deleted in the TransformRecorder module itself
  for ( int i = 0; i < this->WorkflowAlgorithms.size(); i++ )
  {
    WorkflowAlgorithms.at(i)->Delete();
  }
  WorkflowAlgorithms.clear();
  Parser->Delete();
}



void vtkSlicerWorkflowSegmentationLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}



void vtkSlicerWorkflowSegmentationLogic::InitializeEventListeners()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(this->GetMRMLScene(), events.GetPointer());
}



void vtkSlicerWorkflowSegmentationLogic::RegisterNodes()
{
  //assert(this->GetMRMLScene() != 0);
  
  if( ! this->GetMRMLScene() )
  {
    return;
  }
  vtkMRMLWorkflowSegmentationNode* pNode = vtkMRMLWorkflowSegmentationNode::New();
  this->GetMRMLScene()->RegisterNodeClass( pNode );
  pNode->Delete(); 
  
}



void vtkSlicerWorkflowSegmentationLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}



void vtkSlicerWorkflowSegmentationLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{

}

void vtkSlicerWorkflowSegmentationLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
	assert(this->GetMRMLScene() != 0);
}


// Workflow Segmentation methods---------------------------------------------------------------------------

void vtkSlicerWorkflowSegmentationLogic
::SetModuleNode( vtkMRMLWorkflowSegmentationNode* node )
{
  vtkSetMRMLNodeMacro( this->ModuleNode, node );
  this->Modified();
}


vtkMRMLWorkflowSegmentationNode* vtkSlicerWorkflowSegmentationLogic
::GetModuleNode()
{
  return this->ModuleNode;
}


void vtkSlicerWorkflowSegmentationLogic
::ImportWorkflowProcedure( std::string fileName )
{
  this->ModuleNode->ImportWorkflowProcedure( fileName );
}


void vtkSlicerWorkflowSegmentationLogic
::ImportWorkflowInput( std::string fileName )
{
  this->ModuleNode->ImportWorkflowInput( fileName );
}


void vtkSlicerWorkflowSegmentationLogic
::ImportWorkflowTraining( std::string fileName )
{
  this->ModuleNode->ImportWorkflowTraining( fileName );
}


void vtkSlicerWorkflowSegmentationLogic
::SaveWorkflowTraining( std::string fileName )
{
  this->ModuleNode->SaveWorkflowTraining( fileName );
}


void vtkSlicerWorkflowSegmentationLogic
::ResetWorkflowAlgorithms()
{
  // For each tool, create a new workflow algorithm, and let it access that tool
  for ( int i = 0; i < this->WorkflowAlgorithms.size(); i++ )
  {
    this->WorkflowAlgorithms.at(i)->Delete();
  }
  this->WorkflowAlgorithms.clear();

  for ( int i = 0; i < this->ModuleNode->ToolCollection->GetNumTools(); i++ )
  {
    vtkWorkflowAlgorithm* newWorkflowAlgorithm = vtkWorkflowAlgorithm::New();
	newWorkflowAlgorithm->Tool = this->ModuleNode->ToolCollection->GetToolAt(i);
	vtkWorkflowAlgorithm* newCompletionWorkflowAlgorithm = vtkWorkflowAlgorithm::New();
	newCompletionWorkflowAlgorithm->Tool = this->ModuleNode->ToolCompletion->GetToolAt(i);
	newWorkflowAlgorithm->AddCompletionAlgorithm( newCompletionWorkflowAlgorithm );
	this->WorkflowAlgorithms.push_back( newWorkflowAlgorithm );
  }

  this->IndexToProcess = 0;
}


bool vtkSlicerWorkflowSegmentationLogic
::GetWorkflowAlgorithmsDefined()
{
  return this->ModuleNode->ToolCollection->GetDefined();
}


bool vtkSlicerWorkflowSegmentationLogic
::GetWorkflowAlgorithmsInputted()
{
  return this->ModuleNode->ToolCollection->GetInputted();
}


bool vtkSlicerWorkflowSegmentationLogic
::GetWorkflowAlgorithmsTrained()
{
  return this->ModuleNode->ToolCollection->GetTrained();
}


bool vtkSlicerWorkflowSegmentationLogic
::Train()
{
  for ( int i = 0; i < this->WorkflowAlgorithms.size(); i++ )
  {
    this->WorkflowAlgorithms.at(i)->Train();
  }

  return this->ModuleNode->ToolCollection->GetTrained();
}


//TODO: Should this use the transform recorder buffer import function?
void vtkSlicerWorkflowSegmentationLogic
::AddTrainingBuffer( std::string fileName )
{
  vtkMRMLTransformBufferNode* transformBuffer = vtkMRMLTransformBufferNode::New();
  transformBuffer->FromXMLElement( this->ParseXMLFile( fileName ) );  
  std::vector<vtkMRMLTransformBufferNode*> transformBufferVector = transformBuffer->SplitBufferByName();

  for ( int i = 0; i < transformBufferVector.size(); i++ )
  {
    vtkWorkflowAlgorithm* currentAlgorithm = this->GetWorkflowAlgorithmByName( transformBufferVector.at(i)->GetCurrentTransform()->GetDeviceName() );
	if ( currentAlgorithm != NULL )
	{
      vtkRecordBuffer* currentRecordBuffer = vtkRecordBuffer::New();
	  currentRecordBuffer->FromTransformBufferNode( transformBufferVector.at(i), currentAlgorithm->Tool->Procedure->GetTaskNames() ); // Note that this assumes the tool names are ok
	  currentAlgorithm->AddTrainingBuffer( currentRecordBuffer );
	  currentRecordBuffer->Delete(); // This is trimmed and a new copy is created and stored
	}
  }

  transformBuffer->Delete();
  vtkDeleteVector( transformBufferVector );
}



void vtkSlicerWorkflowSegmentationLogic
::Update( vtkMRMLTransformBufferNode* bufferNode )
{
  if ( this->TransformRecorderLogic == NULL || bufferNode == NULL )
  {
    return;
  }

  if ( bufferNode->GetNumTransforms() <= this->IndexToProcess )
  {
    return;
  }

  // If new transfrom, convert to label record and segment based on name
  vtkTransformRecord* currentTransform = bufferNode->GetTransformAt( this->IndexToProcess );
  this->IndexToProcess++;
  
  vtkTrackingRecord* currentRecord = vtkTrackingRecord::New();
  currentRecord->FromTransformRecord( currentTransform );

  // TODO: Should workflow algorithms be specific to a transform or to a buffer?
  vtkWorkflowAlgorithm* currentWorkflowAlgorithm = this->GetWorkflowAlgorithmByName( currentRecord->GetLabel() );

  if ( currentWorkflowAlgorithm == NULL || ! currentWorkflowAlgorithm->Tool->Trained )
  {
    currentRecord->Delete();
	return;
  }

  currentWorkflowAlgorithm->AddSegmentRecord( currentRecord );

  if ( currentWorkflowAlgorithm->CurrentTask != currentWorkflowAlgorithm->PrevTask )
  {
    // this->TransformRecorderLogic->AddMessage( currentWorkflowAlgorithm->CurrentTask->Name, currentTransform->GetTime() );
  }
  if ( currentWorkflowAlgorithm->DoTask != currentWorkflowAlgorithm->DoneTask )
  {
    this->TransformRecorderLogic->AddMessage( bufferNode, currentWorkflowAlgorithm->DoTask->Name, currentTransform->GetTime() );
  }

}


std::string vtkSlicerWorkflowSegmentationLogic
::GetToolInstructions( vtkMRMLTransformBufferNode* bufferNode )
{

  if ( this->TransformRecorderLogic == NULL || bufferNode == NULL )
  {
    return "";
  }

  std::stringstream instructions;
  std::vector<std::string> activeTransforms = bufferNode->GetActiveTransforms();
  for ( int i = 0; i < activeTransforms.size(); i++ )
  {
    vtkWorkflowAlgorithm* currentAlgorithm = this->GetWorkflowAlgorithmByName( activeTransforms.at(i) );

    if ( currentAlgorithm == NULL || currentAlgorithm->DoTask == NULL )
	{
      continue;
	}

    instructions << currentAlgorithm->Tool->Name << ": ";
	instructions << currentAlgorithm->DoTask->Name << " - ";
    instructions << currentAlgorithm->DoTask->Instruction;

	if ( i < activeTransforms.size() - 1 )
	{
      instructions << std::endl;	
	}

  }
  return instructions.str();
}


// Private methods for accessing workflow algorithms -------------------------------------------------------


vtkXMLDataElement* vtkSlicerWorkflowSegmentationLogic
::ParseXMLFile( std::string fileName )
{
  // Parse the file here, not in the widget
  Parser->SetFileName( fileName.c_str() );
  Parser->Parse();
  return Parser->GetRootElement();
}


vtkWorkflowAlgorithm* vtkSlicerWorkflowSegmentationLogic
::GetWorkflowAlgorithmByName( std::string name )
{
  for ( int i = 0; i < this->WorkflowAlgorithms.size(); i++ )
  {
    if ( WorkflowAlgorithms.at(i)->Tool->Name.compare( name ) == 0 )
	{
      return WorkflowAlgorithms.at(i);
	}
  }

  return NULL;
}