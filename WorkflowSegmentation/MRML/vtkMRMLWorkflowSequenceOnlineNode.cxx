
#include "vtkMRMLWorkflowSequenceOnlineNode.h"

// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLWorkflowSequenceOnlineNode* vtkMRMLWorkflowSequenceOnlineNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSequenceOnlineNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSequenceOnlineNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSequenceOnlineNode();
}


vtkMRMLNode* vtkMRMLWorkflowSequenceOnlineNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSequenceOnlineNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSequenceOnlineNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSequenceOnlineNode();
}



void vtkMRMLWorkflowSequenceOnlineNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLWorkflowSequenceOnlineNode
::WriteXML( ostream& of, int nIndent )
{
  this->vtkMRMLWorkflowSequenceNode::WriteXML(of, nIndent);
}


void vtkMRMLWorkflowSequenceOnlineNode
::ReadXMLAttributes( const char** atts )
{
  this->vtkMRMLWorkflowSequenceNode::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);

    // do something...
  }

}


void vtkMRMLWorkflowSequenceOnlineNode
::Copy( vtkMRMLNode* anode )
{
  this->vtkMRMLWorkflowSequenceNode::Copy( anode );
  // Copying is already taken care of my the superclass
}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLWorkflowSequenceOnlineNode
::vtkMRMLWorkflowSequenceOnlineNode()
{
}


vtkMRMLWorkflowSequenceOnlineNode
::~vtkMRMLWorkflowSequenceOnlineNode()
{
}


// Online methods ----------------------------------------------------------------------------------

void vtkMRMLWorkflowSequenceOnlineNode
::DistancesOnline( vtkDoubleArray* testPoints, vtkDoubleArray* distances )
{
  distances->SetNumberOfComponents( testPoints->GetNumberOfTuples() );
  distances->SetNumberOfTuples( 1 );
  
  // Get the "current" double array
  vtkDoubleArray* currDoubleArray = this->GetNthDoubleArray( this->GetNumberOfDataNodes() - 1 );
  if ( currDoubleArray == NULL )
  {
    return;  
  }

  double currSum;

  for ( int i = 0; i < testPoints->GetNumberOfTuples(); i++ )
  {      
    // First, ensure that the double arrays are the same size
    if ( currDoubleArray->GetNumberOfComponents() != testPoints->GetNumberOfComponents() )
    {
      continue;
    }

    // Initialize the sum to zero
    currSum = 0.0;
    for ( int d = 0; d < currDoubleArray->GetNumberOfComponents(); d++ )
    {
      double currDiff = currDoubleArray->GetComponent( 0, d ) - testPoints->GetComponent( i, d );
      currSum += currDiff * currDiff;
	  }
	  
    // Add to the component
	  distances->SetComponent( 0, i, sqrt( currSum ) );
  }

}


void vtkMRMLWorkflowSequenceOnlineNode
::DifferentiateOnline( int order, vtkDoubleArray* derivative )
{
  derivative->SetNumberOfComponents( this->GetNthNumberOfComponents( this->GetNumberOfDataNodes() - 1 ) );
  derivative->SetNumberOfTuples( 1 );

  // To calculate a derivative of arbitrary order, we need arbitrarily many time stamps
  // Assume that order is constant
  if ( this->GetNumberOfDataNodes() < order + 1 )
  {
    vtkDoubleArray* endDoubleArray = this->GetNthDoubleArray( this->GetNumberOfDataNodes() - 1 );
    if ( endDoubleArray == NULL )
    {
      return;
    }
    derivative->SetTuple( 0, 0, endDoubleArray );
    return;
  }
  
  // Just need the last order + 1 timestamps
  vtkSmartPointer< vtkMRMLWorkflowSequenceNode > endSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
  this->GetSubsequence( this->GetNumberOfDataNodes() - ( order + 1 ), this->GetNumberOfDataNodes() - 1, endSequence );
  endSequence->Differentiate( order );
  

  vtkDoubleArray* endDoubleArray = endSequence->GetNthDoubleArray( endSequence->GetNumberOfDataNodes() - 1 );
  if ( endDoubleArray == NULL )
  {
    return;
  }
  derivative->SetTuple( 0, 0, endDoubleArray );
}



void vtkMRMLWorkflowSequenceOnlineNode
::GaussianFilterOnline( double width, vtkDoubleArray* gauss )
{
  gauss->SetNumberOfComponents( this->GetNthNumberOfComponents( this->GetNumberOfDataNodes() - 1 ) );
  gauss->SetNumberOfTuples( 1 );

  // Iterate over all dimensions
  for ( int d = 0; d < gauss->GetNumberOfComponents(); d++ )
  {
    double weightSum = 0;
    double normSum = 0;

    // Iterate over all records nearby
	  int j = this->GetNumberOfDataNodes() - 1;
	  while ( j >= 0 ) // Iterate backward
    {
      // Get the current double array
      vtkDoubleArray* currDoubleArray = this->GetNthDoubleArray( j );
      if ( currDoubleArray == NULL )
      {
        break;
      }

	    // If too far from "peak" of distribution, the stop - we're just wasting time
      double normalizedDistance = ( this->GetNthIndexValueAsDouble( j ) - this->GetNthIndexValueAsDouble( this->GetNumberOfDataNodes() - 1 ) ) / width;
	    if ( abs( normalizedDistance ) > STDEV_CUTOFF )
	    {
	      break;
	    }

      // Calculate the values of the Gaussian distribution at this time
	    double gaussianWeight = exp( - normalizedDistance * normalizedDistance / 2 );
	    // Add the product with the values to function sum
      weightSum = weightSum + currDoubleArray->GetComponent( 0, d ) * gaussianWeight; // Add the product with the values to function sum		
	    // Add the values to normSum
	    normSum = normSum + gaussianWeight;

	    j--;
    }

    // Add to the new values
    gauss->SetComponent( 0, d, weightSum / normSum );
  }

}



void vtkMRMLWorkflowSequenceOnlineNode
::OrthogonalTransformationOnline( int window, int order, vtkDoubleArray* orthogonal )
{
  // Pad the recordlog with values at the beginning only if necessary
  vtkSmartPointer< vtkMRMLWorkflowSequenceNode > subsequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
  
  if ( this->GetNumberOfDataNodes() <= window )
  {
    vtkSmartPointer< vtkMRMLWorkflowSequenceNode > paddedSequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
    paddedSequence->Copy( this );
    paddedSequence->PadStart( window );
    paddedSequence->GetSubsequence( paddedSequence->GetNumberOfDataNodes() - ( window + 1 ), paddedSequence->GetNumberOfDataNodes() - 1, subsequence );
  }
  else
  {
    this->GetSubsequence( this->GetNumberOfDataNodes() - ( window + 1 ), this->GetNumberOfDataNodes() - 1, subsequence );
  }

  // Create a new matrix to which the Legendre coefficients will be assigned
  vtkNew< vtkDoubleArray > legendreCoefficients;
  subsequence->LegendreTransformation( order, legendreCoefficients.GetPointer() );

  orthogonal->SetNumberOfComponents( legendreCoefficients->GetNumberOfTuples() * legendreCoefficients->GetNumberOfComponents() );
  orthogonal->SetNumberOfTuples( 1 );

  // Calculate the Legendre coefficients: 2D -> 1D
  int count = 0;
  for ( int o = 0; o < legendreCoefficients->GetNumberOfTuples(); o++ )
  {
    for ( int d = 0; d < legendreCoefficients->GetNumberOfComponents(); d++ )
    {
      orthogonal->SetComponent( 0, count, legendreCoefficients->GetComponent( o, d ) );
      count++;
    }
  }

}


void vtkMRMLWorkflowSequenceOnlineNode
::TransformByPrincipalComponentsOnline( vtkDoubleArray* prinComps, vtkDoubleArray* meanArray, vtkDoubleArray* transformed )
{
  transformed->SetNumberOfComponents( prinComps->GetNumberOfTuples() );
  transformed->SetNumberOfTuples( 1 );
  vtkMRMLWorkflowSequenceNode::FillDoubleArray( transformed, 0 );

  // Create a vtkLabelRecord* for the transformed record log
  vtkDoubleArray* currDoubleArray = this->GetNthDoubleArray( this->GetNumberOfDataNodes() - 1 );
  if ( currDoubleArray == NULL )
  {
    return;
  }
    
  // Initialize the components of the transformed time record to be zero
	for ( int o = 0; o < prinComps->GetNumberOfTuples(); o++ )
	{
	  // Iterate over all dimensions, and perform the transformation (i.e. vector multiplication)
    for ( int d = 0; d < currDoubleArray->GetNumberOfComponents(); d++ )
	  {
      transformed->SetComponent( 0, o, transformed->GetComponent( 0, o ) + ( currDoubleArray->GetComponent( 0, d ) - meanArray->GetComponent( 0, d ) ) * prinComps->GetComponent( o, d ) );
	  }
	}

}



void vtkMRMLWorkflowSequenceOnlineNode
::fwdkmeansTransformOnline( vtkDoubleArray* centroids, vtkDoubleArray* cluster )
{
  cluster->SetNumberOfComponents( 1 );
  cluster->SetNumberOfTuples( 1 );

  // Calculate closest cluster centroid to last
  // Find the record farthest from any centroid
  vtkSmartPointer< vtkDoubleArray > centroidDistances = vtkSmartPointer< vtkDoubleArray >::New();
  this->DistancesOnline( centroids, centroidDistances );

  double currMinDist = std::numeric_limits< double >::max();
  int currMinCentroid = 0;
  // Minimum for each point
  for ( int c = 0; c < centroidDistances->GetNumberOfComponents(); c++ )
  {
    if ( centroidDistances->GetComponent( 0, c ) < currMinDist )
	  {
      currMinDist = centroidDistances->GetComponent( 0, c );
	    currMinCentroid = c;
	  }
  }

  cluster->SetComponent( 0, 0, currMinCentroid );
}


void vtkMRMLWorkflowSequenceOnlineNode
::AddMarkovModelAttributesOnline( vtkMRMLNode* node )
{
  // We will assume that: label -> state, values[0] -> symbol
  node->SetAttribute( "MarkovState", node->GetAttribute( "Message" ) );
  
  vtkMRMLWorkflowDoubleArrayNode* doubleArrayNode = vtkMRMLWorkflowDoubleArrayNode::SafeDownCast( node );
  if ( doubleArrayNode == NULL || doubleArrayNode->GetArray() == NULL )
  {
    return;
  }
  std::stringstream symbolStream; symbolStream << doubleArrayNode->GetArray()->GetComponent( 0, 0 );
  node->SetAttribute( "MarkovSymbol", symbolStream.str().c_str() );
}