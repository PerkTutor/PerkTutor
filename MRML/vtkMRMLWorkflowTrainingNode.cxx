
#include "vtkMRMLWorkflowTrainingNode.h"

// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLWorkflowTrainingNode* vtkMRMLWorkflowTrainingNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowTrainingNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowTrainingNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowTrainingNode();
}


vtkMRMLNode* vtkMRMLWorkflowTrainingNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowTrainingNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowTrainingNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowTrainingNode();
}



void vtkMRMLWorkflowTrainingNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLWorkflowTrainingNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);
}


void vtkMRMLWorkflowTrainingNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);
}


void vtkMRMLWorkflowTrainingNode
::Copy( vtkMRMLNode *anode )
{
  Superclass::Copy( anode );
  vtkMRMLWorkflowTrainingNode *node = ( vtkMRMLWorkflowTrainingNode* ) anode;
  if ( node == NULL )
  {
    return;
  }

  int startModifyState = this->StartModify();
  
  this->PrinComps.clear();
  for ( int i = 0; i < node->GetPrinComps().size(); i++ )
  {
    vtkSmartPointer< vtkLabelVector > currPrinComp = vtkSmartPointer< vtkLabelVector >::New();
    currPrinComp->Copy( node->GetPrinComps().at( i ) );
    this->PrinComps.push_back( currPrinComp );
  }
  this->Centroids.clear();
  for ( int i = 0; i < node->GetCentroids().size(); i++ )
  {
    vtkSmartPointer< vtkLabelVector > currCentroid = vtkSmartPointer< vtkLabelVector >::New();
    currCentroid->Copy( node->GetCentroids().at( i ) );
    this->Centroids.push_back( currCentroid );
  }
  
  this->Mean->Copy( node->GetMean() );
  this->Markov->Copy( node->GetMarkov() );

  this->EndModify( startModifyState );
}


// Constructors & Destructors --------------------------------------------------------------------------

vtkMRMLWorkflowTrainingNode
::vtkMRMLWorkflowTrainingNode()
{
  // Give default values, don't need to initialize vectors
  this->Mean = vtkSmartPointer< vtkLabelVector >::New();
  this->Markov = vtkSmartPointer< vtkMarkovModelRT >::New();
}


vtkMRMLWorkflowTrainingNode
::~vtkMRMLWorkflowTrainingNode()
{
  this->PrinComps.clear();
  this->Centroids.clear();
}


std::vector< vtkSmartPointer< vtkLabelVector > > vtkMRMLWorkflowTrainingNode
::GetPrinComps()
{
  return this->PrinComps;
}


void vtkMRMLWorkflowTrainingNode
::SetPrinComps( std::vector< vtkSmartPointer< vtkLabelVector > > newPrinComps )
{
  this->PrinComps = newPrinComps;
  this->Modified();
}


std::vector< vtkSmartPointer< vtkLabelVector > > vtkMRMLWorkflowTrainingNode
::GetCentroids()
{
  return this->Centroids;
}


void vtkMRMLWorkflowTrainingNode
::SetCentroids( std::vector< vtkSmartPointer< vtkLabelVector > > newCentroids )
{
  this->Centroids = newCentroids;
  this->Modified();
}

vtkLabelVector* vtkMRMLWorkflowTrainingNode
::GetMean()
{
  return this->Mean;
}


void vtkMRMLWorkflowTrainingNode
::SetMean( vtkLabelVector* newMean )
{
  this->Mean = newMean;
  this->Modified();
}


vtkMarkovModelRT* vtkMRMLWorkflowTrainingNode
::GetMarkov()
{
  return this->Markov;
}


void vtkMRMLWorkflowTrainingNode
::SetMarkov( vtkMarkovModelRT* newMarkov )
{
  this->Markov = newMarkov;
  this->Modified();
}


std::string vtkMRMLWorkflowTrainingNode
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;
  
  xmlstring << indent << "<WorkflowTraining>" << std::endl;
    
  xmlstring << vtkLabelVector::VectorsToXMLString( this->PrinComps, "PrinComps", indent.GetNextIndent() );
  xmlstring << vtkLabelVector::VectorsToXMLString( this->Mean, "Mean", indent.GetNextIndent() );
  xmlstring << vtkLabelVector::VectorsToXMLString( this->Centroids, "Centroids", indent.GetNextIndent() );
  xmlstring << this->Markov->ToXMLString( indent.GetNextIndent() );
  
  xmlstring << indent << "</WorkflowTraining>" << std::endl;
  
  return xmlstring.str();

}


void vtkMRMLWorkflowTrainingNode
::FromXMLElement( vtkXMLDataElement* element )
{
  if ( element == NULL || strcmp( element->GetName(), "WorkflowTraining" ) != 0 )
  {
    return;
  }
  
  int numElements = element->GetNumberOfNestedElements();

  int startModifyState = this->StartModify();

  for ( int i = 0; i < numElements; i++ )
  {

    vtkXMLDataElement* noteElement = element->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "Vectors" ) != 0 && strcmp( noteElement->GetName(), "MarkovModel" ) != 0 || noteElement->GetAttribute( "Type" ) == NULL )
    {
      continue;  // If it's not a "Parameter" or "MarkovModel", jump to the next.
    }

    const char* elementType = noteElement->GetAttribute( "Type" );

	  if ( strcmp( elementType, "PrinComps" ) == 0 )
    {
	    this->SetPrinComps( vtkLabelVector::VectorsFromXMLElement( noteElement, "PrinComps" ) );
    }
    if ( strcmp( elementType, "Centroids" ) == 0 )
    {
	    this->SetCentroids( vtkLabelVector::VectorsFromXMLElement( noteElement, "Centroids" ) );
    }
	  if ( strcmp( elementType, "Mean" ) == 0 )
    {
	    this->SetMean( vtkLabelVector::VectorsFromXMLElement( noteElement, "Mean" ).at(0) );
    }
	  if ( strcmp( elementType, "Markov" ) == 0 )
    {
	    this->Markov->FromXMLElement( noteElement );
	  }

  }

  this->EndModify( startModifyState );

}
