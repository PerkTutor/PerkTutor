
#include "vtkMarkovVector.h"

vtkStandardNewMacro( vtkMarkovVector );


vtkMarkovVector
::vtkMarkovVector()
{
  this->State = "";
  this->Symbol = "";
}


vtkMarkovVector
::~vtkMarkovVector()
{
}


void vtkMarkovVector
::Copy( vtkMarkovVector* otherVector )
{
  this->SetState( otherVector->GetState() );
  this->SetSymbol( otherVector->GetSymbol() );
}


std::string vtkMarkovVector
::GetState()
{
  return this->State;
}


void vtkMarkovVector
::SetState( std::string newState )
{
  this->State = newState;
}

void vtkMarkovVector
::SetState( int newState )
{
  std::stringstream labelstring;
  labelstring << newState;
  this->State = labelstring.str();
}


std::string vtkMarkovVector
::GetSymbol()
{
  return this->Symbol;
}


void vtkMarkovVector
::SetSymbol( std::string newSymbol )
{
  this->Symbol = newSymbol;
}

void vtkMarkovVector
::SetSymbol( int newSymbol )
{
  std::stringstream labelstring;
  labelstring << newSymbol;
  this->Symbol = labelstring.str();
}


std::string vtkMarkovVector
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;

  xmlstring << indent << "<MarkovVector";
  xmlstring << " State=\"" << this->GetState() << "\"";
  xmlstring << " Symbol=\"" << this->GetSymbol() << "\"";
  xmlstring << " />" << std::endl;

  return xmlstring.str();
}


void vtkMarkovVector
::FromXMLElement( vtkXMLDataElement* element )
{

  if ( strcmp( element->GetName(), "MarkovVector" ) != 0 )
  {
    return;  // If it's not a "MarkovVector" jump to the next.
  }

  this->SetState( std::string( element->GetAttribute( "State" ) ) );
  this->SetSymbol( std::string( element->GetAttribute( "Symbol" ) ) );
}