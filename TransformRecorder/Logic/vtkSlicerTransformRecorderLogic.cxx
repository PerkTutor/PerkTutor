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

// TransformRecorder includes
#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkMRMLNode.h"

// MRML includes
// #include "vtkMRMLIGTLConnectorNode.h"
#include "vtkMRMLViewNode.h"

// VTK includes
#include <vtkNew.h>
#include <vtkCollectionIterator.h>

// STD includes
#include <cassert>

// For getting the module logic
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"

#include "vtkSlicerSequencesLogic.h"
#include "vtkSlicerSequenceBrowserLogic.h"
#include "vtkMRMLSequenceBrowserNode.h"


// Helper Methods ------------------------------------------------------------

void Trim(std::string &str)
{
  str.erase(str.find_last_not_of(" \t\r\n")+1);
  str.erase(0,str.find_first_not_of(" \t\r\n"));
}

// Constants for reading transforms
static const int MAX_LINE_LENGTH = 1000;

static std::string SEQMETA_FIELD_FRAME_FIELD_PREFIX = "Seq_Frame";
static std::string SEQMETA_FIELD_IMG_STATUS = "ImageStatus";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTransformRecorderLogic);

//----------------------------------------------------------------------------
vtkSlicerTransformRecorderLogic::vtkSlicerTransformRecorderLogic()
{
}

//----------------------------------------------------------------------------
vtkSlicerTransformRecorderLogic::~vtkSlicerTransformRecorderLogic()
{
}



void vtkSlicerTransformRecorderLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}



void vtkSlicerTransformRecorderLogic::InitializeEventListeners()
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(this->GetMRMLScene(), events.GetPointer());
}



void vtkSlicerTransformRecorderLogic::RegisterNodes()
{
  if( ! this->GetMRMLScene() )
  {
    return;
  }

}



void vtkSlicerTransformRecorderLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}



void vtkSlicerTransformRecorderLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
  assert(this->GetMRMLScene() != 0);
}


void vtkSlicerTransformRecorderLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
  assert(this->GetMRMLScene() != 0);
}


vtkMRMLSequenceNode* vtkSlicerTransformRecorderLogic
::GetMessageSequenceNode( vtkMRMLSequenceBrowserNode* browserNode )
{
  // Do not make the messages sequence the master
  if ( browserNode == NULL || browserNode->GetMasterSequenceNode() == NULL )
  {
    return NULL;
  }

  // Check if there are any text annotation nodes
  vtkNew< vtkCollection > proxyNodes;
  browserNode->GetAllProxyNodes( proxyNodes.GetPointer() );
  vtkNew< vtkCollectionIterator > proxyNodesIt;
  proxyNodesIt->SetCollection( proxyNodes.GetPointer() );

  for ( proxyNodesIt->InitTraversal(); ! proxyNodesIt->IsDoneWithTraversal(); proxyNodesIt->GoToNextItem() )
  {
    vtkMRMLNode* messageNode = vtkMRMLNode::SafeDownCast( proxyNodesIt->GetCurrentObject() );
    if ( messageNode->GetAttribute( "Message" ) != NULL )
    {
      return browserNode->GetSequenceNode( messageNode );
    }
  }

  // If not, create one and add it to the sequence browser
  vtkSlicerSequenceBrowserLogic* sbLogic = vtkSlicerSequenceBrowserLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "SequenceBrowser" ) );
  if ( sbLogic == NULL )
  {
    return NULL;
  }


  vtkSmartPointer< vtkMRMLNode > messageNode;
  messageNode.TakeReference( vtkMRMLNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLScriptedModuleNode" ) ) );
  messageNode->SetName( "Message" );
  messageNode->SetHideFromEditors( false );
  messageNode->SetAttribute( "Message", "" );
  messageNode->SetScene( this->GetMRMLScene() );
	this->GetMRMLScene()->AddNode( messageNode );

  
  int modifyFlag = browserNode->StartModify();
  vtkMRMLSequenceNode* messageSequenceNode = sbLogic->AddSynchronizedNode( NULL, messageNode, browserNode );
  browserNode->SetRecording( messageSequenceNode, false );
  browserNode->SetOverwriteProxyName( NULL, false );
  browserNode->SetSaveChanges( NULL, false );
  browserNode->EndModify( modifyFlag );

  return messageSequenceNode;
}


void vtkSlicerTransformRecorderLogic
::AddMessage( vtkMRMLSequenceBrowserNode* browserNode, std::string messageString, std::string indexValue )
{
  if ( browserNode == NULL )
  {
    return;
  }

  // Record the timestamp
  vtkMRMLSequenceNode* messageSequenceNode = this->GetMessageSequenceNode( browserNode );
  if ( messageSequenceNode == NULL )
  {
    return;
  }
  vtkSmartPointer< vtkMRMLNode > messageNode;
  messageNode.TakeReference( vtkMRMLNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLScriptedModuleNode" ) ) );
  messageNode->SetName( "Message" );
  messageNode->SetAttribute( "Message", messageString.c_str() );

  messageSequenceNode->SetDataNodeAtValue( messageNode, indexValue );
}


void vtkSlicerTransformRecorderLogic
::UpdateMessage( vtkMRMLSequenceBrowserNode* browserNode, std::string messageString, int index )
{
  if ( browserNode == NULL )
  {
    return;
  }

  // Record the timestamp
  vtkMRMLSequenceNode* messageSequenceNode = this->GetMessageSequenceNode( browserNode );
  if ( messageSequenceNode == NULL )
  {
    return;
  }
  std::string indexValue = messageSequenceNode->GetNthIndexValue( index );

  vtkSmartPointer< vtkMRMLNode > messageNode;
  messageNode.TakeReference( vtkMRMLNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLScriptedModuleNode" ) ) );
  messageNode->SetName( "Message" );
  messageNode->SetAttribute( "Message", messageString.c_str() );

  messageSequenceNode->UpdateDataNodeAtValue( messageNode, indexValue );
}


void vtkSlicerTransformRecorderLogic
::RemoveMessage( vtkMRMLSequenceBrowserNode* browserNode, int index )
{
  if ( browserNode == NULL )
  {
    return;
  }

  vtkMRMLSequenceNode* messageSequenceNode = this->GetMessageSequenceNode( browserNode );
  if ( messageSequenceNode == NULL )
  {
    return;
  }
  std::string value = messageSequenceNode->GetNthIndexValue( index );
  messageSequenceNode->RemoveDataNodeAtValue( value );
}


void vtkSlicerTransformRecorderLogic
::ClearMessages( vtkMRMLSequenceBrowserNode* browserNode )
{
  if ( browserNode == NULL )
  {
    return;
  }

  vtkMRMLSequenceNode* messageSequenceNode = this->GetMessageSequenceNode( browserNode );
  if ( messageSequenceNode == NULL )
  {
    return;
  }
  browserNode->RemoveSynchronizedSequenceNode( messageSequenceNode->GetID() );
}


double vtkSlicerTransformRecorderLogic
::GetMaximumIndexValue( vtkMRMLSequenceBrowserNode* browserNode )
{
  if ( browserNode == NULL )
  {
    return 0;
  }

  double maxTime = - std::numeric_limits< double >::max();

  // Check over all 
  vtkNew< vtkCollection > sequenceNodes;
  browserNode->GetSynchronizedSequenceNodes( sequenceNodes.GetPointer(), true );
  vtkNew< vtkCollectionIterator > sequenceNodesIt;
  sequenceNodesIt->SetCollection( sequenceNodes.GetPointer() );

  for ( sequenceNodesIt->InitTraversal(); ! sequenceNodesIt->IsDoneWithTraversal(); sequenceNodesIt->GoToNextItem() )
  {
    vtkMRMLSequenceNode* currSequenceNode = vtkMRMLSequenceNode::SafeDownCast( sequenceNodesIt->GetCurrentObject() );
    if ( currSequenceNode == NULL
      || currSequenceNode->GetNumberOfDataNodes() == 0
      || currSequenceNode->GetIndexType() != vtkMRMLSequenceNode::NumericIndex )
    {
      continue;
    }

    int currNumFrames = currSequenceNode->GetNumberOfDataNodes();
    std::stringstream ss( currSequenceNode->GetNthIndexValue( currNumFrames - 1 ) );
    double currMaxTime; ss >> currMaxTime;
    if ( currMaxTime > maxTime )
    {
      maxTime = currMaxTime;
    }
  }

  if ( maxTime == - std::numeric_limits< double >::max() )
  {
    return 0; // Safe in case there are no recorded data nodes in the sequence
  }

  return maxTime;
}


int vtkSlicerTransformRecorderLogic
::GetMaximumNumberOfDataNodes( vtkMRMLSequenceBrowserNode* browserNode )
{
  if ( browserNode == NULL )
  {
    return 0;
  }

  int maxDataNodes = 0;

  // Check over all 
  vtkNew< vtkCollection > sequenceNodes;
  browserNode->GetSynchronizedSequenceNodes( sequenceNodes.GetPointer(), true );
  vtkNew< vtkCollectionIterator > sequenceNodesIt;
  sequenceNodesIt->SetCollection( sequenceNodes.GetPointer() );

  for ( sequenceNodesIt->InitTraversal(); ! sequenceNodesIt->IsDoneWithTraversal(); sequenceNodesIt->GoToNextItem() )
  {
    vtkMRMLSequenceNode* currSequenceNode = vtkMRMLSequenceNode::SafeDownCast( sequenceNodesIt->GetCurrentObject() );
    if ( currSequenceNode == NULL )
    {
      continue;
    }

    int currNumDataNodes = currSequenceNode->GetNumberOfDataNodes();
    if ( currNumDataNodes > maxDataNodes )
    {
      maxDataNodes = currNumDataNodes;
    }
  }

  return maxDataNodes;
}


// Module logic -------------------------------------------------
vtkMRMLAbstractLogic* vtkSlicerTransformRecorderLogic
::GetSlicerModuleLogic( std::string moduleName )
{
  qSlicerAbstractCoreModule* Module = qSlicerApplication::application()->moduleManager()->module( moduleName.c_str() );
  if ( Module != NULL )
  {
    return Module->logic();
  }
  return NULL;
}
