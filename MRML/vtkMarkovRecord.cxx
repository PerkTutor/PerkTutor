
#include "vtkMarkovRecord.h"

vtkStandardNewMacro( vtkMarkovRecord );


vtkMarkovRecord
::vtkMarkovRecord()
{
  this->State = "";
  this->Symbol = "";
}


vtkMarkovRecord
::~vtkMarkovRecord()
{
}


vtkMarkovRecord* vtkMarkovRecord
::DeepCopy()
{
  vtkMarkovRecord* newMarkovRecord = vtkMarkovRecord::New();
  newMarkovRecord->SetState( this->GetState() );
  newMarkovRecord->SetSymbol( this->GetSymbol() );
  return newMarkovRecord;
}


std::string vtkMarkovRecord
::GetState()
{
  return this->State;
}


void vtkMarkovRecord
::SetState( std::string newState )
{
  this->State = newState;
}

void vtkMarkovRecord
::SetState( int newState )
{
  std::stringstream labelstring;
  labelstring << newState;
  this->State = labelstring.str();
}


std::string vtkMarkovRecord
::GetSymbol()
{
  return this->Symbol;
}


void vtkMarkovRecord
::SetSymbol( std::string newSymbol )
{
  this->Symbol = newSymbol;
}

void vtkMarkovRecord
::SetSymbol( int newSymbol )
{
  std::stringstream labelstring;
  labelstring << newSymbol;
  this->Symbol = labelstring.str();
}


std::string vtkMarkovRecord
::ToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "  <log";
  xmlstring << " State=\"" << this->GetState() << "\"";
  xmlstring << " Symbol=\"" << this->GetSymbol() << "\"";
  xmlstring << " />" << std::endl;

  return xmlstring.str();
}


void vtkMarkovRecord
::FromXMLElement( vtkXMLDataElement* element )
{

  if ( strcmp( element->GetName(), "log" ) != 0 )
  {
    return;  // If it's not a "log" or is the wrong tool jump to the next.
  }

  this->SetState( std::string( element->GetAttribute( "State" ) ) );
  this->SetSymbol( std::string( element->GetAttribute( "Symbol" ) ) );
}