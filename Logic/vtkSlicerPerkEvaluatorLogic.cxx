
// PerkEvaluator Logic includes
#include "vtkSlicerPerkEvaluatorLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLTransformNode.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkModifiedBSPTree.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkSmartPointer.h>

// STD includes
#include <cassert>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>



void PrintToFile( std::string str )
{
  ofstream o( "PerkEvaluatorLog.txt", std::ios_base::app );
  int c = clock();
  o << std::fixed << setprecision( 2 ) << ( c / (double)CLOCKS_PER_SEC ) << " : " << str << std::endl;
  o.close();
}



vtkTransform*
StrToTransform( std::string str, vtkTransform* tr )
{
  std::stringstream ss( str );
  
  double e00; ss >> e00; double e01; ss >> e01; double e02; ss >> e02; double e03; ss >> e03;
  double e10; ss >> e10; double e11; ss >> e11; double e12; ss >> e12; double e13; ss >> e13;
  double e20; ss >> e20; double e21; ss >> e21; double e22; ss >> e22; double e23; ss >> e23;
  double e30; ss >> e30; double e31; ss >> e31; double e32; ss >> e32; double e33; ss >> e33;
  
  tr->Identity();
  
  vtkSmartPointer< vtkMatrix4x4 > matrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  
  matrix->SetElement( 0, 0, e00 );
  matrix->SetElement( 0, 1, e01 );
  matrix->SetElement( 0, 2, e02 );
  matrix->SetElement( 0, 3, e03 );
  
  matrix->SetElement( 1, 0, e10 );
  matrix->SetElement( 1, 1, e11 );
  matrix->SetElement( 1, 2, e12 );
  matrix->SetElement( 1, 3, e13 );
  
  matrix->SetElement( 2, 0, e20 );
  matrix->SetElement( 2, 1, e21 );
  matrix->SetElement( 2, 2, e22 );
  matrix->SetElement( 2, 3, e23 );
  
  matrix->SetElement( 3, 0, e30 );
  matrix->SetElement( 3, 1, e31 );
  matrix->SetElement( 3, 2, e32 );
  matrix->SetElement( 3, 3, e33 );
  
  tr->SetMatrix( matrix );
  
  return tr;
}

std::string
TransformToStr( vtkTransform* tr )
{
	std::stringstream ss( "" );

	ss << tr->GetMatrix()->GetElement( 0, 0 ) << " ";
	ss << tr->GetMatrix()->GetElement( 0, 1 ) << " ";
	ss << tr->GetMatrix()->GetElement( 0, 2 ) << " ";
	ss << tr->GetMatrix()->GetElement( 0, 3 ) << " ";

	ss << tr->GetMatrix()->GetElement( 1, 0 ) << " ";
	ss << tr->GetMatrix()->GetElement( 1, 1 ) << " ";
	ss << tr->GetMatrix()->GetElement( 1, 2 ) << " ";
	ss << tr->GetMatrix()->GetElement( 1, 3 ) << " ";

	ss << tr->GetMatrix()->GetElement( 2, 0 ) << " ";
	ss << tr->GetMatrix()->GetElement( 2, 1 ) << " ";
	ss << tr->GetMatrix()->GetElement( 2, 2 ) << " ";
	ss << tr->GetMatrix()->GetElement( 2, 3 ) << " ";

	ss << tr->GetMatrix()->GetElement( 3, 0 ) << " ";
	ss << tr->GetMatrix()->GetElement( 3, 1 ) << " ";
	ss << tr->GetMatrix()->GetElement( 3, 2 ) << " ";
	ss << tr->GetMatrix()->GetElement( 3, 3 ) << " ";

	return ss.str();
}


//----------------------------------------------------------------------------



vtkStandardNewMacro( vtkSlicerPerkEvaluatorLogic );



void
vtkSlicerPerkEvaluatorLogic
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



void
vtkSlicerPerkEvaluatorLogic
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



vtkSlicerPerkEvaluatorLogic::AnnotationVectorType
vtkSlicerPerkEvaluatorLogic
::GetAnnotations()
{
  return this->Annotations;
}


void
vtkSlicerPerkEvaluatorLogic
::AddAnnotation( std::string annString )
{
  AnnotationType annotation;
  annotation.first = this->GetPlaybackTime();
  annotation.second = annString;
  this->Annotations.push_back( annotation );
}



void
vtkSlicerPerkEvaluatorLogic
::RemoveAnnotation( int row )
{
  Annotations.erase( Annotations.begin() + row );
}



void
vtkSlicerPerkEvaluatorLogic
::SaveAnnotations( std::string fileName )
{

  std::ofstream output( fileName.c_str() );
  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Record file could not be opened!" );
    return;
  }
  
  output << "<TransformRecorderLog>" << std::endl;

  // Save transforms  
  for ( TrajectoryContainerType::iterator tIt = this->ToolTrajectories.begin();
        tIt != this->ToolTrajectories.end(); ++ tIt )
  {
    for ( unsigned int i = 0; i < (*tIt)->GetNumberOfRecords(); ++ i )
    {
      double currTime = (*tIt)->GetTimeAtIndex( i );
	  int intPart = floor( currTime );
	  int fracPart = floor( ( currTime - intPart ) * 1e9 );
	  vtkSmartPointer< vtkMatrix4x4 > currMatrix = (*tIt)->GetMatrixAtIndex( i );
	  vtkSmartPointer< vtkTransform > currTr = vtkSmartPointer< vtkTransform >::New();
	  currTr->SetMatrix( vtkMatrix4x4::SafeDownCast( currMatrix ) );

      output << "  <log";	  
      output << " TimeStampSec=\"" << intPart << "\"";
      output << " TimeStampNSec=\"" << fracPart << "\"";
      output << " type=\"transform\"";
      output << " DeviceName=\"" << (*tIt)->GetToolName() << "\"";
	  output << " transform=\"" << TransformToStr( currTr ) << "\"";
      output << " />" << std::endl;
    }
  }
 
  // Save annotations.
  
  for ( int i = 0; i < Annotations.size(); ++ i )
  {
    int intPart = floor( Annotations[i].first );
	int fracPart = floor( ( Annotations[i].first - intPart ) * 1e9 );
    output << "  <log";
    output << " TimeStampSec=\"" << intPart << "\"";
    output << " TimeStampNSec=\"" << fracPart << "\"";
    output << " type=\"message\"";
    output << " message=\"" << Annotations[i].second << "\"";
    output << " />" << std::endl;
  }
  
  
  output << "</TransformRecorderLog>" << std::endl;
  output.close();

}



void
vtkSlicerPerkEvaluatorLogic
::Analyse()
{
  this->Metrics.clear();
  
  
    // Check conditions.
  
  if (    this->MarkBegin >= this->MarkEnd
       || this->MarkBegin < this->GetMinTime()
       || this->MarkEnd > this->GetMaxTime() )
  {
    return;
  }
  
  
    // Compute common metrics.
  
  MetricType procedureTime;
  procedureTime.first = "Total procedure time (s)";
  procedureTime.second = this->MarkEnd - this->MarkBegin;
  this->Metrics.push_back( procedureTime );
  
  
    // Compute metrics for each trajectory.
  
  for ( TrajectoryContainerType::iterator tIt = this->ToolTrajectories.begin();
        tIt != this->ToolTrajectories.end(); ++ tIt )
  {
    std::string toolName = (*tIt)->GetToolName();
    this->AnalyseTrajectory( *tIt );
  }
  
  
    // Compute tissue damage by needle.
  
  if ( this->NeedleTransformNode != NULL )
  {
    this->AnalyseNeedle( this->NeedleTransformNode );
  }
}



vtkSlicerPerkEvaluatorLogic::MetricVectorType
vtkSlicerPerkEvaluatorLogic
::GetMetrics()
{
  return this->Metrics;
}



void
vtkSlicerPerkEvaluatorLogic
::SetBodyModelNode( vtkMRMLModelNode* node )
{
  vtkSetMRMLNodeMacro( this->BodyModelNode, node );
  this->Modified();
}



void
vtkSlicerPerkEvaluatorLogic
::SetNeedleTransformNode( vtkMRMLLinearTransformNode* node )
{
  vtkSetMRMLNodeMacro( this->NeedleTransformNode, node );
  this->Modified();
}



// Constructor
// 
vtkSlicerPerkEvaluatorLogic
::vtkSlicerPerkEvaluatorLogic()
{
  this->PlaybackTime = 0.0;
  this->MarkBegin = 0.0;
  this->MarkEnd = 0.0;
  
  this->BodyModelNode = NULL;
  this->NeedleTransformNode = NULL;
}



vtkSlicerPerkEvaluatorLogic::
~vtkSlicerPerkEvaluatorLogic()
{
  if ( this->BodyModelNode != NULL )
  {
    this->BodyModelNode->Delete();
    this->BodyModelNode = NULL;
  }
  
  if ( this->NeedleTransformNode != NULL )
  {
    this->NeedleTransformNode->Delete();
    this->NeedleTransformNode = NULL;
  }
}



void
vtkSlicerPerkEvaluatorLogic
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  
  os << indent << "vtkSlicerPerkEvaluatorLogic: " << this->GetClassName() << "\n";
  os << indent << "BodyModelNode: " << ( this->BodyModelNode ? this->BodyModelNode->GetName() : "(none)" ) << "\n";
  os << indent << "NeedleTransformNode: " << ( this->NeedleTransformNode ? this->NeedleTransformNode->GetName() : "(none)" ) << "\n";
}



void
vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneEndClose()
{
  this->ClearData();
}



/**
 * Read XML file that was written by TransformRecorder module.
 */
void
vtkSlicerPerkEvaluatorLogic
::ImportFile( std::string fileName )
{
  this->ClearData();

  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( fileName.c_str() );
  parser->Parse();
  // PrintToFile( "End XML parser" ); // debug
  
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
      const char* msg = noteElement->GetAttribute( "message" );
      if ( msg != NULL )
      {
        annotation.second = std::string( msg );
      }
      else
      {
        annotation.second = "(none)";
      }
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



double
vtkSlicerPerkEvaluatorLogic
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



double
vtkSlicerPerkEvaluatorLogic
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



double
vtkSlicerPerkEvaluatorLogic
::GetPlaybackTime() const
{
  return this->PlaybackTime;
}



void
vtkSlicerPerkEvaluatorLogic
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



void
vtkSlicerPerkEvaluatorLogic
::SetMRMLSceneInternal( vtkMRMLScene * newScene )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}



void
vtkSlicerPerkEvaluatorLogic
::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}



void
vtkSlicerPerkEvaluatorLogic
::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}



void
vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneNodeAdded( vtkMRMLNode* vtkNotUsed( node ) )
{
}



void
vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneNodeRemoved( vtkMRMLNode* vtkNotUsed( node ) )
{
}



void
vtkSlicerPerkEvaluatorLogic
::ClearData()
{
  this->ToolTrajectories.clear();
  this->Annotations.clear();
  this->Metrics.clear();
  
  this->MarkBegin = 0.0;
  this->MarkEnd = 0.0;
  this->PlaybackTime = 0.0;
}



double
vtkSlicerPerkEvaluatorLogic
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



vtkTransformTimeSeries*
vtkSlicerPerkEvaluatorLogic
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



void
vtkSlicerPerkEvaluatorLogic
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



void
vtkSlicerPerkEvaluatorLogic
::AnalyseTrajectory( vtkTransformTimeSeries* Trajectory )
{
  double length = 0.0;
  double insideLength = 0.0;
  double insideTime = 0.0;
  
  double O[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  double P0[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  double P1[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  
  vtkSmartPointer< vtkMatrix4x4 > M0;
  vtkSmartPointer< vtkMatrix4x4 > M1;
  
  
    // Check if the corresponding transform has any parent transforms.
  
  vtkSmartPointer< vtkMatrix4x4 > ParentMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  ParentMatrix->Identity();
  vtkCollection* collection = this->GetMRMLScene()->GetNodesByName( Trajectory->GetToolName().c_str() );
  vtkMRMLLinearTransformNode* tNode = NULL;
  if ( collection->GetNumberOfItems() > 0 )
  {
    tNode = vtkMRMLLinearTransformNode::SafeDownCast( collection->GetItemAsObject( 0 ) );
  }
  if ( tNode != NULL )
  {
    vtkMRMLTransformNode* ptNode = tNode->GetParentTransformNode();
    if ( ptNode != NULL )
    {
      ptNode->GetMatrixTransformToWorld( ParentMatrix );
    }
  }
  collection->Delete();
  
  
    // Prepare inside-outside body measurements.
  
  vtkPolyData* body = NULL;
  vtkSmartPointer< vtkSelectEnclosedPoints > EnclosedFilter = vtkSmartPointer< vtkSelectEnclosedPoints >::New();
  if ( this->BodyModelNode != NULL )
  {
    body = this->BodyModelNode->GetPolyData();
    // EnclosedFilter->SetSurface( body );
    EnclosedFilter->Initialize( body );
  }
  
  
  for ( int i = 1; i < Trajectory->GetNumberOfRecords(); ++ i )
  {
    if ( Trajectory->GetTimeAtIndex( i ) < this->MarkBegin )
    {
      continue;
    }
    
    if ( Trajectory->GetTimeAtIndex( i ) > this->MarkEnd )
    {
      break;
    }
    
    M0 = Trajectory->GetMatrixAtIndex( i - 1 );
    M1 = Trajectory->GetMatrixAtIndex( i );
    
    vtkMatrix4x4::Multiply4x4( ParentMatrix, M0, M0 );
    vtkMatrix4x4::Multiply4x4( ParentMatrix, M1, M1 );
    
    M0->MultiplyPoint( O, P0 );
    M1->MultiplyPoint( O, P1 );
    
    double distance = sqrt( vtkMath::Distance2BetweenPoints( P0, P1 ) );
    
    length += distance;
    
    
      // Check if current point is inside the body.
    
    int Inside = 0;
    if ( body != NULL )
    {
      Inside = EnclosedFilter->IsInsideSurface( P1[ 0 ], P1[ 1 ], P1[ 2 ] );
    }
    
    if ( Inside )
    {
      insideLength += distance;
      insideTime += Trajectory->GetTimeAtIndex( i ) - Trajectory->GetTimeAtIndex( i - 1 );
    }
  }
  
  
    // Recording metrics.
  
  MetricType PathLength;
  PathLength.first = Trajectory->GetToolName() + " path length (mm)";
  PathLength.second = length;
  this->Metrics.push_back( PathLength );
  
  MetricType InsidePathLength;
  InsidePathLength.first = Trajectory->GetToolName() + " inside path (mm)";
  InsidePathLength.second = insideLength;
  this->Metrics.push_back( InsidePathLength );
  
  MetricType InsideTime;
  InsideTime.first = Trajectory->GetToolName() + " inside time (s)";
  InsideTime.second = insideTime;
  this->Metrics.push_back( InsideTime );
}



/**
 * Needle has a specific direction. Long in the positive Y direction (A in RAS coordinate system).
 * Using this condition, we can calculate the potential tissue damage.
 * @param tnode Transform directly under the needle model node.
 */
void
vtkSlicerPerkEvaluatorLogic
::AnalyseNeedle( vtkMRMLLinearTransformNode* tnode )
{
  if ( tnode == NULL )
  {
    return;
  }
  
  if ( this->BodyModelNode == NULL )
  {
    return;  // No body -> no tissue damage.
  }
  
  
    // Check if this needle transform is on a recorded trajectory.
  
  vtkMRMLTransformNode* parent = tnode->GetParentTransformNode();
  
  if ( parent == NULL )
  {
    return;
  }
  
  vtkTransformTimeSeries* ParentTrajectory = NULL;
  for ( TrajectoryContainerType::iterator tIt = this->ToolTrajectories.begin();
        tIt < this->ToolTrajectories.end(); ++ tIt )
  {
    if ( (*tIt)->GetToolName().compare( parent->GetName() ) == 0 )
    {
      ParentTrajectory = *tIt;
    }
  }
  
  if ( ParentTrajectory == NULL )
  {
    return;
  }
  
  
    // Analyse recorded needle trajectory.
  
    // Check if the parent transform has any further parent transform.
  
  vtkSmartPointer< vtkMatrix4x4 > GrandParentMatrix = vtkSmartPointer< vtkMatrix4x4 >::New();
  GrandParentMatrix->Identity();
  vtkCollection* collection = this->GetMRMLScene()->GetNodesByName( ParentTrajectory->GetToolName().c_str() );
  vtkMRMLLinearTransformNode* GrandParentTransformNode = NULL;
  if ( collection->GetNumberOfItems() > 0 )
  {
    GrandParentTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( collection->GetItemAsObject( 0 ) );
  }
  if ( GrandParentTransformNode != NULL )
  {
    vtkMRMLTransformNode* ptNode = GrandParentTransformNode->GetParentTransformNode();
    if ( ptNode != NULL )
    {
      ptNode->GetMatrixTransformToWorld( GrandParentMatrix );
    }
  }
  collection->Delete();
  
  
    // Prepare inside-outside body measurements.
  
  double O[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };     // Needle tip in the needle tip coordinate system.
  double OB[ 4 ] = { 0.0, 300.0, 0.0, 1.0 };  // Needle base in the needle tip coordinate system. 300 mm is an assumption.
  double pcoords[ 3 ] = { 0.0, 0.0, 0.0 };    // Not used. Placeholder for function signature.
  double t = 0.0;                             // Parametric coordinate of intersection. Not used.
  double tolerance = 0.001;
  int    subId;
  
  double P0[ 4 ] = { 0.0, 0.0, 0.0, 1.0 }; // Needle tip at time 0.
  double P1[ 4 ] = { 0.0, 0.0, 0.0, 1.0 }; // Needle tip at time 1.
  
  double B0[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };  // Needle base at time 0.
  double B1[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };  // Needle base at time 0.
  
  double I0[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };  // Needle entry point at time 0.
  double I1[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };  // Needle entry point at time 1.
  
  vtkSmartPointer< vtkMatrix4x4 > M0;  // NeedleTip-To-RAS at time 0.
  vtkSmartPointer< vtkMatrix4x4 > M1;  // NeedleTip-To-RAS at time 1.
  
  vtkMatrix4x4* NeedleTipMatrix = tnode->GetMatrixTransformToParent();
  
  double PTT = 0.0; // Potential tissue damage. Area swept by the needle inside the tissue.
  
  vtkSmartPointer< vtkSelectEnclosedPoints > EnclosedFilter = vtkSmartPointer< vtkSelectEnclosedPoints >::New();
  EnclosedFilter->Initialize( this->BodyModelNode->GetPolyData() );
  
  vtkSmartPointer< vtkModifiedBSPTree > bspTree = vtkSmartPointer< vtkModifiedBSPTree >::New();
  bspTree->SetDataSet( this->BodyModelNode->GetPolyData() );
  bspTree->BuildLocator();
  
  for ( int i = 1; i < ParentTrajectory->GetNumberOfRecords(); ++ i )
  {
    if ( ParentTrajectory->GetTimeAtIndex( i ) < this->MarkBegin )
    {
      continue;
    }
    
    if ( ParentTrajectory->GetTimeAtIndex( i ) > this->MarkEnd )
    {
      break;
    }
    
    M0 = ParentTrajectory->GetMatrixAtIndex( i - 1 );
    M1 = ParentTrajectory->GetMatrixAtIndex( i );
    
      // Transform matrix composition: M = GrandParentMatrix * PartentMatrix * NeedleTipMatrix
    
    vtkMatrix4x4::Multiply4x4( M0, NeedleTipMatrix, M0 );
    vtkMatrix4x4::Multiply4x4( M1, NeedleTipMatrix, M1 );
    
    vtkMatrix4x4::Multiply4x4( GrandParentMatrix, M0, M0 );
    vtkMatrix4x4::Multiply4x4( GrandParentMatrix, M1, M1 );
    
    M0->MultiplyPoint( O, P0 );
    M1->MultiplyPoint( O, P1 );
    
    M0->MultiplyPoint( OB, B0 );
    M1->MultiplyPoint( OB, B1 );
    
    
      // Check if current point is inside the body.
    
    int Inside = 0;
    if (    EnclosedFilter->IsInsideSurface( P0[ 0 ], P0[ 1 ], P0[ 2 ] )
         && EnclosedFilter->IsInsideSurface( P1[ 0 ], P1[ 1 ], P1[ 2 ] ) )
    {
      Inside = true;
    }
    
    if ( Inside )
    {
        // Compute needle entry points.
      
      bspTree->IntersectWithLine( P0, B0, tolerance, t, I0, pcoords, subId );
      bspTree->IntersectWithLine( P1, B1, tolerance, t, I1, pcoords, subId );
      
      /*
      //debug
      std::stringstream ss;
      ss << "Time: " << ParentTrajectory->GetTimeAtIndex( i ) << std::endl;
      ss << "P0: " << P0[ 0 ] << "  " << P0[ 1 ] << "  " << P0[ 2 ] << std::endl;
      ss << "P1: " << P1[ 0 ] << "  " << P1[ 1 ] << "  " << P1[ 2 ] << std::endl;
      ss << "B0: " << B0[ 0 ] << "  " << B0[ 1 ] << "  " << B0[ 2 ] << std::endl;
      ss << "B1: " << B1[ 0 ] << "  " << B1[ 1 ] << "  " << B1[ 2 ] << std::endl;
      ss << "I0: " << I0[ 0 ] << "  " << I0[ 1 ] << "  " << I0[ 2 ] << std::endl;
      ss << "I1: " << I1[ 0 ] << "  " << I1[ 1 ] << "  " << I1[ 2 ] << std::endl;
      PrintToFile( ss.str() );
      */
      
        // Compute area of two triangles.
      
      PTT += SpanArea( I0, I1, P0, P1 );
    }
  }
  
  
    // Record this metric.
  
  MetricType PTTMetric;
  PTTMetric.first = "Potential tissue damage (mm2)";
  PTTMetric.second = PTT;
  
  this->Metrics.push_back( PTTMetric );
  
}



double
vtkSlicerPerkEvaluatorLogic
::SpanArea( double* e1, double* e2, double* t1, double* t2 )
{
    // Approximation: summed area of two triangles, (e1,t1,e2) and (t1,e2,t2).
    // Area of a triangle: cross-product of two edge vectors.  
  
    // First triangle: v = e2 - e1; w = t1 = e1;
  
  double v[ 3 ] = { 0, 0, 0 };
  double w[ 3 ] = { 0, 0, 0 };
  
  for ( int i = 0; i < 3; ++ i )
    {
    v[ i ] = e2[ i ] - e1[ i ];
    w[ i ] = t1[ i ] - e1[ i ];
    }
  
  double cprod[ 3 ] = { 0, 0, 0 };
  vtkMath::Cross( v, w, cprod );
  double area1 = std::sqrt( cprod[ 0 ] * cprod[ 0 ]  +  cprod[ 1 ] * cprod[ 1 ]  +  cprod[ 2 ] * cprod[ 2 ] );
  
    // Second triangle: v = e2 - t1; w = t2 - t1;
  
  for ( int i = 0; i < 3; ++ i )
    {
    v[ i ] = e2[ i ] - t1[ i ];
    w[ i ] = t2[ i ] - t1[ i ];
    }
  
  vtkMath::Cross( v, w, cprod );
  double area2 = std::sqrt( cprod[ 0 ] * cprod[ 0 ]  +  cprod[ 1 ] * cprod[ 1 ]  +  cprod[ 2 ] * cprod[ 2 ] );
  
  return ( area1 + area2 );
}
