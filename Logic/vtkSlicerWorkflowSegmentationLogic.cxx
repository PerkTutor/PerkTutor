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
}

//----------------------------------------------------------------------------
vtkSlicerWorkflowSegmentationLogic::~vtkSlicerWorkflowSegmentationLogic()
{
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
::ResetAllToolBuffers( vtkMRMLWorkflowSegmentationNode* workflowNode )
{
  if ( workflowNode == NULL )
  {
    return;
  }
  
  // Iterate over all tools
  std::vector< std::string > toolIDs = workflowNode->GetToolIDs();
  
  for ( i = 0; i < toolIDs.size(); i++ )
  {
    vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( toolIDs.at( i ) ) );
    if ( toolNode == NULL || ! toolNode->GetDefined() || ! toolNode->GetInputted() || ! toolNode->GetTrained() )
    {
      continue;
    }
    toolNode->ResetBuffers(); 
  }
  
  workflowNode->Modified();
}


void vtkSlicerWorkflowSegmentationLogic
::TrainAllTools( vtkMRMLWorkflowSegmentationNode* workflowNode, std::vector< std::string > trainingBufferIDs )
{
  if ( workflowNode == NULL )
  {
    return;
  }
  
  // Iterate over all tools
  std::vector< std::string > toolIDs = workflowNode->GetToolIDs();
  
  for ( i = 0; i < toolIDs.size(); i++ )
  {
    vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( toolIDs.at( i ) ) );
    if ( toolNode == NULL || ! toolNode->GetDefined() || ! toolNode->GetInputted() || ! toolNode->GetTrained() )
    {
      continue;
    }
    toolNode->GetWorkflowTrainingNode()->SetName( toolNode->GetWorkflowProcedureNode()->GetName() );
    
    // Grab only the relevant components of the transform buffers
    std::vector< vtkWorkflowLogRecordBuffer* > trainingRecordBuffers;
    
    for ( j = 0; j < trainingBufferIDs.size(); itr++ )
    {
      vtkMRMLTransformBufferNode* transformBuffer = vtkMRMLTransformBufferNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( trainingBufferIDs.at( j ) );
      vtkSmartPointer< vtkWorkflowLogRecordBuffer > recordBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New(); 
      recordBuffer->FromTransformBuffer( transformBuffer, toolNode->GetWorkflowProcedureNode()->GetName(), toolNode->GetWorkflowProcedureNode()->GetAllTaskNames() );
      trainingRecordBuffers.push_back( recordBuffer );
    }
    
    toolNode->Train( trainingRecordBuffers );
  }
  
  workflowNode->Modified();
}


bool vtkSlicerWorkflowSegmentationLogic
::GetAllToolsInputted( vtkMRMLWorkflowSegmentationNode* workflowNode )
{
  if ( workflowNode == NULL )
  {
    return false;
  }
  
  std::vector< std::string > toolIDs = workflowNode->GetToolIDs();
  for ( int i = 0; i < toolIDs.size(); i++ )
  {
    vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( toolIDs.at( i ) ) );
    if ( toolNode == NULL || ! toolNode->GetInputted() )
    {
      return false;
    }    
  }
  
  return true;
}


bool vtkSlicerWorkflowSegmentationLogic
::GetAllToolsTrained( vtkMRMLWorkflowSegmentationNode* workflowNode )
{
  if ( workflowNode == NULL )
  {
    return false;
  }
  
  std::vector< std::string > toolIDs = workflowNode->GetToolIDs();
  for ( int i = 0; i < toolIDs.size(); i++ )
  {
    vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( toolIDs.at( i ) ) );
    if ( toolNode == NULL || ! toolNode->GetTrained() )
    {
      return false;
    }    
  }
  
  return true;
}


std::vector< std::string > vtkSlicerWorkflowSegmentationLogic
::GetToolStatusStrings( vtkMRMLWorkflowSegmentationNode* workflowNode )
{
  if ( workflowNode == NULL )
  {
    return "";
  }

  std::vector< std::string > toolStatusStrings;
  std::vector< std::string > toolIDs = workflowNode->GetToolIDs();
  for ( int i = 0; i < toolIDs.size(); i++ )
  {
    vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( toolIDs.at( i ) ) );    
    if ( toolNode == NULL || ! toolNode->GetDefined() )
    {
      continue;
    }

    std::stringstream toolStatus;
    toolStatus << toolNode->GetName() << ": ";
    
    if ( toolNode->GetInputted() )
    {
      toolStatus << "Parameters defined, ";
    }
    else
    {
      toolStatus << "No parameters, ";
    }    
    
    if ( toolNode->GetTrained() )
    {
      toolStatus << "trained.";
    }
    else
    {
      toolStatus << "not trained.";
    }
    
    toolStatusStrings.push_back( toolStatus.str() );

  }

  return toolStatusStrings;
}


std::vector< std::string > vtkSlicerWorkflowSegmentationLogic
::GetInstructionStrings( vtkMRMLWorkflowSegmentationNode* workflowNode )
{
  if ( workflowNode == NULL )
  {
    return "";
  }

  std::vector< std::string > instructionStrings;
  std::vector< std::string > toolIDs = workflowNode->GetToolIDs();
  for ( int i = 0; i < toolIDs.size(); i++ )
  {
    vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( toolIDs.at( i ) ) );    
    if ( toolNode == NULL )
    {
      continue;
    }
    vtkWorkflowTask* currentTask = toolNode->GetCurrentTask();
    if ( currentTask == NULL )
    {
      continue;
    }
    
    std::stringstream instruction;
    instruction << toolNode->GetName() << ": ";
    instruction << currentTask->GetName() << " - ";
    instruction << currentTask->GetInstruction() << ".";
    
    instructionStrings.push_back( instruction.str() );

  }

  return instructionStrings;
}



