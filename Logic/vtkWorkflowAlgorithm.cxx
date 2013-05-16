
#include "vtkWorkflowAlgorithm.h"

vtkStandardNewMacro( vtkWorkflowAlgorithm );


vtkWorkflowAlgorithm
::vtkWorkflowAlgorithm()
{
  this->procedureRT = NULL;
  this->derivativeProcedureRT = NULL;
  this->filterProcedureRT = NULL;
  this->orthogonalProcedureRT = NULL;
  this->pcaProcedureRT = NULL;
  this->centroidProcedureRT = NULL;
  this->MarkovRT = NULL;

  this->IndexToProcess = 0;
  this->CurrentTask = "";
  this->PrevTask = "";

  this->Tool = NULL;
}



vtkWorkflowAlgorithm
::~vtkWorkflowAlgorithm()
{
  for ( int i = 0 ; i < trainingProcedures.size(); i++ )
  {
    trainingProcedures.at(i)->Delete();
  }
  trainingProcedures.clear();

  if ( this->procedureRT != NULL )
  {
    this->procedureRT->Delete();
    this->derivativeProcedureRT->Delete();
    this->filterProcedureRT->Delete();
    this->orthogonalProcedureRT->Delete();
    this->pcaProcedureRT->Delete();
    this->centroidProcedureRT->Delete();
    this->MarkovRT->Delete();
  }
}


void vtkWorkflowAlgorithm
::SetTool( vtkWorkflowTool* newTool )
{
  this->Tool = newTool;
}


vtkWorkflowTool* vtkWorkflowAlgorithm
::GetTool()
{
  return this->Tool;
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
  for ( int i = 0; i < trainingProcedures.size(); i++ )
  {
    for ( int j = 0; j < trainingProcedures.at(i)->GetNumRecords(); j++ )
	{
	  int taskIndex = this->Tool->Procedure->IndexByName( trainingProcedures.at(i)->GetRecordAt(j)->GetLabel() );
	  taskCounts.at(taskIndex) += 1;
	  sum++;
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




std::vector<int> vtkWorkflowAlgorithm
::CalculateTaskCentroids()
{
  // Create a vector of counts for each label
  std::vector<double> taskProportions = this->CalculateTaskProportions();
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

  std::vector<int> taskCentroids ( this->GetTool()->Procedure->GetNumTasks(), 0 );
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
    taskCentroids[i] = floor( taskRawCentroids[i] );
  } 

  return taskCentroids;  

}




void vtkWorkflowAlgorithm
::AddTrainingProcedure( vtkRecordBuffer* newTrainingProcedure )
{
  trainingProcedures.push_back( newTrainingProcedure );
}



void vtkWorkflowAlgorithm
::SegmentProcedure( vtkRecordBuffer* newProcedure )
{
  for ( int i = 0; i < newProcedure->GetNumRecords(); i++ )
  {
    this->AddSegmentRecord( newProcedure->GetRecordAt(i) );
	this->UpdateTask();
  }
}




// Return whether or not training was successful
bool vtkWorkflowAlgorithm
::Train()
{
  // There must exist procedures
  if ( trainingProcedures.empty() )
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
  std::vector<vtkRecordBuffer*> filterProcedures;
  for ( int i = 0; i < trainingProcedures.size(); i++ )
  {
    filterProcedures.push_back( trainingProcedures.at(i)->GaussianFilter( this->Tool->Input->FilterWidth ) );
  }


  // Use velocity and higher order derivatives also
  std::vector<vtkRecordBuffer*> derivativeProcedures;
  for ( int i = 0; i < filterProcedures.size(); i++ )
  {
    derivativeProcedures.push_back( filterProcedures.at(i)->DeepCopy() );
    for ( int d = 1; d <= this->GetTool()->Input->Derivative; d++ )
	{
	  vtkRecordBuffer* currDerivativeProcedure = derivativeProcedures.at(i);
	  vtkRecordBuffer* orderDerivativeProcedure = filterProcedures.at(i)->Derivative(d);	  
	  derivativeProcedures.at(i) = currDerivativeProcedure->ConcatenateValues( orderDerivativeProcedure );
	  currDerivativeProcedure->Delete();
	  orderDerivativeProcedure->Delete();	  
	}
  }


  // Apply orthogonal transformation
  std::vector<vtkRecordBuffer*> orthogonalProcedures;
  for ( int i = 0; i < derivativeProcedures.size(); i++ )
  {
    orthogonalProcedures.push_back( derivativeProcedures.at(i)->OrthogonalTransformation( this->Tool->Input->OrthogonalWindow, this->Tool->Input->OrthogonalOrder ) );
  }

  // Concatenate all of the record logs into one record log
  vtkRecordBuffer* orthogonalCat = vtkRecordBuffer::New();
  orthogonalCat->Initialize( 0, orthogonalProcedures.at(0)->GetCurrentRecord()->Size() );
  for ( int i = 0; i < orthogonalProcedures.size(); i++ )
  {
    vtkRecordBuffer* currOrthogonalCat = orthogonalCat;
    orthogonalCat = currOrthogonalCat->Concatenate( orthogonalProcedures.at(i) );
	currOrthogonalCat->Delete();
  }

  // Calculate and apply the PCA transform
  this->Tool->Training->Mean->SetValues( orthogonalCat->Mean()->GetValues() );
  this->Tool->Training->Mean->SetLabel( "Mean" );
  this->Tool->Training->PrinComps = orthogonalCat->CalculatePCA( this->Tool->Input->NumPrinComps );

  std::vector<vtkRecordBuffer*> pcaProcedures;
  for ( int i = 0; i < orthogonalProcedures.size(); i++ )
  {
    pcaProcedures.push_back( orthogonalProcedures.at(i)->TransformPCA( this->Tool->Training->PrinComps, this->Tool->Training->Mean ) );
  }
  vtkRecordBuffer* pcaCat = orthogonalCat->TransformPCA( this->Tool->Training->PrinComps, this->Tool->Training->Mean );

  // Put together all the tasks together for task by task clustering
  std::vector<vtkRecordBuffer*> recordsByTask = pcaCat->SplitBufferByLabel( this->Tool->Procedure->GetTaskNames() );

  // Add the centroids from each task
  // TODO: Change NumCentroids back to 700
  for ( int i = 0; i < this->Tool->Procedure->GetNumTasks(); i++ )
  {
	std::vector<vtkLabelVector*> currTaskCentroids = recordsByTask.at(i)->fwdkmeans( taskCentroids.at(i) );
	for ( int j = 0; j < taskCentroids[i]; j++ )
	{
	  // Make the centroid numbering continuous over all centroids
      std::stringstream labelstring;
      labelstring << atoi( currTaskCentroids[j]->GetLabel().c_str() ) + cumulativeCentroids.at(i);
	  currTaskCentroids.at(j)->SetLabel( labelstring.str() );
      this->Tool->Training->Centroids.push_back( currTaskCentroids.at(j) );
	}
  }

  // Calculate the sequence of centroids for each procedure
  std::vector<vtkRecordBuffer*> centroidProcedures;
  for ( int i = 0; i < pcaProcedures.size(); i++ )
  {
    centroidProcedures.push_back( pcaProcedures.at(i)->fwdkmeansTransform( this->Tool->Training->Centroids ) );
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
  for ( int i = 0; i < centroidProcedures.size(); i++ )
  {
    Markov->AddEstimationData( centroidProcedures.at(i)->ToMarkovRecordVector() );
  }
  Markov->EstimateParameters();

  this->Tool->Training->MarkovPi = Markov->GetPi();
  this->Tool->Training->MarkovA = Markov->GetA();
  this->Tool->Training->MarkovB = Markov->GetB();


  // Delete objects we have created
  for ( int i = 0; i < filterProcedures.size(); i++ )
  {
    filterProcedures[i]->Delete();
  }
  for ( int i = 0; i < derivativeProcedures.size(); i++ )
  {
    derivativeProcedures[i]->Delete();
  }
  for ( int i = 0; i < orthogonalProcedures.size(); i++ )
  {
    orthogonalProcedures[i]->Delete();
  }
  for ( int i = 0; i < pcaProcedures.size(); i++ )
  {
    pcaProcedures[i]->Delete();
  }
  for ( int i = 0; i < recordsByTask.size(); i++ )
  {
    recordsByTask[i]->Delete();
  }
  for ( int i = 0; i < centroidProcedures.size(); i++ )
  {
    centroidProcedures[i]->Delete();
  }
  
  filterProcedures.clear();
  derivativeProcedures.clear();
  orthogonalProcedures.clear();
  pcaProcedures.clear();
  recordsByTask.clear();
  centroidProcedures.clear();

  orthogonalCat->Delete();
  pcaCat->Delete();
  Markov->Delete();

  return true;
}



void vtkWorkflowAlgorithm
::Reset()
{

  // If the algorithm isn't trained, then we can't initialize it for real-time segmentation
  if ( ! this->Tool->Trained )
  {
    return;
  }

  if ( this->procedureRT != NULL )
  {
    this->procedureRT->Delete();
    this->derivativeProcedureRT->Delete();
    this->filterProcedureRT->Delete();
    this->orthogonalProcedureRT->Delete();
    this->pcaProcedureRT->Delete();
    this->centroidProcedureRT->Delete();
    this->MarkovRT->Delete();
  }

  procedureRT = vtkRecordBufferRT::New();
  derivativeProcedureRT = vtkRecordBufferRT::New();
  filterProcedureRT = vtkRecordBufferRT::New();
  orthogonalProcedureRT = vtkRecordBufferRT::New();
  pcaProcedureRT = vtkRecordBufferRT::New();
  centroidProcedureRT = vtkRecordBufferRT::New();
  MarkovRT = vtkMarkovModelRT::New();
  MarkovRT->SetStates( this->Tool->Procedure->GetTaskNames() );
  MarkovRT->SetSymbols( this->Tool->Input->NumCentroids );
  MarkovRT->SetPi( this->Tool->Training->MarkovPi );
  MarkovRT->SetA( this->Tool->Training->MarkovA );
  MarkovRT->SetB( this->Tool->Training->MarkovB );

  IndexToProcess = 0;
  CurrentTask = "";
  PrevTask = "";

}



void vtkWorkflowAlgorithm
::AddRecord( vtkLabelRecord* newRecord )
{
  procedureRT->AddRecord( newRecord );
}



void vtkWorkflowAlgorithm
::AddSegmentRecord( vtkLabelRecord* newRecord )
{
  AddRecord( newRecord );

  // TODO: Only keep the most recent observations (a few for filtering, a window for orthogonal transformation)

  // Apply Gaussian filtering to each previous records
  filterProcedureRT->AddRecord( procedureRT->GaussianFilterRT( this->Tool->Input->FilterWidth ) );
  
  // Concatenate with derivative (velocity, acceleration, etc...)
  vtkLabelRecord* derivativeRecord = filterProcedureRT->GetRecordRT();
  for ( int d = 1; d <= this->Tool->Input->Derivative; d++ )
  {
    vtkLabelRecord* currDerivativeRecord = procedureRT->DerivativeRT(d);
	for ( int j = 0; j < currDerivativeRecord->Size(); j++ )
	{
      derivativeRecord->Add( currDerivativeRecord->Get(j) );
	}
	currDerivativeRecord->Delete();
  }
  derivativeProcedureRT->AddRecord( derivativeRecord );

  // Apply orthogonal transformation
  orthogonalProcedureRT->AddRecord( derivativeProcedureRT->OrthogonalTransformationRT( this->Tool->Input->OrthogonalWindow, this->Tool->Input->OrthogonalOrder ) );

  // Apply PCA transformation
  pcaProcedureRT->AddRecord( orthogonalProcedureRT->TransformPCART( this->Tool->Training->PrinComps, this->Tool->Training->Mean ) );

  // Apply centroid transformation
  centroidProcedureRT->AddRecord( pcaProcedureRT->fwdkmeansTransformRT( this->Tool->Training->Centroids ) );

  // Use Markov Model calculate states to come up with the current most likely state...
  vtkMarkovRecord* markovState = MarkovRT->CalculateStateRT( centroidProcedureRT->ToMarkovRecordRT() );

  // Now, we will keep a recording of the workflow segmentation in procedureRT - add the label
  procedureRT->GetRecordRT()->SetLabel( markovState->GetState() );

  this->CurrentTask = markovState->GetState();

}



void vtkWorkflowAlgorithm
::UpdateTask()
{
  // Check if there are any new transforms to process
  if ( this->Tool->Buffer->GetNumRecords() > this->IndexToProcess )
  {
    vtkLabelRecord* currentRecord = this->Tool->Buffer->GetRecordAt( this->IndexToProcess );

    if ( this->Tool->Trained )
	{
	  AddSegmentRecord( currentRecord );
	}
	else
	{
	  AddRecord( currentRecord );
	}
	// Add to the segmentation buffer
	if ( this->CurrentTask.compare( this->PrevTask ) != 0 )
	{
      //this->Tool->Buffer->AddMessage( this->currentTask, currentRecord->GetTime() );
      this->PrevTask = this->CurrentTask;
	}

	this->IndexToProcess++;
  }

}


std::string vtkWorkflowAlgorithm
::GetCurrentTask()
{
  if ( this->Tool->Procedure->IsTask( this->CurrentTask ) )
  {
    return "-";
  }
  return this->CurrentTask;
}


std::string vtkWorkflowAlgorithm
::GetCurrentInstruction()
{
  if ( this->Tool->Procedure->IsTask( this->CurrentTask ) )
  {
    return "-";
  }
  return this->Tool->Procedure->GetTaskByName( this->CurrentTask )->Instruction;
}


std::string vtkWorkflowAlgorithm
::GetNextTask()
{
  if ( this->Tool->Procedure->IsTask( this->CurrentTask ) )
  {
    return "-";
  }

  std::string nextTaskName = this->Tool->Procedure->GetTaskByName( this->CurrentTask )->Next;

  if ( this->Tool->Procedure->IsTask( nextTaskName ) )
  {
    return "-";
  }

  return nextTaskName;
}


std::string vtkWorkflowAlgorithm
::GetNextInstruction()
{
  if ( this->Tool->Procedure->IsTask( this->CurrentTask ) )
  {
    return "-";
  }

  std::string nextTaskName = this->Tool->Procedure->GetTaskByName( this->CurrentTask )->Next;

  if ( this->Tool->Procedure->IsTask( nextTaskName ) )
  {
    return "-";
  }

  return this->Tool->Procedure->GetTaskByName( nextTaskName )->Instruction;
}