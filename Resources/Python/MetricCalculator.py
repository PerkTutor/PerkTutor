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
  toolTransforms = vtk.vtkCollection()
  peLogic.GetAnalyzeTransforms( toolTransforms )
  
  for i in range( toolTransforms.GetNumberOfItems() ):

    currentTransform = toolTransforms.GetItemAsObject( i )
    
    #Drop if it requires the needle reference 
    transformMetrics = []
    for j in range( len( metrics ) ):
      if ( ( peLogic.GetNeedleTransformNode() != None and currentTransform.GetName() == peLogic.GetNeedleTransformNode().GetName() ) or metrics[j].RequiresNeedle() == False ):
        transformMetrics.append( metrics[j] )
  
    for j in range( len( transformMetrics ) ):
      transformMetrics[j].Initialize( peLogic.GetBodyModelNode() )
      
    CalculateToolMetric( peLogic, currentTransform, transformMetrics )
    
    for j in range( len( transformMetrics ) ):
      transformMetrics[j].Finalize()
      metricStringList.append( currentTransform.GetName() + " " + transformMetrics[j].GetMetricName() + " (" + str( transformMetrics[j].GetMetricUnit() ) + ") " )
      metricStringList.append( str( transformMetrics[j].GetMetric() ) )  
    
    trajectoryIndex = trajectoryIndex + 1
  
  return metricStringList
  
  

def CalculateToolMetric( peLogic, currentTransform, transformMetrics ):
  
  # Initialize the origin, previous point, current point
  origin = [ 0, 0, 0, 1 ]
  point = [ 0, 0, 0, 1 ]
  # We will calculate the point here, since it is important, otherwise, the metrics are on their own
  
  # Start at the beginning (but remember where we were)
  originalPlaybackTime = peLogic.GetPlaybackTime()
  peLogic.SetPlaybackTime( peLogic.GetMinTime() )
  
  # Get the node associated with the trajectory we are interested in
  transformName = currentTransform.GetName()
  node = slicer.mrmlScene.GetFirstNodeByName( transformName )
  
  # Initialize the matrices
  matrix = vtk.vtkMatrix4x4()
  
  # Now iterate
  for i in range( peLogic.GetTransformBuffer().GetNumTransforms() ):

    time = peLogic.GetTransformBuffer().GetTransformAt(i).GetTime()
    
    peLogic.SetPlaybackTime( time )
    
    matrix.Identity()
    currentTransform.GetMatrixTransformToWorld( matrix )
    
    if ( time < peLogic.GetMarkBegin() or time > peLogic.GetMarkEnd() ):
      continue
      
    matrix.MultiplyPoint( origin, point )
    
    for j in range( len( transformMetrics ) ):
      transformMetrics[j].AddTimestamp( time, matrix, point )
  
  peLogic.SetPlaybackTime( originalPlaybackTime ) 