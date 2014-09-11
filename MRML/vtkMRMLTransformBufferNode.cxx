

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
  
  for ( int i = 0; i < this->activeTransforms.size(); i++ )
  {
    of << indent << " ActiveTransform" << i << "=\"" << this->activeTransforms.at(i) << "\"";
  }
}


void vtkMRMLTransformBufferNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
 
	if ( std::string( attName ).find( "ActiveTransform" ) != std::string::npos )
    {
	  this->activeTransforms.push_back( std::string( attValue ) );
    }
  }

  this->ActiveTransformsStatus++;
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

  for ( int i = 0; i < node->GetActiveTransforms().size(); i++ )
  {
    this->AddActiveTransform( node->GetActiveTransforms().at(i) );
  }

}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLTransformBufferNode
::vtkMRMLTransformBufferNode()
{
  // No need to initialize the vectors
  // Initialize the statuses for updating
  this->TransformsStatus = 0;
  this->MessagesStatus = 0;
  this->ActiveTransformsStatus = 0;
}


vtkMRMLTransformBufferNode
::~vtkMRMLTransformBufferNode()
{
  this->Clear();
}



// Transforms and Messages ------------------------------------------------------------------------


void vtkMRMLTransformBufferNode
::Concatenate( vtkMRMLTransformBufferNode* catBuffer )
{
  // If you want things to be deep copied, just deep copy the buffer
  for ( int i = 0; i < catBuffer->GetNumTransforms(); i++ )
  {
    this->AddTransform( catBuffer->GetTransformAt( i ) );
  }
  for ( int i = 0; i < catBuffer->GetNumMessages(); i++ )
  {
    this->AddMessage( catBuffer->GetMessageAt( i ) );
  }
}


void vtkMRMLTransformBufferNode
::AddTransform( vtkTransformRecord* newTransform )
{
  // Ensure that we put it into sorted order (usually it will go at the end)
  if ( this->GetNumTransforms() < 1 )
  {
    this->transforms.push_back( newTransform );
    this->TransformsStatus++;
	return;
  }
  if ( newTransform->GetTime() >= this->GetCurrentTransform()->GetTime() )
  {
    this->transforms.push_back( newTransform );
    this->TransformsStatus++;
	return;
  }
  if ( newTransform->GetTime() <= this->GetTransformAt(0)->GetTime() )
  {
    this->transforms.insert( transforms.begin() + 0, newTransform );
    this->TransformsStatus++;
	return;
  }

  // Use the binary search
  int insertLocation = this->GetPriorTransformIndex( newTransform->GetTime() ) + 1;
  this->transforms.insert( this->transforms.begin() + insertLocation, newTransform );

}


void vtkMRMLTransformBufferNode
::AddMessage( vtkMessageRecord* newMessage )
{
  // Ensure that we put it into sorted order (usually it will go at the end)
  if ( this->GetNumMessages() < 1 )
  {
    this->messages.push_back( newMessage );
    this->MessagesStatus++;
	return;
  }
  if ( newMessage->GetTime() >= this->GetCurrentMessage()->GetTime() )
  {
    this->messages.push_back( newMessage );
    this->MessagesStatus++;
	return;
  }
  if ( newMessage->GetTime() <= this->GetMessageAt(0)->GetTime() )
  {
    this->messages.insert( messages.begin() + 0, newMessage );
    this->MessagesStatus++;
	return;
  }

  // Use the binary search
  int insertLocation = this->GetPriorMessageIndex( newMessage->GetTime() ) + 1;
  this->messages.insert( this->messages.begin() + insertLocation, newMessage );

}


void vtkMRMLTransformBufferNode
::RemoveTransformAt( int index )
{
  if ( index >= 0 && index < this->GetNumTransforms() )
  {
    this->GetTransformAt(index)->Delete();
    this->transforms.erase( transforms.begin() + index );
  }
  this->TransformsStatus++;
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

  this->TransformsStatus++;
}


void vtkMRMLTransformBufferNode
::RemoveMessageAt( int index )
{
  if ( index >= 0 && index < this->GetNumMessages() )
  {
    this->GetMessageAt(index)->Delete();
    this->messages.erase( messages.begin() + index );
  }
  this->MessagesStatus++;
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

  this->MessagesStatus++;
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
  if ( time <= this->GetTransformAt(0)->GetTime() )
  {
    return this->GetTransformAt( 0 );
  }
  if ( time >= this->GetCurrentTransform()->GetTime() )
  {
    return this->GetCurrentTransform();
  }

  // Binary search since the records are sorted
  return this->GetTransformAt( this->GetClosestTransformIndex( time ) );

}


vtkMessageRecord* vtkMRMLTransformBufferNode
::GetMessageAtTime( double time )
{
  if ( this->GetNumMessages() == 0 )
  {
    return NULL;
  }
  if ( time <= this->GetMessageAt(0)->GetTime() )
  {
    return this->GetMessageAt( 0 );
  }
  if ( time >= this->GetCurrentMessage()->GetTime() )
  {
    return this->GetCurrentMessage();
  }

  // Binary search since the records are sorted
  return this->GetMessageAt( this->GetClosestMessageIndex( time ) );

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


// Report the maximum transform time - use only the message time if no transforms exist
double vtkMRMLTransformBufferNode
::GetMaximumTime()
{
  if ( this->GetNumTransforms() > 0 )
  {
    return this->GetCurrentTransform()->GetTime();
  }

  if ( this->GetNumMessages() > 0 )
  {
    return this->GetCurrentMessage()->GetTime();
  }

  return 0.0;
}


// Report the minimum transform time - use only the message time if no transforms exist
double vtkMRMLTransformBufferNode
::GetMinimumTime()
{
  if ( this->GetNumTransforms() > 0 )
  {
    return this->GetTransformAt(0)->GetTime();
  }

  if ( this->GetNumMessages() > 0 )
  {
    return this->GetMessageAt(0)->GetTime();
  }

  return 0.0;
}


double vtkMRMLTransformBufferNode
::GetTotalTime()
{
  return this->GetMaximumTime() - this->GetMinimumTime();
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
  this->TransformsStatus++;
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
  this->MessagesStatus++;
}


// Implement binary searches to find transforms and/or messages at a particular time
int vtkMRMLTransformBufferNode
::GetPriorTransformIndex( double time )
{
  int lowerBound = 0;
  int upperBound = this->GetNumTransforms() - 1;

  if ( time < this->GetTransformAt( lowerBound )->GetTime() )
  {
    return lowerBound;
  }
  if ( time > this->GetTransformAt( upperBound )->GetTime() )
  {
    return upperBound;
  }

  while ( upperBound - lowerBound > 1 )
  {
    // Note that if middle is equal to either lowerBound or upperBound then upperBound - lowerBound <= 1
    int middle = int( ( lowerBound + upperBound ) / 2 );
    double middleTime = this->GetTransformAt( middle )->GetTime();

    if ( time == middleTime )
    {
      return middle;
    }

    if ( time > middleTime )
    {
      lowerBound = middle;
    }

    if ( time < middleTime )
    {
      upperBound = middle;
    }

  }

  // Since we're returning the prior index, always return the lower bound
  return lowerBound;
}


int vtkMRMLTransformBufferNode
::GetClosestTransformIndex( double time )
{
  int lowerIndex = this->GetPriorTransformIndex( time );
  int upperIndex = lowerIndex + 1;

  if ( upperIndex >= this->GetNumTransforms() || std::abs ( time - this->GetTransformAt( lowerIndex )->GetTime() ) < std::abs ( time - this->GetTransformAt( upperIndex )->GetTime() ) )
  {
    return lowerIndex;
  }
  else
  {
    return upperIndex;
  }
}


// Implement binary searches to find transforms and/or messages at a particular time
int vtkMRMLTransformBufferNode
::GetPriorMessageIndex( double time )
{
  int lowerBound = 0;
  int upperBound = this->GetNumMessages() - 1;

  if ( time < this->GetMessageAt( lowerBound )->GetTime() )
  {
    return lowerBound;
  }
  if ( time > this->GetMessageAt( upperBound )->GetTime() )
  {
    return upperBound;
  }

  while ( upperBound - lowerBound > 1 )
  {
    // Note that if middle is equal to either lowerBound or upperBound then upperBound - lowerBound <= 1
    int middle = int( ( lowerBound + upperBound ) / 2 );
    double middleTime = this->GetMessageAt( middle )->GetTime();

    if ( time == middleTime )
    {
      return middle;
    }

    if ( time > middleTime )
    {
      lowerBound = middle;
    }

    if ( time < middleTime )
    {
      upperBound = middle;
    }

  }

  // Since we're returning the prior index, always return the lower bound
  return lowerBound;
}


int vtkMRMLTransformBufferNode
::GetClosestMessageIndex( double time )
{
  int lowerIndex = this->GetPriorMessageIndex( time );
  int upperIndex = lowerIndex + 1;

  if ( upperIndex >= this->GetNumMessages() || abs ( time - this->GetMessageAt( lowerIndex )->GetTime() ) < abs ( time - this->GetMessageAt( upperIndex )->GetTime() ) )
  {
    return lowerIndex;
  }
  else
  {
    return upperIndex;
  }
}



void vtkMRMLTransformBufferNode
::AddActiveTransform( std::string name )
{
  // Do not add if the name is already in the list
  for ( int i = 0; i < this->activeTransforms.size(); i++ )
  {
    if ( this->activeTransforms.at(i).compare( name ) == 0 )
	{
      return;
	}
  }

  this->activeTransforms.push_back( name );
  this->ActiveTransformsStatus++;
}


void vtkMRMLTransformBufferNode
::RemoveActiveTransform( std::string name )
{
  for ( int i = 0; i < this->activeTransforms.size(); i++ )
  {
    if ( this->activeTransforms.at(i).compare( name ) == 0 )
	{
	  this->activeTransforms.erase( this->activeTransforms.begin() + i );
	  i--;
      this->ActiveTransformsStatus++;
	}
  }

}


std::vector<std::string> vtkMRMLTransformBufferNode
::GetActiveTransforms()
{
  return this->activeTransforms;
}
  

void vtkMRMLTransformBufferNode
::SetActiveTransforms( std::vector<std::string> names )
{
  this->activeTransforms.clear();
  this->activeTransforms = names;
  this->ActiveTransformsStatus++;
}


void vtkMRMLTransformBufferNode
::SetActiveTransformsFromBuffer()
{
  this->activeTransforms.clear();

  for ( int i = 0; i < this->GetNumTransforms(); i++ )
  {
    bool activeTransformExists = false;

    for ( int j = 0; j < this->activeTransforms.size(); j++ )
    {
      if ( this->activeTransforms.at(j).compare( this->GetTransformAt(i)->GetDeviceName() ) == 0 )
      {
        activeTransformExists = true;
      }
    }

    // If we didn't find a mathcing active transform then add one
    if ( ! activeTransformExists )
    {
      this->AddActiveTransform( this->GetTransformAt(i)->GetDeviceName() );
    }

  }

  this->ActiveTransformsStatus++;
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

  // Status updates are taken care of in the add functions
}
