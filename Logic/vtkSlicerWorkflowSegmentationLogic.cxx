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
#include <vtkCollectionIterator.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerWorkflowSegmentationLogic);

//----------------------------------------------------------------------------
vtkSlicerWorkflowSegmentationLogic
::vtkSlicerWorkflowSegmentationLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerWorkflowSegmentationLogic
::~vtkSlicerWorkflowSegmentationLogic()
{
}



void vtkSlicerWorkflowSegmentationLogic
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}



void vtkSlicerWorkflowSegmentationLogic
::SetMRMLSceneInternal( vtkMRMLScene* newScene )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}



void vtkSlicerWorkflowSegmentationLogic
::RegisterNodes()
{
  //assert(this->GetMRMLScene() != 0);  
  if( ! this->GetMRMLScene() )
  {
    return;
  }

  vtkMRMLWorkflowSegmentationNode* wsNode = vtkMRMLWorkflowSegmentationNode::New();
  this->GetMRMLScene()->RegisterNodeClass( wsNode );
  wsNode->Delete();

  vtkMRMLWorkflowProcedureNode* wpNode = vtkMRMLWorkflowProcedureNode::New();
  this->GetMRMLScene()->RegisterNodeClass( wpNode );
  wpNode->Delete();
  
  vtkMRMLWorkflowProcedureStorageNode* wpsNode = vtkMRMLWorkflowProcedureStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass( wpsNode );
  wpsNode->Delete();

  vtkMRMLWorkflowInputNode* wiNode = vtkMRMLWorkflowInputNode::New();
  this->GetMRMLScene()->RegisterNodeClass( wiNode );
  wiNode->Delete();
  
  vtkMRMLWorkflowInputStorageNode* wisNode = vtkMRMLWorkflowInputStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass( wisNode );
  wisNode->Delete();

  vtkMRMLWorkflowTrainingNode* wtNode = vtkMRMLWorkflowTrainingNode::New();
  this->GetMRMLScene()->RegisterNodeClass( wtNode );
  wtNode->Delete();
  
  vtkMRMLWorkflowTrainingStorageNode* wtsNode = vtkMRMLWorkflowTrainingStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass( wtsNode );
  wtsNode->Delete();

  vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::New();
  this->GetMRMLScene()->RegisterNodeClass( toolNode );
  toolNode->Delete();

  vtkMRMLWorkflowSequenceNode* workflowSequenceNode = vtkMRMLWorkflowSequenceNode::New();
  this->GetMRMLScene()->RegisterNodeClass( workflowSequenceNode );
  workflowSequenceNode->Delete(); 
}



void vtkSlicerWorkflowSegmentationLogic
::UpdateFromMRMLScene()
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


vtkMRMLWorkflowToolNode* vtkSlicerWorkflowSegmentationLogic
::GetToolByProxyNodeID( vtkMRMLWorkflowSegmentationNode* workflowNode, std::string proxyNodeID )
{
  if ( workflowNode == NULL )
  {
    return NULL;
  }
  
  // Iterate over all tools
  std::vector< std::string > toolIDs = workflowNode->GetToolIDs();  
  for ( int i = 0; i < toolIDs.size(); i++ )
  {
    vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( toolIDs.at( i ) ) );
    if ( toolNode != NULL && toolNode->GetToolTransformID().compare( proxyNodeID ) == 0 )
    {
      return toolNode;
    }
  }
  
  return NULL;
}


void vtkSlicerWorkflowSegmentationLogic
::ResetAllToolSequences( vtkMRMLWorkflowSegmentationNode* workflowNode )
{
  if ( workflowNode == NULL )
  {
    return;
  }
  
  // Iterate over all tools
  std::vector< std::string > toolIDs = workflowNode->GetToolIDs();  
  for ( int i = 0; i < toolIDs.size(); i++ )
  {
    vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( toolIDs.at( i ) ) );
    if ( toolNode == NULL || ! toolNode->IsWorkflowProcedureSet() || ! toolNode->IsWorkflowInputSet() || ! toolNode->IsWorkflowTrainingSet() )
    {
      continue;
    }
    toolNode->ResetWorkflowSequences(); 
  }
  
  workflowNode->Modified();
}


void vtkSlicerWorkflowSegmentationLogic
::TrainAllTools( vtkMRMLWorkflowSegmentationNode* workflowNode, vtkCollection* trainingTrackedSequenceBrowserNodes )
{
  if ( workflowNode == NULL )
  {
    return;
  }
  
  // Iterate over all tools
  std::vector< std::string > toolIDs = workflowNode->GetToolIDs();
  
  for ( int i = 0; i < toolIDs.size(); i++ )
  {
    vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( toolIDs.at( i ) ) );
    if ( toolNode == NULL || ! toolNode->IsWorkflowProcedureSet() || ! toolNode->IsWorkflowInputSet() || ! toolNode->IsWorkflowTrainingSet() )
    {
      continue;
    }
    
    // Add each recorded tracked sequence
    vtkNew< vtkCollection > trainingWorkflowSequences;
    vtkNew< vtkCollectionIterator > trainingTrackedSequenceBrowserNodesIt;
    trainingTrackedSequenceBrowserNodesIt->SetCollection( trainingTrackedSequenceBrowserNodes );
    
    for ( trainingTrackedSequenceBrowserNodesIt->InitTraversal(); ! trainingTrackedSequenceBrowserNodesIt->IsDoneWithTraversal(); trainingTrackedSequenceBrowserNodesIt->GoToNextItem() )
    {
      vtkMRMLSequenceBrowserNode* currTrainingTrackedSequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast( trainingTrackedSequenceBrowserNodesIt->GetCurrentObject() );
      if ( currTrainingTrackedSequenceBrowserNode == NULL )
      {
        continue;
      }

      // Determine the IDs for the relevant tool and messages nodes
      vtkSlicerTransformRecorderLogic* trLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "TransformRecorder" ) ); // TODO: Can we just create an instance of the logic?
      vtkMRMLSequenceNode* messageSequenceNode = trLogic->GetMessageSequenceNode( currTrainingTrackedSequenceBrowserNode ); // Guaranteed to always output a valid message
      vtkMRMLNode* messageProxyNode = currTrainingTrackedSequenceBrowserNode->GetProxyNode( messageSequenceNode );
      if ( messageProxyNode == NULL )
      {
        continue;
      }
      
      vtkNew< vtkMRMLWorkflowSequenceNode > currWorkflowSequence;
      currWorkflowSequence->FromTrackedSequenceBrowserNode( currTrainingTrackedSequenceBrowserNode, toolNode->GetToolTransformID(), messageProxyNode->GetID(), toolNode->GetWorkflowProcedureNode()->GetAllTaskNames() );
      trainingWorkflowSequences->AddItem( currWorkflowSequence.GetPointer() );
    }
    
    

    toolNode->Train( trainingWorkflowSequences.GetPointer() );
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
    if ( toolNode == NULL || ! toolNode->IsWorkflowInputSet() )
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
    if ( toolNode == NULL || ! toolNode->IsWorkflowTrainingSet() )
    {
      return false;
    }    
  }
  
  return true;
}


std::vector< std::string > vtkSlicerWorkflowSegmentationLogic
::GetToolStatusStrings( vtkMRMLWorkflowSegmentationNode* workflowNode )
{
  std::vector< std::string > toolStatusStrings;
  if ( workflowNode == NULL )
  {
    return toolStatusStrings;
  }


  std::vector< std::string > toolIDs = workflowNode->GetToolIDs();
  for ( int i = 0; i < toolIDs.size(); i++ )
  {
    vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( toolIDs.at( i ) ) );    
    if ( toolNode == NULL || ! toolNode->IsWorkflowProcedureSet() )
    {
      continue;
    }

    std::stringstream toolStatus;
    toolStatus << toolNode->GetName() << " (" << toolNode->GetToolName() << "): ";
    
    if ( toolNode->IsWorkflowInputSet() )
    {
      toolStatus << "Parameters defined, ";
    }
    else
    {
      toolStatus << "No parameters, ";
    }    
    
    if ( toolNode->IsWorkflowTrainingSet() )
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
  std::vector< std::string > instructionStrings;
  if ( workflowNode == NULL )
  {
    return instructionStrings;
  }

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


std::vector< std::string > vtkSlicerWorkflowSegmentationLogic
::GetOrderedWorkflowTaskStrings( vtkMRMLWorkflowToolNode* toolNode )
{
  std::vector< std::string > taskStrings;
  if ( toolNode == NULL || toolNode->GetWorkflowProcedureNode() == NULL )
  {
    return taskStrings;
  }

  // TODO: This is a trick to get the list in an intelligible order
  // Need better SPMs to make this correct

  std::vector< std::string > taskNames = toolNode->GetWorkflowProcedureNode()->GetAllTaskNames();

  // Find a task whose prerequisite is itself
  for ( int i = 0; i < taskNames.size(); i++ )
  {
    vtkWorkflowTask* checkTask = toolNode->GetWorkflowProcedureNode()->GetTask( taskNames.at( i ) );
    if ( checkTask == NULL )
    {
      continue;
    }
    vtkWorkflowTask* prereqTask = toolNode->GetWorkflowProcedureNode()->GetTask( checkTask->GetPrerequisite() );
    if ( checkTask == prereqTask )
    {
      taskStrings.push_back( checkTask->GetName() );
    }
  }

  if ( taskStrings.size() == 0 )
  {
    return taskStrings;
  }

  // Now, find a task whose prerequisite is the most recently added task
  bool taskAdded = true;
  while ( taskAdded )
  {
    taskAdded = false;

    for ( int i = 0; i < taskNames.size(); i++ )
    {
      vtkWorkflowTask* checkTask = toolNode->GetWorkflowProcedureNode()->GetTask( taskNames.at( i ) );
      if ( checkTask == NULL )
      {
        continue;
      }
      vtkWorkflowTask* prereqTask = toolNode->GetWorkflowProcedureNode()->GetTask( checkTask->GetPrerequisite() );
      vtkWorkflowTask* addedTask = toolNode->GetWorkflowProcedureNode()->GetTask( taskStrings.at( taskStrings.size() - 1 ) );
      if ( prereqTask == addedTask )
      {
        taskStrings.push_back( checkTask->GetName() );
        taskAdded = true;
      }
    }

  }

  return taskStrings;
}



// Node update methods ----------------------------------------------------------

void vtkSlicerWorkflowSegmentationLogic
::SetupRealTimeProcessing( vtkMRMLWorkflowSegmentationNode* wsNode )
{
  // Check conditions
  if ( wsNode == NULL )
  {
    return;
  }

  // Use the python metrics calculator module
  this->ResetAllToolSequences( wsNode );
}


void vtkSlicerWorkflowSegmentationLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLWorkflowSegmentationNode* wsNode = vtkMRMLWorkflowSegmentationNode::SafeDownCast( caller );

  // The caller must be a vtkMRMLWorkflowSegmentationNode

  // Setup the real-time processing
  if ( wsNode != NULL && event == vtkMRMLWorkflowSegmentationNode::RealTimeProcessingStartedEvent )
  {
    this->SetupRealTimeProcessing( wsNode );
  }

  // Handle an event in the real-time processing
  if ( wsNode != NULL && wsNode->GetRealTimeProcessing() && event == vtkMRMLWorkflowSegmentationNode::TransformRealTimeAddedEvent )
  {
    vtkMRMLSequenceNode* masterSequenceNode = wsNode->GetTrackedSequenceBrowserNode()->GetMasterSequenceNode();
    if ( masterSequenceNode == NULL )
    {
      return;
    }

    std::string timeString = masterSequenceNode->GetNthIndexValue( masterSequenceNode->GetNumberOfDataNodes() - 1 );

    // Update for all transforms
    vtkNew< vtkCollection > sequenceNodes;
    wsNode->GetTrackedSequenceBrowserNode()->GetSynchronizedSequenceNodes( sequenceNodes.GetPointer(), true );
    vtkNew< vtkCollectionIterator > sequenceNodesIt; sequenceNodesIt->SetCollection( sequenceNodes.GetPointer() );
    for ( sequenceNodesIt->InitTraversal(); ! sequenceNodesIt->IsDoneWithTraversal(); sequenceNodesIt->GoToNextItem() )
    {
      vtkMRMLSequenceNode* currSequenceNode = vtkMRMLSequenceNode::SafeDownCast( sequenceNodesIt->GetCurrentObject() );
      vtkMRMLNode* currProxyNode = wsNode->GetTrackedSequenceBrowserNode()->GetProxyNode( currSequenceNode );
      if ( currSequenceNode == NULL || currProxyNode == NULL )
      {
        return;
      }
      vtkMRMLLinearTransformNode* currLinearTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( currSequenceNode->GetDataNodeAtValue( timeString ) ); // Require exact match
      if ( currLinearTransformNode == NULL )
      {
        return;
      }
      vtkMRMLWorkflowToolNode* toolNode = this->GetToolByProxyNodeID( wsNode, currProxyNode->GetID() );
      if ( toolNode == NULL )
      {
        return;
      }

      // Get the original task
      vtkWorkflowTask* originalTask = toolNode->GetCurrentTask();
      toolNode->AddAndSegmentTransform( currLinearTransformNode, timeString );
      
      if ( toolNode->GetCurrentTask() != NULL && toolNode->GetCurrentTask() != originalTask )
      {        
        vtkSlicerTransformRecorderLogic* trLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "TransformRecorder" ) ); // TODO: Can we just create an instance of the logic?
        trLogic->AddMessage( wsNode->GetTrackedSequenceBrowserNode(), toolNode->GetCurrentTask()->GetName(), timeString );
      }
    }

  }

}


void vtkSlicerWorkflowSegmentationLogic
::ProcessMRMLSceneEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLScene* callerNode = vtkMRMLScene::SafeDownCast( caller );

  // If the added node was a perk evaluator node then observe it
  vtkMRMLNode* addedNode = reinterpret_cast< vtkMRMLNode* >( callData );
  vtkMRMLWorkflowSegmentationNode* wsNode = vtkMRMLWorkflowSegmentationNode::SafeDownCast( addedNode );
  if ( event == vtkMRMLScene::NodeAddedEvent && wsNode != NULL )
  {
    // Observe if a real-time transform event is added
    wsNode->AddObserver( vtkMRMLWorkflowSegmentationNode::TransformRealTimeAddedEvent, ( vtkCommand* ) this->GetMRMLNodesCallbackCommand() );
    wsNode->AddObserver( vtkMRMLWorkflowSegmentationNode::RealTimeProcessingStartedEvent, ( vtkCommand* ) this->GetMRMLNodesCallbackCommand() );
  }

}