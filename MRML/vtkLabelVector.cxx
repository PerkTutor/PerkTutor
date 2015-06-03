
#include "vtkLabelVector.h"

vtkStandardNewMacro( vtkLabelVector );


// Class methods -----------------------------------------------------------------------

vtkLabelVector
::vtkLabelVector()
{
  this->Label = "";
}


vtkLabelVector
::~vtkLabelVector()
{
  this->Values.clear();
}


void vtkLabelVector
::Copy( vtkLabelVector* otherVector )
{
  vtkLabelVector* newLabelVector = vtkLabelVector::New();
  
  this->SetLabel( otherVector->GetLabel() );
  
  this->FillElements( otherVector->Size(), 0.0 );
  for( int i = 0; i < otherVector->Size(); i++ )
  {
    this->SetElement( i, otherVector->GetElement( i ) );
  }
}


void vtkLabelVector
::AddElement( double newValue )
{
  this->Values.push_back( newValue );
}


void vtkLabelVector
::SetElement( int index, double newValue )
{
  this->Values.at( index ) = newValue;
}


void vtkLabelVector
::IncrementElement( int index, double step )
{
  this->Set( index, this->Get( index ) + step );
}


double vtkLabelVector
::GetElement( int index )
{
  return this->Values.at( index );
}


void vtkLabelVector
::FillElements( int size, double value )
{
  this->Values = std::vector< double >( size, value );
}


int vtkLabelVector
::Size()
{
  return this->Values.size();
}


void vtkLabelVector
::SetAllValues( std::vector<double> newValues )
{
  this->Values = newValues;
}


std::vector<double> vtkLabelVector
::GetAllValues()
{
  return this->Values;
}


void vtkLabelVector
::Concatenate( vtkLabelVector* otherVector )
{
  for ( int i = 0; i < otherVector->Size(); i++ )
  {
    this->AddElement( otherVector->GetElement( i );
  }
}


std::string vtkLabelVector
::ToString()
{
  std::stringstream outstring;
  for ( int i = 0; i < this->Size(); i++ )
  {
    outstring << this->GetElement( i ) << " ";
  }
  return outstring.str();
}


void vtkLabelVector
::FromString( std::string s, int size )
{
  std::stringstream instring( s );
  double value;
  for( int i = 0; i < size; i++ )
  {
    instring >> value;
	  this->AddElement( value );
  }
}


std::string vtkLabelVector
::GetLabel()
{
  return this->Label;
}


void vtkLabelVector
::SetLabel( std::string newLabel )
{
  this->Label = newLabel;
}


void vtkLabelVector
::SetLabel( int newLabel )
{
  std::stringstream labelstring;
  labelstring << newLabel;
  this->Label = labelstring.str();
}


std::string vtkLabelVector
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;

  xmlstring << indent << "<Vector" ;
  xmlstring << " Label=\"" << this->GetLabel() << "\"";
  xmlstring << " Size=\"" << this->Size() << "\"";
  xmlstring << " Values=\"" << this->ToString() << "\"";
  xmlstring << " />" << std::endl;

  return xmlstring.str();
}


void vtkLabelVector
::FromXMLElement( vtkXMLDataElement* element )
{
  if ( element == NULL || strcmp( element->GetName(), "Vector" ) != 0 )
  {
    return;  // If it's not a "log" or is the wrong tool jump to the next.
  }

  this->FromString( std::string( element->GetAttribute( "Values" ) ), atoi( element->GetAttribute( "Size" ) ) );
  this->SetLabel( std::string( element->GetAttribute( "Label" ) ) );
}


// Static helper functions -----------------------------------------------------------------------

std::string vtkLabelVector
::VectorsToXMLString( std::vector< vtkLabelVector* > vectors, std::string name, vtkIndent indent )
{
  std::stringstream xmlstring;

  xmlstring << indent << "<Vectors Type=\"" << name << "\" >" << std::endl;
  for ( int i = 0; i < vectors.size(); i++ )
  {
    xmlstring << vectors.at(i)->ToXMLString( indent.GetNextIndent() );
  }
  xmlstring << indent << "</Vectors>" << std::endl;

  return xmlstring.str();
}


std::string vtkLabelVector
::VectorsToXMLString( vtkLabelVector* vector, std::string name, vtkIndent indent )
{
  std::vector< vtkLabelVector* > vectors;
  vectors.push_back( vector );
  return vtkLabelVector::VectorsToXMLString( name, vectors );
}

std::vector< vtkLabelVector* > vtkLabelVector
::VectorsFromXMLElement( vtkXMLDataElement* element, std::string name )
{
  if ( element == NULL || name.compare( element->GetAttribute( "Type" ) ) != 0 )
  {
    return vectors;
  }
  
  // Initialize the vector of LabelRecords to improve speed
  int numElements = element->GetNumberOfNestedElements();
  std::vector< vtkLabelVector* > vectors( numElements, NULL );

  for ( int i = 0; i < numElements; i++ )
  {
    vtkSmartPointer< vtkLabelVector > currentVector = vtkSmartPointer< vtkLabelVector >::New();

    vtkXMLDataElement* noteElement = element->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), name.c_str() ) != 0 )
    {
      continue;  // If it's not a "Parameter", jump to the next.
    }

	  currentVector->FromString( std::string( noteElement->GetAttribute( "Values" ) ), atoi( noteElement->GetAttribute( "Size" ) ) );
	  currentVector->SetLabel( std::string( noteElement->GetAttribute( "Label" ) ) );

	  vectors.push_back( currentVector );
  }

  return vectors;
}