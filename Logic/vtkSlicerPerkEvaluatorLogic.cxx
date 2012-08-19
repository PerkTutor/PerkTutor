
// PerkEvaluator Logic includes
#include "vtkSlicerPerkEvaluatorLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cassert>
#include <limits>



vtkTransform*
StrToTransform( std::string str, vtkTransform* tr )
{
  std::stringstream ss( str );
  
  double e00; ss >> e00; double e01; ss >> e01; double e02; ss >> e02; double e03; ss >> e03;
  double e10; ss >> e10; double e11; ss >> e11; double e12; ss >> e12; double e13; ss >> e13;
  double e20; ss >> e20; double e21; ss >> e21; double e22; ss >> e22; double e23; ss >> e23;
  double e30; ss >> e30; double e31; ss >> e31; double e32; ss >> e32; double e33; ss >> e33;
  
  tr->Identity();
  
  tr->GetMatrix()->SetElement( 0, 0, e00 );
  tr->GetMatrix()->SetElement( 0, 1, e01 );
  tr->GetMatrix()->SetElement( 0, 2, e02 );
  tr->GetMatrix()->SetElement( 0, 3, e03 );
  
  tr->GetMatrix()->SetElement( 1, 0, e10 );
  tr->GetMatrix()->SetElement( 1, 1, e11 );
  tr->GetMatrix()->SetElement( 1, 2, e12 );
  tr->GetMatrix()->SetElement( 1, 3, e13 );
  
  tr->GetMatrix()->SetElement( 2, 0, e20 );
  tr->GetMatrix()->SetElement( 2, 1, e21 );
  tr->GetMatrix()->SetElement( 2, 2, e22 );
  tr->GetMatrix()->SetElement( 2, 3, e23 );
  
  tr->GetMatrix()->SetElement( 3, 0, e30 );
  tr->GetMatrix()->SetElement( 3, 1, e31 );
  tr->GetMatrix()->SetElement( 3, 2, e32 );
  tr->GetMatrix()->SetElement( 3, 3, e33 );
  
  return tr;
}



//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerPerkEvaluatorLogic);



void vtkSlicerPerkEvaluatorLogic
::SetMarkBegin( double begin )
{
  if ( begin <= this->GetMinTime() )
  {
    this->MarkBegin = this->GetMinTime();
  }
  else if ( begin >= this->GetMaxTime() )
  {
    this->MarkBegin = this->GetMaxTime();
  }
  else
  {
    this->MarkBegin = begin;
  }
}



void vtkSlicerPerkEvaluatorLogic
::SetMarkEnd( double end )
{
  if ( end <= this->GetMinTime() )
  {
    this->MarkEnd = this->GetMinTime();
  }
  else if ( end >= this->GetMaxTime() )
  {
    this->MarkEnd = this->GetMaxTime();
  }
  else
  {
    this->MarkEnd = end;
  }
}



void vtkSlicerPerkEvaluatorLogic
::Analyse()
{
    // Check conditions.
  
  if (    this->MarkBegin >= this->MarkEnd
       || this->MarkBegin < this->GetMinTime()
       || this->MarkEnd > this->GetMaxTime() )
  {
    this->Metrics.clear();
    return;
  }
  
  
    // Compute metrics.
  
  for ( TrajectoryContainerType::iterator tIt = this->ToolTrajectories.begin();
        tIt != this->ToolTrajectories.end(); ++ tIt )
  {
    std::string toolName = (*tIt)->GetToolName();
    
    MetricType pathLength;
    pathLength.first = toolName + " path length (mm)";
    pathLength.second = this->PathLength( this->MarkBegin, this->MarkEnd, (*tIt) );
    
    this->Metrics.push_back( pathLength );
  }
}



vtkSlicerPerkEvaluatorLogic::MetricVectorType
vtkSlicerPerkEvaluatorLogic
::GetMetrics()
{
  return this->Metrics;
}



vtkSlicerPerkEvaluatorLogic
::vtkSlicerPerkEvaluatorLogic()
{
  this->PlaybackTime = 0.0;
  this->MarkBegin = 0.0;
  this->MarkEnd = 0.0;
}


//----------------------------------------------------------------------------
vtkSlicerPerkEvaluatorLogic::~vtkSlicerPerkEvaluatorLogic()
{
}


//----------------------------------------------------------------------------
void vtkSlicerPerkEvaluatorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}



/**
 * Read XML file that was written by TransformRecorder module.
 */
void vtkSlicerPerkEvaluatorLogic
::ImportFile( std::string fileName )
{
  this->ClearData();
  
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( fileName.c_str() );
  parser->Parse();
  
  vtkXMLDataElement* element = parser->GetRootElement();
  if ( ! element )
  {
    return;
  }
  
  int num = element->GetNumberOfNestedElements();  // Number of saved records (including transforms and messages).
  
  for ( int i = 0; i < num; ++ i )
  {
    vtkXMLDataElement* noteElement = element->GetNestedElement( i );
    if ( strcmp( noteElement->GetName(), "log" ) != 0 )
    {
      continue;  // If it's not a "log", jump to the next.
    }
    const char* type = noteElement->GetAttribute( "type" );
    
    vtkTransformTimeSeries* trajectory = NULL;
    if ( strcmp( type, "transform" ) == 0 )
    {
      const char* deviceName = noteElement->GetAttribute( "DeviceName" );
      trajectory = this->UpdateToolList( std::string( deviceName ) );
    }
    
    
    double time = this->GetTimestampFromElement( noteElement );
    
    
    if ( strcmp( type, "message" ) == 0 )
    {
      AnnotationType annotation;
      annotation.first = time;
      annotation.second = std::string( noteElement->GetAttribute( "message" ) );
      this->Annotations.push_back( annotation );
    }

    else if ( strcmp( type, "transform" ) == 0 )
    {
      const char* traC = noteElement->GetAttribute( "transform" );
      vtkSmartPointer< vtkTransform > tr = vtkSmartPointer< vtkTransform >::New();
      StrToTransform( std::string( traC ), tr );
      trajectory->AddRecord( time, tr );
    }
  }
  
  this->CreateTransformNodes();
}



double vtkSlicerPerkEvaluatorLogic
::GetTotalTime() const
{
  double minTime = this->GetMinTime();
  double maxTime = this->GetMaxTime();
  
  double totalTime = maxTime - minTime;
  
  if ( totalTime < 0 )
  {
    return 0.0;
  }
  else
  {
    return totalTime;
  }
}



double vtkSlicerPerkEvaluatorLogic
::GetMinTime() const
{
  double minTime = std::numeric_limits< double >::max();
  
  for ( TrajectoryContainerType::const_iterator tIt = this->ToolTrajectories.begin();
        tIt != this->ToolTrajectories.end(); ++ tIt )
  {
    if ( (*tIt)->GetMinTime() < minTime )
    {
      minTime = (*tIt)->GetMinTime();
    }
  }
  
  return minTime;
}



double vtkSlicerPerkEvaluatorLogic
::GetMaxTime() const
{
  double maxTime = std::numeric_limits< double >::min();
  
  for ( TrajectoryContainerType::const_iterator tIt = this->ToolTrajectories.begin();
        tIt != this->ToolTrajectories.end(); ++ tIt )
  {
    if ( (*tIt)->GetMaxTime() > maxTime )
    {
      maxTime = (*tIt)->GetMaxTime();
    }
  }
  
  return maxTime;
}



double vtkSlicerPerkEvaluatorLogic
::GetPlaybackTime() const
{
  return this->PlaybackTime;
}



void vtkSlicerPerkEvaluatorLogic
::SetPlaybackTime( double time )
{
  if ( time < this->GetMinTime()  ||  time > this->GetMaxTime() )
  {
    return;
  }
  
  this->PlaybackTime = time;
  
  for ( TrajectoryContainerType::iterator tIt = this->ToolTrajectories.begin();
        tIt != this->ToolTrajectories.end(); ++ tIt )
  {
    std::string toolName = (*tIt)->GetToolName();
    int currentIndex = 0;
    for ( int i = 1; i < (*tIt)->GetNumberOfRecords(); ++ i )
    {
      if ( time < (*tIt)->GetTimeAtIndex( i ) )
      {
        currentIndex = i - 1;
        break;
      }
    }
    
    vtkCollection* collection = this->GetMRMLScene()->GetNodesByName( toolName.c_str() );
    collection->InitTraversal();
    vtkMRMLLinearTransformNode* tNode = NULL;
    if ( collection->GetNumberOfItems() > 0 )
    {
      tNode = vtkMRMLLinearTransformNode::SafeDownCast( collection->GetItemAsObject( 0 ) );
    }
    if ( tNode != NULL )
    {
      tNode->GetMatrixTransformToParent()->DeepCopy( (*tIt)->GetMatrixAtIndex( currentIndex ) );
    }
    collection->Delete();
  }
}



//---------------------------------------------------------------------------
void vtkSlicerPerkEvaluatorLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerPerkEvaluatorLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}


//---------------------------------------------------------------------------
void vtkSlicerPerkEvaluatorLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}


//---------------------------------------------------------------------------
void vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node))
{
}


//---------------------------------------------------------------------------
void vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node))
{
}


void vtkSlicerPerkEvaluatorLogic
::ClearData()
{
  this->ToolTrajectories.clear();
  this->Annotations.clear();
}



double vtkSlicerPerkEvaluatorLogic
::GetTimestampFromElement( vtkXMLDataElement* element )
{
    // Check if time was coded according to the old or new fashion.

  int numAttribs = element->GetNumberOfAttributes();
  bool old_time_style = false;
  for ( int j = 0; j < numAttribs; ++ j )
  {
    const char* attName = element->GetAttributeName( j );
    if ( !strcmp( attName, "time" ) )
    {
      old_time_style = true;
      break;
    }
  }
  
  double time = 0.0;
  if ( old_time_style )
  {
    double time = atof( element->GetAttribute( "time" ) );
  }
  else
  {
    double time_sec = atof( element->GetAttribute( "TimeStampSec" ) );
    double time_nsec = atof( element->GetAttribute( "TimeStampNSec" ) );
    time = time_sec + time_nsec * 1e-9;
  }

  return time;
}



vtkTransformTimeSeries* vtkSlicerPerkEvaluatorLogic
::UpdateToolList( std::string name )
{
    // Check if any of the existing tools have this name.
  
  for ( TrajectoryContainerType::iterator tIt = this->ToolTrajectories.begin();
        tIt != this->ToolTrajectories.end(); ++ tIt )
  {
    if ( name.compare( (*tIt)->GetToolName() ) == 0 )
    {
      return (*tIt);
    };
  }
  
    // Create a new tool trajectory.
  
  vtkSmartPointer< vtkTransformTimeSeries > tts = vtkSmartPointer< vtkTransformTimeSeries >::New();
  tts->SetToolName( name );
  this->ToolTrajectories.push_back( tts );
  return tts;
}



void vtkSlicerPerkEvaluatorLogic
::CreateTransformNodes()
{
  for ( TrajectoryContainerType::iterator tIt = this->ToolTrajectories.begin();
        tIt != this->ToolTrajectories.end(); ++ tIt )
  {
    std::string toolName = (*tIt)->GetToolName();
    vtkCollection* collection = this->GetMRMLScene()->GetNodesByName( toolName.c_str() );
    vtkMRMLLinearTransformNode* tNode = NULL;
    if ( collection->GetNumberOfItems() > 0 )
    {
      tNode = vtkMRMLLinearTransformNode::SafeDownCast( collection->GetItemAsObject( 0 ) );
    }
    if ( tNode == NULL )
    {
      vtkSmartPointer< vtkMRMLLinearTransformNode > newNode = vtkSmartPointer< vtkMRMLLinearTransformNode >::New();
      newNode->SetName( toolName.c_str() );
      newNode->SetScene( this->GetMRMLScene() );
      this->GetMRMLScene()->AddNode( newNode );
    }
    collection->Delete();
  }
}



double vtkSlicerPerkEvaluatorLogic
::PathLength( double From, double To, vtkTransformTimeSeries* Trajectory )
{
  double length = 0.0;
  
  double O[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  double P0[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  double P1[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  
  vtkSmartPointer< vtkMatrix4x4 > M0;
  vtkSmartPointer< vtkMatrix4x4 > M1;
  
  for ( int i = 1; i < Trajectory->GetNumberOfRecords(); ++ i )
  {
    if ( Trajectory->GetTimeAtIndex( i ) < From )
    {
      continue;
    }
    
    if ( Trajectory->GetTimeAtIndex( i ) > To )
    {
      break;
    }
    
    M0 = Trajectory->GetMatrixAtIndex( i - 1 );
    M1 = Trajectory->GetMatrixAtIndex( i );
    
    M0->MultiplyPoint( O, P0 );
    M1->MultiplyPoint( O, P1 );
    
    double distance = sqrt( vtkMath::Distance2BetweenPoints( P0, P1 ) );
    
    length += distance;
  }
  
  return length;
}
