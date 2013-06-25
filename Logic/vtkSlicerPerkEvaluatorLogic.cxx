
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



// Helper functions ------------------------------------------------------------------------


double*
MatrixStrToDouble( std::string str )
{
  std::stringstream ss( str );
  
  double e00; ss >> e00; double e01; ss >> e01; double e02; ss >> e02; double e03; ss >> e03;
  double e10; ss >> e10; double e11; ss >> e11; double e12; ss >> e12; double e13; ss >> e13;
  double e20; ss >> e20; double e21; ss >> e21; double e22; ss >> e22; double e23; ss >> e23;
  double e30; ss >> e30; double e31; ss >> e31; double e32; ss >> e32; double e33; ss >> e33;

  double* dmat = new double[16];

  dmat[0] = e00;
  dmat[1] = e01;
  dmat[2] = e02;
  dmat[3] = e03;

  dmat[4] = e10;
  dmat[5] = e11;
  dmat[6] = e12;
  dmat[7] = e13;

  dmat[8] = e20;
  dmat[9] = e21;
  dmat[10] = e22;
  dmat[11] = e23;

  dmat[12] = e30;
  dmat[13] = e31;
  dmat[14] = e32;
  dmat[15] = e33;
  
  return dmat;
}


//----------------------------------------------------------------------------

vtkStandardNewMacro( vtkSlicerPerkEvaluatorLogic );


// Constructors and Desctructors ----------------------------------------------

vtkSlicerPerkEvaluatorLogic
::vtkSlicerPerkEvaluatorLogic()
{
  this->PlaybackTime = 0.0;
  this->MarkBegin = 0.0;
  this->MarkEnd = 0.0;
  
  this->BodyModelNode = NULL;
  this->NeedleTransformNode = NULL;

  this->TransformRecorderLogic = NULL;

  this->Parser = vtkXMLDataParser::New();
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
  this->Parser->Delete();
}


void vtkSlicerPerkEvaluatorLogic
::ClearData()
{
  this->ToolTrajectories.clear();
  
  this->MarkBegin = 0.0;
  this->MarkEnd = 0.0;
  this->PlaybackTime = 0.0;
}



// Slicer functions ---------------------------------------------------------------

void vtkSlicerPerkEvaluatorLogic
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  
  os << indent << "vtkSlicerPerkEvaluatorLogic: " << this->GetClassName() << "\n";
  os << indent << "BodyModelNode: " << ( this->BodyModelNode ? this->BodyModelNode->GetName() : "(none)" ) << "\n";
  os << indent << "NeedleTransformNode: " << ( this->NeedleTransformNode ? this->NeedleTransformNode->GetName() : "(none)" ) << "\n";
}



void vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneEndClose()
{
  this->ClearData();
}


void vtkSlicerPerkEvaluatorLogic
::SetMRMLSceneInternal( vtkMRMLScene * newScene )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}


void vtkSlicerPerkEvaluatorLogic
::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}


void vtkSlicerPerkEvaluatorLogic
::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}


void vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneNodeAdded( vtkMRMLNode* vtkNotUsed( node ) )
{
}


void vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneNodeRemoved( vtkMRMLNode* vtkNotUsed( node ) )
{
}



// -----------------------------------------------------------------------------------




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


std::vector<vtkSlicerPerkEvaluatorLogic::MetricType> vtkSlicerPerkEvaluatorLogic
::GetMetrics()
{
  std::vector<MetricType> metrics;

  // Check conditions
  if ( this->MarkBegin >= this->MarkEnd || this->MarkBegin < this->GetMinTime() || this->MarkEnd > this->GetMaxTime() )
  {
    return metrics;
  }
  
  MetricType procedureTime;
  procedureTime.first = "Total procedure time (s)";
  procedureTime.second = this->MarkEnd - this->MarkBegin;
  metrics.push_back( procedureTime );

  // Calculate the metrics individually for each tool
  for ( int i = 0; i < this->ToolTrajectories.size(); i++ )
  {
    std::vector<MetricType> toolMetrics = this->CalculateToolMetrics( this->ToolTrajectories.at(i) );

	for ( int j = 0; j < toolMetrics.size(); j++ )
	{
      metrics.push_back( toolMetrics.at(j) );
	}
  }

  /*
  // If the needle is specified, calculate needle-specific metrics
  if ( this->NeedleTransformNode != NULL )
  {
    std::vector<MetricType> needleMetrics = this->CalculateNeedleMetrics( this->NeedleTransformNode );

    for ( int j = 0; j < needleMetrics.size(); j++ )
	{
      metrics.push_back( needleMetrics.at(j) );
	}
  }
  */

  return metrics;
}


void vtkSlicerPerkEvaluatorLogic
::SetBodyModelNode( vtkMRMLModelNode* node )
{
  vtkSetMRMLNodeMacro( this->BodyModelNode, node );
  this->Modified();
}



void vtkSlicerPerkEvaluatorLogic
::SetNeedleTransformNode( vtkMRMLLinearTransformNode* node )
{
  vtkSetMRMLNodeMacro( this->NeedleTransformNode, node );
  this->Modified();
}



//Read XML file that was written by TransformRecorder module.
void vtkSlicerPerkEvaluatorLogic
::ImportFile( std::string fileName )
{
  this->ClearData();

  // Use the built in import method from the transform buffer node
  this->TransformRecorderLogic->GetModuleNode()->TransformBuffer->FromXMLElement( this->ParseXMLFile( fileName ) );
  this->ToolTrajectories = this->TransformRecorderLogic->GetModuleNode()->TransformBuffer->SplitBufferByName();

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
  
  for ( int i = 0; i < this->ToolTrajectories.size(); i++ )
  {
    if ( ToolTrajectories.at(i)->GetTransformAt(0)->GetTime() < minTime )
    {
      minTime = ToolTrajectories.at(i)->GetTransformAt(0)->GetTime();
    }
  }
  
  return minTime;
}



double vtkSlicerPerkEvaluatorLogic
::GetMaxTime() const
{
  double maxTime = - std::numeric_limits< double >::max();
  
  for ( int i = 0; i < this->ToolTrajectories.size(); i++ )
  {
    if ( ToolTrajectories.at(i)->GetCurrentTransform()->GetTime() > maxTime )
    {
      maxTime = ToolTrajectories.at(i)->GetCurrentTransform()->GetTime();
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
  if ( time < this->GetMinTime() || time > this->GetMaxTime() )
  {
    return;
  }
  
  this->PlaybackTime = time;  

  for ( int i = 0; i < this->ToolTrajectories.size(); i++ )
  {	
    std::string toolName = this->ToolTrajectories.at(i)->GetCurrentTransform()->GetDeviceName();

    vtkMRMLLinearTransformNode* node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetFirstNode( toolName.c_str(), "vtkMRMLLinearTransformNode" ) );
    if ( node == NULL )
    {
      node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLLinearTransformNode" ) );
	  this->GetMRMLScene()->AddNode( node );
	  node->SetScene( this->GetMRMLScene() );
	  node->SetName( toolName.c_str() );
    }

	node->GetMatrixTransformToParent()->DeepCopy( MatrixStrToDouble( this->ToolTrajectories.at(i)->GetTransformAtTime( time )->GetTransform() ) );
  }

}



void vtkSlicerPerkEvaluatorLogic
::CreateTransformNodes()
{

  for ( int i = 0; i < this->ToolTrajectories.size(); i++ )
  {	
    std::string toolName = this->ToolTrajectories.at(i)->GetCurrentTransform()->GetDeviceName();

    vtkMRMLLinearTransformNode* node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetFirstNode( toolName.c_str(), "vtkMRMLLinearTransformNode" ) );
    if ( node == NULL )
    {
      node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLLinearTransformNode" ) );
	  this->GetMRMLScene()->AddNode( node );
	  node->SetScene( this->GetMRMLScene() );
	  node->SetName( toolName.c_str() );
    }
  }

}



std::vector<vtkSlicerPerkEvaluatorLogic::MetricType> vtkSlicerPerkEvaluatorLogic
::CalculateToolMetrics( vtkMRMLTransformBufferNode* Trajectory )
{
  std::vector<MetricType> toolMetrics;

  double totalPath = 0.0;
  double insidePath = 0.0;
  double insideTime = 0.0;
  
  double Origin[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  double P0[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };
  double P1[ 4 ] = { 0.0, 0.0, 0.0, 1.0 };

  // Get a reference to the relevant transform node, update the transform node, compute the metric for that time step
  double originalPlaybackTime = this->GetPlaybackTime();
  this->SetPlaybackTime( this->GetMinTime() );

  // Check if the corresponding transform has any parent transforms  
  std::string toolName = Trajectory->GetCurrentTransform()->GetDeviceName();
  vtkMRMLLinearTransformNode* node = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetFirstNode( toolName.c_str(), "vtkMRMLLinearTransformNode" ) );

  if ( node == NULL )
  {
    return toolMetrics;
  }

  // Prepare inside-outside body measurements.  
  vtkPolyData* body = NULL;
  vtkSmartPointer< vtkSelectEnclosedPoints > EnclosedFilter = vtkSmartPointer< vtkSelectEnclosedPoints >::New();
  if ( this->BodyModelNode != NULL )
  {
    body = this->BodyModelNode->GetPolyData();
    EnclosedFilter->Initialize( body );
  }

  vtkSmartPointer< vtkMatrix4x4 > M0 = vtkMatrix4x4::New(); node->GetMatrixTransformToWorld( M0 );
  vtkSmartPointer< vtkMatrix4x4 > M1 = vtkMatrix4x4::New(); node->GetMatrixTransformToWorld( M1 );  
  
  for ( int i = 1; i < Trajectory->GetNumTransforms(); i++ )
  {
    
	// Set the playback time to update the node, and get data from the node
	this->SetPlaybackTime( Trajectory->GetTransformAt(i)->GetTime() );

	M0->Identity();
	M0->DeepCopy( M1 );

	M1->Identity();
    node->GetMatrixTransformToWorld( M1 );  

    if ( Trajectory->GetTransformAt(i)->GetTime() < this->MarkBegin || Trajectory->GetTransformAt(i)->GetTime() > this->MarkEnd )
    {
      continue;
    }
        
    M0->MultiplyPoint( Origin, P0 );
    M1->MultiplyPoint( Origin, P1 );

	double pathDistance = sqrt( vtkMath::Distance2BetweenPoints( P0, P1 ) );
    totalPath += pathDistance;
    
    
    // Check if current point is inside the body    
    int Inside = 0;
    if ( body != NULL )
    {
      Inside = EnclosedFilter->IsInsideSurface( P1[ 0 ], P1[ 1 ], P1[ 2 ] );
    }    
    if ( Inside )
    {
      insidePath += pathDistance;
      insideTime += Trajectory->GetTransformAt(i)->GetTime() - Trajectory->GetTransformAt(i-1)->GetTime();
    }

  }
  
  // Reset the play back time
  this->SetPlaybackTime( originalPlaybackTime );
  
  // Recording metrics
  MetricType pathLengthMetric;
  pathLengthMetric.first = toolName + " path length (mm)";
  pathLengthMetric.second = totalPath;
  toolMetrics.push_back( pathLengthMetric );
  
  MetricType insidePathMetric;
  insidePathMetric.first = toolName + " inside path (mm)";
  insidePathMetric.second = insidePath;
  toolMetrics.push_back( insidePathMetric );
  
  MetricType insideTimeMetric;
  insideTimeMetric.first = toolName + " inside time (s)";
  insideTimeMetric.second = insideTime;
  toolMetrics.push_back( insideTimeMetric );

  return toolMetrics;
}


/*
// Needle has a specific direction. Long in the positive Y direction (A in RAS coordinate system).
// Using this condition, we can calculate the potential tissue damage.
void vtkSlicerPerkEvaluatorLogic
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



double vtkSlicerPerkEvaluatorLogic
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

*/



// Private method for reading xml files ---------------------------------------------------------------------------

vtkXMLDataElement* vtkSlicerPerkEvaluatorLogic
::ParseXMLFile( std::string fileName )
{
  // Parse the file here, not in the widget
  Parser->SetFileName( fileName.c_str() );
  Parser->Parse();
  return Parser->GetRootElement();
}
