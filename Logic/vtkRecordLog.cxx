
#include "vtkSmartPointer.h"
#include "vtkRecordLog.h"
#include "vtkObjectFactory.h"


#include <string>
#include <sstream>
#include <cmath>


vtkStandardNewMacro( vtkRecordLog );

vtkRecordLog
::vtkRecordLog()
{
  this->numRecords = 0;
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
	ValueRecord currRecord;
	std::vector<double> currValues;

	for ( int j = 0; j < records[i].values.size(); j++ )
	{
      currValues.push_back( records[i].values[j] );
	}

	currRecord.values = currValues;
	currRecord.time = records[i].time;
    newRecordLog->AddRecord( currRecord );
  }
  
  return newRecordLog;
}


vtkRecordLog* vtkRecordLog
::Trim( int start, int end )
{

  // Create a new procedure object
  vtkRecordLog* trimRecordLog = vtkRecordLog::New();

  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = start; i < end; i++ )
  {
	ValueRecord currRecord;
	std::vector<double> currValues;

	for ( int j = 0; j < records[i].values.size(); j++ )
	{
      currValues.push_back( records[i].values[j] );
	}

	currRecord.values = currValues;
	currRecord.time = records[i].time;
    trimRecordLog->AddRecord( currRecord );
  }

  return trimRecordLog;

}


void vtkRecordLog
::AddRecord( ValueRecord newRecord )
{
  records.push_back( newRecord );
  numRecords++;
}


ValueRecord vtkRecordLog
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
  std::vector<double> firstValues;

  for( int d = 0; d < records[0].values.size(); d++ )
  {
    firstValues.push_back( ( records[1].values[d] - records[0].values[d] ) / DT );
  }
	
  ValueRecord firstRecord;
  firstRecord.values = firstValues;
  firstRecord.time = records[0].time;
  derivRecordLog->AddRecord( firstRecord );

  // Middle
  for ( int i = 1; i < numRecords - 1; i++ )
  {

	DT = records[i+1].time - records[i-1].time;
	std::vector<double> midValues;

	for( int d = 0; d < records[i].values.size(); d++ )
	{
      midValues.push_back( ( records[i+1].values[d] - records[i-1].values[d] ) / ( 2 * DT ) );
	}
	
	ValueRecord midRecord;
	midRecord.values = midValues;
	midRecord.time = records[i].time;
	derivRecordLog->AddRecord( midRecord );

  }

  // Last
  DT = records[numRecords-1].time - records[numRecords-2].time;
  std::vector<double> lastValues;

  for( int d = 0; d < records[numRecords-1].values.size(); d++ )
  {
    lastValues.push_back( ( records[numRecords-1].values[d] - records[numRecords-2].values[d] ) / DT );
  }
	
  ValueRecord lastRecords;
  lastRecords.values = lastValues;
  lastRecords.time = records[numRecords-1].time;
  derivRecordLog->AddRecord( lastRecords );

  // Return the order - 1 derivative
  return derivRecordLog->Derivative( order - 1 );

}



std::vector<double> vtkRecordLog
::Integrate()
{
  // The record log will only hold one record at the end
  std::vector<double> intValues;
  double DT;

  // Initialize the sums to zero
  for ( int d = 0; d < records[0].values.size(); d++ )
  {
    intValues.push_back( 0.0 );
  }

  // For each time
  for ( int i = 0; i < numRecords - 1; i++ )
  {
	// Find the time difference
    DT = records[i+1].time - records[i-1].time;

	// Iterate over all dimensions
	for ( int d = 0; d < records[i].values.size(); d++ )
	{
	  intValues[d] += ( records[i].values[d] + records[i].values[d] ) / 2 * DT ;
	}

  }

  return intValues;

}


std::vector<double> vtkRecordLog
::LegendreCoefficient( int order )
{

  
  // Create a copy of the current record log
  vtkRecordLog* recLogCopy;
  recLogCopy = this->DeepCopy();

  
  std::vector<double> legendreCoeff;
  
  // Calculate the time adjustment (need range -1 to 1)
  double initialTime = recLogCopy->records[0].time;
  double rangeTime = recLogCopy->records[numRecords-1].time - recLogCopy->records[0].time;
  
  for ( int i = 0; i < numRecords; i++ )
  {
    recLogCopy->records[i].time -= initialTime; // ( 0 to tmax )
    recLogCopy->records[i].time *= 2.0 / rangeTime; // ( 0 to 2 )
	recLogCopy->records[i].time -= 1; // ( -1 to 1 )
  }

  // Create a copy of the record log for each degree of Legendre polynomial
  vtkRecordLog* orderRecordLogCopy;

  orderRecordLogCopy = recLogCopy->DeepCopy();

  // Multiply the values by the Legendre polynomials
  for ( int i = 0; i < numRecords; i++ )
  {
    for ( int d = 0; d < records[i].values.size(); d++ )
	{
      orderRecordLogCopy->records[i].values[d] *= LegendrePolynomial( orderRecordLogCopy->records[i].time, order );
	}
  }

  return orderRecordLogCopy->Integrate();

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
	std::vector<double> currValues;
	ValueRecord padRecord;

	for ( int d = 0; d < records[0].values.size(); d++ )
	{
	   currValues.push_back( records[0].values[d] );
	}

	padRecord.values = currValues;
	padRecord.time = currTime;
	padRecordLog->AddRecord( padRecord );

  }

  return padRecordLog;
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
::GaussianFilter( double width )
{
  // Assume a Gaussian filter
  vtkRecordLog* gaussRecordLog = vtkRecordLog::New();

  // For each record
  for ( int i = 0; i < numRecords; i++ )
  {
    // Create a new record valuestor
    ValueRecord gaussRecord;
	std::vector<double> gaussValues;

    // Iterate over all dimensions
	for ( int d = 0; d < records[0].values.size(); d++ )
	{
      double weightSum = 0;
      double normSum = 0;

      // Iterate over all records
      for ( int j = 0; j < numRecords; j++ )
      {
        // Calculate the values of the Gaussian distribution at this time
	    double gaussWeight = exp( - ( records[j].time - records[i].time ) * ( records[j].time - records[i].time ) / width );
		// Add the product with the values to function sum
        weightSum = weightSum + records[j].values[d] * gaussWeight;
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
	
	// Create a new vector to which the Legendre coefficients will be assigned
	std::vector<double> currLegCoeffOne;

	// Calculate the Legendre coefficients
	for ( int o = 0; o < order; o++ )
    {
	  std::vector<double> currLegCoeff = trimRecordLog->LegendreCoefficient( order );
      for ( int d = 0; d < records[i].values.size(); d++ )
	  {
        currLegCoeffOne.push_back( currLegCoeff[d] );
	  }
    }

	// New value record to add to the record log
	ValueRecord currLegRecord;
    currLegRecord.time = this->records[i].time;
    currLegRecord.values = currLegCoeffOne;
	orthRecordLog->AddRecord( currLegRecord );

  }

	return orthRecordLog;
}
