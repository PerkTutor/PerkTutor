
#include "vtkSmartPointer.h"
#include "vtkRecordLog.h"
#include "vtkObjectFactory.h"
#include "vnl\vnl_matrix.h"
#include "vnl\algo\vnl_symmetric_eigensystem.h"

#include <string>
#include <sstream>
#include <cmath>


vtkStandardNewMacro( vtkRecordLog );

vtkRecordLog
::vtkRecordLog()
{
  this->numRecords = 0;
  this->recordSize = 0;
}


vtkRecordLog
::~vtkRecordLog()
{
  // Iterate over all items in the valuestor and delete
  for( int i = 0; i < numRecords; i++ )
    delete [] &records[i];

  records.clear();
}

vtkRecordLog* vtkRecordLog
::DeepCopy()
{
  // Create a new procedure object
  vtkRecordLog* newRecordLog = vtkRecordLog::New();

  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = 0; i < numRecords; i++ )
  {
	TimeLabelRecord currRecord;

	for ( int d = 0; d < recordSize; d++ )
	{
      currRecord.add( GetRecordAt(i).get(d) );
	}

	currRecord.setTime( GetRecordAt(i).getTime() );
	currRecord.setLabel( GetRecordAt(i).getLabel() );
    newRecordLog->AddRecord( currRecord );

  }
  
  return newRecordLog;
}


void vtkRecordLog
::AddRecord( TimeLabelRecord newRecord )
{
  // Change the record size if this is the first element
  if( numRecords == 0 )
  {
    recordSize = newRecord.size();
  }
  // Only add the record if it is correct size
  if ( newRecord.size() == recordSize )
  {
    records.push_back( newRecord );
    numRecords++;
  }

}


TimeLabelRecord vtkRecordLog
::GetRecordAt( int index )
{
  return records[index];
}


int vtkRecordLog
::Size()
{
  return numRecords;
}



vtkRecordLog* vtkRecordLog
::Trim( int start, int end )
{

  // Create a new procedure object
  vtkRecordLog* trimRecordLog = vtkRecordLog::New();

  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = start; i <= end; i++ )
  {

	TimeLabelRecord currRecord;

	for ( int d = 0; d < recordSize; d++ )
	{
      currRecord.add( GetRecordAt(i).get(d) );
	}

	currRecord.setTime( GetRecordAt(i).getTime() );
	currRecord.setLabel( GetRecordAt(i).getLabel() );
    trimRecordLog->AddRecord( currRecord );

  }

  return trimRecordLog;

}



vtkRecordLog* vtkRecordLog
::Concatenate( vtkRecordLog* otherRecordLog )
{
  // First, deep copy the current record log and the concatenating one
  vtkRecordLog* currRecordLogCopy;
  vtkRecordLog* otherRecordLogCopy;

  currRecordLogCopy = this->DeepCopy();
  otherRecordLogCopy = otherRecordLog->DeepCopy();

  // Create a new record log and assign the copies to it
  vtkRecordLog* catRecordLog = vtkRecordLog::New();

  for( int i = 0; i < currRecordLogCopy->numRecords; i++ )
  {
    catRecordLog->AddRecord( currRecordLogCopy->records[i] );
  }

  for( int i = 0; i < otherRecordLogCopy->numRecords; i++ )
  {
    catRecordLog->AddRecord( otherRecordLogCopy->records[i] );
  }

  return catRecordLog;
}



vtkRecordLog* vtkRecordLog
::ConcatenateValues( vtkRecordLog* otherRecordLog )
{
  // Only works if the number of records are the same for both record logs
  if ( this->numRecords != otherRecordLog->numRecords )
  {
    return this->DeepCopy();
  }

  // Iterate over all records in both logs and stick them together
  vtkRecordLog* catRecordLog = vtkRecordLog::New();

  for ( int i = 0; i < this->numRecords; i++ )
  {
    TimeLabelRecord currRecord;

    for ( int d = 0; d < this->recordSize; d++ )
	{
      currRecord.add( this->GetRecordAt(i).get(d) );
	}
	for ( int d = 0; d < otherRecordLog->recordSize; d++ )
	{
	  currRecord.add( otherRecordLog->GetRecordAt(i).get(d) );
	}

	currRecord.setTime( this->GetRecordAt(i).getTime() );
	currRecord.setLabel( this->GetRecordAt(i).getLabel() );
    catRecordLog->AddRecord( currRecord );
  }

  return catRecordLog;
}


// Note: Does not add to start, just creates start - must concatenate it
vtkRecordLog* vtkRecordLog
::PadStart( int window )
{
  // Create a new record log of padded values only
  vtkRecordLog* padRecordLog = vtkRecordLog::New();

  // Find the average time stamp
  double DT = ( GetRecordAt(numRecords-1).getTime() - GetRecordAt(0).getTime() ) / numRecords;

  // Calculate the values and time stamp
  for ( int i = window; i > 0; i-- )
  {
	TimeLabelRecord padRecord;

	for ( int d = 0; d < recordSize; d++ )
	{
	   padRecord.add( GetRecordAt(0).get(d) );
	}

	padRecord.setTime( GetRecordAt(0).getTime() - i * DT );
	padRecord.setLabel( GetRecordAt(0).getLabel() );
	padRecordLog->AddRecord( padRecord );

  }

  return padRecordLog;
}


ValueRecord vtkRecordLog
::Mean()
{
  // The record log will only hold one record at the end
  ValueRecord meanRecord;

  // Iterate over all dimensions
  for ( int d = 0; d < recordSize; d++ )
  {
    meanRecord.add( 0.0 );
  }

  // For each time
  for ( int i = 0; i < numRecords; i++ )
  {
	// Iterate over all dimensions
	for ( int d = 0; d < recordSize; d++ )
	{
	  meanRecord.set( d, meanRecord.get( d ) + GetRecordAt(i).get(d) );
	}
  }

  // Divide by the number of records
  for ( int d = 0; d < recordSize; d++ )
  {
    meanRecord.set( d, meanRecord.get(d) / numRecords );
  }

  return meanRecord;
}



std::vector<LabelRecord> vtkRecordLog
::Distances( vtkRecordLog* otherRecLog )
{
  // Create a vector of order records
  std::vector<LabelRecord> dists;

  // First, ensure that the records are the same size
  if ( this->recordSize != otherRecLog->recordSize )
  {
    return dists;
  }

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < numRecords; i++ )
  {
    // Create a new order record
    LabelRecord distRecord;

    for ( int j = 0; j < otherRecLog->numRecords; j++ )
	{
      // Initialize the sum to zero
	  double currSum = 0.0;
	  for ( int d = 0; d < recordSize; d++ )
	  {
        currSum = currSum + ( this->GetRecordAt(i).get(d) - otherRecLog->GetRecordAt(j).get(d) ) * ( this->GetRecordAt(i).get(d) - otherRecLog->GetRecordAt(j).get(d) );
	  }
	  // Add to the current order record
	  distRecord.add( currSum );
	}

	// Add the current order record to the vector
	distRecord.setLabel( i );
	dists.push_back( distRecord );

  }

  return dists;

}



std::vector<LabelRecord> vtkRecordLog
::Distances( std::vector<ValueRecord> valueRecords )
{
  // Create a vector of order records
  std::vector<LabelRecord> dists;

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < numRecords; i++ )
  {
    // Create a new order record
    LabelRecord distRecord;

    for ( int j = 0; j < valueRecords.size(); j++ )
	{
      
      // First, ensure that the records are the same size
      if ( this->recordSize != valueRecords[j].size() )
      {
        return dists;
      }

      // Initialize the sum to zero
	  double currSum = 0.0;
	  for ( int d = 0; d < recordSize; d++ )
	  {
        currSum = currSum + ( this->GetRecordAt(i).get(d) - valueRecords[j].get(d) ) * ( this->GetRecordAt(i).get(d) - valueRecords[j].get(d) );
	  }
	  // Add to the current order record
	  distRecord.add( currSum );
	}

	// Add the current order record to the vector
	distRecord.setLabel( i );
	dists.push_back( distRecord );

  }

  return dists;

}




vtkRecordLog* vtkRecordLog
::Derivative( int order )
{
  // If a derivative of order zero is required, then return a copy of this
  if ( order == 0 )
  {
    return this->DeepCopy();
  }

  // Otherwise, calculate the derivative
  vtkRecordLog* derivRecordLog = vtkRecordLog::New();
  double DT;


  // First
  DT = GetRecordAt(1).getTime() - GetRecordAt(0).getTime();
  TimeLabelRecord firstRecord;

  for( int d = 0; d < recordSize; d++ )
  {
    firstRecord.add( ( GetRecordAt(1).get(d) - GetRecordAt(0).get(d) ) / DT );
  }
	
  firstRecord.setTime( GetRecordAt(0).getTime() );
  firstRecord.setLabel( GetRecordAt(0).getLabel() );
  derivRecordLog->AddRecord( firstRecord );


  // Middle
  for ( int i = 1; i < numRecords - 1; i++ )
  {

	DT = GetRecordAt(i+1).getTime() - GetRecordAt(i-1).getTime();
    TimeLabelRecord midRecord;

	for( int d = 0; d < recordSize; d++ )
	{
      midRecord.add( ( GetRecordAt(i+1).get(d) - GetRecordAt(i-1).get(d) ) / ( 2 * DT ) );
	}
	
	midRecord.setTime( GetRecordAt(i).getTime() );
	midRecord.setLabel( GetRecordAt(i).getLabel() );
	derivRecordLog->AddRecord( midRecord );

  }

  // Last
  DT = GetRecordAt(numRecords-1).getTime() - GetRecordAt(numRecords-2).getTime();
  TimeLabelRecord lastRecord;

  for( int d = 0; d < recordSize; d++ )
  {
    lastRecord.add( ( GetRecordAt(numRecords-1).get(d) - GetRecordAt(numRecords-2).get(d) ) / DT );
  }
	
  lastRecord.setTime( GetRecordAt(numRecords-1).getTime() );
  lastRecord.setLabel( GetRecordAt(numRecords-1).getLabel() );
  derivRecordLog->AddRecord( lastRecord );

  // Return the order - 1 derivative
  return derivRecordLog->Derivative( order - 1 );

}



ValueRecord vtkRecordLog
::Integrate()
{
  // The record log will only hold one record at the end
  ValueRecord intRecord;

  // Iterate over all dimensions
  for ( int d = 0; d < recordSize; d++ )
  {
    intRecord.add( 0.0 );
  }

  // For each time
  for ( int i = 1; i < numRecords - 1; i++ )
  {
	// Find the time difference
    double DT = GetRecordAt(i).getTime() - GetRecordAt(i-1).getTime();

	// Iterate over all dimensions
	for ( int d = 0; d < recordSize; d++ )
	{
	  intRecord.set( d, intRecord.get( d ) + DT * ( GetRecordAt(i).get(d) + GetRecordAt(i-1).get(d) ) / 2 );
	}

  }

  return intRecord;

}


std::vector<LabelRecord> vtkRecordLog
::LegendreTransformation( int order )
{

  // Create a copy of the current record log
  vtkRecordLog* recLogCopy = vtkRecordLog::New();  
  std::vector<LabelRecord> legVector;
  
  // Calculate the time adjustment (need range -1 to 1)
  double startTime = GetRecordAt(0).getTime();
  double endTime = GetRecordAt( numRecords - 1 ).getTime();
  double rangeTime = endTime - startTime;
  
  for ( int i = 0; i < numRecords; i++ )
  {
    TimeLabelRecord currRecord;
	currRecord = GetRecordAt(i);
	currRecord.setTime( ( currRecord.getTime() - startTime ) * 2.0 / rangeTime - 1 ); // tmin - tmax --> -1 - 1
	recLogCopy->AddRecord( currRecord );
  }

  // Create a copy of the record log for each degree of Legendre polynomial
  for ( int o = 0; o <= order; o++ )
  {
    vtkRecordLog* orderRecLogCopy = recLogCopy->DeepCopy();

    // Multiply the values by the Legendre polynomials
    for ( int i = 0; i < numRecords; i++ )
    {
	  double legPoly = LegendrePolynomial( orderRecLogCopy->GetRecordAt(i).getTime(), o );

      for ( int d = 0; d < recordSize; d++ )
	  {	    
        orderRecLogCopy->records[i].set( d, orderRecLogCopy->GetRecordAt(i).get(d) * legPoly );
	  }

    }

	// Integrate to get the Legendre coefficients for the particular order
	LabelRecord legRecord;
	legRecord.values = orderRecLogCopy->Integrate().values;
    legVector.push_back( legRecord );	
	legVector[o].setLabel( o );

  }

  return legVector;

}



double vtkRecordLog
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



vtkRecordLog* vtkRecordLog
::GaussianFilter( double width )
{
  // Assume a Gaussian filter
  vtkRecordLog* gaussRecordLog = vtkRecordLog::New();

  // For each record
  for ( int i = 0; i < numRecords; i++ )
  {
    // Create a new record valuestor
    TimeLabelRecord gaussRecord;

    // Iterate over all dimensions
	for ( int d = 0; d < recordSize; d++ )
	{
      double weightSum = 0;
      double normSum = 0;

      // Iterate over all records
      for ( int j = 0; j < numRecords; j++ )
      {
        // Calculate the values of the Gaussian distribution at this time
	    double gaussWeight = exp( - ( GetRecordAt(j).getTime() - GetRecordAt(i).getTime() ) * ( GetRecordAt(j).getTime() - GetRecordAt(i).getTime() ) / width );
		// Add the product with the values to function sum
        weightSum = weightSum + GetRecordAt(j).get(d) * gaussWeight;
		// Add the values to normSum
		normSum = normSum + gaussWeight;
      }

	  // Add to the new values
	  gaussRecord.add( weightSum / normSum );

	}

	  // Add the new record vector to the record log
	  gaussRecord.setTime( GetRecordAt(i).getTime() );
	  gaussRecord.setLabel( GetRecordAt(i).getLabel() );
      gaussRecordLog->AddRecord( gaussRecord );
  }

  return gaussRecordLog;
}



vtkRecordLog* vtkRecordLog
::OrthogonalTransformation( int window, int order )
{
  // Pad the recordlog with values at the beginning
  vtkRecordLog* padRecordLog = this->PadStart( window )->Concatenate( this );

  // Create a new record log with the orthogonally transformed data
  vtkRecordLog* orthRecordLog = vtkRecordLog::New();

  // Iterate over all data, and calculate Legendre expansion coefficients
  for ( int i = 0; i < this->numRecords; i++ )
  {
    // Calculate the record log to include
    vtkRecordLog* trimRecordLog = padRecordLog->Trim( i, i + window );
	
	// Create a new matrix to which the Legendre coefficients will be assigned
	std::vector<LabelRecord> legCoeffMatrix = trimRecordLog->LegendreTransformation( order );
	TimeLabelRecord currLegRecord;

	// Calculate the Legendre coefficients: 2D -> 1D
	for ( int o = 0; o <= order; o++ )
    {
      for ( int d = 0; d < recordSize; d++ )
	  {
        currLegRecord.add( legCoeffMatrix[o].get(d) );
	  }
    }

	// New value record to add to the record log
    currLegRecord.setTime( GetRecordAt(i).getTime() );
	currLegRecord.setLabel( GetRecordAt(i).getLabel() );
	orthRecordLog->AddRecord( currLegRecord );

  }

  return orthRecordLog;

}


vnl_matrix<double>* vtkRecordLog
::CovarianceMatrix()
{
  // Copy the current record log
  vtkRecordLog* covRecLog = this->DeepCopy();

  // Construct a recordSize by recordSize vnl matrix
  vnl_matrix<double> *cov = new vnl_matrix<double>( recordSize, recordSize );
  cov->fill( 0.0 );

  // Determine the mean, subtract the mean from each record
  ValueRecord meanRecord = this->Mean();

  for ( int i = 0; i < numRecords; i++ )
  {
    for( int d = 0; d < recordSize; d++ )
	{
	  covRecLog->records[i].set( d, covRecLog->GetRecordAt(i).get(d) - meanRecord.get(d) );
	}
  }

  // Pick two dimensions, and find their covariance
  for ( int d1 = 0; d1 < recordSize; d1++ )
  {
    for ( int d2 = 0; d2 < recordSize; d2++ )
	{
	  // Iterate over all times
	  for ( int i = 0; i < numRecords; i++ )
	  {
	    cov->put( d1, d2, cov->get( d1, d2 ) + covRecLog->GetRecordAt(i).get(d1) * covRecLog->GetRecordAt(i).get(d2) );
	  }
	  // Divide by the number of records
	  cov->put( d1, d2, cov->get( d1, d2 ) / numRecords );
	}
  }

  return cov;

}



std::vector<LabelRecord> vtkRecordLog
::CalculatePCA( int numComp )
{
  // Calculate the covariance matrix
  vnl_matrix<double>* cov = this->CovarianceMatrix();

  //Calculate the eigenvectors of the covariance matrix
  vnl_matrix<double> eigenvectors( cov->rows(), cov->cols(), 0.0 );
  vnl_vector<double> eigenvalues( cov->rows(), 0.0 );
  vnl_symmetric_eigensystem_compute( *cov, eigenvectors, eigenvalues );

  // Grab only the most important eigenvectors
  std::vector<LabelRecord> prinComps;

  for ( int i = 0; i < numComp; i++ )
  {

    // Add a new principal component
    LabelRecord currPrinComp;
    
    for ( int j = 0; j < recordSize; j++ )
	{
	  currPrinComp.add( eigenvectors.get( j, i ) );
	}

	currPrinComp.setLabel( i );
	prinComps.push_back( currPrinComp );

  }

  return prinComps;

}



vtkRecordLog* vtkRecordLog
::TransformPCA( std::vector<LabelRecord> prinComps, ValueRecord mean )
{
  // Record log with the PCA transformed data
  vtkRecordLog* pcaRecLog = vtkRecordLog::New();

  // Iterate over all time stamps
  for( int i = 0; i < numRecords; i++ )
  {
    // Create a TimeLabelRecord for the transformed record log
    TimeLabelRecord transRecord;

    // Initialize the components of the transformed time record to be zero
	for ( int o = 0; o < prinComps.size(); o++ )
	{
      transRecord.add( 0.0 );
	  
	  // Iterate over all dimensions, and perform the transformation (ie vector multiplcation)
      for ( int d = 0; d < recordSize; d++ )
	  {
        transRecord.set( o, transRecord.get(o) + ( GetRecordAt(i).get(d) - mean.get(d) ) * prinComps[o].get(d) );
	  }
	}

    // Add the time record to the new transformed record log
    transRecord.setTime( GetRecordAt(i).getTime() );
	transRecord.setLabel( GetRecordAt(i).getLabel() );
    pcaRecLog->AddRecord( transRecord );

  }

  return pcaRecLog;

}



std::vector<LabelRecord> vtkRecordLog
::fwdkmeans( int numClusters )
{
  // Create a new vector of centroids
  std::vector<LabelRecord> centroids;

  // A vector of cluster memberships
  std::vector<int> membership;
  for ( int i = 0; i < numRecords; i++ )
  {
    membership.push_back( 0 );
  }

  // Iterate until all of the clusters have been added
  for ( int k = 0; k < numClusters; k++ )
  {

	// Use the mean of all points for the first clusters
    if ( k == 0 )
	{
	  // An order record for the current cluster
      LabelRecord currCentroid;
      currCentroid.values = this->Mean().values;
	  // Add the current centroid to the vector of centroids
	  currCentroid.setLabel( k );
	  centroids.push_back( currCentroid );
	  continue;
	}

	centroids = this->AddNextCentroid( centroids );

	// Iterate until there are no more changes in membership and no clusters are empty
    bool change = true;
	while ( change )
	{
	  // Reassign the cluster memberships
	  std::vector<int> newMembership = this->ReassignMembership( centroids );

	  // Calculate change
	  for ( int i = 0; i < membership.size(); i++ )
	  {
        if ( membership[i] == newMembership[i] )
		{
          change = false;
		}
	  }
	  membership = newMembership;

	  // Remove emptiness
      centroids = this->MoveEmptyClusters( centroids, membership );


	  // Recalculate centroids
      centroids = this->RecalculateCentroids( membership, k + 1 );

	}

  }

  return centroids;

}


std::vector<LabelRecord> vtkRecordLog
::AddNextCentroid( std::vector<LabelRecord> centroids )
{

  // Find the record farthest from any centroid
  // Tricky way to cast vector of LabelRecord to vector of ValeuRecord
  std::vector<LabelRecord> centDist = this->Distances( std::vector<ValueRecord>( centroids.begin(), centroids.end() ) );
	
  int candidateRecord = 0;
  double candidateDistance = centDist[0].get(0);

  for ( int i = 0; i < numRecords; i++ )
  {
    double currMinDist = centDist[i].get(0);
    // Minimum for each point
    for ( int c = 0; c < centroids.size(); c++ )
	{
      if ( centDist[i].get(c) < currMinDist )
	  {
        currMinDist = centDist[i].get(c);
	  }
	}
	
	// Maximum of the minimums
    if ( currMinDist < candidateDistance )
    {
      candidateDistance = currMinDist;
      candidateRecord = i;
	}
  }

  // Add the candidate point to the list of centroids
  LabelRecord currCentroid;
  currCentroid.values = this->records[candidateRecord].values;
  currCentroid.setLabel( centroids.size() );
  centroids.push_back( currCentroid );

  return centroids;

}


std::vector<LabelRecord> vtkRecordLog
::MoveEmptyClusters( std::vector<LabelRecord> centroids, std::vector<int> membership )
{
  std::vector<bool> emptyVector;

  // Calculate the empty clusters
  for ( int c = 0; c < centroids.size(); c++ )
  {
    emptyVector.push_back(true);
  }
  for ( int i = 0; i < numRecords; i++ )
  {
    emptyVector[ membership[i] ] = false;
  }
	  
  // Remove any emptyness
  for ( int c = 0; c < centroids.size(); c++ )
  {
    if ( emptyVector[c] == false )
	{
	  continue;
	}

    // Find the record farthest from any centroid
    // Tricky way to cast vector of LabelRecord to vector of ValeuRecord
    std::vector<LabelRecord> centDist = this->Distances( std::vector<ValueRecord>( centroids.begin(), centroids.end() ) );
	
    int candidateRecord = 0;
    double candidateDistance = centDist[0].get(0);

    for ( int i = 0; i < numRecords; i++ )
    {
      double currMinDist = centDist[i].get(0);
      // Minimum for each point
      for ( int c = 0; c < centroids.size(); c++ )
	  {
        if ( centDist[i].get(c) < currMinDist )
	    {
          currMinDist = centDist[i].get(c);
	    }
	  }
	
	  // Maximum of the minimums
      if ( currMinDist < candidateDistance )
      {
        candidateDistance = currMinDist;
        candidateRecord = i;
	  }
    }

	// Change the empty centroid
    LabelRecord currCentroid;
    currCentroid.values = this->records[candidateRecord].values;
    currCentroid.setLabel( c );
    centroids[c] = currCentroid;

  }

  return centroids;

}


std::vector<int> vtkRecordLog
::ReassignMembership( std::vector<LabelRecord> centroids )
{

	
  // Find the record farthest from any centroid
  // Tricky way to cast vector of LabelRecord to vector of ValeuRecord
  std::vector<LabelRecord> centDist = this->Distances( std::vector<ValueRecord>( centroids.begin(), centroids.end() ) );
  
  std::vector<int> membership;

  for ( int i = 0; i < numRecords; i++ )
  {
    double currMinDist = centDist[i].get(0);
	int currMinCentroid = 0;
    // Minimum for each point
    for ( int c = 0; c < centroids.size(); c++ )
	{
      if ( centDist[i].get(c) < currMinDist )
	  {
        currMinDist = centDist[i].get(c);
		currMinCentroid = c;
	  }
	}
    membership.push_back( currMinCentroid );
  }
	
  return membership;
}



std::vector<LabelRecord> vtkRecordLog
::RecalculateCentroids( std::vector<int> membership, int numClusters )
{

  // For each cluster, have an order record and a count
  std::vector<LabelRecord> centroids;
  std::vector<int> memberCount;

  // Initialize our vector of centroids, since we know the number of clusters
  for ( int c = 0; c < numClusters; c++ )
  {
    LabelRecord blankRecord;
	for ( int d = 0; d < recordSize; d++ )
	{
	  blankRecord.add( 0.0 );
	}
	blankRecord.setLabel( c );
	centroids.push_back( blankRecord );
	memberCount.push_back( 0 );
  }


  // Iterate over all time
  for ( int i = 0; i < numRecords; i++ )
  {
    // For each dimension
    for ( int d = 0; d < recordSize; d++ )
	{
	  centroids[ membership[i] ].set( d, centroids[ membership[i] ].get(d) + GetRecordAt(i).get(d) );
	}
    memberCount[ membership[i] ] = memberCount[ membership[i] ] + 1;
  }

  // Divide by the number of records in the cluster to get the mean
  for ( int c = 0; c < numClusters; c++ )
  {
    // For each dimension
    for ( int d = 0; d < recordSize; d++ )
	{
	  centroids[c].set( d, centroids[c].get(d) / memberCount[c] );
	}
  }

  return centroids;

}



vtkRecordLog* vtkRecordLog
::fwdkmeansTransform( std::vector<LabelRecord> centroids )
{
  // Use the reassign membership function to calculate closest centroids
  std::vector<int> membership = this->ReassignMembership( centroids );

  // Create a copy with the record values replaced by the membership label
  vtkRecordLog* clusterRecordLog = vtkRecordLog::New();

  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = 0; i < numRecords; i++ )
  {
	TimeLabelRecord currRecord;
	currRecord.add( membership[i] );
	currRecord.setTime( GetRecordAt(i).getTime() );
	currRecord.setLabel( GetRecordAt(i).getLabel() );
    clusterRecordLog->AddRecord( currRecord );
  }
  
  return clusterRecordLog;

}


std::vector<vtkRecordLog*> vtkRecordLog
::GroupRecordsByLabel( int MaxLabel )
{
  std::vector<vtkRecordLog*> recordsByTask;

  // For each label, create a new recordLog
  for ( int i = 0; i < MaxLabel; i++ )
  {
	recordsByTask.push_back( vtkRecordLog::New() );
  }

  // Iterate over all records
  for ( int i = 0; i < this->Size(); i++ )
  {
    recordsByTask[ this->GetRecordAt(i).getLabel() ]->AddRecord( this->GetRecordAt(i) );
  }

  return recordsByTask;

}


std::vector<MarkovRecord> vtkRecordLog
::ToMarkovRecordVector()
{
  std::vector<MarkovRecord> markovRecords;

  // We will assume that: label -> state, values[0] -> symbol
  for ( int i = 0; i < this->Size(); i++ )
  {
    MarkovRecord currMarkovRecord;
    currMarkovRecord.setState( this->GetRecordAt(i).getLabel() );
	currMarkovRecord.setSymbol( this->GetRecordAt(i).get(0) );
	markovRecords.push_back( currMarkovRecord );
  }

  return markovRecords;

}