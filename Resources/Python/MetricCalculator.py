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
  
  # Exit if there are no metrics (e.g. no metrics directory was specified)
  if ( len( metrics ) == 0 ):
    return metricStringList

  # Now iterate over all of the trajectories
  for i in range( peLogic.GetNumTools() ):

    trajectory = peLogic.GetToolBuffer( trajectoryIndex )
    
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
  origin = [ 0, 0, 0, 1 ]
  point = [ 0, 0, 0, 1 ]
  # We will calculate the point here, since it is important, otherwise, the metrics are on their own
  
  # Start at the beginning (but remember where we were)
  originalPlaybackTime = peLogic.GetPlaybackTime()
  peLogic.SetPlaybackTime( peLogic.GetMinTime() )
  
  # Get the node associated with the trajectory we are interested in
  toolName = trajectory.GetCurrentTransform().GetDeviceName()
  node = slicer.mrmlScene.GetFirstNodeByName( toolName )
  
  # Initialize the matrices
  matrix = vtk.vtkMatrix4x4()
  
  # Now iterate
  for i in range( trajectory.GetNumTransforms() ):

    time = trajectory.GetTransformAt(i).GetTime()
    
    peLogic.SetPlaybackTime( time )
    
    matrix.Identity()
    node.GetMatrixTransformToWorld( matrix )
    
    if ( time < peLogic.GetMarkBegin() or time > peLogic.GetMarkEnd() ):
      continue
      
    matrix.MultiplyPoint( origin, point )
    
    for j in range( len( trajectoryMetrics ) ):
      trajectoryMetrics[j].AddTimestamp( time, matrix, point )
  
  peLogic.SetPlaybackTime( originalPlaybackTime ) 