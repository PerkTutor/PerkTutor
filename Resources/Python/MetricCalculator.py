# These are so we can use the metric scripts
import glob
import imp


def CalculateAllToolMetrics():

  # Get the logic for the Perk Evaluator
  peLogic = slicer.modules.perkevaluator.logic()
  
  # For testing purposes
  metricPath = peLogic.GetMetricsDirectory()
  # Read the metric scripts
  allScripts = glob.glob( metricPath + "/*.py" )
    
  # Compile a list of metric objects
  metrics = []
  for j in range( len( allScripts ) ):
    currentMetricModule = imp.load_source( "PerkEvaluatorMetric" + str( j ), allScripts[j] )
    currentMetric = currentMetricModule.PerkEvaluatorMetric()
    # Drop if it requires a tissue node and none is specified
    if ( currentMetric.RequiresTissueNode() == False or peLogic.GetBodyModelNode() != None ):
      metrics.append( currentMetric )
  
  metricStringList = []
  trajectoryIndex = 0
  # Now iterate over all of the trajectories
  for i in range( peLogic.GetNumToolTrajectories() ):

    trajectory = peLogic.GetToolTrajectory( trajectoryIndex )
    
    #Drop if it requires the needle reference 
    trajectoryMetrics = []
    for j in range( len( metrics ) ):
      if ( ( peLogic.GetNeedleTransformNode() != None and trajectory.GetCurrentTransform().GetDeviceName() == peLogic.GetNeedleTransformNode().GetName() ) or metrics[j].RequiresNeedle() == False ):
        trajectoryMetrics.append( metrics[j] )
  
    for j in range( len( trajectoryMetrics ) ):
      trajectoryMetrics[j].Initialize( peLogic.GetBodyModelNode() )
      
    CalculateToolMetric( peLogic, trajectory, trajectoryMetrics )
    
    for j in range( len( trajectoryMetrics ) ):
      trajectoryMetrics[j].Finalize()
      metricStringList.append( trajectory.GetCurrentTransform().GetDeviceName() + " " + trajectoryMetrics[j].GetMetricName() + " (" + str( trajectoryMetrics[j].GetMetricUnit() ) + ") " )
      metricStringList.append( str( trajectoryMetrics[j].GetMetric() ) )  
    
    trajectoryIndex = trajectoryIndex + 1
  
  return metricStringList
  
  

def CalculateToolMetric( peLogic, trajectory, trajectoryMetrics ):
  
  # Initialize the origin, previous point, current point
  Origin = [ 0, 0, 0, 1 ]
  P_Curr = [ 0, 0, 0, 1 ]
  P_Prev = [ 0, 0, 0, 1 ]
  # We will calculate the point here, since it is important, otherwise, the metrics are on their own
  
  # Start at the beginning (but remember where we were)
  originalPlaybackTime = peLogic.GetPlaybackTime()
  peLogic.SetPlaybackTime( peLogic.GetMinTime() )
  
  # Get the node associated with the trajectory we are interested in
  toolName = trajectory.GetCurrentTransform().GetDeviceName()
  node = slicer.mrmlScene.GetFirstNodeByName( toolName )
  
  # Initialize the matrices
  M_Prev = vtk.vtkMatrix4x4()
  node.GetMatrixTransformToWorld( M_Prev )
  M_Curr = vtk.vtkMatrix4x4()
  node.GetMatrixTransformToWorld( M_Curr )
  
  # Now iterate
  for i in range( 1, trajectory.GetNumTransforms() ):
    
    T_Prev = trajectory.GetTransformAt(i-1).GetTime()
    T_Curr = trajectory.GetTransformAt(i).GetTime()
    
    peLogic.SetPlaybackTime( T_Curr )
    
    M_Prev.Identity()
    M_Prev.DeepCopy( M_Curr )
    
    M_Curr.Identity()
    node.GetMatrixTransformToWorld( M_Curr )
    
    if ( trajectory.GetTransformAt(i).GetTime() < peLogic.GetMarkBegin() or trajectory.GetTransformAt(i).GetTime() > peLogic.GetMarkEnd() ):
      continue
      
    M_Prev.MultiplyPoint( Origin, P_Prev )
    M_Curr.MultiplyPoint( Origin, P_Curr )
    
    for j in range( len( trajectoryMetrics ) ):
      trajectoryMetrics[j].AddTimestamp( T_Prev, T_Curr, M_Prev, M_Curr, P_Prev, P_Curr )
  
  peLogic.SetPlaybackTime( originalPlaybackTime ) 