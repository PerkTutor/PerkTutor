# These are so we can use the metric scripts
import glob
import imp
import vtk

class PythonMetricCalculator:

  def __init__( self ):
    self.metrics = []
    self.fileStream = open( "C:/Devel/PerkTutor/PerkTutorMetricCalculatorLog.txt", "w" )
    # Use the module's active logic by default
    #self.peLogic = slicer.modules.perkevaluator.logic()
    self.fileStream.write( "Python metric calculator called! \n" )
    self.fileStream.flush()
    #print "Python metric calculator called!"
    
  def SetPerkEvaluatorLogic( self, newPELogicDecorator ):
    self.peLogic = newPELogicDecorator#.GetObject()
    self.fileStream.write( str( self.peLogic ) )
    self.fileStream.flush()
    #print self.peLogic

  def AddPythonMetric( self, newMetric ):
    self.fileStream.write( str( newMetric ) )
    self.fileStream.flush()
    self.fileStream.write( str( self.peLogic ) )
    self.fileStream.write( str( "BODY MODEL NODE!!!\n" ) )
    self.fileStream.write( str( self.peLogic.GetBodyModelNode() ) )
    self.fileStream.flush()
    if ( newMetric.RequiresTissueNode() == False or self.peLogic.GetBodyModelNode() != None ):
      self.metrics.append( newMetric )
      self.fileStream.write( newMetric.GetMetricName() )
      self.fileStream.write( str( self.metrics ) )
      self.fileStream.flush()
      
      
  def AddAllScriptedMetrics( self ):
 
    # Read the metric scripts 
    metricPath = self.peLogic.GetMetricsDirectory()    
    if ( metricPath == "" ):
      return
    
    allScripts = glob.glob( metricPath + "/*.py" )
  
    for j in range( len( allScripts ) ):
      currentMetricModule = imp.load_source( "PerkEvaluatorUserMetric" + str( j ), allScripts[j] )
      currentMetric = currentMetricModule.PerkEvaluatorMetric()
      # Drop if it requires a tissue node and none is specified
      if ( currentMetric.RequiresTissueNode() == False or self.peLogic.GetBodyModelNode() != None ):
        self.metrics.append( currentMetric )
  
  def GetNumMetrics( self ):
    self.fileStream.write( str( len( self.metrics ) ) )
    self.fileStream.flush()
    return len( self.metrics )

  def CalculateAllToolMetrics( self ):
  
    metricStringList = []
    trajectoryIndex = 0
      
    # Exit if there are no metrics (e.g. no metrics directory was specified)
    if ( len( self.metrics ) == 0 ):
      return metricStringList
      
    self.fileStream.write( "Before call to PE logic\n" )
    self.fileStream.flush()

    # Now iterate over all of the trajectories
    toolTransforms = vtk.vtkCollection()
    self.peLogic.GetAnalyzeTransforms( toolTransforms )
    
    self.fileStream.write( "After call to PE logic\n" )
    self.fileStream.flush()
  
    for i in range( toolTransforms.GetNumberOfItems() ):

      currentTransform = toolTransforms.GetItemAsObject( i )
    
      #Drop if it requires the needle reference 
      transformMetrics = []
      for j in range( len( self.metrics ) ):
        if ( ( self.peLogic.GetNeedleTransformNode() != None and currentTransform.GetName() == self.peLogic.GetNeedleTransformNode().GetName() ) or self.metrics[j].RequiresNeedle() == False ):
          transformMetrics.append( self.metrics[j] )
  
      for j in range( len( transformMetrics ) ):
        transformMetrics[j].Initialize( self.peLogic.GetBodyModelNode() )
        
      self.fileStream.write( "Start tool metric calculation for " )
      self.fileStream.write( str( self.metrics[j].GetMetricName() ) )
      self.fileStream.flush()
      
      self.CalculateToolMetric( currentTransform, transformMetrics )
      
      self.fileStream.write( "Stop tool metric calculation for " )
      self.fileStream.write( str( self.metrics[j].GetMetricName() ) )
      self.fileStream.write( "\n" )
      self.fileStream.flush()
    
      for j in range( len( transformMetrics ) ):
        transformMetrics[j].Finalize()
        metricStringList.append( currentTransform.GetName() + " " + transformMetrics[j].GetMetricName() + " (" + str( transformMetrics[j].GetMetricUnit() ) + ")" )
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
  
    # Initialize the matrices
    matrix = vtk.vtkMatrix4x4()
    
    self.fileStream.write( "Start iteration!" )
    self.fileStream.write( "\n" )
    self.fileStream.flush()
  
    # Now iterate
    for i in range( self.peLogic.GetTransformBuffer().GetNumTransforms() ):
    
      self.fileStream.write( "Mid iteration 1!" )
      self.fileStream.write( "\n" )
      self.fileStream.flush()

      time = self.peLogic.GetTransformBuffer().GetTransformAt(i).GetTime()
      
      self.fileStream.write( "Mid iteration 2!" )
      self.fileStream.write( "\n" )
      self.fileStream.flush()
    
      self.peLogic.SetPlaybackTime( time )
      
      self.fileStream.write( str( self.peLogic.GetPlaybackTime() ) )
      self.fileStream.write( "\n" )
      self.fileStream.flush()
    
      matrix.Identity()
      currentTransform.GetMatrixTransformToWorld( matrix )
      
      self.fileStream.write( str( matrix ) )
      self.fileStream.write( "\n" )
      self.fileStream.flush()
      
      self.fileStream.write( str( currentTransform.GetParentTransformNode() ) )
      self.fileStream.write( "\n" )
      self.fileStream.flush()
    
      if ( time < self.peLogic.GetMarkBegin() or time > self.peLogic.GetMarkEnd() ):
        continue
      
      matrix.MultiplyPoint( origin, point )
      
      self.fileStream.write( "Mid iteration 3!" )
      self.fileStream.write( "\n" )
      self.fileStream.flush()
    
      for j in range( len( transformMetrics ) ):
        transformMetrics[j].AddTimestamp( time, matrix, point )
  
    self.peLogic.SetPlaybackTime( originalPlaybackTime )
    
    self.fileStream.write( "Stop iteration!" )
    self.fileStream.write( "\n" )
    self.fileStream.flush()