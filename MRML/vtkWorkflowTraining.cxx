
#include "vtkWorkflowTraining.h"

vtkStandardNewMacro( vtkWorkflowTraining );


//------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------


std::string
VectorBufferToXMLString( std::string name, std::vector<vtkLabelVector*> vectors )
{
  std::stringstream xmlstring;

  xmlstring << "    <Parameter Type=\"" << name << "\" />" << std::endl;
  for ( int i = 0; i < vectors.size(); i++ )
  {
    xmlstring << vectors.at(i)->ToXMLString( name );
  }
  xmlstring << "    </Parameter>" << std::endl;

  return xmlstring.str();
}


std::string
VectorBufferToXMLString( std::string name, vtkLabelVector* vector )
{
  std::vector<vtkLabelVector*> vectors;
  vectors.push_back( vector );
  return VectorBufferToXMLString( name, vectors );
}


std::vector<vtkLabelVector*>
VectorBufferFromXMLElement( std::string name, vtkXMLDataElement* element )
{
  // Initialize the vector of LabelRecords to improve speed
  std::vector<vtkLabelVector*> vectors;

  if ( name.compare( element->GetAttribute( "Type" ) ) != 0 )
  {
    return vectors;
  }

  int numElements = element->GetNumberOfNestedElements();

  for ( int i = 0; i < numElements; i++ )
  {
    vtkLabelVector* currentVector = vtkLabelVector::New();

    vtkXMLDataElement* noteElement = element->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), name.c_str() ) != 0 )
    {
      continue;  // If it's not a "Parameter", jump to the next.
    }

	currentVector->FromString( std::string( noteElement->GetAttribute( "Value" ) ), atoi( noteElement->GetAttribute( "Size" ) ) );
	currentVector->SetLabel( std::string( noteElement->GetAttribute( "Label" ) ) );

	vectors.push_back( currentVector );
  }

  return vectors;
}



// Object Functions --------------------------------------------------------------------------

vtkWorkflowTraining
::vtkWorkflowTraining()
{
  // Give default values, don't need to initialize vectors
  this->Mean = vtkLabelVector::New();
  this->MarkovPi = vtkLabelVector::New();
}


vtkWorkflowTraining
::~vtkWorkflowTraining()
{
  this->PrinComps.clear();
  this->Mean = blankRecord;
  this->Centroids.clear();
  this->MarkovPi = blankRecord;
  this->MarkovA.clear();
  this->MarkovB.clear();
}


vtkWorkflowTraining* vtkWorkflowTraining
::DeepCopy()
{
  vtkWorkflowTraining* newWorkflowTraining = vtkWorkflowTraining::New();
  for ( int i = 0; i < this->PrinComps.size(); i++ )
  {
    newWorkflowTraining->PrinComps.push_back( this->PrinComps.at(i)->DeepCopy() );
  }
  newWorkflowTraining->Mean = this->Mean->DeepCopy();
  for ( int i = 0; i < this->Centroids.size(); i++ )
  {
    newWorkflowTraining->Centroids.push_back( this->Centroids.at(i)->DeepCopy() );
  }
  newWorkflowTraining->MarkovPi = this->MarkovPi->DeepCopy();
  for ( int i = 0; i < this->MarkovA.size(); i++ )
  {
    newWorkflowTraining->MarkovA.push_back( this->MarkovA.at(i)->DeepCopy() );
  }
  for ( int i = 0; i < this->MarkovB.size(); i++ )
  {
    newWorkflowTraining->MarkovB.push_back( this->MarkovB.at(i)->DeepCopy() );
  }
  return newWorkflowTraining;
}


std::string vtkWorkflowTraining
::ToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << VectorBufferToXMLString( "PrinComps", this->PrinComps );
  xmlstring << VectorBufferToXMLString( "Mean", this->Mean );
  xmlstring << VectorBufferToXMLString( "Centroids", this->Centroids );
  xmlstring << VectorBufferToXMLString( "MarkovPi", this->MarkovPi );
  xmlstring << VectorBufferToXMLString( "MarkovA", this->MarkovA );
  xmlstring << VectorBufferToXMLString( "MarkovB", this->MarkovB );

  return xmlstring.str();

}


void vtkWorkflowTraining
::FromXMLElement( vtkXMLDataElement* element, vtkWorkflowProcedure* procedure, vtkWorkflowInput* input )
{
  int numElements = element->GetNumberOfNestedElements();

  for ( int i = 0; i < numElements; i++ )
  {

    vtkXMLDataElement* noteElement = element->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "Parameter" ) != 0 )
    {
      continue;  // If it's not a "Parameter", jump to the next.
    }

	const char* elementType = noteElement->GetAttribute( "Type" );

	if ( strcmp( elementType, "PrinComps" ) == 0 )
    {
	  this->PrinComps = VectorBufferFromXMLElement( "PrinComps", noteElement );
    }
	if ( strcmp( elementType, "Mean" ) == 0 )
    {
	  this->Mean = VectorBufferFromXMLElement( "Mean", noteElement ).at(0);
    }
	if ( strcmp( elementType, "Centroids" ) == 0 )
    {
	  this->Centroids = VectorBufferFromXMLElement( "Centroids", noteElement );
    }
	if ( strcmp( elementType, "MarkovPi" ) == 0 )
    {
	  this->MarkovPi = VectorBufferFromXMLElement( "MarkovPi", noteElement ).at(0);
    }
	if ( strcmp( elementType, "MarkovA" ) == 0 )
    {
	  this->MarkovA = VectorBufferFromXMLElement( "MarkovA", noteElement );
    }
    if ( strcmp( elementType, "MarkovB" ) == 0 )
    {
	  this->MarkovB = VectorBufferFromXMLElement( "MarkovB", noteElement );
    }

  }

}
