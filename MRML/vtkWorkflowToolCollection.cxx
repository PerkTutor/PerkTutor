
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
  vtkDeleteVector( this->tools );
}


vtkWorkflowToolCollection* vtkWorkflowToolCollection
::DeepCopy()
{
  vtkWorkflowToolCollection* newWorkflowToolCollection = vtkWorkflowToolCollection::New();
  newWorkflowToolCollection->tools = vtkDeepCopyVector( this->tools ); 
  return newWorkflowToolCollection;
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

  return NULL;
}


void vtkWorkflowToolCollection
::AddTool( vtkWorkflowTool* newTool )
{
  if ( newTool != NULL )
  {
    this->tools.push_back( newTool );
  }
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

  return ( this->GetNumTools() > 0 );
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

  return ( this->GetNumTools() > 0 );
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

  return ( this->GetNumTools() > 0 );
}


double vtkWorkflowToolCollection
::GetMinTime()
{
  double minTime = this->GetToolAt(0)->Buffer->GetRecordAt(0)->GetTime(); 
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    if ( this->GetToolAt(i)->Buffer->GetRecordAt(0)->GetTime() < minTime )
	{
      minTime = this->GetToolAt(i)->Buffer->GetRecordAt(0)->GetTime();
	}
  }
  return minTime;
}


double vtkWorkflowToolCollection
::GetMaxTime()
{
  double maxTime = this->GetToolAt(0)->Buffer->GetCurrentRecord()->GetTime(); 
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    if ( this->GetToolAt(i)->Buffer->GetCurrentRecord()->GetTime() > maxTime )
	{
      maxTime = this->GetToolAt(i)->Buffer->GetCurrentRecord()->GetTime();
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
::ProcedureToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "<WorkflowProcedure>" << std::endl;
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    xmlstring << this->GetToolAt(i)->ProcedureToXMLString();
  }
  xmlstring << "</WorkflowProcedure>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowToolCollection
::ProcedureFromXMLElement( vtkXMLDataElement* element )
{

  if ( ! element || strcmp( element->GetName(), "WorkflowProcedure" ) != 0 )
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
    
	// This should add tools to the collection if they don't already exist
	vtkWorkflowTool* currentTool = this->GetToolByName( std::string( noteElement->GetAttribute( "Name" ) ) );
	if ( currentTool == NULL || currentTool->Name.compare( "" ) == 0 )
	{
      vtkWorkflowTool* newTool = vtkWorkflowTool::New();
      newTool->Name = std::string( noteElement->GetAttribute( "Name" ) );
      newTool->ProcedureFromXMLElement( noteElement );
	  this->AddTool( newTool );
	}
	else
	{
      currentTool->ProcedureFromXMLElement( noteElement );
	}

  }

}


std::string vtkWorkflowToolCollection
::InputToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "<WorkflowInput>" << std::endl;
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    xmlstring << this->GetToolAt(i)->InputToXMLString();
  }
  xmlstring << "</WorkflowInput>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowToolCollection
::InputFromXMLElement( vtkXMLDataElement* element )
{

  if ( ! element || strcmp( element->GetName(), "WorkflowInput" ) != 0 || ! this->GetDefined() )
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

	vtkWorkflowTool* currentTool = this->GetToolByName( std::string( noteElement->GetAttribute( "Name" ) ) );
	if ( currentTool != NULL )
	{
	  currentTool->InputFromXMLElement( noteElement );
	}

  }

}


std::string vtkWorkflowToolCollection
::TrainingToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "<WorkflowTraining>" << std::endl;
  for ( int i = 0; i < this->GetNumTools(); i++ )
  {
    xmlstring << this->GetToolAt(i)->TrainingToXMLString();
  }
  xmlstring << "</WorkflowTraining>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowToolCollection
::TrainingFromXMLElement( vtkXMLDataElement* element )
{

  if ( ! element || strcmp( element->GetName(), "WorkflowTraining" ) != 0 || ! this->GetInputted() )
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

    vtkWorkflowTool* currentTool = this->GetToolByName( std::string( noteElement->GetAttribute( "Name" ) ) );
	if ( currentTool != NULL )
	{
	  currentTool->TrainingFromXMLElement( noteElement );
	}

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
  transformBufferNode->FromXMLElement( element );
  std::vector<vtkMRMLTransformBufferNode*> transformBufferNodeVector = transformBufferNode->SplitBufferByName();
  for ( int i = 0; i < transformBufferNodeVector.size(); i++ )
  {
    vtkWorkflowTool* currentTool = this->GetToolByName( transformBufferNodeVector.at(i)->GetCurrentTransform()->GetDeviceName() );
	vtkRecordBuffer* currentBuffer = vtkRecordBuffer::New();
	currentBuffer->FromTransformBufferNode( transformBufferNodeVector.at(i) );
    currentTool->Buffer = currentBuffer;
  }

}