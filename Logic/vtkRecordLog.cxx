
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
  newRecordLog->Initialize( this->numRecords, this->recordSize );

  TimeLabelRecord currRecord;
  currRecord.initialize( this->recordSize, 0.0 );

  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = 0; i < this->numRecords; i++ )
  {

	for ( int d = 0; d < this->recordSize; d++ )
	{
      currRecord.set( d, this->GetRecordAt(i).get(d) );
	}

	currRecord.setTime( this->GetRecordAt(i).getTime() );
	currRecord.setLabel( this->GetRecordAt(i).getLabel() );
    newRecordLog->SetRecord( i, currRecord );

  }
  
  return newRecordLog;
}



void vtkRecordLog
::Initialize( int initNumRecords, int initRecordSize )
{
  // Create an empty time label record
  TimeLabelRecord emptyRecord;
  emptyRecord.initialize( initRecordSize, 0.0 );
  emptyRecord.setLabel( 0 );
  emptyRecord.setTime( 0.0 );

  //Set record to be a vector of these empty records
  records = std::vector<TimeLabelRecord>( initNumRecords, emptyRecord );

  numRecords = initNumRecords;
  recordSize = initRecordSize;

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


void vtkRecordLog
::SetRecord( int index, TimeLabelRecord newRecord )
{
  // Only do something if the index and size are valid
  if ( index >= 0 && index < numRecords && newRecord.size() == recordSize )
  {
    records[index] = newRecord;
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


int vtkRecordLog
::RecordSize()
{
  return recordSize;
}



vtkRecordLog* vtkRecordLog
::Trim( int start, int end )
{

  // Include the endpoints
  vtkRecordLog* trimRecordLog = vtkRecordLog::New();
  trimRecordLog->Initialize( end - start + 1, this->recordSize );

  TimeLabelRecord currRecord;
  currRecord.initialize( this->recordSize, 0.0 );

  // Iterate over all tracking records in the current procedure, and add to new
  int count = 0;
  for ( int i = start; i <= end; i++ )
  {
	for ( int d = 0; d < this->recordSize; d++ )
	{
      currRecord.set( d, this->GetRecordAt(i).get(d) );
	}

	currRecord.setTime( this->GetRecordAt(i).getTime() );
	currRecord.setLabel( this->GetRecordAt(i).getLabel() );
    trimRecordLog->SetRecord( count, currRecord );
	count++;

  }

  return trimRecordLog;

}



vtkRecordLog* vtkRecordLog
::Concatenate( vtkRecordLog* otherRecordLog )
{
  // First, deep copy the current record log and the concatenating one, so two record logs do not have access to the same record object
  vtkRecordLog* currRecordLogCopy;
  vtkRecordLog* otherRecordLogCopy;

  currRecordLogCopy = this->DeepCopy();
  otherRecordLogCopy = otherRecordLog->DeepCopy();

  // Create a new record log and assign the copies to it
  vtkRecordLog* catRecordLog = vtkRecordLog::New();

  // Ensure that the record size are the same
  if ( this->recordSize != otherRecordLog->recordSize )
  {
    return currRecordLogCopy;
  }

  catRecordLog->Initialize( this->numRecords + otherRecordLog->numRecords, this->recordSize );

  int count = 0;
  for( int i = 0; i < currRecordLogCopy->numRecords; i++ )
  {
    catRecordLog->SetRecord( count, currRecordLogCopy->records[i] );
	count++;
  }

  for( int i = 0; i < otherRecordLogCopy->numRecords; i++ )
  {
    catRecordLog->SetRecord( count, otherRecordLogCopy->records[i] );
	count++;
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
  catRecordLog->Initialize( this->numRecords, this->recordSize + otherRecordLog->recordSize );

  TimeLabelRecord currRecord;
  currRecord.initialize( this->recordSize + otherRecordLog->recordSize, 0.0 );

  for ( int i = 0; i < this->numRecords; i++ )
  {
	int count = 0;
    for ( int d = 0; d < this->recordSize; d++ )
	{
      currRecord.set( count, this->GetRecordAt(i).get(d) );
	  count++;
	}
	for ( int d = 0; d < otherRecordLog->recordSize; d++ )
	{
	  currRecord.set( count, otherRecordLog->GetRecordAt(i).get(d) );
	  count++;
	}

	currRecord.setTime( this->GetRecordAt(i).getTime() );
	currRecord.setLabel( this->GetRecordAt(i).getLabel() );
    catRecordLog->SetRecord( i, currRecord );
  }

  return catRecordLog;
}


vtkRecordLog* vtkRecordLog
::ConcatenateValues( ValueRecord record )
{

  // Iterate over all records in log and stick them together
  vtkRecordLog* catRecordLog = vtkRecordLog::New();
  catRecordLog->Initialize( this->numRecords, this->recordSize + record.size() );

  TimeLabelRecord currRecord;
  currRecord.initialize( this->recordSize + record.size(), 0.0 );

  for ( int i = 0; i < this->numRecords; i++ )
  {
    int count = 0;
    for ( int d = 0; d < this->recordSize; d++ )
	{
      currRecord.set( count, this->GetRecordAt(i).get(d) );
	  count++;
	}
	for ( int d = 0; d < record.size(); d++ )
	{
	  currRecord.set( count, record.get(d) );
	  count++;
	}

	currRecord.setTime( this->GetRecordAt(i).getTime() );
	currRecord.setLabel( this->GetRecordAt(i).getLabel() );
    catRecordLog->SetRecord( i, currRecord );
  }

  return catRecordLog;
}



// Note: Does not add to start, just creates start - must concatenate it
vtkRecordLog* vtkRecordLog
::PadStart( int window )
{
  // Create a new record log of padded values only
  vtkRecordLog* padRecordLog = vtkRecordLog::New();
  padRecordLog->Initialize( window, this->recordSize );

  TimeLabelRecord padRecord;
  padRecord.initialize( this->recordSize, 0.0 );

  // Find the average time stamp
  // Divide by numRecords - 1 because there are one fewer differences than there are stamps
  double DT = 1.0; // TODO: Get a more intelligible default value (not a magic number)
  if ( numRecords > 1 )
  {
    DT = ( GetRecordAt(numRecords-1).getTime() - GetRecordAt(0).getTime() ) / ( numRecords - 1 );
  }

  // Calculate the values and time stamp
  for ( int i = 0; i < window; i++ )
  {

	for ( int d = 0; d < recordSize; d++ )
	{
	   padRecord.set( d, this->GetRecordAt(0).get(d) );
	}

	padRecord.setTime( this->GetRecordAt(0).getTime() - ( i + 1 ) * DT );
	padRecord.setLabel( this->GetRecordAt(0).getLabel() );
	padRecordLog->SetRecord( window - ( i + 1 ), padRecord );

  }

  return padRecordLog;
}


ValueRecord vtkRecordLog
::Mean()
{
  // The record log will only hold one record at the end
  ValueRecord meanRecord;
  meanRecord.initialize( this->recordSize, 0.0 );

  // For each time
  for ( int i = 0; i < this->numRecords; i++ )
  {
	// Iterate over all dimensions
	for ( int d = 0; d < this->recordSize; d++ )
	{
	  meanRecord.set( d, meanRecord.get( d ) + GetRecordAt(i).get(d) );
	}
  }

  // Divide by the number of records
  for ( int d = 0; d < this->recordSize; d++ )
  {
    meanRecord.set( d, meanRecord.get(d) / numRecords );
  }

  return meanRecord;
}



std::vector<LabelRecord> vtkRecordLog
::Distances( vtkRecordLog* otherRecLog )
{

   // Create a vector of order records
  LabelRecord distRecord;
  std::vector<LabelRecord> dists ( numRecords, distRecord );  

  // First, ensure that the records are the same size
  if ( this->recordSize != otherRecLog->recordSize )
  {
    return dists;
  }
  
  double currSum;
  double currDiff;

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < numRecords; i++ )
  {
    // Initialize values, so we don't change its size so many times
	distRecord.initialize( otherRecLog->Size(), 0.0 );

    for ( int j = 0; j < otherRecLog->Size(); j++ )
	{
      
      // Initialize the sum to zero
	  currSum = 0.0;
	  for ( int d = 0; d < recordSize; d++ )
	  {
	    currDiff = this->GetRecordAt(i).get(d) - otherRecLog->GetRecordAt(j).get(d);
        currSum += currDiff * currDiff;
	  }
	  // Add to the current order record
	  distRecord.set( j, sqrt( currSum ) );
	}

	// Add the current order record to the vector
	distRecord.setLabel( i );
	dists[i] = distRecord;

  }

  return dists;

}



std::vector<LabelRecord> vtkRecordLog
::Distances( std::vector<ValueRecord> valueRecords )
{
  // Create a vector of order records
  LabelRecord distRecord;
  std::vector<LabelRecord> dists ( numRecords, distRecord );  
  
  double currSum;
  double currDiff;

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < numRecords; i++ )
  {
    // Initialize values, so we don't change its size so many times
	distRecord.initialize( valueRecords.size(), 0.0 );

    for ( int j = 0; j < valueRecords.size(); j++ )
	{
      
      // First, ensure that the records are the same size
      if ( this->recordSize != valueRecords[j].size() )
      {
        return dists;
      }

      // Initialize the sum to zero
	  currSum = 0.0;
	  for ( int d = 0; d < recordSize; d++ )
	  {
	    currDiff = this->GetRecordAt(i).get(d) - valueRecords[j].get(d);
        currSum += currDiff * currDiff;
	  }
	  // Add to the current order record
	  distRecord.set( j, sqrt( currSum ) );
	}

	// Add the current order record to the vector
	distRecord.setLabel( i );
	dists[i] = distRecord;

  }

  return dists;

}


int vtkRecordLog
::ClosestRecord( ValueRecord valueRecord )
{
  if ( this->recordSize != valueRecord.size() )
  {
    return -1;
  }

  // Calculate the distance to this point
  std::vector<ValueRecord> valueRecords ( 1, valueRecord );
  std::vector<LabelRecord> dist = this->Distances( valueRecords );

  // Now find the closest point
  double minDist = dist[0].get( 0 );
  int minRecord = 0;
  for ( int i = 0; i < this->numRecords; i++ )
  {
    if ( dist[i].get( 0 ) < minDist )
	{
      minDist = dist[i].get( 0 );
	  minRecord = i;
	}
  }

  return minRecord;

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
  derivRecordLog->Initialize( this->numRecords, this->recordSize );

  TimeLabelRecord derivRecord;
  derivRecord.initialize( this->recordSize, 0.0 );

  double DT;

  // First
  DT = this->GetRecordAt(1).getTime() - this->GetRecordAt(0).getTime();

  for( int d = 0; d < this->recordSize; d++ )
  {
    derivRecord.set( d, ( this->GetRecordAt(1).get(d) - this->GetRecordAt(0).get(d) ) / DT );
  }
	
  derivRecord.setTime( this->GetRecordAt(0).getTime() );
  derivRecord.setLabel( this->GetRecordAt(0).getLabel() );
  derivRecordLog->SetRecord( 0, derivRecord );


  // Middle
  for ( int i = 1; i < this->numRecords - 1; i++ )
  {

	DT = this->GetRecordAt(i+1).getTime() - this->GetRecordAt(i-1).getTime();

	for( int d = 0; d < this->recordSize; d++ )
	{
      derivRecord.set( d, ( this->GetRecordAt(i+1).get(d) - this->GetRecordAt(i-1).get(d) ) / DT );
	}
	
	derivRecord.setTime( this->GetRecordAt(i).getTime() );
	derivRecord.setLabel( this->GetRecordAt(i).getLabel() );
	derivRecordLog->SetRecord( i, derivRecord );

  }


  // Last
  DT = this->GetRecordAt(numRecords-1).getTime() - this->GetRecordAt(numRecords-2).getTime();

  for( int d = 0; d < this->recordSize; d++ )
  {
    derivRecord.set( d, ( this->GetRecordAt(numRecords-1).get(d) - this->GetRecordAt(numRecords-2).get(d) ) / DT );
  }
	
  derivRecord.setTime( this->GetRecordAt(numRecords-1).getTime() );
  derivRecord.setLabel( this->GetRecordAt(numRecords-1).getLabel() );
  derivRecordLog->SetRecord( this->numRecords - 1, derivRecord );

  // Return the order - 1 derivative
  return derivRecordLog->Derivative( order - 1 );

}



ValueRecord vtkRecordLog
::Integrate()
{
  // The record log will only hold one record at the end
  ValueRecord intRecord;
  intRecord.initialize( this->recordSize, 0.0 );

  // For each time
  for ( int i = 1; i < numRecords; i++ )
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
  recLogCopy->Initialize( this->numRecords, this->recordSize );

  LabelRecord legRecord;
  legRecord.initialize( recordSize, 0.0 );
  std::vector<LabelRecord> legVector( order + 1, legRecord );

  TimeLabelRecord currRecord;

  // Calculate the time adjustment (need range -1 to 1)
  double startTime = GetRecordAt(0).getTime();
  double endTime = GetRecordAt( numRecords - 1 ).getTime();
  double rangeTime = endTime - startTime;
  
  for ( int i = 0; i < numRecords; i++ )
  {
	currRecord = GetRecordAt(i);
	currRecord.setTime( ( currRecord.getTime() - startTime ) * 2.0 / rangeTime - 1 ); // tmin - tmax --> -1 - 1
	recLogCopy->SetRecord( i, currRecord );
  }

  // Create a copy of the record log for each degree of Legendre polynomial
  for ( int o = 0; o <= order; o++ )
  {
	vtkRecordLog* orderRecLogCopy = vtkRecordLog::New();
    orderRecLogCopy->Initialize( this->numRecords, this->recordSize );
	currRecord.initialize( this->recordSize, 0.0 );

    // Multiply the values by the Legendre polynomials
    for ( int i = 0; i < numRecords; i++ )
    {
	  double legPoly = LegendrePolynomial( recLogCopy->GetRecordAt(i).getTime(), o );

      for ( int d = 0; d < recordSize; d++ )
	  {	    
        currRecord.set( d, recLogCopy->GetRecordAt(i).get(d) * legPoly );
	  }

	  currRecord.setTime( recLogCopy->GetRecordAt(i).getTime() );
	  currRecord.setLabel( recLogCopy->GetRecordAt(i).getLabel() );

	  orderRecLogCopy->SetRecord( i, currRecord );

    }

	// Integrate to get the Legendre coefficients for the particular order
	legVector[o].values = orderRecLogCopy->Integrate().values;
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
  gaussRecordLog->Initialize( this->numRecords, this->recordSize );

  TimeLabelRecord gaussRecord;

  // For each record
  for ( int i = 0; i < numRecords; i++ )
  {
    // Create a new record    
	gaussRecord.initialize( recordSize, 0.0 );

    // Iterate over all dimensions
	for ( int d = 0; d < recordSize; d++ )
	{
      double weightSum = 0;
      double normSum = 0;

      // Iterate over all records
      for ( int j = 0; j < numRecords; j++ )
      {
        // Calculate the values of the Gaussian distribution at this time
	    double gaussWeight = exp( - ( ( GetRecordAt(j).getTime() - GetRecordAt(i).getTime() ) / width ) * ( ( GetRecordAt(j).getTime() - GetRecordAt(i).getTime() ) / width ) / 2 );
		// Add the product with the values to function sum
        weightSum = weightSum + GetRecordAt(j).get(d) * gaussWeight;
		// Add the values to normSum
		normSum = normSum + gaussWeight;
      }

	  // Add to the new values
	  gaussRecord.set( d, weightSum / normSum );

	}

	// Add the new record vector to the record log
	gaussRecord.setTime( GetRecordAt(i).getTime() );
	gaussRecord.setLabel( GetRecordAt(i).getLabel() );
    gaussRecordLog->SetRecord( i, gaussRecord );

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
  orthRecordLog->Initialize( this->numRecords, this->recordSize * ( order + 1 ) );

  TimeLabelRecord currLegRecord;
  currLegRecord.initialize( this->recordSize * ( order + 1 ), 0.0 );

  // Iterate over all data, and calculate Legendre expansion coefficients
  for ( int i = 0; i < this->numRecords; i++ )
  {
    // Calculate the record log to include
    vtkRecordLog* trimRecordLog = padRecordLog->Trim( i, i + window );
	
	// Create a new matrix to which the Legendre coefficients will be assigned
	std::vector<LabelRecord> legCoeffMatrix = trimRecordLog->LegendreTransformation( order );

	// Calculate the Legendre coefficients: 2D -> 1D
	int count = 0;
	for ( int o = 0; o <= order; o++ )
    {
      for ( int d = 0; d < recordSize; d++ )
	  {
        currLegRecord.set( count, legCoeffMatrix[o].get(d) );
		count++;
	  }
    }

	// New value record to add to the record log
    currLegRecord.setTime( GetRecordAt(i).getTime() );
	currLegRecord.setLabel( GetRecordAt(i).getLabel() );
	orthRecordLog->SetRecord( i, currLegRecord );

  }

  return orthRecordLog;

}


vnl_matrix<double>* vtkRecordLog
::CovarianceMatrix()
{
  // Copy the current record log
  vtkRecordLog* covRecLog = vtkRecordLog::New();
  covRecLog->Initialize( this->numRecords, this->recordSize );

  // Construct a recordSize by recordSize vnl matrix
  vnl_matrix<double> *cov = new vnl_matrix<double>( recordSize, recordSize );
  cov->fill( 0.0 );

  // Determine the mean, subtract the mean from each record
  ValueRecord meanRecord = this->Mean();
  TimeLabelRecord currRecord;
  currRecord.initialize( this->recordSize, 0.0 );

  for ( int i = 0; i < this->numRecords; i++ )
  {
    for( int d = 0; d < this->recordSize; d++ )
	{
	  currRecord.set( d, this->GetRecordAt(i).get(d) - meanRecord.get(d) );
	}

	currRecord.setTime( this->GetRecordAt(i).getTime() );
	currRecord.setLabel( this->GetRecordAt(i).getLabel() );

	covRecLog->SetRecord( i, currRecord );
  }

  // Pick two dimensions, and find their covariance
  for ( int d1 = 0; d1 < this->recordSize; d1++ )
  {
    for ( int d2 = 0; d2 < this->recordSize; d2++ )
	{
	  // Iterate over all times
	  for ( int i = 0; i < this->numRecords; i++ )
	  {
	    cov->put( d1, d2, cov->get( d1, d2 ) + covRecLog->GetRecordAt(i).get(d1) * covRecLog->GetRecordAt(i).get(d2) );
	  }
	  // Divide by the number of records
	  cov->put( d1, d2, cov->get( d1, d2 ) / this->numRecords );
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
  // Note: eigenvectors are ordered in increasing eigenvalue ( 0 = smallest, end = biggest )

  // Grab only the most important eigenvectors
  LabelRecord currPrinComp;
  currPrinComp.initialize( this->recordSize, 0.0 );

  std::vector<LabelRecord> prinComps ( numComp, currPrinComp );

  // TODO: Prevent more prinicipal components than original dimensions
  for ( int i = eigenvectors.cols() - 1; i > eigenvectors.cols() - 1 - numComp; i-- )
  {
    
    for ( int d = 0; d < this->recordSize; d++ )
	{
	  currPrinComp.set( d, eigenvectors.get( d, i ) );
	}

	currPrinComp.setLabel( eigenvectors.cols() - 1 - i );
	prinComps[ eigenvectors.cols() - 1 - i ] = currPrinComp;

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

	// Use closest point to the mean of all points for the first centroid
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

	centroids.push_back( this->FindNextCentroid( centroids ) );

	// Iterate until there are no more changes in membership and no clusters are empty
    bool change = true;
	while ( change )
	{
	  // Reassign the cluster memberships
	  std::vector<int> newMembership = this->ReassignMembership( centroids );

	  // Calculate change
	  change = this->MembershipChanged( membership, newMembership );
      if ( ! change )
	  {
        break;
	  }
	  membership = newMembership;

	  // Remove emptiness
	  std::vector<bool> emptyVector = this->FindEmptyClusters( centroids, membership );
      if ( this->HasEmptyClusters( emptyVector ) )
	  {
        centroids = this->MoveEmptyClusters( centroids, emptyVector );
		// At the end of this, we are guaranteed no empty clusters
        continue;
	  }	  

	  // Recalculate centroids
      centroids = this->RecalculateCentroids( membership, k + 1 );

	}

  }

  return centroids;

}




LabelRecord vtkRecordLog
::FindNextCentroid( std::vector<LabelRecord> centroids )
{

  // Find the record farthest from any centroid
  // Tricky way to cast vector of LabelRecord to vector of ValeuRecord
  std::vector<LabelRecord> centDist = this->Distances( std::vector<ValueRecord>( centroids.begin(), centroids.end() ) );
	
  int candidateRecord = 0;
  double candidateDistance = 0;

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
    if ( currMinDist > candidateDistance )
    {
      candidateDistance = currMinDist;
      candidateRecord = i;
	}
  }

  // Create new centroid for candidate
  LabelRecord currCentroid;
  currCentroid.values = this->records[candidateRecord].values;
  currCentroid.setLabel( centroids.size() );

  return currCentroid;

}



bool vtkRecordLog
::MembershipChanged( std::vector<int> oldMembership, std::vector<int> newMembership )
{

  for ( int i = 0; i < oldMembership.size(); i++ )
  {
    if ( oldMembership[i] != newMembership[i] )
	{
      return true;
	}
  }

  return false;
}



std::vector<bool> vtkRecordLog
::FindEmptyClusters( std::vector<LabelRecord> centroids, std::vector<int> membership )
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

  return emptyVector;
}



bool vtkRecordLog
::HasEmptyClusters( std::vector<bool> emptyVector )
{
  for ( int c = 0; c < emptyVector.size(); c++ )
  {
    if ( emptyVector[c] )
	{
      return true;
	}
  }
 
  return false;
}



std::vector<LabelRecord> vtkRecordLog
::MoveEmptyClusters( std::vector<LabelRecord> centroids, std::vector<bool> emptyVector )
{

  // Remove any emptyness
  for ( int c = 0; c < centroids.size(); c++ )
  {
    if ( emptyVector[c] == false )
	{
	  continue;
	}

    centroids[c] = this->FindNextCentroid( ( centroids ) );

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
  LabelRecord initCentroid;
  initCentroid.initialize( this->recordSize, 0.0 );
  std::vector<LabelRecord> centroids ( numClusters, initCentroid );
  std::vector<int> memberCount ( numClusters, 0 );;

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
	centroids[c].setLabel( c );
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