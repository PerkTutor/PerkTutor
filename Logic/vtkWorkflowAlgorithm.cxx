
#include "vtkWorkflowAlgorithm.h"


vtkStandardNewMacro( vtkWorkflowAlgorithm );


vtkWorkflowAlgorithm
::vtkWorkflowAlgorithm()
{
  this->BufferRT = vtkRecordBufferRT::New();
  this->DerivativeBufferRT = vtkRecordBufferRT::New();
  this->FilterBufferRT = vtkRecordBufferRT::New();
  this->OrthogonalBufferRT = vtkRecordBufferRT::New();
  this->PcaBufferRT = vtkRecordBufferRT::New();
  this->CentroidBufferRT = vtkRecordBufferRT::New();

  this->CurrentTask = NULL;
  this->PrevTask = NULL;
  this->DoTask = NULL;
  this->DoneTask = NULL;

  this->Tool = NULL;

  this->CompletionAlgorithm = NULL;
}



vtkWorkflowAlgorithm
::~vtkWorkflowAlgorithm()
{
  vtkDeleteVector( this->TrainingBuffers );

  vtkDelete( this->BufferRT );
  vtkDelete( this->DerivativeBufferRT );
  vtkDelete( this->FilterBufferRT );
  vtkDelete( this->OrthogonalBufferRT );
  vtkDelete( this->PcaBufferRT );
  vtkDelete( this->CentroidBufferRT );

  vtkDelete( this->CompletionAlgorithm );
}



std::vector<double> vtkWorkflowAlgorithm
::CalculateTaskProportions()
{
  // Create a vector of counts for each label
  std::vector<int> taskCounts;
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
    taskCounts.push_back( 0 );
  }

  int sum = 0;

  // Iterate over all record logs and count label (task) instances
  for ( int i = 0; i < this->TrainingBuffers.size(); i++ )
  {
    for ( int j = 0; j < this->TrainingBuffers.at(i)->GetNumRecords(); j++ )
	{
	  int taskIndex = this->Tool->Procedure->IndexByName( this->TrainingBuffers.at(i)->GetRecordAt(j)->GetLabel() );
	  if ( taskIndex >= 0 && taskIndex < this->Tool->Procedure->GetNumTasks() )
	  {
	    taskCounts.at(taskIndex) += 1;
	    sum++;
	  }
	}
  }

  std::vector<double> taskProportions;
  // Calculate the proportion of each task
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
    taskProportions.push_back( ( 1.0 * taskCounts[i] ) / sum );
  }

  return taskProportions;
}



std::vector<double> vtkWorkflowAlgorithm
::EqualizeTaskProportions()
{
  //Find the mean and standard deviation of the task centroids
  std::vector<double> taskProportions = this->CalculateTaskProportions();

  double mean = 0;
  for ( int i = 0; i < taskProportions.size(); i++ )
  {
    mean += taskProportions.at(i);
  }
  mean = mean / taskProportions.size();

  // Reduce the standard deviation by the equalizing parameter
  for ( int i = 0; i < taskProportions.size(); i++ )
  {
    taskProportions.at(i) = ( taskProportions.at(i) - mean ) / this->Tool->Input->Equalization + mean;
  }

  return taskProportions;
}




std::vector<int> vtkWorkflowAlgorithm
::CalculateTaskCentroids()
{
  // Create a vector of counts for each label
  std::vector<double> taskProportions = this->EqualizeTaskProportions();
  std::vector<double> taskRawCentroids ( this->Tool->Procedure->GetNumTasks(), 0 );
  std::vector<double> fracPart ( this->Tool->Procedure->GetNumTasks(), 0 );

  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
    taskRawCentroids[i] = taskProportions[i] * this->Tool->Input->NumCentroids;
  }

  bool allRounded = true;
  int sumCentroids = 0;
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
    sumCentroids += floor( taskRawCentroids[i] );
  }    

  while ( sumCentroids < this->Tool->Input->NumCentroids )
  {

    allRounded = true;
    for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
	{
      if ( floor( taskRawCentroids[i] ) != ceil( taskRawCentroids[i] ) )
	  {
        allRounded = false;
	  }
	}

	if ( allRounded ) // This should never happen
	{
      break;
	}

    // Find the highest "priority" (with largest fractional part) centroid count to increase
	int priority = 0;
    for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
    {
      fracPart[i] = taskRawCentroids[i] - floor( taskRawCentroids[i] );
	  if ( fracPart[i] > fracPart[priority] )
	  {
        priority = i;
	  }
    }

	taskRawCentroids[priority] = ceil( taskRawCentroids[ priority ] );

	sumCentroids = 0;
	for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
	{
      sumCentroids += taskRawCentroids[i];
	}    

  }

  std::vector<int> taskCentroids ( this->Tool->Procedure->GetNumTasks(), 0 );
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
    taskCentroids[i] = floor( taskRawCentroids[i] );
  } 

  return taskCentroids;  

}




void vtkWorkflowAlgorithm
::AddTrainingBuffer( vtkRecordBuffer* newTrainingBuffer )
{
  vtkRecordBuffer* trimRecordBuffer = newTrainingBuffer->TrimBufferByLabel( this->Tool->Procedure->GetTaskNames() );
  this->TrainingBuffers.push_back( trimRecordBuffer );

  // Train the completion algorithm associated
  if ( this->CompletionAlgorithm != NULL )
  {
    vtkRecordBuffer* completionTrainingBuffer = newTrainingBuffer->AddCompletion( this->CompletionAlgorithm->Tool->Input->CompletionTime );
    this->CompletionAlgorithm->AddTrainingBuffer( completionTrainingBuffer );
	completionTrainingBuffer->Delete(); // Because a deep copy is created in the above anyway
  }
}



// Return whether or not training was successful
bool vtkWorkflowAlgorithm
::Train()
{
  // There must exist procedures
  if ( this->TrainingBuffers.empty() )
  {
    return false;
  }

  // Calculate the number of centroids for each task
  std::vector<int> taskCentroids = this->CalculateTaskCentroids();
  std::vector<int> cumulativeCentroids;
  int currSum = 0;

  // Calculate the cumulative number of centroids for cluster numbering
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
	// Make sure that every task is represented in the procedures
	if ( taskCentroids[i] == 0 )
	{
      return false;
	}

	cumulativeCentroids.push_back( currSum );
	currSum += taskCentroids.at(i);
  }

  // Apply Gaussian filtering to each record log
  std::vector<vtkRecordBuffer*> filterBuffers;
  for ( int i = 0; i < TrainingBuffers.size(); i++ )
  {
    filterBuffers.push_back( TrainingBuffers.at(i)->GaussianFilter( this->Tool->Input->FilterWidth ) );
  }


  // Use velocity and higher order derivatives also
  std::vector<vtkRecordBuffer*> derivativeBuffers;
  for ( int i = 0; i < filterBuffers.size(); i++ )
  {
    derivativeBuffers.push_back( filterBuffers.at(i)->DeepCopy() );
    for ( int d = 1; d <= this->Tool->Input->Derivative; d++ )
	{
	  vtkRecordBuffer* currDerivativeProcedure = derivativeBuffers.at(i);
	  vtkRecordBuffer* orderDerivativeProcedure = filterBuffers.at(i)->Derivative(d);	  
	  derivativeBuffers.at(i) = currDerivativeProcedure->ConcatenateValues( orderDerivativeProcedure );
	  currDerivativeProcedure->Delete();
	  orderDerivativeProcedure->Delete();	  
	}
  }


  // Apply orthogonal transformation
  std::vector<vtkRecordBuffer*> orthogonalBuffers;
  for ( int i = 0; i < derivativeBuffers.size(); i++ )
  {
    orthogonalBuffers.push_back( derivativeBuffers.at(i)->OrthogonalTransformation( this->Tool->Input->OrthogonalWindow, this->Tool->Input->OrthogonalOrder ) );
  }

  // Concatenate all of the record logs into one record log
  // Observe that the concatenated buffers are sorted by time stamp - its order is not maintained by procedure, but this is ok
  vtkRecordBuffer* orthogonalCat = vtkRecordBuffer::New();
  for ( int i = 0; i < orthogonalBuffers.size(); i++ )
  {
    vtkRecordBuffer* currOrthogonalCat = orthogonalCat;
    orthogonalCat = currOrthogonalCat->Concatenate( orthogonalBuffers.at(i) );
	currOrthogonalCat->Delete();
  }

  // Calculate and apply the PCA transform
  this->Tool->Training->Mean->Delete();
  this->Tool->Training->Mean = orthogonalCat->Mean();
  this->Tool->Training->Mean->SetLabel( "Mean" );
  this->Tool->Training->PrinComps = orthogonalCat->CalculatePCA( this->Tool->Input->NumPrinComps );

  std::vector<vtkRecordBuffer*> pcaBuffers;
  for ( int i = 0; i < orthogonalBuffers.size(); i++ )
  {
    pcaBuffers.push_back( orthogonalBuffers.at(i)->TransformPCA( this->Tool->Training->PrinComps, this->Tool->Training->Mean ) );
  }
  vtkRecordBuffer* pcaCat = orthogonalCat->TransformPCA( this->Tool->Training->PrinComps, this->Tool->Training->Mean );

  // Put together all the tasks together for task by task clustering
  std::vector<vtkRecordBuffer*> buffersByLabel = pcaCat->SplitBufferByLabel( this->Tool->Procedure->GetTaskNames() );

  // Add the centroids from each task
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
	std::vector<vtkLabelVector*> currTaskCentroids = buffersByLabel.at(i)->fwdkmeans( taskCentroids.at(i) );
	for ( int j = 0; j < taskCentroids[i]; j++ )
	{
	  // Make the centroid numbering continuous over all centroids
	  currTaskCentroids.at(j)->SetLabel( atoi( currTaskCentroids[j]->GetLabel().c_str() ) + cumulativeCentroids.at(i) );
      this->Tool->Training->Centroids.push_back( currTaskCentroids.at(j) );
	}
  }

  // Calculate the sequence of centroids for each procedure
  std::vector<vtkRecordBuffer*> centroidBuffers;
  for ( int i = 0; i < pcaBuffers.size(); i++ )
  {
    centroidBuffers.push_back( pcaBuffers.at(i)->fwdkmeansTransform( this->Tool->Training->Centroids ) );
  }

  // Assume that all the estimation matrices are associated with the pseudo scales
  vtkLabelVector* PseudoPi = vtkLabelVector::New();
  PseudoPi->Initialize( this->Tool->Procedure->GetNumTasks(), this->Tool->Input->MarkovPseudoScalePi );
  PseudoPi->SetLabel( "Pi" );

  std::vector<vtkLabelVector*> PseudoA;
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
	vtkLabelVector* currPseudoA = vtkLabelVector::New();
	currPseudoA->Initialize( this->Tool->Procedure->GetNumTasks(), this->Tool->Input->MarkovPseudoScaleA );
	currPseudoA->SetLabel( this->Tool->Procedure->GetTaskAt(i)->Name );
	PseudoA.push_back( currPseudoA );
  }

  std::vector<vtkLabelVector*> PseudoB;
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
    vtkLabelVector* currPseudoB = vtkLabelVector::New();
	currPseudoB->Initialize( this->Tool->Input->NumCentroids, this->Tool->Input->MarkovPseudoScaleB );
	currPseudoB->SetLabel( this->Tool->Procedure->GetTaskAt(i)->Name );
	PseudoB.push_back( currPseudoB );
  }

  // Create a new Markov Model, and estimate its parameters
  vtkMarkovModel* Markov = vtkMarkovModel::New();
  Markov->SetStates( this->Tool->Procedure->GetTaskNames() );  
  Markov->SetSymbols( this->Tool->Input->NumCentroids );
  Markov->InitializeEstimation();
  Markov->AddPseudoData( PseudoPi, PseudoA, PseudoB );
  for ( int i = 0; i < centroidBuffers.size(); i++ )
  {
    std::vector<vtkMarkovRecord*> markovRecords = centroidBuffers.at(i)->ToMarkovRecordVector();
    Markov->AddEstimationData( markovRecords );
    vtkDeleteVector( markovRecords );
  }
  Markov->EstimateParameters();

  this->Tool->Training->Markov->SetStates( this->Tool->Procedure->GetTaskNames() );
  this->Tool->Training->Markov->SetSymbols( this->Tool->Input->NumCentroids );
  this->Tool->Training->Markov->SetPi( Markov->GetPi()->DeepCopy() );
  this->Tool->Training->Markov->SetA( vtkDeepCopyVector( Markov->GetA() ) );
  this->Tool->Training->Markov->SetB( vtkDeepCopyVector( Markov->GetB() ) );

  // Delete objects we have created
  vtkDeleteVector( filterBuffers );
  vtkDeleteVector( derivativeBuffers );
  vtkDeleteVector( orthogonalBuffers );
  vtkDeleteVector( pcaBuffers );
  vtkDeleteVector( buffersByLabel );
  vtkDeleteVector( centroidBuffers );


  orthogonalCat->Delete();
  pcaCat->Delete();
  Markov->Delete();
  PseudoPi->Delete();
  vtkDeleteVector( PseudoA );
  vtkDeleteVector( PseudoB );

  this->Tool->Trained = true;

  // Train the completion algorithm associated
  if ( this->CompletionAlgorithm != NULL )
  {
    this->CompletionAlgorithm->Train();
  }

  return true;
}



void vtkWorkflowAlgorithm
::AddCompletionAlgorithm( vtkWorkflowAlgorithm* newCompletionAlgorithm )
{
  if ( this->CompletionAlgorithm != NULL )
  {
    this->CompletionAlgorithm->Delete();
  }
  this->CompletionAlgorithm = newCompletionAlgorithm;
  std::vector<bool> newCompletionVector( this->Tool->Procedure->GetNumTasks(), false );
  this->CompletionVector = newCompletionVector;
}



void vtkWorkflowAlgorithm
::AddRecord( vtkLabelRecord* newRecord )
{
  this->BufferRT->AddRecord( newRecord );
}



void vtkWorkflowAlgorithm
::AddSegmentRecord( vtkLabelRecord* newRecord )
{
  AddRecord( newRecord );
  // Add and segment record to completion algorithm
  if ( this->CompletionAlgorithm != NULL )
  {
    this->CompletionAlgorithm->AddSegmentRecord( newRecord->DeepCopy() );
  }

  // Apply Gaussian filtering to each previous records
  this->FilterBufferRT->AddRecord( this->BufferRT->GaussianFilterRT( this->Tool->Input->FilterWidth ) );
  
  // Concatenate with derivative (velocity, acceleration, etc...)
  vtkLabelRecord* derivativeRecord = this->FilterBufferRT->GetRecordRT()->DeepCopy();
  for ( int d = 1; d <= this->Tool->Input->Derivative; d++ )
  {
    vtkLabelRecord* currDerivativeRecord = this->BufferRT->DerivativeRT(d);
	for ( int j = 0; j < currDerivativeRecord->Size(); j++ )
	{
      derivativeRecord->Add( currDerivativeRecord->Get(j) );
	}
	currDerivativeRecord->Delete();
  }
  this->DerivativeBufferRT->AddRecord( derivativeRecord );

  // Apply orthogonal transformation
  this->OrthogonalBufferRT->AddRecord( this->DerivativeBufferRT->OrthogonalTransformationRT( this->Tool->Input->OrthogonalWindow, this->Tool->Input->OrthogonalOrder ) );

  // Apply PCA transformation
  this->PcaBufferRT->AddRecord( this->OrthogonalBufferRT->TransformPCART( this->Tool->Training->PrinComps, this->Tool->Training->Mean ) );

  // Apply centroid transformation
  this->CentroidBufferRT->AddRecord( this->PcaBufferRT->fwdkmeansTransformRT( this->Tool->Training->Centroids ) );

  // Use Markov Model calculate states to come up with the current most likely state...
  vtkMarkovRecord* markovState = this->Tool->Training->Markov->CalculateStateRT( CentroidBufferRT->ToMarkovRecordRT() );

  // Now, we will keep a recording of the workflow segmentation in BufferRT - add the label
  this->BufferRT->GetRecordRT()->SetLabel( markovState->GetState() );

  this->PrevTask = this->CurrentTask;
  this->CurrentTask = this->Tool->Procedure->GetTaskByName( markovState->GetState() );


  // Finally, calculate the appropriate instruction
  if ( this->CompletionAlgorithm == NULL )
  {
    return;
  }

  this->SetCompletionVector( this->CurrentTask->Name, this->CompletionAlgorithm->CurrentTask->Name );

  this->DoneTask = this->DoTask;
  if ( ! this->GetCompletionVector( this->CurrentTask->Prerequisite ) )
  {
    this->DoTask = this->Tool->Procedure->GetTaskByName( this->CurrentTask->Recovery );
  }
  else if ( this->GetCompletionVector( this->CurrentTask->Name ) )
  {
    this->DoTask = this->Tool->Procedure->GetTaskByName( this->CurrentTask->Next );
  }
  else
  {
    this->DoTask = this->CurrentTask;
  }

}


void vtkWorkflowAlgorithm
::SetCompletionVector( std::string currentTask, std::string currentCompletion )
{
  // Assign completion of a task appropriately
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
    if ( this->Tool->Procedure->GetTaskAt(i)->Name.compare( currentTask ) == 0 && currentCompletion.compare( currentTask ) == 0 )
	{
	  this->CompletionVector.at(i) = false;
	}
    if ( this->Tool->Procedure->GetTaskAt(i)->Name.compare( currentTask ) == 0 && currentCompletion.compare( currentTask + "_Completion" ) == 0 )
	{
	  this->CompletionVector.at(i) = true;
	}
  }
}


bool vtkWorkflowAlgorithm
::GetCompletionVector( std::string currentTask )
{
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
    if ( this->Tool->Procedure->GetTaskAt(i)->Name.compare( currentTask ) == 0 )
	{
      return this->CompletionVector.at(i);
	}
  }

  return false;
}

