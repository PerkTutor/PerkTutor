
#include "vtkWorkflowTraining.h"

vtkStandardNewMacro( vtkWorkflowTraining );


//------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------


std::string
LabelRecordVectorToXMLString( std::string name, std::vector<vtkLabelVector*> records )
{
  std::stringstream xmlstring;

  xmlstring << "<Parameter Type=\"" << name << "\" />" << std::endl;
  for ( int i = 0; i < records.size(); i++ )
  {
    xmlstring << records.at(i)->ToXMLString( name );
  }
  xmlstring << "</Parameter>" << std::endl;

  return xmlstring.str();

}


std::vector<vtkLabelVector*>
LabelRecordVectorFromXMLElement( std::string name, vtkXMLDataElement* element, int numRecords, int recordSize )
{

  int numElements = element->GetNumberOfNestedElements();

  // Initialize the vector of LabelRecords to improve speed
  vtkLabelVector* blankRecord;
  blankRecord->Initialize( recordSize, 0.0 );
  std::vector<vtkLabelVector*> recordVector( numElements, blankRecord );

  for ( int i = 0; i < numElements; i++ )
  {

    vtkXMLDataElement* noteElement = element->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), name.c_str() ) != 0 )
    {
      continue;  // If it's not a "Parameter", jump to the next.
    }

	recordVector.at(i)->SetLabel( std::string( noteElement->GetAttribute( "Label" ) ) );
    recordVector.at(i)->FromString( std::string( noteElement->GetAttribute( "Value" ) ), recordSize );

  }

  return recordVector;
}



std::string
LabelRecordToXMLString( std::string name, vtkLabelVector* record )
{
  std::vector<vtkLabelVector*> records;
  records.push_back( record );
  return LabelRecordVectorToXMLString( name, records );
}



vtkLabelVector*
LabelRecordFromXMLElement( std::string name, vtkXMLDataElement* element, int recordSize )
{
  return LabelRecordVectorFromXMLElement( name, element, 1, recordSize ).at(0);
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
  vtkLabelVector* blankRecord;
  blankRecord->Initialize( 0, 0.0 );
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

  xmlstring << LabelRecordVectorToXMLString( "PrinComps", this->PrinComps );
  xmlstring << LabelRecordToXMLString( "Mean", this->Mean );
  xmlstring << LabelRecordVectorToXMLString( "Centroids", this->Centroids );
  xmlstring << LabelRecordToXMLString( "MarkovPi", this->MarkovPi );
  xmlstring << LabelRecordVectorToXMLString( "MarkovA", this->MarkovA );
  xmlstring << LabelRecordVectorToXMLString( "MarkovB", this->MarkovB );

  return xmlstring.str();

}


void vtkWorkflowTraining
::FromXMLElement( vtkXMLDataElement* element, vtkWorkflowProcedure* procedure, vtkWorkflowInput* input )
{
  int numElements = element->GetNumberOfNestedElements();
  int SizePrinComps = ( input->OrthogonalOrder + 1 ) * ( vtkTrackingRecord::TRACKINGRECORD_SIZE ) * ( input->Derivative );

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
	  this->PrinComps = LabelRecordVectorFromXMLElement( "PrinComps", noteElement, input->NumPrinComps, SizePrinComps );
    }
	if ( strcmp( elementType, "Mean" ) == 0 )
    {
	  this->Mean = LabelRecordFromXMLElement( "Mean", noteElement, SizePrinComps );
    }
	if ( strcmp( elementType, "Centroids" ) == 0 )
    {
	  this->Centroids = LabelRecordVectorFromXMLElement( "Centroids", noteElement, input->NumCentroids, input->NumPrinComps );
    }
	if ( strcmp( elementType, "MarkovPi" ) == 0 )
    {
	  this->MarkovPi = LabelRecordFromXMLElement( "MarkovPi", noteElement, procedure->GetNumTasks() );
    }
	if ( strcmp( elementType, "MarkovA" ) == 0 )
    {
	  this->MarkovA = LabelRecordVectorFromXMLElement( "MarkovA", noteElement, procedure->GetNumTasks(), procedure->GetNumTasks() );
    }
    if ( strcmp( elementType, "MarkovB" ) == 0 )
    {
	  this->MarkovB = LabelRecordVectorFromXMLElement( "MarkovB", noteElement, procedure->GetNumTasks(), input->NumCentroids );
    }

  }

}
