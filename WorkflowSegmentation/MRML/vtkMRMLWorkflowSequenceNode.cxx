

#include "vtkMRMLWorkflowSequenceNode.h"


// Constants ---------------------------------------------------------------------------------------

const double vtkMRMLWorkflowSequenceNode::STDEV_CUTOFF = 5.0;

// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLWorkflowSequenceNode* vtkMRMLWorkflowSequenceNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSequenceNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSequenceNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSequenceNode();
}


vtkMRMLNode* vtkMRMLWorkflowSequenceNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSequenceNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSequenceNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSequenceNode();
}



void vtkMRMLWorkflowSequenceNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLWorkflowSequenceNode
::WriteXML( ostream& of, int nIndent )
{
  this->vtkMRMLSequenceNode::WriteXML(of, nIndent);
}


void vtkMRMLWorkflowSequenceNode
::ReadXMLAttributes( const char** atts )
{
  this->vtkMRMLSequenceNode::ReadXMLAttributes(atts);

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


void vtkMRMLWorkflowSequenceNode
::Copy( vtkMRMLNode* anode )
{
  this->vtkMRMLSequenceNode::Copy( anode );
  // Copying is already taken care of my the superclass
}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLWorkflowSequenceNode
::vtkMRMLWorkflowSequenceNode()
{
}


vtkMRMLWorkflowSequenceNode
::~vtkMRMLWorkflowSequenceNode()
{
}



// Helper function ----------------------------------------------------------------------------------
double vtkMRMLWorkflowSequenceNode
::GetNthIndexValueAsDouble( int itemNumber )
{
  if ( this->GetIndexType() != this->NumericIndex )
  {
    return 0;
  }

  std::string indexValue = this->GetNthIndexValue( itemNumber );

  char* end;
  double val = std::strtod( indexValue.c_str(), &end );
  if (*end != 0) // Parsing failed due to non-numeric character
  {
    return 0;
  }

  return val;
}


int vtkMRMLWorkflowSequenceNode
::GetNthNumberOfComponents( int itemNumber )
{
  vtkDoubleArray* doubleArray = this->GetNthDoubleArray( itemNumber );
  if ( doubleArray == NULL )
  {
    return 0;
  }

  return doubleArray->GetNumberOfComponents();
}

vtkDoubleArray* vtkMRMLWorkflowSequenceNode
::GetNthDoubleArray( int itemNumber )
{
  vtkMRMLWorkflowDoubleArrayNode* doubleArrayNode = vtkMRMLWorkflowDoubleArrayNode::SafeDownCast( this->GetNthDataNode( itemNumber ) );
  if ( doubleArrayNode == NULL )
  {
    return NULL;
  }

  return doubleArrayNode->GetArray();
}

// Conversion from sequence browser -----------------------------------------------------------------
// TODO: Do we need conversion to sequence browser? Probably not...

// Only use transforms with the correct transform name
// Only keep the transforms who have associated messages that are relevant for this tool
void vtkMRMLWorkflowSequenceNode
::FromTrackedSequenceBrowserNode( vtkMRMLSequenceBrowserNode* newTrackedSequenceBrowserNode, std::string proxyNodeID, std::string messagesProxyNodeID, std::vector< std::string > relevantMessages )
{
  this->RemoveAllDataNodes();
  
  // Populate the list of finishing messages
  std::vector< std::string > finishingMessages;
  finishingMessages.push_back( "Done" );
  finishingMessages.push_back( "End" );
  finishingMessages.push_back( "Finished" );

  vtkMRMLLinearTransformNode* proxyNode = vtkMRMLLinearTransformNode::SafeDownCast( newTrackedSequenceBrowserNode->GetScene()->GetNodeByID( proxyNodeID ) );
  if ( proxyNode == NULL )
  {
    return;
  }

  vtkMRMLSequenceNode* sequenceNode = newTrackedSequenceBrowserNode->GetSequenceNode( proxyNode );
  if ( sequenceNode == NULL )
  {
    return;
  }

  vtkMRMLNode* messagesProxyNode = newTrackedSequenceBrowserNode->GetScene()->GetNodeByID( messagesProxyNodeID );
  vtkMRMLSequenceNode* messagesSequenceNode = newTrackedSequenceBrowserNode->GetSequenceNode( messagesProxyNode );
  if ( messagesSequenceNode == NULL )
  {
    return;
  }

  // Add the transform nodes to this workflow sequence as double array nodes
  vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode > doubleArrayNode = vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode >::New();
  for ( int i = 0; i < sequenceNode->GetNumberOfDataNodes(); i++ )
  {    
    this->LinearTransformToDoubleArray( vtkMRMLLinearTransformNode::SafeDownCast( sequenceNode->GetNthDataNode( i ) ), doubleArrayNode, QUATERNION_ARRAY ); // Use quaternions
    this->SetDataNodeAtValue( doubleArrayNode, sequenceNode->GetNthIndexValue( i ) );
  }

  // We need to convert the transforms into vtkDoubleArrays
  // This is a NumTransforms by NumMessages order algorithm anyway...
  // This will work because the messages are sorted by increased time
  for ( int i = 0; i < messagesSequenceNode->GetNumberOfDataNodes(); i++ )
  {
    // Only accept if message is relevant
    bool relevant = false;
    std::string messageValue = messagesSequenceNode->GetNthDataNode( i )->GetAttribute( "Message" );
    for ( int j = 0; j < relevantMessages.size(); j++ )
	  {
      if ( messageValue.compare( relevantMessages.at( j ) ) == 0 )
	    {
        relevant = true;
	    }
	  }
    for ( int j = 0; j < finishingMessages.size(); j++ )
    {
      if ( messageValue.compare( finishingMessages.at( j ) ) == 0 )
	    {
        relevant = true;
	    }
    }

	  // Skip if the message is not relevant 
    if ( ! relevant )
    {
      continue;
    }

    std::stringstream messageTimeStream( messagesSequenceNode->GetNthIndexValue( i ) ); // Does not have subclass method
    double messageTime; messageTimeStream >> messageTime;
    
	  for ( int j = 0; j < this->GetNumberOfDataNodes(); j++ )
	  {
      double doubleArrayTime = this->GetNthIndexValueAsDouble( j );

      if ( doubleArrayTime >= messageTime )
	    {
        this->GetNthDataNode( j )->SetAttribute( "Message", messageValue.c_str() );
	    }
	  }

  }

  // Only get the subsequence that actually has relevant messages
  vtkNew< vtkMRMLWorkflowSequenceNode > relevantSubsequence;
  this->GetLabelledSubsequence( relevantMessages, relevantSubsequence.GetPointer() );
  this->Copy( relevantSubsequence.GetPointer() );
}



// Methods specific for workflow segmentation -------------------------------------------------------------

// Note: This is inclusive (end points will appear in the result)
void vtkMRMLWorkflowSequenceNode
::GetSubsequence( int startItemNumber, int endItemNumber, vtkMRMLWorkflowSequenceNode* subsequence )
{
  subsequence->RemoveAllDataNodes();
  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = startItemNumber; i <= endItemNumber; i++ )
  {
    subsequence->SetDataNodeAtValue( this->GetNthDataNode( i ), this->GetNthIndexValue( i ) ); // OK because the data node is always deep copied
  }
}


void vtkMRMLWorkflowSequenceNode
::GetLabelledSubsequence( std::vector< std::string > labels, vtkMRMLWorkflowSequenceNode* subsequence )
{
  subsequence->RemoveAllDataNodes();
  // Iterate over all item in the sequence
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    vtkMRMLNode* currentDataNode = this->GetNthDataNode( i );
    if ( currentDataNode == NULL
      || currentDataNode->GetAttribute( "Message" ) == NULL
      || strcmp( currentDataNode->GetAttribute( "Message" ), "" ) == 0 )
    {
      continue;
    }
    
    // Check if the current record satisfies one of the labels
    for ( int j = 0; j < labels.size(); j++ )
	  {
      if ( labels.at( j ).compare( currentDataNode->GetAttribute( "Message" ) ) == 0 )
	    {        
        subsequence->SetDataNodeAtValue( currentDataNode, this->GetNthIndexValue( i ) ); // OK because the data node is always deep copied
	    }
	  }
    
  }

}



void vtkMRMLWorkflowSequenceNode
::Concatenate( vtkMRMLWorkflowSequenceNode* sequence, bool enforceUniqueIndexValues /* = false */ )
{
  // Note: If there are identical indices in the two sequences, there will only be one instance of it in the resulting concatenated sequence.
  // It will be the other sequence's node at that index
  // Handle some edge cases, ensuring that both sequences have at least one item
  if ( sequence->GetNumberOfDataNodes() == 0 )
  {
    return;
  }
  if ( this->GetNumberOfDataNodes() == 0 )
  {
    this->Copy( sequence );
    return;
  }

  // Need to check that the size of the double array nodes are the same...
  if ( this->GetNthNumberOfComponents() != sequence->GetNthNumberOfComponents() )
  {
    vtkWarningMacro("vtkMRMLWorkflowSequenceNode::Concatenate: Sequences are incompatible, could not concatenate.");
    return;
  }

  // Create a copy for grabbing indices from
  vtkNew< vtkMRMLWorkflowSequenceNode > tempConcatenatedSequence;

  // Note: This works if the index is numeric or text
  int uniqueIndexInt = 0;
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    std::string indexValue = this->GetNthIndexValue( i );
    if ( enforceUniqueIndexValues )
    {
      std::stringstream uniqueIndexStream; uniqueIndexStream << uniqueIndexInt;
      indexValue = uniqueIndexStream.str();
      uniqueIndexInt++;
    }
    tempConcatenatedSequence->SetDataNodeAtValue( this->GetNthDataNode( i ), indexValue ); // OK because the data node is always deep copied
  }
  for ( int i = 0; i < sequence->GetNumberOfDataNodes(); i++ )
  {
    std::string indexValue = sequence->GetNthIndexValue( i );
    if ( enforceUniqueIndexValues )
    {
      std::stringstream uniqueIndexStream; uniqueIndexStream << uniqueIndexInt;
      indexValue = uniqueIndexStream.str();
      uniqueIndexInt++;
    }
    tempConcatenatedSequence->SetDataNodeAtValue( sequence->GetNthDataNode( i ), indexValue ); // OK because the data node is always deep copied
  }

  this->Copy( tempConcatenatedSequence.GetPointer() );
}



void vtkMRMLWorkflowSequenceNode
::ConcatenateValues( vtkMRMLWorkflowSequenceNode* sequence )
{
  // Only works if the number of data nodes are the same for both sequences
  if ( this->GetNumberOfDataNodes() != sequence->GetNumberOfDataNodes() )
  {
    return;
  }

  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    vtkDoubleArray* thisDoubleArray = this->GetNthDoubleArray( i );
    vtkDoubleArray* sequenceDoubleArray = sequence->GetNthDoubleArray( i );
    if ( thisDoubleArray == NULL || sequenceDoubleArray == NULL )
    {
      continue;
    }

    vtkSmartPointer< vtkDoubleArray > concatenatedDoubleArray = vtkSmartPointer< vtkDoubleArray >::New();
    vtkMRMLWorkflowSequenceNode::ConcatenateDoubleArrays( thisDoubleArray, sequenceDoubleArray, concatenatedDoubleArray );

    thisDoubleArray->DeepCopy( concatenatedDoubleArray );
  }
  
}


void vtkMRMLWorkflowSequenceNode
::ConcatenateValues( vtkDoubleArray* doubleArray )
{
  if ( doubleArray == NULL )
  {
    return;
  }

  // Iterate over all nodes in this sequence and stick the array onto each
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    vtkDoubleArray* thisDoubleArray = this->GetNthDoubleArray( i );
    if ( thisDoubleArray == NULL )
    {
      continue;
    }

    vtkSmartPointer< vtkDoubleArray > concatenatedDoubleArray = vtkSmartPointer< vtkDoubleArray >::New();
    concatenatedDoubleArray->SetNumberOfComponents( thisDoubleArray->GetNumberOfComponents() + doubleArray->GetNumberOfComponents() );
    concatenatedDoubleArray->SetNumberOfTuples( 1 );

    int concatenationIndex = 0;
    for ( int j = 0; j < thisDoubleArray->GetNumberOfComponents(); j++ )
    {
      concatenatedDoubleArray->SetComponent( 0, concatenationIndex, thisDoubleArray->GetComponent( 0, j ) );
      concatenationIndex++;
    }
    for ( int j = 0; j < doubleArray->GetNumberOfComponents(); j++ )
    {
      concatenatedDoubleArray->SetComponent( 0, concatenationIndex, doubleArray->GetComponent( 0, j ) );
      concatenationIndex++;
    }

    thisDoubleArray->DeepCopy( concatenatedDoubleArray );
  }

}



// Note: This adds the padding to the start of the current sequence
void vtkMRMLWorkflowSequenceNode
::PadStart( int window )
{
  // Find the average time stamp
  // Divide by one less than the number of data nodes because there are one fewer differences than there are stamps
  double deltaTime = 1.0;
  if ( this->GetNumberOfDataNodes() > 1 )
  {
    deltaTime = ( this->GetNthIndexValueAsDouble( this->GetNumberOfDataNodes() - 1 ) - this->GetNthIndexValueAsDouble( 0 ) )/ ( this->GetNumberOfDataNodes() - 1 );
  }

  // Calculate the values and time stamp
  vtkMRMLNode* initialNode = this->GetNthDataNode( 0 );
  if ( initialNode == NULL )
  {
    return;
  }
  
  // It will be sorted automatically
  // But complexity will be better if we are always adding to the front
  double initialTime = this->GetNthIndexValueAsDouble( 0 ); // Need this out front because the initial time will keep changing as we add preceding items
  for ( int i = 1; i <= window; i++ )
  {
    std::stringstream timeStream;
    timeStream << ( initialTime - i * deltaTime );

    this->SetDataNodeAtValue( initialNode, timeStream.str() ); // Automatically deep copies
  }
}


void vtkMRMLWorkflowSequenceNode
::Mean( vtkDoubleArray* meanArray )
{
  if ( meanArray == NULL )
  {
    return;
  }

  meanArray->SetNumberOfComponents( this->GetNthNumberOfComponents( 0 ) );
  meanArray->SetNumberOfTuples( 1 );
  this->FillDoubleArray( meanArray, 0 );

  // For each time
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    vtkDoubleArray* currDoubleArray = this->GetNthDoubleArray( i );
    if ( currDoubleArray == NULL || currDoubleArray->GetNumberOfComponents() != meanArray->GetNumberOfComponents() )
    {
      continue;
    }

	  // Iterate over all dimensions
	  for ( int d = 0; d < meanArray->GetNumberOfComponents(); d++ )
	  {
      meanArray->SetComponent( 0, d, meanArray->GetComponent( 0, d ) + currDoubleArray->GetComponent( 0, d ) );
	  }

  }

  // Divide by the number of records
  for ( int d = 0; d < meanArray->GetNumberOfComponents(); d++ )
  {
    meanArray->SetComponent( 0, d, meanArray->GetComponent( 0, d ) / this->GetNumberOfDataNodes() );
  }

}


void vtkMRMLWorkflowSequenceNode
::Distances( vtkMRMLWorkflowSequenceNode* sequence, vtkDoubleArray* distances )
{
  // Put the other sequence into a double array
  vtkSmartPointer< vtkDoubleArray > doubleArray = vtkSmartPointer< vtkDoubleArray >::New();
  doubleArray->SetNumberOfComponents( sequence->GetNthNumberOfComponents() );
  doubleArray->SetNumberOfTuples( sequence->GetNumberOfDataNodes() );  
    
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    vtkDoubleArray* doubleArray = this->GetNthDoubleArray( i );
    if ( doubleArray == NULL )
    {
      continue;
    }
    
    doubleArray->SetTuple( i, 0, doubleArray );
  }
  
  this->Distances( doubleArray, distances );
}



void vtkMRMLWorkflowSequenceNode
::Distances( vtkDoubleArray* testPoints, vtkDoubleArray* distances )
{
  // Create a vector of vectors
  distances->SetNumberOfComponents( testPoints->GetNumberOfTuples() );
  distances->SetNumberOfTuples( this->GetNumberOfDataNodes() );
  

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    for ( int j = 0; j < testPoints->GetNumberOfTuples(); j++ )
	  {      
      // First, ensure that the records are the same size
      vtkDoubleArray* currThisDoubleArray = this->GetNthDoubleArray( i );
      if ( currThisDoubleArray == NULL || testPoints->GetNumberOfComponents() != currThisDoubleArray->GetNumberOfComponents() )
      {
        continue;
      }
      
      // Initialize the sum to zero
	    double currSum = 0;
      for ( int d = 0; d < currThisDoubleArray->GetNumberOfComponents(); d++ )
	    {
        double currDiff = currThisDoubleArray->GetComponent( 0, d ) - testPoints->GetComponent( j, d );
        currSum += currDiff * currDiff;
	    }
	    
      // Add to the current order record
	    distances->SetComponent( i, j, sqrt( currSum ) );
	  }
  }

}


// Calculate the record in the buffer that is closest to a particular point
void vtkMRMLWorkflowSequenceNode
::ClosestRecords( vtkDoubleArray* testPoints, vtkDoubleArray* closest )
{
  // Initialize the output
  closest->SetNumberOfComponents( testPoints->GetNumberOfComponents() );
  closest->SetNumberOfTuples( testPoints->GetNumberOfTuples() );

  if ( this->GetNthNumberOfComponents() != testPoints->GetNumberOfComponents() )
  {
    return;
  }

  // Calculate the distance to this point
  vtkNew< vtkDoubleArray > distances;
  this->Distances( testPoints, distances.GetPointer() );

  // Now find the closest point
  
  for ( int i = 0; i < testPoints->GetNumberOfTuples(); i++ )
  {
    for ( int j = 0; j < this->GetNumberOfDataNodes(); j++ )
    {
      double minimumDistance = std::numeric_limits< double >::max(); 
      if ( distances->GetComponent( j, i ) < minimumDistance )
	    {
        minimumDistance = distances->GetComponent( j, i );
        closest->SetTuple( i, 0, this->GetNthDoubleArray( j ) );
	    }
    }
  }

}




void vtkMRMLWorkflowSequenceNode
::Differentiate( int order )
{
  // If a derivative of order zero is required, then return a copy of this
  if ( this->GetNumberOfDataNodes() < 2 || order == 0 )
  {
    return;
  }

  // Otherwize calculate the derivative  
  vtkSmartPointer< vtkMRMLWorkflowSequenceNode > derivativeSequenceNode = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
  derivativeSequenceNode->Copy( this );

  // Use a centred difference formula (except at the endpoints, use a forward/backwar difference formula)
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    int lowerItemNumber = std::max( i - 1, 0 );
    int middleItemNumber = i;
    int upperItemNumber = std::min( i + 1, this->GetNumberOfDataNodes() - 1 );

    vtkDoubleArray* lowerDoubleArray = this->GetNthDoubleArray( lowerItemNumber );
    vtkDoubleArray* upperDoubleArray = this->GetNthDoubleArray( upperItemNumber );
    if ( lowerDoubleArray == NULL || upperDoubleArray == NULL || lowerDoubleArray->GetNumberOfComponents() != upperDoubleArray->GetNumberOfComponents() )
    {
      vtkWarningMacro( "vtkMRMLWorkflowSequenceNode::Differentiate: Cannot perform computation - sequence data nodes are incompatible.");
      continue;
    }

    vtkSmartPointer< vtkDoubleArray > tempDerivativeDoubleArray = vtkSmartPointer< vtkDoubleArray >::New();
    tempDerivativeDoubleArray->SetNumberOfComponents( lowerDoubleArray->GetNumberOfComponents() );
    tempDerivativeDoubleArray->SetNumberOfTuples( 1 );

    double deltaTime = this->GetNthIndexValueAsDouble( upperItemNumber ) - this->GetNthIndexValueAsDouble( lowerItemNumber );
    for( int d = 0; d < lowerDoubleArray->GetNumberOfComponents(); d++ )
    {
      tempDerivativeDoubleArray->SetComponent( 0, d, ( upperDoubleArray->GetComponent( 0, d ) - lowerDoubleArray->GetComponent( 0, d ) ) / deltaTime );
    }

    vtkDoubleArray* derivativeDoubleArray = derivativeSequenceNode->GetNthDoubleArray( i );
    if ( derivativeDoubleArray == NULL )
    {
      continue; 
    }
    derivativeDoubleArray->DeepCopy( tempDerivativeDoubleArray );
  }
  
  // Set this to be the differentiated buffer
  this->Copy( derivativeSequenceNode );
  this->Differentiate( order - 1 );
}



void vtkMRMLWorkflowSequenceNode
::Integrate( vtkDoubleArray* integration )
{
  // Initialize the output
  integration->SetNumberOfComponents( this->GetNthNumberOfComponents( 0 ) );
  integration->SetNumberOfTuples( 1 );
  this->FillDoubleArray( integration, 0 );

  // Use the trapezoidal rule
  for ( int i = 1; i < this->GetNumberOfDataNodes(); i++ )
  {
	  // Find the time difference
    vtkDoubleArray* lowerDoubleArray = this->GetNthDoubleArray( i - 1 );
    vtkDoubleArray* upperDoubleArray = this->GetNthDoubleArray( i );
    if ( lowerDoubleArray == NULL || upperDoubleArray == NULL || lowerDoubleArray->GetNumberOfComponents() != upperDoubleArray->GetNumberOfComponents() )
    {
      vtkWarningMacro("vtkMRMLWorkflowSequenceNode::Differentiate: Cannot perform computation - sequence data nodes are incompatible.");
    }

    double deltaTime = this->GetNthIndexValueAsDouble( i ) - this->GetNthIndexValueAsDouble( i - 1 );
	  for ( int d = 0; d < lowerDoubleArray->GetNumberOfComponents(); d++ )
	  {
	    integration->SetComponent( 0, d, integration->GetComponent( 0, d ) + deltaTime * ( lowerDoubleArray->GetComponent( 0, d ) + upperDoubleArray->GetComponent( 0, d ) ) / 2 );
	  }
  }

}


void vtkMRMLWorkflowSequenceNode
::LegendreTransformation( int order, vtkDoubleArray* legendreCoefficients )
{
  legendreCoefficients->SetNumberOfComponents( this->GetNthNumberOfComponents( 0 ) );
  legendreCoefficients->SetNumberOfTuples( order + 1 );

  // Calculate the time adjustment (need range -1 to 1)
  double startTime = this->GetNthIndexValueAsDouble( 0 );
  double endTime = this->GetNthIndexValueAsDouble( this->GetNumberOfDataNodes() - 1 );
  double timeRange = endTime - startTime;
  if ( timeRange <= 0 )
  {
    vtkWarningMacro( "vtkMRMLWorkflowSequenceNode::LegendreTransformation: Improper time range." );
    return;
  }

  // Populate the coefficient matrix
  for ( int o = 0; o <= order; o++ )
  {
	  vtkSmartPointer< vtkMRMLWorkflowSequenceNode > unintegratedSequenceNode = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();

    // Multiply the values by the Legendre polynomials
    for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
    {
      double adjustedTime = 2 * ( this->GetNthIndexValueAsDouble( i ) - startTime ) / timeRange - 1;
      double legendrePolynomial = this->LegendrePolynomial( adjustedTime, o );

	    vtkSmartPointer< vtkDoubleArray > tempUnintegratedDoubleArray = vtkSmartPointer< vtkDoubleArray >::New();
      tempUnintegratedDoubleArray->SetNumberOfComponents( this->GetNthNumberOfComponents( i ) );
      tempUnintegratedDoubleArray->SetNumberOfTuples( 1 );

      vtkDoubleArray* currentDoubleArray = this->GetNthDoubleArray( i );
      for ( int d = 0; d < this->GetNthNumberOfComponents( i ); d++ )
	    {	    
        tempUnintegratedDoubleArray->SetComponent( 0, d, currentDoubleArray->GetComponent( 0, d ) * legendrePolynomial );
	    }

      vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode > unintegratedDataNode = vtkSmartPointer< vtkMRMLWorkflowDoubleArrayNode >::New();
      vtkDoubleArray* unintegratedDoubleArray = unintegratedDataNode->GetArray();
      if ( unintegratedDoubleArray == NULL )
      {
        continue;
      }
      unintegratedDoubleArray->DeepCopy( tempUnintegratedDoubleArray );

      std::stringstream adjustedTimeString; adjustedTimeString << adjustedTime;
      unintegratedSequenceNode->SetDataNodeAtValue( unintegratedDataNode, adjustedTimeString.str() );
    }

	  // Integrate to get the Legendre coefficients for the particular order
    vtkSmartPointer< vtkDoubleArray > integration = vtkSmartPointer< vtkDoubleArray >::New();
    unintegratedSequenceNode->Integrate( integration );
    legendreCoefficients->SetTuple( o, 0, integration );
  }

}



double vtkMRMLWorkflowSequenceNode
::LegendrePolynomial( double time, int order )
{
  if ( order == 0 )
  {
    return 1;
  }
  if ( order == 1 )
  {
    return time;
  }
  if ( order == 2 )
  {
    return 3.0 / 2.0 * pow( time, 2.0 ) - 1.0 / 2.0;
  }
  if ( order == 3 )
  {
    return 5.0 / 2.0 * pow( time, 3.0 ) - 3.0 / 2.0 * time;
  }
  if ( order == 4 )
  {
    return 35.0 / 8.0 * pow( time, 4.0 ) - 15.0 / 4.0 * pow( time, 2.0 ) + 3.0 / 8.0;
  }
  if ( order == 5 )
  {
    return 63.0 / 8.0 * pow( time, 5.0 ) - 35.0 / 4.0 * pow( time, 3.0 ) + 15.0 / 8.0 * time;
  }
  if ( order == 6 )
  {
    return 231.0 / 16.0 * pow( time, 6.0 ) - 315.0 / 16.0 * pow( time, 4.0 ) + 105.0 / 16.0 * pow( time, 2.0 ) - 5.0 / 16.0;
  }

  return 0.0;
}



void vtkMRMLWorkflowSequenceNode
::GaussianFilter( double width )
{
  // Assume a Gaussian filter
  vtkSmartPointer< vtkMRMLWorkflowSequenceNode > gaussSequenceNode = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
  gaussSequenceNode->Copy( this );

  // For each record
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    // Get the current record
    vtkSmartPointer< vtkDoubleArray > tempGaussDoubleArray = vtkSmartPointer< vtkDoubleArray >::New();
    tempGaussDoubleArray->SetNumberOfComponents( this->GetNthNumberOfComponents( i ) );
    tempGaussDoubleArray->SetNumberOfTuples( 1 );

    // Iterate over all dimensions
	  for ( int d = 0; d < this->GetNthNumberOfComponents( i ); d++ )
	  {
      double weightSum = 0.0;
      double normSum = 0.0;

      // Iterate over all records nearby to the left
	    int j = i;
	    while ( j >= 0 ) // Iterate backward
      {
        // Get the current double array
        vtkDoubleArray* currDoubleArray = this->GetNthDoubleArray( j );
        if ( currDoubleArray == NULL )
        {
          break;
        }

	      // If too far from "peak" of distribution, the stop - we're just wasting time
	      double normalizedDistance = ( this->GetNthIndexValueAsDouble( j ) - this->GetNthIndexValueAsDouble( i ) ) / width;
		    if ( abs( normalizedDistance ) > STDEV_CUTOFF )
		    {
		      break;
		    }
        
        // Calculate the values of the Gaussian distribution at this time
        double gaussianWeight = exp( - normalizedDistance * normalizedDistance / 2 );		    
        weightSum = weightSum + currDoubleArray->GetComponent( 0, d ) * gaussianWeight; // Add the product with the values to function sum		    
		    normSum = normSum + gaussianWeight; // Add the values to normSum

		    j--;
      }

	    // Iterate over all records nearby to the right
	    j = i + 1;
	    while ( j < this->GetNumberOfDataNodes() ) // Iterate forward
      {
        // Get the current double array
        vtkDoubleArray* currDoubleArray = this->GetNthDoubleArray( j );
        if ( currDoubleArray == NULL )
        {
          break;
        }

	      // If too far from "peak" of distribution, the stop - we're just wasting time
	      double normalizedDistance = ( this->GetNthIndexValueAsDouble( j ) - this->GetNthIndexValueAsDouble( i ) ) / width;
		    if ( abs( normalizedDistance ) > STDEV_CUTOFF )
		    {
		      break;
		    }
        
        // Calculate the values of the Gaussian distribution at this time
        double gaussianWeight = exp( - normalizedDistance * normalizedDistance / 2 );		    
        weightSum = weightSum + currDoubleArray->GetComponent( 0, d ) * gaussianWeight; // Add the product with the values to function sum		    
		    normSum = normSum + gaussianWeight; // Add the values to normSum

		    j++;
      }

	    // Add to the new values
	    tempGaussDoubleArray->SetComponent( 0, d, weightSum / normSum );
	  }

	  // Add the new record vector to the record log
    vtkDoubleArray* gaussDoubleArray = gaussSequenceNode->GetNthDoubleArray( i );
    if ( gaussDoubleArray == NULL )
    {
      continue; 
    }
    gaussDoubleArray->DeepCopy( tempGaussDoubleArray );
  }

  this->Copy( gaussSequenceNode );
}



void vtkMRMLWorkflowSequenceNode
::OrthogonalTransformation( int window, int order )
{
  // Pad the sequence with values at the beginning
  vtkSmartPointer< vtkMRMLWorkflowSequenceNode > paddedSequenceNode = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
  paddedSequenceNode->Copy( this );
  paddedSequenceNode->PadStart( window );

  // Create a new record log with the orthogonally transformed data
  vtkSmartPointer< vtkMRMLWorkflowSequenceNode > orthogonalSequenceNode = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
  orthogonalSequenceNode->Copy( this );

  // Iterate over all data, and calculate Legendre expansion coefficients
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    // Get a subsequence of the padded sequence
    vtkSmartPointer< vtkMRMLWorkflowSequenceNode > subsequence = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
    paddedSequenceNode->GetSubsequence( i, i + window, subsequence );
	  
    vtkSmartPointer< vtkDoubleArray > legendreCoefficients = vtkSmartPointer< vtkDoubleArray >::New();
    subsequence->LegendreTransformation( order, legendreCoefficients );

    vtkSmartPointer< vtkDoubleArray > currLegendreDoubleArray = vtkSmartPointer< vtkDoubleArray >::New(); // To be passed to double array node
    currLegendreDoubleArray->SetNumberOfComponents( legendreCoefficients->GetNumberOfTuples() * legendreCoefficients->GetNumberOfComponents() );
    currLegendreDoubleArray->SetNumberOfTuples( 1 );

	  // Calculate the Legendre coefficients: 2D -> 1D
	  int count = 0;
	  for ( int o = 0; o < legendreCoefficients->GetNumberOfTuples(); o++ )
    {
      for ( int d = 0; d < legendreCoefficients->GetNumberOfComponents(); d++ )
	    {
        currLegendreDoubleArray->SetComponent( 0, count, legendreCoefficients->GetComponent( o, d ) );
		    count++;
	    }
    }

	  // Add the computed values to the sequence
    vtkDoubleArray* orthogonalDoubleArray = orthogonalSequenceNode->GetNthDoubleArray( i );
    if ( orthogonalDoubleArray == NULL )
    {
      continue; 
    }
    orthogonalDoubleArray->DeepCopy( currLegendreDoubleArray );
  }

  this->Copy( orthogonalSequenceNode );
}


vnl_matrix< double >* vtkMRMLWorkflowSequenceNode
::CovarianceMatrix()
{
  // Copy the current record log
  vtkSmartPointer< vtkMRMLWorkflowSequenceNode > zeroMeanSequenceNode = vtkSmartPointer< vtkMRMLWorkflowSequenceNode >::New();
  zeroMeanSequenceNode->Copy( this );

  // Construct a recordSize by recordSize vnl matrix
  vnl_matrix< double >* covariance = new vnl_matrix< double >( this->GetNthNumberOfComponents( 0 ), this->GetNthNumberOfComponents( 0 ) );
  covariance->fill( 0.0 );

  // Determine the mean, subtract the mean from each record
  vtkSmartPointer< vtkDoubleArray > meanArray = vtkSmartPointer< vtkDoubleArray >::New();
  this->Mean( meanArray );

  // Iterate over all times
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
	{
    vtkDoubleArray* currDoubleArray = this->GetNthDoubleArray( i );
    if ( currDoubleArray == NULL )
    {
      continue;
    }

    // Pick two dimensions, and find their covariance
    for ( int d1 = 0; d1 < currDoubleArray->GetNumberOfComponents(); d1++ )
    {
      for ( int d2 = 0; d2 < currDoubleArray->GetNumberOfComponents(); d2++ )
	    {                
        double meanSubtractedDimension1 = currDoubleArray->GetComponent( 0, d1 ) - meanArray->GetComponent( 0, d1 );
        double meanSubtractedDimension2 = currDoubleArray->GetComponent( 0, d2 ) - meanArray->GetComponent( 0, d2 );
	      covariance->put( d1, d2, covariance->get( d1, d2 ) + meanSubtractedDimension1 * meanSubtractedDimension2 );
	    }
	  }
  }

  // Divide by the number of data nodes
  for ( int d1 = 0; d1 < covariance->rows(); d1++ )
  {
    for ( int d2 = 0; d2 < covariance->columns(); d2++ )
	  {                
	    covariance->put( d1, d2, covariance->get( d1, d2 ) / this->GetNumberOfDataNodes() );
	  }
	}


  return covariance;
}



void vtkMRMLWorkflowSequenceNode
::CalculatePrincipalComponents( int numComp, vtkDoubleArray* prinComps )
{
  // Calculate the covariance matrix
  vnl_matrix< double >* covariance = this->CovarianceMatrix();

  prinComps->SetNumberOfComponents( covariance->rows() );
  prinComps->SetNumberOfTuples( numComp );

  //Calculate the eigenvectors of the covariance matrix
  vnl_matrix<double> eigenvectors( covariance->rows(), covariance->cols(), 0.0 );
  vnl_vector<double> eigenvalues( covariance->rows(), 0.0 );
  vnl_symmetric_eigensystem_compute( *covariance, eigenvectors, eigenvalues );
  // Note: eigenvectors are ordered in increasing eigenvalue ( 0 = smallest, end = biggest )

  // Prevent more prinicipal components than original dimensions
  if ( numComp > eigenvectors.cols() )
  {
    numComp = eigenvectors.cols();
  }

  for ( int i = eigenvectors.cols() - 1; i > eigenvectors.cols() - 1 - numComp; i-- )
  {
    for ( int d = 0; d < eigenvectors.rows(); d++ )
	  {
	    prinComps->SetComponent( eigenvectors.cols() - 1 - i, d, eigenvectors.get( d, i ) );
	  }
  }

}



void vtkMRMLWorkflowSequenceNode
::TransformByPrincipalComponents( vtkDoubleArray* prinComps, vtkDoubleArray* meanArray )
{
  // Iterate over all time stamps
  for( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    vtkDoubleArray* currDoubleArray = this->GetNthDoubleArray( i );
    if ( currDoubleArray == NULL )
    {
      return;
    }

    vtkSmartPointer< vtkDoubleArray > tempTransformedDoubleArray = vtkSmartPointer< vtkDoubleArray >::New();
    tempTransformedDoubleArray->SetNumberOfComponents( prinComps->GetNumberOfTuples() );
    tempTransformedDoubleArray->SetNumberOfTuples( 1 );
    this->FillDoubleArray( tempTransformedDoubleArray, 0 );
    
    // Initialize the components of the transformed time record to be zero
	  for ( int o = 0; o < prinComps->GetNumberOfTuples(); o++ )
	  {
	    // Iterate over all dimensions, and perform the transformation (i.e. vector multiplication)
      for ( int d = 0; d < currDoubleArray->GetNumberOfComponents(); d++ )
	    {
        tempTransformedDoubleArray->SetComponent( 0, o, tempTransformedDoubleArray->GetComponent( 0, o ) + ( currDoubleArray->GetComponent( 0, d ) - meanArray->GetComponent( 0, d ) ) * prinComps->GetComponent( o, d ) );
	    }
	  }

	  // Add the computed values to the sequence
    vtkDoubleArray* transformedDoubleArray = this->GetNthDoubleArray( i );
    if ( transformedDoubleArray == NULL )
    {
      continue; 
    }
    transformedDoubleArray->DeepCopy( tempTransformedDoubleArray );
  }

}



void vtkMRMLWorkflowSequenceNode
::fwdkmeans( int numClusters, vtkDoubleArray* centroids )
{
  centroids->SetNumberOfComponents( this->GetNthNumberOfComponents( 0 ) );
  centroids->SetNumberOfTuples( 0 );
  

  // A vector of cluster memberships
  std::vector< int > membership( this->GetNumberOfDataNodes(), 0 );

  // Iterate until all of the clusters have been added
  for ( int k = 0; k < numClusters; k++ )
  {

	  // Use closest point to the mean of all points for the first centroid
    if ( k == 0 )
	  {
      vtkSmartPointer< vtkDoubleArray > initialCentroid = vtkSmartPointer< vtkDoubleArray >::New();
      this->Mean( initialCentroid );
      centroids->InsertNextTuple( 0, initialCentroid );
	    continue;
	  }

    vtkSmartPointer< vtkDoubleArray > nextCentroid = vtkSmartPointer< vtkDoubleArray >::New();
    this->FindNextCentroid( centroids, nextCentroid );
    centroids->InsertNextTuple( 0, nextCentroid );

	  // Iterate until there are no more changes in membership and no clusters are empty
    bool change = true;
	  while ( change )
	  {
	    // Reassign the cluster memberships
	    std::vector< int > newMembership = this->ReassignMembership( centroids );

	    // Calculate change
	    change = this->MembershipChanged( membership, newMembership );
      if ( ! change )
	    {
        break;
	    }
	    membership = newMembership;

	    // Remove emptiness
	    std::vector< bool > emptyVector = this->FindEmptyClusters( centroids, membership );
      if ( this->HasEmptyClusters( emptyVector ) )
	    {
        this->MoveEmptyClusters( centroids, emptyVector );
		    // At the end of this, we are guaranteed no empty clusters
        continue;
	    }	  

	    // Recalculate centroids
	    this->RecalculateCentroids( membership, k + 1, centroids );

	  }

  }

}




void vtkMRMLWorkflowSequenceNode
::FindNextCentroid( vtkDoubleArray* centroids, vtkDoubleArray* nextCentroid )
{
  // Initialize
  nextCentroid->SetNumberOfComponents( centroids->GetNumberOfComponents() );
  nextCentroid->SetNumberOfTuples( 1 );


  // Find the record farthest from any centroid
  vtkSmartPointer< vtkDoubleArray > centroidDistances = vtkSmartPointer< vtkDoubleArray >::New();
  this->Distances( centroids, centroidDistances );
	
  int candidateItemNumber = 0;
  double candidateDistance = 0;

  for ( int i = 0; i < centroidDistances->GetNumberOfTuples(); i++ )
  {
    double currMinDist = std::numeric_limits< double >::max();
    // Minimum for each point
    for ( int c = 0; c < centroidDistances->GetNumberOfComponents(); c++ )
	  {
      if ( centroidDistances->GetComponent( i, c ) < currMinDist )
	    {
        currMinDist = centroidDistances->GetComponent( i, c );
	    }
	  }
	
	  // Maximum of the minimums
    if ( currMinDist > candidateDistance )
    {
      candidateDistance = currMinDist;
      candidateItemNumber = i;
	  }
  }

  // Create new centroid for candidate
  nextCentroid->SetTuple( 0, 0, this->GetNthDoubleArray( candidateItemNumber ) );
}



bool vtkMRMLWorkflowSequenceNode
::MembershipChanged( std::vector< int > oldMembership, std::vector< int > newMembership )
{
  for ( int i = 0; i < oldMembership.size(); i++ )
  {
    if ( oldMembership.at( i ) != newMembership.at( i ) )
	  {
      return true;
	  }
  }

  return false;
}



std::vector< bool > vtkMRMLWorkflowSequenceNode
::FindEmptyClusters( vtkDoubleArray* centroids, std::vector< int > membership )
{
  std::vector< bool > emptyVector( centroids->GetNumberOfTuples(), true );

  // Calculate the empty clusters
  for ( int i = 0; i < membership.size(); i++ )
  {
    emptyVector.at( membership.at(i) ) = false;
  }

  return emptyVector;
}



bool vtkMRMLWorkflowSequenceNode
::HasEmptyClusters( std::vector< bool > emptyVector )
{
  for ( int c = 0; c < emptyVector.size(); c++ )
  {
    if ( emptyVector.at( c ) )
	  {
      return true;
	  }
  }
 
  return false;
}



void vtkMRMLWorkflowSequenceNode
::MoveEmptyClusters( vtkDoubleArray* centroids, std::vector< bool > emptyVector )
{
  // Remove any emptyness (does this inline)
  for ( int c = 0; c < centroids->GetNumberOfTuples(); c++ )
  {
    if ( emptyVector.at(c) == false )
	  {
	    continue;
	  }

    vtkSmartPointer< vtkDoubleArray > nextCentroid = vtkSmartPointer< vtkDoubleArray >::New();
    this->FindNextCentroid( centroids, nextCentroid );
    centroids->SetTuple( c, 0, nextCentroid ); // Overwrites the tuple current at that item number
  }

}


std::vector< int > vtkMRMLWorkflowSequenceNode
::ReassignMembership( vtkDoubleArray* centroids )
{
  // Find the record farthest from any centroid
  vtkSmartPointer< vtkDoubleArray > centroidDistances = vtkSmartPointer< vtkDoubleArray >::New();
  this->Distances( centroids, centroidDistances );
  
  std::vector< int > membership;

  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    double currMinDist = std::numeric_limits< double >::max();
	  int currMinCentroid = 0;
    // Minimum for each point
    for ( int c = 0; c < centroidDistances->GetNumberOfComponents(); c++ )
	  {
      if ( centroidDistances->GetComponent( i, c ) < currMinDist )
	    {
        currMinDist = centroidDistances->GetComponent( i, c );
		    currMinCentroid = c;
	    }
	  }
    
    membership.push_back( currMinCentroid );
  }

  return membership;
}



void vtkMRMLWorkflowSequenceNode
::RecalculateCentroids( std::vector< int > membership, int numClusters, vtkDoubleArray* centroids )
{
  // Initialize (we need to reset everything anyway)
  centroids->SetNumberOfComponents( this->GetNthNumberOfComponents( 0 ) );
  centroids->SetNumberOfTuples( numClusters );
  this->FillDoubleArray( centroids, 0 );

  std::vector< int > memberCount( numClusters, 0 );

  // Iterate over all time
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    vtkDoubleArray* currDoubleArray = this->GetNthDoubleArray( i );
    if ( currDoubleArray == NULL )
    {
      return;
    }

    // For each dimension
    for ( int d = 0; d < centroids->GetNumberOfComponents(); d++ )
	  {      
	    centroids->SetComponent( membership.at( i ), d, centroids->GetComponent( membership.at( i ), d ) + currDoubleArray->GetComponent( 0, d ) );
	  }
    
    memberCount.at( membership.at( i ) )++;
  }

  // Divide by the number of records in the cluster to get the mean
  for ( int c = 0; c < centroids->GetNumberOfTuples(); c++ )
  {
    // For each dimension
    for ( int d = 0; d < centroids->GetNumberOfComponents(); d++ )
	  {
	    centroids->SetComponent( c, d, centroids->GetComponent( c, d ) / memberCount.at( c ) );
	  }
  }

}



void vtkMRMLWorkflowSequenceNode
::fwdkmeansTransform( vtkDoubleArray* centroids )
{
  // Use the reassign membership function to calculate closest centroids
  std::vector< int > membership = this->ReassignMembership( centroids );

  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    vtkDoubleArray* currDoubleArray = this->GetNthDoubleArray( i );
    if ( currDoubleArray == NULL )
    {
      continue;
    }
    
    currDoubleArray->SetNumberOfComponents( 1 );
    currDoubleArray->SetNumberOfTuples( 1 );    
    currDoubleArray->SetComponent( 0, 0, membership.at( i ) );
  }

}





void vtkMRMLWorkflowSequenceNode
::AddMarkovModelAttributes()
{
  // We will assume that: Message attribute -> state, values[0] -> symbol
  for ( int i = 0; i < this->GetNumberOfDataNodes(); i++ )
  {
    vtkMRMLNode* currDataNode = this->GetNthDataNode( i );
    if ( currDataNode == NULL )
    {
      continue;
    }
    currDataNode->SetAttribute( "MarkovState", currDataNode->GetAttribute( "Message" ) );

    vtkMRMLWorkflowDoubleArrayNode* currDoubleArrayNode = vtkMRMLWorkflowDoubleArrayNode::SafeDownCast( currDataNode );
    if ( currDoubleArrayNode == NULL || currDoubleArrayNode->GetArray() == NULL )
    {
      return;
    }
    std::stringstream symbolStream; symbolStream << currDoubleArrayNode->GetArray()->GetComponent( 0, 0 );
    currDataNode->SetAttribute( "MarkovSymbol", symbolStream.str().c_str() );
  }
}



void vtkMRMLWorkflowSequenceNode
::LinearTransformFromDoubleArray( vtkMRMLLinearTransformNode* transformNode, vtkMRMLWorkflowDoubleArrayNode* doubleArrayNode, ArrayType type )
{
  vtkDoubleArray* doubleArray = doubleArrayNode->GetArray();
  if ( doubleArray == NULL || doubleArray->GetNumberOfTuples() != 1 )
  {
    return;
  }

  vtkSmartPointer< vtkMatrix4x4 > transformMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  transformMatrix->Identity();
  if ( type == QUATERNION_ARRAY && doubleArray->GetNumberOfComponents() == QUATERNION_ARRAY ) // If it is in quaternion format
  {
    double quaternion[ 4 ];
    quaternion[ 0 ] = doubleArray->GetComponent( 0, 3 );
    quaternion[ 1 ] = doubleArray->GetComponent( 0, 4 );
    quaternion[ 2 ] = doubleArray->GetComponent( 0, 5 );
    quaternion[ 3 ] = doubleArray->GetComponent( 0, 6 );
    
    double matrix[ 3 ][ 3 ];
    vtkMath::QuaternionToMatrix3x3( quaternion, matrix );
    
    transformMatrix->SetElement( 0, 0, matrix[ 0 ][ 0 ] );
    transformMatrix->SetElement( 0, 1, matrix[ 0 ][ 1 ] );
    transformMatrix->SetElement( 0, 2, matrix[ 0 ][ 2 ] );
    transformMatrix->SetElement( 0, 3, doubleArray->GetComponent( 0, 0 ) );
    transformMatrix->SetElement( 1, 0, matrix[ 1 ][ 0 ] );
    transformMatrix->SetElement( 1, 1, matrix[ 1 ][ 1 ] );
    transformMatrix->SetElement( 1, 2, matrix[ 1 ][ 2 ] );
    transformMatrix->SetElement( 1, 3, doubleArray->GetComponent( 0, 1 ) );
    transformMatrix->SetElement( 2, 0, matrix[ 2 ][ 0 ] );
    transformMatrix->SetElement( 2, 1, matrix[ 2 ][ 1 ] );
    transformMatrix->SetElement( 2, 2, matrix[ 2 ][ 2 ] );
    transformMatrix->SetElement( 2, 3, doubleArray->GetComponent( 0, 2 ) );
  }
  else if ( type == MATRIX_ARRAY && doubleArray->GetNumberOfComponents() == MATRIX_ARRAY ) // If it is in matrix format
  {
    transformMatrix->SetElement( 0, 0, doubleArray->GetComponent( 0, 0 ) );
    transformMatrix->SetElement( 0, 1, doubleArray->GetComponent( 0, 1 ) );
    transformMatrix->SetElement( 0, 2, doubleArray->GetComponent( 0, 2 ) );
    transformMatrix->SetElement( 0, 3, doubleArray->GetComponent( 0, 3 ) );
    transformMatrix->SetElement( 1, 0, doubleArray->GetComponent( 0, 4 ) );
    transformMatrix->SetElement( 1, 1, doubleArray->GetComponent( 0, 5 ) );
    transformMatrix->SetElement( 1, 2, doubleArray->GetComponent( 0, 6 ) );
    transformMatrix->SetElement( 1, 3, doubleArray->GetComponent( 0, 7 ) );
    transformMatrix->SetElement( 2, 0, doubleArray->GetComponent( 0, 8 ) );
    transformMatrix->SetElement( 2, 1, doubleArray->GetComponent( 0, 9 ) );
    transformMatrix->SetElement( 2, 2, doubleArray->GetComponent( 0, 10 ) );
    transformMatrix->SetElement( 2, 3, doubleArray->GetComponent( 0, 11 ) );
    transformMatrix->SetElement( 3, 0, doubleArray->GetComponent( 0, 12 ) );
    transformMatrix->SetElement( 3, 1, doubleArray->GetComponent( 0, 13 ) );
    transformMatrix->SetElement( 3, 2, doubleArray->GetComponent( 0, 14 ) );
    transformMatrix->SetElement( 3, 3, doubleArray->GetComponent( 0, 15 ) );
  }
  
  transformNode->SetMatrixTransformToParent( transformMatrix );
}


void vtkMRMLWorkflowSequenceNode
::LinearTransformToDoubleArray( vtkMRMLLinearTransformNode* transformNode, vtkMRMLWorkflowDoubleArrayNode* doubleArrayNode, ArrayType type )
{
  vtkDoubleArray* doubleArray = doubleArrayNode->GetArray();

  vtkNew< vtkMatrix4x4 > transformMatrix;
  transformNode->GetMatrixTransformToParent( transformMatrix.GetPointer() );

  if ( type == QUATERNION_ARRAY ) // If it is in quaternion format
  {
    double matrix[ 3 ][ 3 ];
    matrix[ 0 ][ 0 ] = transformMatrix->GetElement( 0, 0 );
    matrix[ 0 ][ 1 ] = transformMatrix->GetElement( 0, 1 );
    matrix[ 0 ][ 2 ] = transformMatrix->GetElement( 0, 2 );
    matrix[ 1 ][ 0 ] = transformMatrix->GetElement( 1, 0 );
    matrix[ 1 ][ 1 ] = transformMatrix->GetElement( 1, 1 );
    matrix[ 1 ][ 2 ] = transformMatrix->GetElement( 1, 2 );
    matrix[ 2 ][ 0 ] = transformMatrix->GetElement( 2, 0 );
    matrix[ 2 ][ 1 ] = transformMatrix->GetElement( 2, 1 );
    matrix[ 2 ][ 2 ] = transformMatrix->GetElement( 2, 2 );
    
    double quaternion[ 4 ];
    vtkMath::Matrix3x3ToQuaternion( matrix, quaternion );
    
    doubleArray->SetNumberOfComponents( QUATERNION_ARRAY );
    doubleArray->SetNumberOfTuples( 1 );

    doubleArray->SetComponent( 0, 0, transformMatrix->GetElement( 0, 3 ) );
    doubleArray->SetComponent( 0, 1, transformMatrix->GetElement( 1, 3 ) );
    doubleArray->SetComponent( 0, 2, transformMatrix->GetElement( 2, 3 ) );
    doubleArray->SetComponent( 0, 3, quaternion[ 0 ] );
    doubleArray->SetComponent( 0, 4, quaternion[ 1 ] );
    doubleArray->SetComponent( 0, 5, quaternion[ 2 ] );
    doubleArray->SetComponent( 0, 6, quaternion[ 3 ] );
  }
  else if ( type == MATRIX_ARRAY ) // If it is in matrix format
  {
    doubleArray->SetNumberOfComponents( MATRIX_ARRAY );
    doubleArray->SetNumberOfTuples( 1 );

    doubleArray->SetComponent( 0, 0, transformMatrix->GetElement( 0, 0 ) );
    doubleArray->SetComponent( 0, 1, transformMatrix->GetElement( 0, 1 ) );
    doubleArray->SetComponent( 0, 2, transformMatrix->GetElement( 0, 2 ) );
    doubleArray->SetComponent( 0, 3, transformMatrix->GetElement( 0, 3 ) );
    doubleArray->SetComponent( 0, 4, transformMatrix->GetElement( 1, 0 ) );
    doubleArray->SetComponent( 0, 5, transformMatrix->GetElement( 1, 1 ) );
    doubleArray->SetComponent( 0, 6, transformMatrix->GetElement( 1, 2 ) );
    doubleArray->SetComponent( 0, 7, transformMatrix->GetElement( 1, 3 ) );
    doubleArray->SetComponent( 0, 8, transformMatrix->GetElement( 2, 0 ) );
    doubleArray->SetComponent( 0, 9, transformMatrix->GetElement( 2, 1 ) );
    doubleArray->SetComponent( 0, 10, transformMatrix->GetElement( 2, 2 ) );
    doubleArray->SetComponent( 0, 11, transformMatrix->GetElement( 2, 3 ) );
    doubleArray->SetComponent( 0, 12, transformMatrix->GetElement( 3, 0 ) );
    doubleArray->SetComponent( 0, 13, transformMatrix->GetElement( 3, 1 ) );
    doubleArray->SetComponent( 0, 14, transformMatrix->GetElement( 3, 2 ) );
    doubleArray->SetComponent( 0, 15, transformMatrix->GetElement( 3, 3 ) );
  }

}


// Helper method (since this isn't implemented in the version of VTK that Slicer uses)
void vtkMRMLWorkflowSequenceNode
::FillDoubleArray( vtkDoubleArray* doubleArray, double fillValue )
{
  if ( doubleArray == NULL )
  {
    return;
  }

  for ( int i = 0; i < doubleArray->GetNumberOfComponents(); i++ )
  {
    doubleArray->FillComponent( i, fillValue );
  }
}


void vtkMRMLWorkflowSequenceNode
::ConcatenateDoubleArrays( vtkDoubleArray* inArray1, vtkDoubleArray* inArray2, vtkDoubleArray* outArray )
{
  if ( inArray1 == NULL || inArray2 == NULL || outArray == NULL )
  {
    return;
  }

  outArray->SetNumberOfComponents( inArray1->GetNumberOfComponents() + inArray2->GetNumberOfComponents() );
  outArray->SetNumberOfTuples( 1 );

  int concatenationIndex = 0;
  for ( int j = 0; j < inArray1->GetNumberOfComponents(); j++ )
  {
    outArray->SetComponent( 0, concatenationIndex, inArray1->GetComponent( 0, j ) );
    concatenationIndex++;
  }
  for ( int j = 0; j < inArray2->GetNumberOfComponents(); j++ )
  {
    outArray->SetComponent( 0, concatenationIndex, inArray2->GetComponent( 0, j ) );
    concatenationIndex++;
  }
}