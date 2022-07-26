
#include "vtkMRMLWorkflowInputNode.h"

// Standard MRML Node Methods ------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLWorkflowInputNode);

void vtkMRMLWorkflowInputNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLWorkflowInputNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);
}


void vtkMRMLWorkflowInputNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);
}


void vtkMRMLWorkflowInputNode
::Copy( vtkMRMLNode *anode )
{
  Superclass::Copy( anode );
  vtkMRMLWorkflowInputNode *node = ( vtkMRMLWorkflowInputNode* ) anode;
  if ( node == NULL )
  {
    return;
  }

  int startModifyState = this->StartModify();
  
  this->SetFilterWidth( node->GetFilterWidth() );
  this->SetOrthogonalOrder( node->GetOrthogonalOrder() );
  this->SetOrthogonalWindow( node->GetOrthogonalWindow() );
  this->SetDerivative( node->GetDerivative() );
  this->SetNumCentroids( node->GetNumCentroids() );
  this->SetNumPrinComps( node->GetNumPrinComps() );
  this->SetMarkovPseudoScalePi( node->GetMarkovPseudoScalePi() );
  this->SetMarkovPseudoScaleA( node->GetMarkovPseudoScaleA() );
  this->SetMarkovPseudoScaleB( node->GetMarkovPseudoScaleB() );
  this->SetCompletionTime( node->GetCompletionTime() );
  this->SetEqualization( node->GetEqualization() );

  this->EndModify( startModifyState );
}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLWorkflowInputNode
::vtkMRMLWorkflowInputNode()
{
  // By default, make these the recommended values from Holden et al. IEEE TBME 2014
  // That way, the user can construct a workable set of parameters by default
  // Some may be slightly different because of slight algorithmic changes
  this->FilterWidth = 0.05;
  this->OrthogonalOrder = 3;
  this->OrthogonalWindow = 5;
  this->Derivative = 1;
  this->NumCentroids = 70;
  this->NumPrinComps = 6;
  this->MarkovPseudoScalePi = 0.2;
  this->MarkovPseudoScaleA = 0.2;
  this->MarkovPseudoScaleB = 0.2;
  this->CompletionTime = 0.8;
  this->Equalization = 2.0;
}


vtkMRMLWorkflowInputNode
::~vtkMRMLWorkflowInputNode()
{
}


std::string vtkMRMLWorkflowInputNode
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;

  xmlstring << indent << "<WorkflowInput>" << std::endl;
  xmlstring << indent.GetNextIndent() << "<Parameter Type=\"FilterWidth\" Value=\"" << this->FilterWidth << "\" />" << std::endl;
  xmlstring << indent.GetNextIndent() << "<Parameter Type=\"OrthogonalOrder\" Value=\"" << this->OrthogonalOrder << "\" />" << std::endl;
  xmlstring << indent.GetNextIndent() << "<Parameter Type=\"OrthogonalWindow\" Value=\"" << this->OrthogonalWindow << "\" />" << std::endl;
  xmlstring << indent.GetNextIndent() << "<Parameter Type=\"Derivative\" Value=\"" << this->Derivative << "\" />" << std::endl;
  xmlstring << indent.GetNextIndent() << "<Parameter Type=\"NumCentroids\" Value=\"" << this->NumCentroids << "\" />" << std::endl;
  xmlstring << indent.GetNextIndent() << "<Parameter Type=\"NumPrinComps\" Value=\"" << this->NumPrinComps << "\" />" << std::endl;
  xmlstring << indent.GetNextIndent() << "<Parameter Type=\"MarkovPseudoScalePi\" Value=\"" << this->MarkovPseudoScalePi << "\" />" << std::endl;
  xmlstring << indent.GetNextIndent() << "<Parameter Type=\"MarkovPseudoScaleA\" Value=\"" << this->MarkovPseudoScaleA << "\" />" << std::endl;
  xmlstring << indent.GetNextIndent() << "<Parameter Type=\"MarkovPseudoScaleB\" Value=\"" << this->MarkovPseudoScaleB << "\" />" << std::endl;
  xmlstring << indent.GetNextIndent() << "<Parameter Type=\"CompletionTime\" Value=\"" << this->CompletionTime << "\" />" << std::endl;
  xmlstring << indent.GetNextIndent() << "<Parameter Type=\"Equalization\" Value=\"" << this->Equalization << "\" />" << std::endl;
  xmlstring << indent << "</WorkflowInput>" << std::endl;

  return xmlstring.str();
}


void vtkMRMLWorkflowInputNode
::FromXMLElement( vtkXMLDataElement* element )
{
  if ( element == NULL || strcmp( element->GetName(), "WorkflowInput" ) != 0 )
  {
    return;
  }
  
  int numElements = element->GetNumberOfNestedElements();

  int startModifyState = this->StartModify();

  for ( int i = 0; i < numElements; i++ )
  {

    vtkXMLDataElement* noteElement = element->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "Parameter" ) != 0 || noteElement->GetAttribute( "Type" ) == NULL )
    {
      continue;  // If it's not a "Parameter", jump to the next.
    }

	  const char* elementType = noteElement->GetAttribute( "Type" );
	
	  std::stringstream ss( std::string( noteElement->GetAttribute( "Value" ) ) );
	  double value;
	  ss >> value;

	  if ( strcmp( elementType, "Derivative" ) == 0 )
    {
	    this->SetDerivative( value );
    }
	  if ( strcmp( elementType, "FilterWidth" ) == 0 )
    {
	    this->SetFilterWidth( value );
    }
	  if ( strcmp( elementType, "OrthogonalOrder" ) == 0 )
    {
	    this->SetOrthogonalOrder( value );
    }
	  if ( strcmp( elementType, "OrthogonalWindow" ) == 0 )
    {
	    this->SetOrthogonalWindow( value );
    }
	  if ( strcmp( elementType, "NumPrinComps" ) == 0 )
    {
	    this->SetNumPrinComps( value );
    }
    if ( strcmp( elementType, "NumCentroids" ) == 0 )
    {
	    this->SetNumCentroids( value );
    }
	  if ( strcmp( elementType, "MarkovPseudoScalePi" ) == 0 )
    {
	    this->SetMarkovPseudoScalePi( value );
    }
	  if ( strcmp( elementType, "MarkovPseudoScaleA" ) == 0 )
    {
	    this->SetMarkovPseudoScaleA( value );
    }
	  if ( strcmp( elementType, "MarkovPseudoScaleB" ) == 0 )
    {
	    this->SetMarkovPseudoScaleB( value );
    }
	  if ( strcmp( elementType, "CompletionTime" ) == 0 )
    {
	    this->SetCompletionTime( value );
    }
    if ( strcmp( elementType, "Equalization" ) == 0 )
    {
	    this->SetEqualization( value );
    }

  }

  this->EndModify( startModifyState );
}