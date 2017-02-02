
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
  
  this->Mean->DeepCopy( node->GetMean() );
  this->PrinComps->DeepCopy( node->GetPrinComps() );
  this->Centroids->DeepCopy( node->GetCentroids() );
  this->Markov->Copy( node->GetMarkov() );

  this->EndModify( startModifyState );
}


// Constructors & Destructors --------------------------------------------------------------------------

vtkMRMLWorkflowTrainingNode
::vtkMRMLWorkflowTrainingNode()
{
  // Give default values, don't need to initialize vectors
  this->Mean = vtkSmartPointer< vtkDoubleArray >::New();
  this->PrinComps = vtkSmartPointer< vtkDoubleArray >::New();
  this->Centroids = vtkSmartPointer< vtkDoubleArray >::New();
  this->Markov = vtkSmartPointer< vtkMarkovModelOnline >::New();
}


vtkMRMLWorkflowTrainingNode
::~vtkMRMLWorkflowTrainingNode()
{
  // Smart pointers take care of this
}


vtkDoubleArray* vtkMRMLWorkflowTrainingNode
::GetMean()
{
  return this->Mean;
}


void vtkMRMLWorkflowTrainingNode
::SetMean( vtkDoubleArray* newMean )
{
  this->Mean = newMean;
  this->Modified();
}


vtkDoubleArray* vtkMRMLWorkflowTrainingNode
::GetPrinComps()
{
  return this->PrinComps;
}


void vtkMRMLWorkflowTrainingNode
::SetPrinComps( vtkDoubleArray* newPrinComps )
{
  this->PrinComps = newPrinComps;
  this->Modified();
}


vtkDoubleArray* vtkMRMLWorkflowTrainingNode
::GetCentroids()
{
  return this->Centroids;
}


void vtkMRMLWorkflowTrainingNode
::SetCentroids( vtkDoubleArray* newCentroids )
{
  this->Centroids = newCentroids;
  this->Modified();
}


vtkMarkovModelOnline* vtkMRMLWorkflowTrainingNode
::GetMarkov()
{
  return this->Markov;
}


void vtkMRMLWorkflowTrainingNode
::SetMarkov( vtkMarkovModelOnline* newMarkov )
{
  this->Markov = newMarkov;
  this->Modified();
}


std::string vtkMRMLWorkflowTrainingNode
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;
  
  xmlstring << indent << "<WorkflowTraining>" << std::endl;
    
  xmlstring << vtkMarkovModel::MarkovMatrixToXMLString( this->PrinComps, "PrinComps", indent.GetNextIndent() ); // TODO: Better way to re-use writing vtkDoubleArray to file?
  xmlstring << vtkMarkovModel::MarkovMatrixToXMLString( this->Mean, "Mean", indent.GetNextIndent() );
  xmlstring << vtkMarkovModel::MarkovMatrixToXMLString( this->Centroids, "Centroids", indent.GetNextIndent() );
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

    if ( strcmp( elementType, "Mean" ) == 0 )
    {
      vtkNew< vtkDoubleArray > mean;
      vtkMarkovModel::MarkovMatrixFromXMLElement( noteElement, "Mean", mean.GetPointer() );
      this->SetMean( mean.GetPointer() );
    }
	  if ( strcmp( elementType, "PrinComps" ) == 0 )
    {
      vtkNew< vtkDoubleArray > prinComps;
      vtkMarkovModel::MarkovMatrixFromXMLElement( noteElement, "PrinComps", prinComps.GetPointer() );
      this->SetPrinComps( prinComps.GetPointer() );
    }
    if ( strcmp( elementType, "Centroids" ) == 0 )
    {
      vtkNew< vtkDoubleArray > centroids;
      vtkMarkovModel::MarkovMatrixFromXMLElement( noteElement, "Centroids", centroids.GetPointer() );
      this->SetCentroids( centroids.GetPointer() );
    }
	  if ( strcmp( elementType, "Markov" ) == 0 )
    {
	    this->Markov->FromXMLElement( noteElement );
	  }

  }

  this->EndModify( startModifyState );

}
