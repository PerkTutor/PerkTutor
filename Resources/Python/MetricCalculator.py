# These are so we can use the metric scripts
import glob
import imp

class PythonMetricCalculator:

  def __init__( self ):
    self.metrics = []
    self.peLogic = slicer.modules.perkevaluator.logic()

  def AddPythonMetric( self, newMetric ):
    if ( newMetric.RequiresTissueNode() == False or self.peLogic.GetBodyModelNode() != None ):
      self.metrics.append( newMetric )
      
  def AddAllScriptedMetrics( self ):
 
    # Read the metric scripts 
    metricPath = self.peLogic.GetMetricsDirectory()
    allScripts = glob.glob( metricPath + "/*.py" )
  
    for j in range( len( allScripts ) ):
      currentMetricModule = imp.load_source( "PerkEvaluatorUserMetric" + str( j ), allScripts[j] )
      currentMetric = currentMetricModule.PerkEvaluatorMetric()
      # Drop if it requires a tissue node and none is specified
      if ( currentMetric.RequiresTissueNode() == False or self.peLogic.GetBodyModelNode() != None ):
        self.metrics.append( currentMetric )
    

  def CalculateAllToolMetrics( self ):
  
    metricStringList = []
    trajectoryIndex = 0
  
    # Exit if there are no metrics (e.g. no metrics directory was specified)
    if ( len( self.metrics ) == 0 ):
      return metricStringList

    # Now iterate over all of the trajectories
    toolTransforms = vtk.vtkCollection()
    self.peLogic.GetAnalyzeTransforms( toolTransforms )
  
    for i in range( toolTransforms.GetNumberOfItems() ):

      currentTransform = toolTransforms.GetItemAsObject( i )
    
      #Drop if it requires the needle reference 
      transformMetrics = []
      for j in range( len( self.metrics ) ):
        if ( ( self.peLogic.GetNeedleTransformNode() != None and currentTransform.GetName() == self.peLogic.GetNeedleTransformNode().GetName() ) or self.metrics[j].RequiresNeedle() == False ):
          transformMetrics.append( self.metrics[j] )
  
      for j in range( len( transformMetrics ) ):
        transformMetrics[j].Initialize( self.peLogic.GetBodyModelNode() )
      
      self.CalculateToolMetric( currentTransform, transformMetrics )
    
      for j in range( len( transformMetrics ) ):
        transformMetrics[j].Finalize()
        metricStringList.append( currentTransform.GetName() + " " + transformMetrics[j].GetMetricName() + " (" + str( transformMetrics[j].GetMetricUnit() ) + ") " )
        metricStringList.append( str( transformMetrics[j].GetMetric() ) )  
    
      trajectoryIndex = trajectoryIndex + 1
  
    return metricStringList
  
  

  def CalculateToolMetric( self, currentTransform, transformMetrics ):
  
    # Initialize the origin, previous point, current point
    origin = [ 0, 0, 0, 1 ]
    point = [ 0, 0, 0, 1 ]
    # We will calculate the point here, since it is important, otherwise, the metrics are on their own
  
    # Start at the beginning (but remember where we were)
    originalPlaybackTime = self.peLogic.GetPlaybackTime()
    self.peLogic.SetPlaybackTime( self.peLogic.GetMinTime() )
  
    # Get the node associated with the trajectory we are interested in
    transformName = currentTransform.GetName()
    node = slicer.mrmlScene.GetFirstNodeByName( transformName )
  
    # Initialize the matrices
    matrix = vtk.vtkMatrix4x4()
  
    # Now iterate
    for i in range( self.peLogic.GetTransformBuffer().GetNumTransforms() ):

      time = self.peLogic.GetTransformBuffer().GetTransformAt(i).GetTime()
    
      self.peLogic.SetPlaybackTime( time )
    
      matrix.Identity()
      currentTransform.GetMatrixTransformToWorld( matrix )
    
      if ( time < self.peLogic.GetMarkBegin() or time > self.peLogic.GetMarkEnd() ):
        continue
      
      matrix.MultiplyPoint( origin, point )
    
      for j in range( len( transformMetrics ) ):
        transformMetrics[j].AddTimestamp( time, matrix, point )
  
    self.peLogic.SetPlaybackTime( originalPlaybackTime ) 