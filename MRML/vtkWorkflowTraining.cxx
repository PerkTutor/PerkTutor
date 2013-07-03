
#include "vtkWorkflowTraining.h"

vtkStandardNewMacro( vtkWorkflowTraining );


//------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------


std::string
VectorBufferToXMLString( std::string name, std::vector<vtkLabelVector*> vectors )
{
  std::stringstream xmlstring;

  xmlstring << "    <Parameter Type=\"" << name << "\" >" << std::endl;
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

	currentVector->FromString( std::string( noteElement->GetAttribute( "Values" ) ), atoi( noteElement->GetAttribute( "Size" ) ) );
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
  this->Markov = vtkMarkovModelRT::New();
}


vtkWorkflowTraining
::~vtkWorkflowTraining()
{
  vtkDeleteVector( this->PrinComps );
  vtkDeleteVector( this->Centroids );
  vtkDelete( this->Markov );
  vtkDelete( this->Mean );
}


vtkWorkflowTraining* vtkWorkflowTraining
::DeepCopy()
{
  vtkWorkflowTraining* newWorkflowTraining = vtkWorkflowTraining::New();
  newWorkflowTraining->PrinComps = vtkDeepCopyVector( this->PrinComps );
  newWorkflowTraining->Centroids = vtkDeepCopyVector( this->Centroids );
  newWorkflowTraining->Mean = vtkDeleteAssign( newWorkflowTraining->Mean, this->Mean->DeepCopy() );
  newWorkflowTraining->Markov = vtkDeleteAssign( newWorkflowTraining->Markov, this->Markov->DeepCopy() );
  return newWorkflowTraining;
}


std::string vtkWorkflowTraining
::ToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << VectorBufferToXMLString( "PrinComps", this->PrinComps );
  xmlstring << VectorBufferToXMLString( "Mean", this->Mean );
  xmlstring << VectorBufferToXMLString( "Centroids", this->Centroids );
  xmlstring << this->Markov->ToXMLString();

  return xmlstring.str();

}


void vtkWorkflowTraining
::FromXMLElement( vtkXMLDataElement* element )
{
  int numElements = element->GetNumberOfNestedElements();

  for ( int i = 0; i < numElements; i++ )
  {

    vtkXMLDataElement* noteElement = element->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "Parameter" ) != 0 && strcmp( noteElement->GetName(), "MarkovModel" ) != 0 )
    {
      continue;  // If it's not a "Parameter", jump to the next.
    }

	const char* elementType = noteElement->GetAttribute( "Type" );

	if ( strcmp( elementType, "PrinComps" ) == 0 )
    {
	  vtkDeleteVector( this->PrinComps );
	  this->PrinComps = VectorBufferFromXMLElement( "PrinComps", noteElement );
    }
	if ( strcmp( elementType, "Mean" ) == 0 )
    {
	  this->Mean->Delete();
	  this->Mean = VectorBufferFromXMLElement( "Mean", noteElement ).at(0);
    }
	if ( strcmp( elementType, "Centroids" ) == 0 )
    {
	  vtkDeleteVector( this->Centroids );
	  this->Centroids = VectorBufferFromXMLElement( "Centroids", noteElement );
    }
	if ( strcmp( elementType, "Markov" ) == 0 )
    {
	  this->Markov->FromXMLElement( noteElement );
	}

  }

}
