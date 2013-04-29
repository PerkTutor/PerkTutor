

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
  os << indent << "FileName: " << this->fileName << "\n";
}


void vtkMRMLTransformBufferNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);
  vtkIndent indent(nIndent);  
  of << indent << " FileName=\"" << this->fileName << "\"";
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
    
    if ( ! strcmp( attName, "FileName" ) )
    {
	  this->ReadFromFile( std::string( attValue ) );
    }
  }
}


void vtkMRMLTransformBufferNode
::Copy( vtkMRMLNode *anode )
{
  Superclass::Copy( anode );
  vtkMRMLTransformBufferNode *node = ( vtkMRMLTransformBufferNode* ) anode;

  this->Clear();
  this->fileName = node->fileName;

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
  this->fileName = "";
  // No need to initialize the vectors
}


vtkMRMLTransformBufferNode
::~vtkMRMLTransformBufferNode()
{
  this->fileName = "";
  this->transforms.clear(); // Clear function automatically deconstructs all objects in the vector
  this->messages.clear();
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
  if ( newTransform->GetTime() > this->GetCurrentTransform()->GetTime() )
  {
    this->transforms.push_back( newTransform );
	return;
  }
  if ( newTransform->GetTime() < this->GetTransformAt(0)->GetTime() )
  {
    this->transforms.insert( transforms.begin() + 0, newTransform );
	return;
  }

  // TODO: Use binary search
  for ( int i = this->GetNumTransforms() - 1; i >= 0; i-- )
  {
    if ( newTransform->GetTime() > this->GetTransformAt(i)->GetTime() )
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
  if ( newMessage->GetTime() > this->GetCurrentTransform()->GetTime() )
  {
    this->messages.push_back( newMessage );
	return;
  }
  if ( newMessage->GetTime() < this->GetMessageAt(0)->GetTime() )
  {
    this->messages.insert( messages.begin() + 0, newMessage );
	return;
  }

  // TODO: Use binary search
  for ( int i = this->GetNumMessages() - 1; i >= 0; i-- )
  {
    if ( newMessage->GetTime() > this->GetMessageAt(i)->GetTime() )
	{
      this->messages.insert( messages.begin() + i + 1, newMessage );
	  return;
	}
  }

}


void vtkMRMLTransformBufferNode
::RemoveTransformAt( int index )
{
  this->transforms.erase( transforms.begin() + index );
}


void vtkMRMLTransformBufferNode
::RemoveMessageAt( int index )
{
  this->messages.erase( messages.begin() + index );
}


vtkTransformRecord* vtkMRMLTransformBufferNode
::GetTransformAt( int index )
{
  return this->transforms.at( index );
}


vtkTransformRecord* vtkMRMLTransformBufferNode
::GetCurrentTransform()
{
  return this->transforms.at( this->GetNumTransforms() );
}


vtkMessageRecord* vtkMRMLTransformBufferNode
::GetMessageAt( int index )
{
  return this->messages.at( index );
}


vtkMessageRecord* vtkMRMLTransformBufferNode
::GetCurrentMessage()
{
  return this->messages.at( this->GetNumMessages() );
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

  // TODO: Use binary search
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

  // TODO: Use binary search
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
  // Note that the clear function calls the destructor for all of the objects in the vector
  this->transforms.clear();
  this->messages.clear();
}


void vtkMRMLTransformBufferNode
::WriteToFile( std::string fileName )
{
  this->fileName = fileName;

  std::ofstream output( this->fileName.c_str() );
  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Record file could not be opened!" );
    return;
  }
  
  output << "<TransformRecorderLog>" << std::endl;

  for ( int i = 0; i < this->GetNumTransforms(); i++ )
  {
    output << this->GetTransformAt(i)->ToXMLString();
  }

  for ( int i = 0; i < this->GetNumMessages(); i++ )
  {
    output << this->GetMessageAt(i)->ToXMLString();
  }

  output << "</TransformRecorderLog>" << std::endl;
  output.close();

}


void vtkMRMLTransformBufferNode
::ReadFromFile( std::string fileName )
{
  this->fileName = fileName;

  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( fileName.c_str() );
  parser->Parse();
  
  vtkXMLDataElement* rootElement = parser->GetRootElement();
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
