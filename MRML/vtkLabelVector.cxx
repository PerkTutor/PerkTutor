
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


vtkLabelVector* vtkLabelVector
::DeepCopy()
{
  vtkLabelVector* newLabelVector = vtkLabelVector::New();
  newLabelVector->SetValues( this->GetValues() ); // Observe that this does a deep copy
  newLabelVector->SetLabel( this->GetLabel() );
  return newLabelVector;
}


void vtkLabelVector
::Initialize( int size, double value )
{
  this->Values = std::vector<double>( size, value );
}


void vtkLabelVector
::Add( double newValue )
{
  this->Values.push_back( newValue );
}


void vtkLabelVector
::Set( int index, double newValue )
{
  this->Values.at(index) = newValue;
}


void vtkLabelVector
::Crement( int index, double step )
{
  this->Set( index, this->Get(index) + step );
}


double vtkLabelVector
::Get( int index )
{
  return this->Values.at(index);
}


int vtkLabelVector
::Size()
{
  return this->Values.size();
}


void vtkLabelVector
::SetValues( std::vector<double> newValues )
{
  this->Values = newValues;
}


std::vector<double> vtkLabelVector
::GetValues()
{
  return this->Values;
}


std::string vtkLabelVector
::ToString()
{
  std::stringstream outstring;
  for ( int i = 0; i < this->Size(); i++ )
  {
    outstring << this->Get(i) << " ";
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
	this->Add( value );
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
::ToXMLString( std::string name )
{
  std::stringstream xmlstring;

  xmlstring << "      <" << name;
  xmlstring << " Label=\"" << this->GetLabel() << "\"";
  xmlstring << " Size=\"" << this->Size() << "\"";
  xmlstring << " Values=\"" << this->ToString() << "\"";
  xmlstring << " />" << std::endl;

  return xmlstring.str();
}


void vtkLabelVector
::FromXMLElement( vtkXMLDataElement* element, std::string name )
{

  if ( strcmp( element->GetName(), name.c_str() ) != 0 )
  {
    return;  // If it's not a "log" or is the wrong tool jump to the next.
  }

  this->FromString( std::string( element->GetAttribute( "Values" ) ), atoi( element->GetAttribute( "Size" ) ) );
  this->SetLabel( std::string( element->GetAttribute( "Label" ) ) );
}