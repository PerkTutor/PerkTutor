
#include "vtkMRMLWorkflowTrainingNode.h"

// Standard MRML Node Methods ------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLWorkflowTrainingNode);

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
    
  xmlstring << vtkMRMLWorkflowTrainingNode::DoubleArrayToXMLString( this->PrinComps, "PrinComps", indent.GetNextIndent() ); // TODO: Better way to re-use writing vtkDoubleArray to file?
  xmlstring << vtkMRMLWorkflowTrainingNode::DoubleArrayToXMLString( this->Mean, "Mean", indent.GetNextIndent() );
  xmlstring << vtkMRMLWorkflowTrainingNode::DoubleArrayToXMLString( this->Centroids, "Centroids", indent.GetNextIndent() );
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
    const char* elementType = noteElement->GetAttribute( "Type" );
    if ( strcmp( noteElement->GetName(), "Matrix" ) == 0 && noteElement->GetAttribute( "Type" ) != NULL )
    {
      if ( strcmp( elementType, "Mean" ) == 0 )
      {
        vtkMRMLWorkflowTrainingNode::DoubleArrayFromXMLElement( noteElement, "Mean", this->Mean );
      }
	    if ( strcmp( elementType, "PrinComps" ) == 0 )
      {
        vtkMRMLWorkflowTrainingNode::DoubleArrayFromXMLElement( noteElement, "PrinComps", this->PrinComps );
      }
      if ( strcmp( elementType, "Centroids" ) == 0 )
      {
        vtkMRMLWorkflowTrainingNode::DoubleArrayFromXMLElement( noteElement, "Centroids", this->Centroids );
      }
    }

	  if ( strcmp( noteElement->GetName(), "MarkovModel" ) == 0 )
    {
	    this->Markov->FromXMLElement( noteElement );
	  }

  }

  this->EndModify( startModifyState );

}


std::string vtkMRMLWorkflowTrainingNode
::DoubleArrayToXMLString( vtkDoubleArray* matrix, std::string name, vtkIndent indent )
{
  std::stringstream xmlStream;

  xmlStream << indent << "<Matrix Type=\"" << name << "\" ";
  xmlStream << "NumberOfTuples=\"" << matrix->GetNumberOfTuples() << "\" ";
  xmlStream << "NumberOfComponents=\"" << matrix->GetNumberOfComponents() << "\" " << std::endl;
  xmlStream << indent.GetNextIndent() << "MatrixValues=\"" << std::endl;
  
  for ( int i = 0; i < matrix->GetNumberOfTuples(); i++ )
  {
    for ( int j = 0; j < matrix->GetNumberOfComponents(); j++ )
    {
      xmlStream << indent.GetNextIndent() << matrix->GetComponent( i, j ) << " ";
    }
    xmlStream << std::endl;
  }
  xmlStream << indent.GetNextIndent() << "\" >" << std::endl;

  xmlStream << indent << "</Matrix>" << std::endl;

  return xmlStream.str();
}


void vtkMRMLWorkflowTrainingNode
::DoubleArrayFromXMLElement( vtkXMLDataElement* element, std::string name, vtkDoubleArray* matrix )
{
  if ( element == NULL || element->GetAttribute( "Type" ) == NULL || name.compare( element->GetAttribute( "Type" ) ) != 0 )
  {
    return; // Make sure we match the type of matrix (Pi/A/B) that we are trying to read
  }
  
  // Add each element
  std::stringstream xmlStream;

  matrix->Initialize();
  
  xmlStream.clear();
  xmlStream << element->GetAttribute( "NumberOfComponents" );
  double numberOfComponents; xmlStream >> numberOfComponents;
  matrix->SetNumberOfComponents( numberOfComponents );

  xmlStream.clear();
  xmlStream << element->GetAttribute( "NumberOfTuples" );
  double numberOfTuples; xmlStream >> numberOfTuples;
  matrix->SetNumberOfTuples( numberOfTuples );

  xmlStream.clear();
  xmlStream << element->GetAttribute( "MatrixValues" );

  for ( int i = 0; i < matrix->GetNumberOfTuples(); i++ )
  {
    for ( int j = 0; j < matrix->GetNumberOfComponents(); j++ )
    {
      double currMatrixValue; xmlStream >> currMatrixValue;
      matrix->SetComponent( i, j, currMatrixValue );
    }
  }
}