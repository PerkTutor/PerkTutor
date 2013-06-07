
#include "vtkWorkflowInput.h"

vtkStandardNewMacro( vtkWorkflowInput );


vtkWorkflowInput
::vtkWorkflowInput()
{
  // Give default values, don't need to initialize vectors
  this->FilterWidth = 0.0;
  this->OrthogonalOrder = 0;
  this->OrthogonalWindow = 0;
  this->Derivative = 0;
  this->NumCentroids = 0;
  this->NumPrinComps = 0;
  this->MarkovPseudoScalePi = 0.0;
  this->MarkovPseudoScaleA = 0.0;
  this->MarkovPseudoScaleB = 0.0;
  this->CompletionTime = 0.0;
  this->Equalization = 0.0;
}


vtkWorkflowInput
::~vtkWorkflowInput()
{
  this->FilterWidth = 0.0;
  this->OrthogonalOrder = 0;
  this->OrthogonalWindow = 0;
  this->Derivative = 0;
  this->NumCentroids = 0;
  this->NumPrinComps = 0;
  this->MarkovPseudoScalePi = 0.0;
  this->MarkovPseudoScaleA = 0.0;
  this->MarkovPseudoScaleB = 0.0;
  this->CompletionTime = 0.0;
  this->Equalization = 0.0;
}


vtkWorkflowInput* vtkWorkflowInput
::DeepCopy()
{
  vtkWorkflowInput* newWorkflowInput = vtkWorkflowInput::New();
  newWorkflowInput->FilterWidth = this->FilterWidth;
  newWorkflowInput->OrthogonalOrder = this->OrthogonalOrder;
  newWorkflowInput->OrthogonalWindow = this->OrthogonalWindow;
  newWorkflowInput->Derivative = this->Derivative;
  newWorkflowInput->NumCentroids = this->NumCentroids;
  newWorkflowInput->NumPrinComps = this->NumPrinComps;
  newWorkflowInput->MarkovPseudoScalePi = this->MarkovPseudoScalePi;
  newWorkflowInput->MarkovPseudoScaleA = this->MarkovPseudoScaleA;
  newWorkflowInput->MarkovPseudoScaleB = this->MarkovPseudoScaleB;
  newWorkflowInput->CompletionTime = this->CompletionTime;
  newWorkflowInput->Equalization = this->Equalization;
  return newWorkflowInput;
}


std::string vtkWorkflowInput
::ToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "<Parameter Type=\"FilterWidth\" Value=\"" << this->FilterWidth << "\" />" << std::endl;
  xmlstring << "<Parameter Type=\"OrthogonalOrder\" Value=\"" << this->OrthogonalOrder << "\" />" << std::endl;
  xmlstring << "<Parameter Type=\"OrthogonalWindow\" Value=\"" << this->OrthogonalWindow << "\" />" << std::endl;
  xmlstring << "<Parameter Type=\"Derivative\" Value=\"" << this->Derivative << "\" />" << std::endl;
  xmlstring << "<Parameter Type=\"NumCentroids\" Value=\"" << this->NumCentroids << "\" />" << std::endl;
  xmlstring << "<Parameter Type=\"NumPrinComps\" Value=\"" << this->NumPrinComps << "\" />" << std::endl;
  xmlstring << "<Parameter Type=\"MarkovPseudoScalePi\" Value=\"" << this->MarkovPseudoScalePi << "\" />" << std::endl;
  xmlstring << "<Parameter Type=\"MarkovPseudoScaleA\" Value=\"" << this->MarkovPseudoScaleA << "\" />" << std::endl;
  xmlstring << "<Parameter Type=\"MarkovPseudoScaleB\" Value=\"" << this->MarkovPseudoScaleB << "\" />" << std::endl;
  xmlstring << "<Parameter Type=\"CompletionTime\" Value=\"" << this->CompletionTime << "\" />" << std::endl;
  xmlstring << "<Parameter Type=\"Equalization\" Value=\"" << this->Equalization << "\" />" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowInput
::FromXMLElement( vtkXMLDataElement* element )
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
	
	std::stringstream ss( std::string( noteElement->GetAttribute( "Value" ) ) );
	double value;
	ss >> value;

	if ( strcmp( elementType, "Derivative" ) == 0 )
    {
	  this->Derivative = value;
    }
	if ( strcmp( elementType, "FilterWidth" ) == 0 )
    {
	  this->FilterWidth = value;
    }
	if ( strcmp( elementType, "OrthogonalOrder" ) == 0 )
    {
	  this->OrthogonalOrder = value;
    }
	if ( strcmp( elementType, "OrthogonalWindow" ) == 0 )
    {
	  this->OrthogonalWindow = value;
    }
	if ( strcmp( elementType, "NumPrinComps" ) == 0 )
    {
	  this->NumPrinComps = value;
    }
    if ( strcmp( elementType, "NumCentroids" ) == 0 )
    {
	  this->NumCentroids = value;
    }
	if ( strcmp( elementType, "MarkovPseudoScalePi" ) == 0 )
    {
	  this->MarkovPseudoScalePi = value;
    }
	if ( strcmp( elementType, "MarkovPseudoScaleA" ) == 0 )
    {
	  this->MarkovPseudoScaleA = value;
    }
	if ( strcmp( elementType, "MarkovPseudoScaleB" ) == 0 )
    {
	  this->MarkovPseudoScaleB = value;
    }
	if ( strcmp( elementType, "CompletionTime" ) == 0 )
    {
	  this->CompletionTime = value;
    }
    if ( strcmp( elementType, "Equalization" ) == 0 )
    {
	  this->Equalization = value;
    }

  }

}