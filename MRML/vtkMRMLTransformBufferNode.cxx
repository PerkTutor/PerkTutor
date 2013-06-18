

#include "vtkMRMLTransformBufferNode.h"


// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLTransformBufferNode* vtkMRMLTransformBufferNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLTransformBufferNode" );
  if( ret )
    {
      return ( vtkMRMLTransformBufferNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransformBufferNode();
}


vtkMRMLNode* vtkMRMLTransformBufferNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLTransformBufferNode" );
  if( ret )
    {
      return ( vtkMRMLTransformBufferNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransformBufferNode();
}



void vtkMRMLTransformBufferNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLTransformBufferNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);
}


void vtkMRMLTransformBufferNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);
}


void vtkMRMLTransformBufferNode
::Copy( vtkMRMLNode *anode )
{
  Superclass::Copy( anode );
  vtkMRMLTransformBufferNode *node = ( vtkMRMLTransformBufferNode* ) anode;

  this->Clear();

  for ( int i = 0; i < node->GetNumTransforms(); i++ )
  {
    this->AddTransform( node->GetTransformAt(i)->DeepCopy() );
  }

  for ( int i = 0; i < node->GetNumMessages(); i++ )
  {
    this->AddMessage( node->GetMessageAt(i)->DeepCopy() );
  }

}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLTransformBufferNode
::vtkMRMLTransformBufferNode()
{
  // No need to initialize the vectors
}


vtkMRMLTransformBufferNode
::~vtkMRMLTransformBufferNode()
{
  this->Clear();
}



// Transforms and Messages ------------------------------------------------------------------------


void vtkMRMLTransformBufferNode
::AddTransform( vtkTransformRecord* newTransform )
{
  // Ensure that we put it into sorted order (usually it will go at the end)
  if ( this->GetNumTransforms() < 1 )
  {
    this->transforms.push_back( newTransform );
	return;
  }
  if ( newTransform->GetTime() >= this->GetCurrentTransform()->GetTime() )
  {
    this->transforms.push_back( newTransform );
	return;
  }
  if ( newTransform->GetTime() <= this->GetTransformAt(0)->GetTime() )
  {
    this->transforms.insert( transforms.begin() + 0, newTransform );
	return;
  }

  // Records are probably near the end so this is more efficient than binary search
  for ( int i = this->GetNumTransforms() - 1; i >= 0; i-- )
  {
    if ( newTransform->GetTime() >= this->GetTransformAt(i)->GetTime() )
	{
      this->transforms.insert( transforms.begin() + i + 1, newTransform );
	  return;
	}
  }

}


void vtkMRMLTransformBufferNode
::AddMessage( vtkMessageRecord* newMessage )
{
  // Ensure that we put it into sorted order (usually it will go at the end)
  if ( this->GetNumMessages() < 1 )
  {
    this->messages.push_back( newMessage );
	return;
  }
  if ( newMessage->GetTime() >= this->GetCurrentMessage()->GetTime() )
  {
    this->messages.push_back( newMessage );
	return;
  }
  if ( newMessage->GetTime() <= this->GetMessageAt(0)->GetTime() )
  {
    this->messages.insert( messages.begin() + 0, newMessage );
	return;
  }

  // Records are probably near the end, so this is more efficient than binary search
  for ( int i = this->GetNumMessages() - 1; i >= 0; i-- )
  {
    if ( newMessage->GetTime() >= this->GetMessageAt(i)->GetTime() )
	{
      this->messages.insert( messages.begin() + i + 1, newMessage );
	  return;
	}
  }

}


void vtkMRMLTransformBufferNode
::RemoveTransformAt( int index )
{
  this->GetTransformAt(index)->Delete();
  this->transforms.erase( transforms.begin() + index );
}


void vtkMRMLTransformBufferNode
::RemoveTransformsByName( std::string name )
{
  for ( int i = 0; i < this->transforms.size(); i++ )
  {
    if ( this->GetTransformAt(i)->GetDeviceName().compare( name ) == 0 )
	{
	  this->GetTransformAt(i)->Delete();
	  this->transforms.erase( transforms.begin() + i );
	  i--;
	}
  }
}


void vtkMRMLTransformBufferNode
::RemoveMessageAt( int index )
{
  if ( index >= 0 && index < this->GetNumMessages() )
  {
    this->GetMessageAt(index)->Delete();
    this->messages.erase( messages.begin() + index );
  }
}


void vtkMRMLTransformBufferNode
::RemoveMessagesByName( std::string name )
{
  for ( int i = 0; i < this->messages.size(); i++ )
  {
    if ( this->GetMessageAt(i)->GetName().compare( name ) == 0 )
	{
      this->GetMessageAt(i)->Delete();
	  this->messages.erase( messages.begin() + i );
	  i--;
	}
  }
}


vtkTransformRecord* vtkMRMLTransformBufferNode
::GetTransformAt( int index )
{
  return this->transforms.at( index );
}


vtkTransformRecord* vtkMRMLTransformBufferNode
::GetCurrentTransform()
{
  return this->transforms.at( this->GetNumTransforms() - 1 );
}


vtkTransformRecord* vtkMRMLTransformBufferNode
::GetTransformByName( std::string name )
{
  for ( int i = 0; i < this->transforms.size(); i++ )
  {
    if ( this->GetTransformAt(i)->GetDeviceName().compare( name ) == 0 )
	{
	  return this->GetTransformAt(i);
	}
  }
  return NULL;
}


vtkMessageRecord* vtkMRMLTransformBufferNode
::GetMessageAt( int index )
{
  return this->messages.at( index );
}


vtkMessageRecord* vtkMRMLTransformBufferNode
::GetCurrentMessage()
{
  return this->messages.at( this->GetNumMessages() - 1 );
}


vtkMessageRecord* vtkMRMLTransformBufferNode
::GetMessageByName( std::string name )
{
  for ( int i = 0; i < this->messages.size(); i++ )
  {
    if ( this->GetMessageAt(i)->GetName().compare( name ) == 0 )
	{
	  return this->GetMessageAt(i);
	}
  }
  return NULL;
}


vtkTransformRecord* vtkMRMLTransformBufferNode
::GetTransformAtTime( double time )
{
  if ( this->GetNumTransforms() == 0 )
  {
    return NULL;
  }
  if ( time < this->GetTransformAt(0)->GetTime() )
  {
    return this->GetTransformAt( 0 );
  }
  if ( time > this->GetCurrentTransform()->GetTime() )
  {
    return this->GetCurrentTransform();
  }

  // Records are probably near the end, so this is more efficient than binary search
  int candidate1, candidate2;
  for ( int i = this->GetNumTransforms() - 1; i >= 0; i-- )
  {
    if ( time > this->GetTransformAt(i)->GetTime() )
	{
	  candidate1 = i;
	  candidate2 = i + 1;
	  break;
	}
  }
  	  
  if ( abs ( time - this->GetTransformAt(candidate1)->GetTime() ) < abs ( time - this->GetTransformAt(candidate2)->GetTime() ) )
  {
    return this->GetTransformAt( candidate1 );
  }
  else
  {
    return this->GetTransformAt( candidate2 );
  }

}


vtkMessageRecord* vtkMRMLTransformBufferNode
::GetMessageAtTime( double time )
{
  if ( this->GetNumMessages() == 0 )
  {
    return NULL;
  }
  if ( time < this->GetMessageAt(0)->GetTime() )
  {
    return this->GetMessageAt( 0 );
  }
  if ( time > this->GetCurrentMessage()->GetTime() )
  {
    return this->GetCurrentMessage();
  }

  // Records are probably near the end so this is more efficient than binary search
  int candidate1, candidate2;
  for ( int i = this->GetNumMessages() - 1; i >= 0; i-- )
  {
    if ( time > this->GetMessageAt(i)->GetTime() )
	{
	  candidate1 = i;
	  candidate2 = i + 1;
	  break;
	}
  }
  	  
  if ( abs ( time - this->GetMessageAt(candidate1)->GetTime() ) < abs ( time - this->GetMessageAt(candidate2)->GetTime() ) )
  {
    return this->GetMessageAt( candidate1 );
  }
  else
  {
    return this->GetMessageAt( candidate2 );
  }

}


int vtkMRMLTransformBufferNode
::GetNumTransforms()
{
  return this->transforms.size();
}


int vtkMRMLTransformBufferNode
::GetNumMessages()
{
  return this->messages.size();
}


void vtkMRMLTransformBufferNode
::Clear()
{
  this->ClearTransforms();
  this->ClearMessages();
}


void vtkMRMLTransformBufferNode
::ClearTransforms()
{
  // Need to explicitly call the VTK delete function on each of these VTK objects
  for ( int i = 0; i < this->GetNumTransforms(); i++ )
  {
    this->GetTransformAt(i)->Delete();
  }
  this->transforms.clear();
}


void vtkMRMLTransformBufferNode
::ClearMessages()
{
  // Need to explicitly call the VTK delete function on each of these VTK objects
  for ( int i = 0; i < this->GetNumMessages(); i++ )
  {
    this->GetMessageAt(i)->Delete();
  }
  this->messages.clear();
}


// We store many transforms of many different devices here possibly
// This method will return an array of buffers that each only refer to one tool
std::vector<vtkMRMLTransformBufferNode*> vtkMRMLTransformBufferNode
::SplitBufferByName()
{
  std::vector<vtkMRMLTransformBufferNode*> deviceBuffers;

  // Separate the transforms by their device name
  for ( int i = 0; i < this->GetNumTransforms(); i++ )
  {
    bool deviceExists = false;
    for ( int j = 0; j < deviceBuffers.size(); j++ )
	{
	  // Observe that a device buffer only exists if it has a transform, thus, the current transform is always available
      if ( deviceBuffers.at(j)->GetCurrentTransform()->GetDeviceName().compare( this->GetTransformAt(i)->GetDeviceName() ) == 0 )
	  {
        deviceBuffers.at(j)->AddTransform( this->GetTransformAt(i)->DeepCopy() );
		deviceExists = true;
	  }
	}

	if ( ! deviceExists )
	{
	  vtkMRMLTransformBufferNode* newDeviceBuffer = vtkMRMLTransformBufferNode::New();
	  newDeviceBuffer->AddTransform( this->GetTransformAt(i)->DeepCopy() );
	  deviceBuffers.push_back( newDeviceBuffer );
	}

  }

  // Add the messages to all of the device buffers
  for ( int i = 0; i < this->GetNumMessages(); i++ )
  {
    for ( int j = 0; j < deviceBuffers.size(); j++ )
	{
      deviceBuffers.at(j)->AddMessage( this->GetMessageAt(i)->DeepCopy() );
	}
  }

  return deviceBuffers;
  
}


// This method makes a buffer out of everything with the specified device name
vtkMRMLTransformBufferNode* vtkMRMLTransformBufferNode
::GetBufferByName( std::string name )
{
  std::vector<vtkMRMLTransformBufferNode*> deviceBuffers = this->SplitBufferByName();

  vtkMRMLTransformBufferNode* outputBuffer = NULL;
  for ( int j = 0; j < deviceBuffers.size(); j++ )
  {
    if ( deviceBuffers.at(j)->GetCurrentTransform()->GetDeviceName().compare( name ) == 0 )
	{
	  outputBuffer->Copy( deviceBuffers.at(j) );
	}
  }

  deviceBuffers.clear();
  return outputBuffer;

}


std::string vtkMRMLTransformBufferNode
::ToXMLString()
{
  std::stringstream xmlstring;
  
  xmlstring << "<TransformRecorderLog>" << std::endl;

  for ( int i = 0; i < this->GetNumTransforms(); i++ )
  {
    xmlstring << this->GetTransformAt(i)->ToXMLString();
  }

  for ( int i = 0; i < this->GetNumMessages(); i++ )
  {
    xmlstring << this->GetMessageAt(i)->ToXMLString();
  }

  xmlstring << "</TransformRecorderLog>" << std::endl;

  return xmlstring.str();
}


void vtkMRMLTransformBufferNode
::FromXMLElement( vtkXMLDataElement* rootElement )
{
  if ( ! rootElement || strcmp( rootElement->GetName(), "TransformRecorderLog" ) != 0 ) 
  {
    return;
  }

  int numElements = rootElement->GetNumberOfNestedElements();  // Number of saved records (including transforms and messages).
  
  for ( int i = 0; i < numElements; i++ )
  {
    vtkXMLDataElement* element = rootElement->GetNestedElement( i );

	if ( element == NULL || strcmp( element->GetName(), "log" ) != 0 )
	{
      continue;
	}

	if ( strcmp( element->GetAttribute( "type" ), "transform" ) == 0 )
    {
		vtkTransformRecord* newTransform = vtkTransformRecord::New();
		newTransform->FromXMLElement( element );
		this->AddTransform( newTransform );
		continue;
    }

    if ( strcmp( element->GetAttribute( "type" ), "message" ) == 0 )
    {
		vtkMessageRecord* newMessage = vtkMessageRecord::New();
		newMessage->FromXMLElement( element );
		this->AddMessage( newMessage );
		continue;
    }
   
  }

}
