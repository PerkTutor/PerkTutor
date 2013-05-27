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
  this->ToolCollection = vtkWorkflowToolCollection::New();
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
  if ( this->TransformRecorderLogic != NULL )
  {
    this->TransformRecorderLogic->Delete();
    this->TransformRecorderLogic = NULL;
  }
  this->ToolCollection->Delete();
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
::ImportWorkflowProcedure( std::string fileName )
{
  this->ModuleNode->SetWorkflowProcedureFileName( fileName );

  vtkXMLDataElement* element = this->ParseXMLFile( fileName );
  this->ToolCollection->ProcedureFromXMLElement( element );
}


void vtkSlicerWorkflowSegmentationLogic
::ImportWorkflowInput( std::string fileName )
{
  this->ModuleNode->SetWorkflowInputFileName( fileName );

  vtkXMLDataElement* element = this->ParseXMLFile( fileName );
  this->ToolCollection->InputFromXMLElement( element );
}


void vtkSlicerWorkflowSegmentationLogic
::ImportWorkflowTraining( std::string fileName )
{
  this->ModuleNode->SetWorkflowTrainingFileName( fileName );

  vtkXMLDataElement* element = this->ParseXMLFile( fileName );
  this->ToolCollection->TrainingFromXMLElement( element );
}


void vtkSlicerWorkflowSegmentationLogic
::SaveWorkflowTraining( std::string fileName )
{
  this->ModuleNode->SetWorkflowTrainingFileName( fileName );

  std::ofstream output( fileName.c_str() );  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Record file could not be opened!" );
    return;
  }  
  output << this->ToolCollection->TrainingToXMLString();

  output.close();
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

  for ( int i = 0; i < this->ToolCollection->GetNumTools(); i++ )
  {
    vtkWorkflowAlgorithm* newWorkflowAlgorithm = vtkWorkflowAlgorithm::New();
	newWorkflowAlgorithm->Tool = this->ToolCollection->GetToolAt(i);
	this->WorkflowAlgorithms.push_back( newWorkflowAlgorithm );
  }

  this->IndexToProcess = 0;
}


bool vtkSlicerWorkflowSegmentationLogic
::Train()
{
  for ( int i = 0; i < this->WorkflowAlgorithms.size(); i++ )
  {
    WorkflowAlgorithms.at(i)->Train();
  }

  return this->ToolCollection->GetTrained();
}


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
	  currentRecordBuffer->FromTransformBufferNode( transformBufferVector.at(i) ); // Note that this assumes the tool names are ok
	  currentAlgorithm->AddTrainingBuffer( currentRecordBuffer );
	}
  }

}


void vtkSlicerWorkflowSegmentationLogic
::SegmentBuffer( std::string fileName )
{
  this->ResetWorkflowAlgorithms();

  vtkMRMLTransformBufferNode* transformBuffer = vtkMRMLTransformBufferNode::New();
  transformBuffer->FromXMLElement( this->ParseXMLFile( fileName ) );

  while ( transformBuffer->GetNumTransforms() > this->IndexToProcess )
  {
    vtkTransformRecord* currentTransform = transformBuffer->GetTransformAt( this->IndexToProcess );
  
    vtkTrackingRecord* currentRecord = vtkTrackingRecord::New();
    currentRecord->FromTransformRecord( currentTransform );

    vtkWorkflowAlgorithm* currentWorkflowAlgorithm = this->GetWorkflowAlgorithmByName( currentRecord->GetLabel() );
    currentWorkflowAlgorithm->AddSegmentRecord( currentRecord );

    this->IndexToProcess++;
  }
}


void vtkSlicerWorkflowSegmentationLogic
::Update()
{
  if ( this->TransformRecorderLogic->GetBuffer()->GetNumTransforms() <= this->IndexToProcess )
  {
    return;
  }

  // If new transfrom, convert to label record and segment based on name
  vtkTransformRecord* currentTransform = this->TransformRecorderLogic->GetBuffer()->GetTransformAt( this->IndexToProcess );
  
  vtkTrackingRecord* currentRecord = vtkTrackingRecord::New();
  currentRecord->FromTransformRecord( currentTransform );

  vtkWorkflowAlgorithm* currentWorkflowAlgorithm = this->GetWorkflowAlgorithmByName( currentRecord->GetLabel() );
  currentWorkflowAlgorithm->AddSegmentRecord( currentRecord );

  this->IndexToProcess++;
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