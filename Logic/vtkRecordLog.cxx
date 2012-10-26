
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
	TimeRecord currRecord;

	for ( int d = 0; d < recordSize; d++ )
	{
      currRecord.add( records[i].get(d) );
	}

	currRecord.time = records[i].time;
    newRecordLog->AddRecord( currRecord );

  }
  
  return newRecordLog;
}


void vtkRecordLog
::AddRecord( TimeRecord newRecord )
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


TimeRecord vtkRecordLog
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
  for ( int i = start; i < end; i++ )
  {

	TimeRecord currRecord;

	for ( int d = 0; d < recordSize; d++ )
	{
      currRecord.add( records[i].get(d) );
	}

	currRecord.time = records[i].time;
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
::PadStart( int window )
{
  // Create a new record log of padded values only
  vtkRecordLog* padRecordLog = vtkRecordLog::New();

  // Find the average time stamp
  double DT = ( records[0].time - records[numRecords-1].time ) / numRecords;

  // Calculate the values and time stamp
  for ( int i = window; i > 0; i-- )
  {

    double currTime = records[0].time - i * DT;
	TimeRecord padRecord;

	for ( int d = 0; d < recordSize; d++ )
	{
	   padRecord.add( records[0].get(d) );
	}

	padRecord.time = currTime;
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
	  meanRecord.set( d, meanRecord.get( d ) + records[i].get(d) );
	}
  }

  // Divide by the number of records
  for ( int d = 0; d < recordSize; d++ )
  {
    meanRecord.set( d, meanRecord.get(d) / numRecords );
  }

  return meanRecord;
}



std::vector<OrderRecord> vtkRecordLog
::Distances( vtkRecordLog* otherRecLog )
{
  // Create a vector of order records
  std::vector<OrderRecord> dists;

  // First, ensure that the records are the same size
  if ( this->recordSize != otherRecLog->recordSize )
  {
    return dists;
  }

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < numRecords; i++ )
  {
    // Create a new order record
    OrderRecord distRecord;

    for ( int j = 0; j < otherRecLog->numRecords; j++ )
	{
      // Initialize the sum to zero
	  double currSum = 0.0;
	  for ( int d = 0; d < recordSize; d++ )
	  {
        currSum = currSum + ( this->records[i].get(d) - otherRecLog->records[j].get(d) ) * ( this->records[i].get(d) - otherRecLog->records[j].get(d) );
	  }
	  // Add to the current order record
	  distRecord.add( currSum );
	}

	// Add the current order record to the vector
	distRecord.order = i;
	dists.push_back( distRecord );

  }

  return dists;

}



std::vector<OrderRecord> vtkRecordLog
::Distances( std::vector<ValueRecord> valueRecords )
{
  // Create a vector of order records
  std::vector<OrderRecord> dists;

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < numRecords; i++ )
  {
    // Create a new order record
    OrderRecord distRecord;

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
        currSum = currSum + ( this->records[i].get(d) - valueRecords[j].get(d) ) * ( this->records[i].get(d) - valueRecords[i].get(d) );
	  }
	  // Add to the current order record
	  distRecord.add( currSum );
	}

	// Add the current order record to the vector
	distRecord.order = i;
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
  DT = records[1].time - records[0].time;
  TimeRecord firstRecord;

  for( int d = 0; d < recordSize; d++ )
  {
    firstRecord.add( ( records[1].get(d) - records[0].get(d) ) / DT );
  }
	
  firstRecord.time = records[0].time;
  derivRecordLog->AddRecord( firstRecord );


  // Middle
  for ( int i = 1; i < numRecords - 1; i++ )
  {

	DT = records[i+1].time - records[i-1].time;
    TimeRecord midRecord;

	for( int d = 0; d < recordSize; d++ )
	{
      midRecord.add( ( records[i+1].get(d) - records[i-1].get(d) ) / ( 2 * DT ) );
	}
	
	midRecord.time = records[i].time;
	derivRecordLog->AddRecord( midRecord );

  }

  // Last
  DT = records[numRecords-1].time - records[numRecords-2].time;
  TimeRecord lastRecord;

  for( int d = 0; d < recordSize; d++ )
  {
    lastRecord.add( ( records[numRecords-1].get(d) - records[numRecords-2].get(d) ) / DT );
  }
	
  lastRecord.time = records[numRecords-1].time;
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
  for ( int i = 0; i < numRecords - 1; i++ )
  {
	// Find the time difference
    double DT = records[i+1].time - records[i-1].time;

	// Iterate over all dimensions
	for ( int d = 0; d < recordSize; d++ )
	{
	  intRecord.set( d, intRecord.get( d ) + DT * ( records[i].get(d) + records[i].get(d) ) / 2 );
	}

  }

  return intRecord;

}


std::vector<OrderRecord> vtkRecordLog
::LegendreTransformation( int order )
{

  // Create a copy of the current record log
  vtkRecordLog* recLogCopy = this->DeepCopy();  
  std::vector<OrderRecord> legVector;
  
  // Calculate the time adjustment (need range -1 to 1)
  double startTime = recLogCopy->records[0].time;
  double endTime = recLogCopy->records[numRecords-1].time;
  double rangeTime = endTime - startTime;
  
  for ( int i = 0; i < numRecords; i++ )
  {
    recLogCopy->records[i].time -= startTime; // ( 0 to tmax )
    recLogCopy->records[i].time *= 2.0 / rangeTime; // ( 0 to 2 )
	recLogCopy->records[i].time -= 1; // ( -1 to 1 )
  }

  // Create a copy of the record log for each degree of Legendre polynomial
  for ( int o = 0; o <= order; o++ )
  {
    vtkRecordLog* orderRecLogCopy = recLogCopy->DeepCopy();

    // Multiply the values by the Legendre polynomials
    for ( int i = 0; i < numRecords; i++ )
    {
      for ( int d = 0; d < recordSize; d++ )
	  {
	    double legPoly = LegendrePolynomial( orderRecLogCopy->records[i].time, order );
        orderRecLogCopy->records[i].set( d, orderRecLogCopy->records[i].get(d) * legPoly );
	  }
    }

	// Integrate to get the Legendre coefficients for the particular order
	OrderRecord legRecord;
	legRecord.values = orderRecLogCopy->Integrate().values;
	legVector[o].order = o;
    legVector.push_back( legRecord );
	

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
    TimeRecord gaussRecord;
	std::vector<double> gaussValues;

    // Iterate over all dimensions
	for ( int d = 0; d < recordSize; d++ )
	{
      double weightSum = 0;
      double normSum = 0;

      // Iterate over all records
      for ( int j = 0; j < numRecords; j++ )
      {
        // Calculate the values of the Gaussian distribution at this time
	    double gaussWeight = exp( - ( records[j].time - records[i].time ) * ( records[j].time - records[i].time ) / width );
		// Add the product with the values to function sum
        weightSum = weightSum + records[j].get(d) * gaussWeight;
		// Add the values to normSum
		normSum = normSum + gaussWeight;
      }

	  // Add to the new values
	  gaussValues.push_back( weightSum / normSum );

	}

	  // Add the new record valuestor to the record log
	  gaussRecord.values = gaussValues;
	  gaussRecord.time = records[i].time;
      gaussRecordLog->AddRecord( gaussRecord );
  }

  return gaussRecordLog;
}


vtkRecordLog* vtkRecordLog
::OrthogonalTransformation( int window, int order )
{
  // Pad the recordlog with values at the beginning
  vtkRecordLog* padRecordLog = this->PadStart( window );

  // Create a new record log with the orthogonally transformed data
  vtkRecordLog* orthRecordLog = vtkRecordLog::New();

  // Iterate over all data, and calculate Legendre expansion coefficients
  for ( int i = 0; i < this->numRecords; i++ )
  {
    // Calculate the record log to include
    vtkRecordLog* trimRecordLog = padRecordLog->Trim( i, i + window );
	
	// Create a new matrix to which the Legendre coefficients will be assigned
	std::vector<OrderRecord> legVector= trimRecordLog->LegendreTransformation( order );
	std::vector<double> currLegCoeffOne;

	// Calculate the Legendre coefficients
	for ( int o = 0; o <= order; o++ )
    {
      for ( int d = 0; d < recordSize; d++ )
	  {
        currLegCoeffOne.push_back( legVector[o].get(d) );
	  }
    }

	// New value record to add to the record log
	TimeRecord currLegRecord;
    currLegRecord.time = this->records[i].time;
    currLegRecord.values = currLegCoeffOne;
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
	  covRecLog->records[i].set( d, covRecLog->records[i].get(d) - meanRecord.get(d) );
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
	    cov->put( d1, d2, cov->get( d1, d2 ) + covRecLog->records[i].get(d1) * covRecLog->records[i].get(d2) );
	  }
	  // Divide by the number of records
	  cov->put( d1, d2, cov->get( d1, d2 ) / numRecords );
	}
  }

  return cov;

}



std::vector<OrderRecord> vtkRecordLog
::CalculatePCA( int numComp )
{
  // Calculate the covariance matrix
  vnl_matrix<double>* cov = this->CovarianceMatrix();

  //Calculate the eigenvectors of the covariance matrix
  vnl_matrix<double> eigenvectors;
  vnl_vector<double> eigenvalues;
  vnl_symmetric_eigensystem_compute( *cov, eigenvectors, eigenvalues );

  // Grab only the most important eigenvectors
  std::vector<OrderRecord> prinComps;
  for ( int i = 0; i < numComp; i++ )
  {

    // Add a new principal component
    OrderRecord currPrinComp;
    
    for ( int j = 0; j < recordSize; j++ )
	{
	  currPrinComp.add( eigenvectors.get( j, i ) );
	}

	currPrinComp.order = i;
	prinComps.push_back( currPrinComp );

  }

  return prinComps;

}



vtkRecordLog* vtkRecordLog
::TransformPCA( std::vector<OrderRecord> prinComps )
{
  // Record log with the PCA transformed data
	vtkRecordLog* pcaRecLog = vtkRecordLog::New();

  // Iterate over all time stamps
  for( int i = 0; i < numRecords; i++ )
  {
    // Create a TimeRecord for the transformed record log
    TimeRecord transRecord;

    // Initialize the components of the transformed time record to be zero
	for ( int o = 0; o < prinComps.size(); o++ )
	{
      transRecord.add( 0.0 );
	  
	  // Iterate over all dimensions, and perform the transformation
      for ( int d = 0; d < recordSize; d++ )
	  {
        transRecord.set( o, transRecord.get(o) + this->records[i].get(d) * prinComps[o].get(d) );
	  }
	}

    // Add the time record to the new transformed record log
    transRecord.time = this->records[i].time;
    pcaRecLog->AddRecord( transRecord );

  }

  return pcaRecLog;

}



std::vector<OrderRecord> vtkRecordLog
::fwdkmeans( int numClusters, ValueRecord weights )
{
  // Create a new vector of centroids
  std::vector<OrderRecord> centroids;

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
      OrderRecord currCentroid;
      currCentroid.values = this->Mean().values;
	  // Add the current centroid to the vector of centroids
	  currCentroid.order = k;
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
        if ( membership[i] != newMembership[i] )
		{
          change = true;
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


std::vector<OrderRecord> vtkRecordLog
::AddNextCentroid( std::vector<OrderRecord> centroids )
{

  // Find the record farthest from any centroid
  // Tricky way to cast vector of OrderRecord to vector of ValeuRecord
  std::vector<OrderRecord> centDist = this->Distances( std::vector<ValueRecord>( centroids.begin(), centroids.end() ) );
	
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
  OrderRecord currCentroid;
  currCentroid.values = this->records[candidateRecord].values;
  currCentroid.order = centroids.size();
  centroids.push_back( currCentroid );

  return centroids;

}


std::vector<OrderRecord> vtkRecordLog
::MoveEmptyClusters( std::vector<OrderRecord> centroids, std::vector<int> membership )
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
	  break;
	}

    // Find the record farthest from any centroid
    // Tricky way to cast vector of OrderRecord to vector of ValeuRecord
    std::vector<OrderRecord> centDist = this->Distances( std::vector<ValueRecord>( centroids.begin(), centroids.end() ) );
	
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
    OrderRecord currCentroid;
    currCentroid.values = this->records[candidateRecord].values;
    currCentroid.order = centroids.size();
    centroids[c] = currCentroid;

  }

  return centroids;

}


std::vector<int> vtkRecordLog
::ReassignMembership( std::vector<OrderRecord> centroids )
{

	
  // Find the record farthest from any centroid
  // Tricky way to cast vector of OrderRecord to vector of ValeuRecord
  std::vector<OrderRecord> centDist = this->Distances( std::vector<ValueRecord>( centroids.begin(), centroids.end() ) );
  
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



std::vector<OrderRecord> vtkRecordLog
::RecalculateCentroids( std::vector<int> membership, int numClusters )
{

  // For each cluster, have an order record and a count
  std::vector<OrderRecord> centroids;
  std::vector<int> memberCount;

  // Initialize our vector of centroids, since we know the number of clusters
  for ( int c = 0; c < numClusters; c++ )
  {
    OrderRecord blankRecord;
	for ( int d = 0; d < recordSize; d++ )
	{
	  blankRecord.add( 0.0 );
	}
	blankRecord.order = c;
	centroids.push_back( blankRecord );
  }


  // Iterate over all time
  for ( int i = 0; i < numRecords; i++ )
  {
    // For each dimension
    for ( int d = 0; d < recordSize; d++ )
	{
	  centroids[ membership[i] ].set( d, centroids[ membership[i] ].get(d) + this->records[i].get(d) );
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