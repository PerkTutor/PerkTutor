
#include "vtkWorkflowToolCollection.h"

vtkStandardNewMacro( vtkWorkflowToolCollection );


vtkWorkflowToolCollection
::vtkWorkflowToolCollection()
{
// Nothing to do since we have a vector
  this->minTime = 0;
  this->maxTime = 0;
}


vtkWorkflowToolCollection
::~vtkWorkflowToolCollection()
{
  this->tools.clear();
}


int vtkWorkflowToolCollection
::GetNumTools()
{
  return this->tools.size();
}


vtkWorkflowTool* vtkWorkflowToolCollection
::GetToolAt( int index )
{
  return this->tools.at(index);
}


vtkWorkflowTool* vtkWorkflowToolCollection
::GetToolByName( std::string name )
{
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
	if ( strcmp( name.c_str(), this->GetToolAt(i)->Name.c_str() ) == 0 )
	{
	  return this->GetToolAt(i);
    }
  }

  vtkWorkflowTool* tool = vtkWorkflowTool::New();
  return tool;
}


bool vtkWorkflowToolCollection
::GetDefined()
{
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    if ( ! this->GetToolAt(i)->Defined )
	{
      return false;
	}
  }

  return true;
}


bool vtkWorkflowToolCollection
::GetInputted()
{
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    if ( ! this->GetToolAt(i)->Inputted )
	{
      return false;
	}
  }

  return true;
}


bool vtkWorkflowToolCollection
::GetTrained()
{
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    if ( ! this->GetToolAt(i)->Trained )
	{
      return false;
	}
  }

  return true;
}


double vtkWorkflowToolCollection
::GetMinTime()
{
  double minTime = this->GetToolAt(0)->Buffer->GetMinTime(); 
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    if ( this->GetToolAt(i)->Buffer->GetMinTime() < minTime )
	{
      minTime = this->GetToolAt(i)->Buffer->GetMinTime();
	}
  }
  return minTime;
}


double vtkWorkflowToolCollection
::GetMaxTime()
{
  double maxTime = this->GetToolAt(0)->Buffer->GetMaxTime(); 
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    if ( this->GetToolAt(i)->Buffer->GetMaxTime() < maxTime )
	{
      maxTime = this->GetToolAt(i)->Buffer->GetMaxTime();
	}
  }
  return maxTime;
}


double vtkWorkflowToolCollection
::GetTotalTime()
{
  return ( this->GetMaxTime() - this->GetMinTime() );
}


std::string vtkWorkflowToolCollection
::PerkProcedureToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "<PerkProcedure>" << std::endl;
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    xmlstring << this->GetToolAt(i)->PerkProcedureToXMLString();
  }
  xmlstring << "</PerkProcedure>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowToolCollection
::PerkProcedureFromXMLElement( vtkXMLDataElement* element )
{

  if ( ! element || strcmp( element->GetName(), "PerkProcedure" ) != 0 )
  {
    return;
  }

  int numElements = element->GetNumberOfNestedElements();

  for ( int i = 0; i < numElements; i++ )
  {

    vtkXMLDataElement* noteElement = element->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "Tool" ) != 0 )
    {
      continue;  // If it's not a "Tool", jump to the next.
    }

	this->GetToolByName( std::string( noteElement->GetAttribute( "Name" ) ) )->PerkProcedureFromXMLElement( noteElement );

  }

}


std::string vtkWorkflowToolCollection
::InputParameterToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "<WorkflowSegmentationParameters>" << std::endl;
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    xmlstring << this->GetToolAt(i)->InputParameterToXMLString();
  }
  xmlstring << "</WorkflowSegmentationParameters>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowToolCollection
::InputParameterFromXMLElement( vtkXMLDataElement* element )
{

  if ( ! element || strcmp( element->GetName(), "WorkflowSegmentationParameters" ) != 0 || ! this->GetDefined() )
  {
    return;
  }

  int numElements = element->GetNumberOfNestedElements();

  for ( int i = 0; i < numElements; i++ )
  {

    vtkXMLDataElement* noteElement = element->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "Tool" ) != 0 )
    {
      continue;  // If it's not a "Tool", jump to the next.
    }

	this->GetToolByName( std::string( noteElement->GetAttribute( "Name" ) ) )->InputParameterFromXMLElement( noteElement );

  }

}


std::string vtkWorkflowToolCollection
::TrainingParameterToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "<WorkflowSegmentationParameters>" << std::endl;
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    xmlstring << this->GetToolAt(i)->TrainingParameterToXMLString();
  }
  xmlstring << "</WorkflowSegmentationParameters>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowToolCollection
::TrainingParameterFromXMLElement( vtkXMLDataElement* element )
{

  if ( ! element || strcmp( element->GetName(), "WorkflowSegmentationParameters" ) != 0 || ! this->GetInputted() )
  {
    return;
  }

  int numElements = element->GetNumberOfNestedElements();

  for ( int i = 0; i < numElements; i++ )
  {

    vtkXMLDataElement* noteElement = element->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "Tool" ) != 0 )
    {
      continue;  // If it's not a "Tool", jump to the next.
    }

	this->GetToolByName( std::string( noteElement->GetAttribute( "Name" ) ) )->TrainingParameterFromXMLElement( noteElement );

  }

}


std::string vtkWorkflowToolCollection
::BuffersToXMLString()
{
  std::stringstream xmlstring;

  // Create a new vtkMRMLTransformBufferNode and then write its xml string
  vtkMRMLTransformBufferNode* transformBufferNode = vtkMRMLTransformBufferNode::New();

  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    vtkMRMLTransformBufferNode* tempTransformBufferNode = this->GetToolAt(i)->Buffer->ToTransformBufferNode();
    for ( int j = 0; j < tempTransformBufferNode->GetNumTransforms(); j++ )
	{      
      transformBufferNode->AddTransform( tempTransformBufferNode->GetTransformAt(j) );
	}
	for ( int j = 0; j < tempTransformBufferNode->GetNumMessages(); j++ )
	{      
      transformBufferNode->AddMessage( tempTransformBufferNode->GetMessageAt(j) );
	}
  }

  return transformBufferNode->ToXMLString();
}


void vtkWorkflowToolCollection
::BuffersFromXMLElement( vtkXMLDataElement* element )
{

  if ( strcmp( element->GetName(), "TransformRecorderLog" ) != 0 )
  {
    return;
  }

  // For this one, check all tools each of which will handle all elements
  vtkMRMLTransformBufferNode* transformBufferNode = vtkMRMLTransformBufferNode::New();
  std::vector<vtkMRMLTransformBufferNode*> transformBufferNodeVector = transformBufferNode->FromXMLElement( element );
  for ( int i = 0; i < transformBufferNodeVector.size(); i++ )
  {
    vtkWorkflowTool* currentTool = this->GetToolByName( transformBufferNodeVector.at(i)->GetCurrentTranform()->GetDeviceName() );
	vtkRecordBuffer* currentBuffer = vtkRecordBuffer::New();
	currentBuffer->FromTransformBufferNode( transformBufferNodeVector.at(i) );
    currentTool->Buffer = currentBuffer;
  }

}