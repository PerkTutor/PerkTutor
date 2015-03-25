import os, imp, glob, sys
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# Python Metrics Calculator
#

class PythonMetricsCalculator:
  def __init__(self, parent):
    parent.title = "Python Metrics Calculator" # TODO make this more human readable by adding spaces
    parent.categories = [ "Perk Tutor" ]
    parent.dependencies = [ "TransformRecorder", "PerkEvaluator" ]
    parent.contributors = [ "Matthew Holden (Queen's University), Tamas Ungi (Queen's University)" ] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    The Python Metric Calculator module is a hidden module for calculating metrics for transform buffers. For help on how to use this module visit: <a href='http://www.github.com/PerkTutor/PythonMetricsCalculator/wiki'>Python Metric Calculator</a>.
    """
    parent.acknowledgementText = """
    This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).
    """ # replace with organization, grant and thanks.
    parent.icon = qt.QIcon( "PythonMetricsCalculator.png" )
    parent.hidden = True # TODO: Set to "True" when deploying module
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['PythonMetricsCalculator'] = self.runTest

  def runTest(self):
    tester = PythonMetricsCalculatorTest()
    tester.runTest()

#
# qPythonMetricsCalculatorWidget
#

class PythonMetricsCalculatorWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setup(self):
    # Instantiate and connect widgets ...

    # Comment these out when not debugging
    #
    # Reload and Test area
    #
    reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    reloadCollapsibleButton.text = "Reload && Test"
    self.layout.addWidget(reloadCollapsibleButton)
    reloadFormLayout = qt.QFormLayout(reloadCollapsibleButton)

    # reload button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadButton = qt.QPushButton("Reload")
    self.reloadButton.toolTip = "Reload this module."
    self.reloadButton.name = "PythonMetricsCalculator Reload"
    reloadFormLayout.addWidget(self.reloadButton)
    self.reloadButton.connect('clicked()', self.onReload)

    # reload and test button
    # (use this during development, but remove it when delivering
    #  your module to users)
    self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    reloadFormLayout.addWidget(self.reloadAndTestButton)
    self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)


  def cleanup(self):
    pass

  def onReload(self,moduleName="PythonMetricsCalculator"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    import imp, sys, os, slicer

    widgetName = moduleName + "Widget"

    # reload the source code
    # - set source file path
    # - load the module to the global space
    filePath = eval('slicer.modules.%s.path' % moduleName.lower())
    p = os.path.dirname(filePath)
    if not sys.path.__contains__(p):
      sys.path.insert(0,p)
    fp = open(filePath, "r")
    globals()[moduleName] = imp.load_module(
        moduleName, fp, filePath, ('.py', 'r', imp.PY_SOURCE))
    fp.close()

    # rebuild the widget
    # - find and hide the existing widget
    # - create a new widget in the existing parent
    parent = slicer.util.findChildren(name='%s Reload' % moduleName)[0].parent().parent()
    for child in parent.children():
      try:
        child.hide()
      except AttributeError:
        pass
    # Remove spacer items
    item = parent.layout().itemAt(0)
    while item:
      parent.layout().removeItem(item)
      item = parent.layout().itemAt(0)

    # delete the old widget instance
    if hasattr(globals()['slicer'].modules, widgetName):
      getattr(globals()['slicer'].modules, widgetName).cleanup()

    # create new widget inside existing parent
    globals()[widgetName.lower()] = eval(
        'globals()["%s"].%s(parent)' % (moduleName, widgetName))
    globals()[widgetName.lower()].setup()
    setattr(globals()['slicer'].modules, widgetName, globals()[widgetName.lower()])

  def onReloadAndTest(self,moduleName="PythonMetricsCalculator"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
      tester.runTest()
    except Exception, e:
      import traceback
      traceback.print_exc()
      qt.QMessageBox.warning(slicer.util.mainWindow(), 
          "Reload and Test", 'Exception!\n\n' + str(e) + "\n\nSee Python Console for Stack Trace")


#
# PythonMetricsCalculatorLogic
#

class PythonMetricsCalculatorLogic:
  """This class should implement all the actual 
  computation done by your module.  The interface 
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__( self ):
    self.metrics = []
    # By default, grab the instantiated module's logic (though other logics are possible)
    self.SetPerkEvaluatorLogic( slicer.modules.perkevaluator.logic() )
    
  
  # We need this in order to determine the tissue model node, etc.  
  def SetPerkEvaluatorLogic( self, newPELogic ):
    self.peLogic = newPELogic  
    self.mrmlScene = self.peLogic.GetMRMLScene()
    
    
  def SetPerkEvaluatorNodeID( self, newPENodeID ):
    self.peNode = self.mrmlScene.GetNodeByID( newPENodeID )
    
    
  def SetMetricsNodeID( self, newMetricsNodeID ):
    self.metricsNode = self.mrmlScene.GetNodeByID( newMetricsNodeID )
    
    
  def InitializeMetricsTable( self ):
    self.metricsNode.GetTable().Initialize()
    
    transformNameColumn = vtk.vtkStringArray()
    transformNameColumn.SetName( "TransformName" )
    metricNameColumn = vtk.vtkStringArray()
    metricNameColumn.SetName( "MetricName" )
    metricUnitColumn = vtk.vtkStringArray()
    metricUnitColumn.SetName( "MetricUnit" )
    metricValueColumn = vtk.vtkStringArray()
    metricValueColumn.SetName( "MetricValue" )
    
    self.metricsNode.GetTable().AddColumn( transformNameColumn )
    self.metricsNode.GetTable().AddColumn( metricNameColumn )
    self.metricsNode.GetTable().AddColumn( metricUnitColumn )
    self.metricsNode.GetTable().AddColumn( metricValueColumn )
      
      
  def AddAllUserMetrics( self ): 
    # Read the metric scripts 
    metricPath = self.peNode.GetMetricsDirectory()    
    if ( metricPath == "" ):
      return
    
    allScripts = glob.glob( metricPath + "/*.py" )
  
    for j in range( len( allScripts ) ):
    
      metricModuleString = "PerkEvaluatorUserMetric_" + os.path.splitext( os.path.basename( allScripts[j] ) )[0] # this puts the file name at the end
      
      try:
        # If it can't load properly, then just ignore
        currentMetricModule = imp.load_source( metricModuleString, allScripts[j] )
        metricInstance = currentMetricModule.PerkEvaluatorMetric() # Test that we can create an instance of the metric
        self.metrics.append( metricInstance )
      except:
        print "Could not load metric: ", metricModuleString, "."
      
      
  def FilterMetricsByAnatomyRole( self, inMetrics, specifiedAnatomyRoles ):
    # Only output metrics for which all of the required anatomy roles are fulfilled
    outMetrics = []
    
    for i in range( len( inMetrics ) ):
      currentMetricRoles = inMetrics[i].GetRequiredAnatomyRoles()
      
      anatomyRolesSatisfied = True
      for j in range( len( currentMetricRoles ) ):
        if ( currentMetricRoles[j] not in specifiedAnatomyRoles ):
          anatomyRolesSatisfied = False
          
      if ( anatomyRolesSatisfied == True ):
        outMetrics.append( inMetrics[i] )      
        
    return outMetrics
        
        
  def FilterMetricsByTransformRole( self, inMetrics, currentTransformRole ):     
    # Only output metrics which accept the current transform's role
    outMetrics = []
    
    # Discard if the transform has no role
    if ( currentTransformRole == "" or currentTransformRole == "None" ):
      return outMetrics
    
    for i in range( len( inMetrics ) ):
      currentMetricRoles = inMetrics[i].GetAcceptedTransformRoles()
      
      for j in range( len( currentMetricRoles ) ):
        if ( currentMetricRoles[j] == currentTransformRole or currentMetricRoles[j] == "Any" ):
          outMetrics.append( inMetrics[i] )
        
    return outMetrics
    
    
  def ReloadAllMetrics( self ):
    import PythonMetrics
    self.metrics = PythonMetrics.PerkTutorCoreMetrics[:]
    self.AddAllUserMetrics()
    
    
  def GetAllAnatomyRoles( self ):
    # Re-initialize all the metrics (in case new ones were added)
    self.ReloadAllMetrics()
  
    allAnatomyRoles = []
    for i in range( len( self.metrics ) ):
      currentRequiredRoles = self.metrics[i].GetRequiredAnatomyRoles()
      # Each metric may require multiple roles, so we must check all of them
      for j in range( len( currentRequiredRoles ) ):
        if ( currentRequiredRoles[j] not in allAnatomyRoles ):
          allAnatomyRoles.append( currentRequiredRoles[j] )
          
    return allAnatomyRoles
    
    
  def GetAllTransformRoles( self ):
    # Re-initialize all the metrics (in case new ones were added)
    self.ReloadAllMetrics()
  
    allTransformRoles = []
    for i in range( len( self.metrics ) ):
      currentAcceptedRoles = self.metrics[i].GetAcceptedTransformRoles()
      # Each metric may accept multiple roles, so we must check all of them
      for j in range( len( currentAcceptedRoles ) ):
        if ( currentAcceptedRoles[j] not in allTransformRoles ):
          allTransformRoles.append( currentAcceptedRoles[j] )
          
    return allTransformRoles


  def CalculateAllMetrics( self ):  
    # Initialize all the metrics    
    self.ReloadAllMetrics()
    
    # Initialize the metrics output
    self.InitializeMetricsTable()
      
    # Exit if there are no metrics (e.g. no metrics directory was specified)
    if ( len( self.metrics ) == 0 ):
      return metricStringList
      
    # Get all of the specified anatomy roles
    specifiedAnatomyRoles = []
    for role in ( self.GetAllAnatomyRoles() ):
      if ( self.peNode.GetAnatomyNodeName( role ) != "" ):
        specifiedAnatomyRoles.append( role )
        
    # Filter out all metrics for which the anatomies are not available
    anatomyMetrics = self.FilterMetricsByAnatomyRole( self.metrics, specifiedAnatomyRoles )

    # Now iterate over all of the trajectories
    toolTransforms = vtk.vtkCollection()
    self.peLogic.GetSceneVisibleTransformNodes( toolTransforms )
  
    for i in range( toolTransforms.GetNumberOfItems() ):

      currentTransform = toolTransforms.GetItemAsObject( i )
      currentTransformRole = self.peNode.GetTransformRole( currentTransform.GetName() )
    
      #Drop if based on tissue and needle as appropriate
      transformMetrics = self.FilterMetricsByTransformRole( anatomyMetrics, currentTransformRole )
  
      # Initialization
      for j in range( len( transformMetrics ) ):
        transformMetrics[j].Initialize()
        
      # Grab anatomy nodes, removing any metrics for which the required nodes are not available
      transformMetrics = self.AddMetricAnatomyNodes( transformMetrics )            
      
      # Calculation
      self.CalculateTransformMetrics( currentTransform, transformMetrics )
      
      # Finalize all the metric computations
      for j in range( len( transformMetrics ) ):
        transformMetrics[j].Finalize()
    
      # Get the metrics
      for j in range( len( transformMetrics ) ):
        currentMetricRow = vtk.vtkVariantArray()
        currentMetricRow.InsertNextValue( currentTransform.GetName() )
        currentMetricRow.InsertNextValue( transformMetrics[j].GetMetricName() )
        currentMetricRow.InsertNextValue( transformMetrics[j].GetMetricUnit() )
        currentMetricRow.InsertNextValue( transformMetrics[j].GetMetric() )
        self.metricsNode.GetTable().InsertNextRow( currentMetricRow )        

        
    
  def AddMetricAnatomyNodes( self, transformMetrics ):
  
    # Keep track of which metrics all anatomies are sucessfully delivered to
    anatomiesFulfilled = [ True ] * len( transformMetrics )
  
    for i in range( len( transformMetrics ) ):
      currentMetricAnatomyRoles = transformMetrics[i].GetRequiredAnatomyRoles()
      
      for j in range( len( currentMetricAnatomyRoles ) ):
        currentAnatomyNodeName = self.peNode.GetAnatomyNodeName( currentMetricAnatomyRoles[j] )
        currentAnatomyNode = self.mrmlScene.GetFirstNodeByName( currentAnatomyNodeName )
        added = transformMetrics[i].AddAnatomyRole( currentMetricAnatomyRoles[j], currentAnatomyNode )
        
        if ( added == False ):
          anatomiesFulfilled[i] = False
          
    newTransformMetrics = []
    for i in range( len( anatomiesFulfilled ) ):
      if ( anatomiesFulfilled[i] == True ):
        newTransformMetrics.append( transformMetrics[i] )
        
    return newTransformMetrics
  
  

  def CalculateTransformMetrics( self, currentTransform, transformMetrics ):
    # Initialize the origin, previous point, current point
    origin = [ 0, 0, 0, 1 ]
    point = [ 0, 0, 0, 1 ]
    # We will calculate the point here, since it is important, otherwise, the metrics are on their own
    
    # Start at the beginning (but remember where we were)
    originalPlaybackTime = self.peLogic.GetPlaybackTime()
    self.peLogic.SetPlaybackTime( self.peLogic.GetMinTime() )
    
    # Get the node associated with the trajectory we are interested in
    transformName = currentTransform.GetName()
    node = self.mrmlScene.GetFirstNodeByName( transformName )
    
    # Get the self and parent transform buffer
    selfAndParentBuffer = self.peLogic.GetSelfAndParentTransformBuffer( node )
    
    # Initialize the matrices
    matrix = vtk.vtkMatrix4x4()
    
    # Now iterate
    for i in range( selfAndParentBuffer.GetNumTransforms() ):
      
      absTime = selfAndParentBuffer.GetTransformAt(i).GetTime()
      self.peLogic.SetPlaybackTime( absTime )
      relTime = absTime - self.peLogic.GetMinTime()
      
      matrix.Identity()
      currentTransform.GetMatrixTransformToWorld( matrix )
      
      if ( relTime < self.peNode.GetMarkBegin() or relTime > self.peNode.GetMarkEnd() ):
        continue
      
      matrix.MultiplyPoint( origin, point )
      
      for j in range( len( transformMetrics ) ):
        transformMetrics[j].AddTimestamp( absTime, matrix, point )
    
    self.peLogic.SetPlaybackTime( originalPlaybackTime )    




    


class PythonMetricsCalculatorTest(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear( 0 )

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    
    try:
      self.test_PythonMetricsCalculatorLumbar()
    
    except Exception, e:
      self.delayDisplay( "Test caused exception!\n" + str(e) )

  def test_PythonMetricsCalculatorLumbar(self):
    
    # These are the IDs of the relevant nodes
    transformBufferID = "vtkMRMLTransformBufferNode1"
    tissueModelID = "vtkMRMLModelNode4"
    needleTransformID = "vtkMRMLLinearTransformNode4"
    
    # TODO: Does this work for all OS?
    sceneFile = os.path.dirname( os.path.abspath( __file__ ) ) + "/Data/Scenes/Lumbar/TransformBuffer_Lumbar_Scene.mrml"
    resultsFile = os.path.dirname( os.path.abspath( __file__ ) ) + "/Data/Results/Lumbar.xml"
    
    # Load the scene
    activeScene = slicer.mrmlScene
    activeScene.Clear( 0 )
    activeScene.SetURL( sceneFile )
    if ( activeScene.Import() != 1 ):
      raise Exception( "Scene import failed. Scene file:" + sceneFile )   
    
    
    transformBufferNode = activeScene.GetNodeByID( transformBufferID )
    if ( transformBufferNode == None ):
      raise Exception( "Bad transform buffer." )
      
    tissueModelNode = activeScene.GetNodeByID( tissueModelID )
    if ( tissueModelNode == None ):
      raise Exception( "Bad tissue model." )
      
    needleTransformNode = activeScene.GetNodeByID( needleTransformID )
    if ( needleTransformNode == None ):
      raise Exception( "Bad needle transform." )
    
    # Parse the results xml file
    resultsParser = vtk.vtkXMLDataParser()
    resultsParser.SetFileName( resultsFile )
    resultsParser.Parse()
    rootElement = resultsParser.GetRootElement()
    if ( rootElement == None or rootElement.GetName() != "PythonMetricsResults" ):
      raise Exception( "Reading results failed. Results file:" + resultsFile )   
    
    # Create a dictionary to store results
    metricsDict = dict()
    
    for i in range( rootElement.GetNumberOfNestedElements() ):
      element = rootElement.GetNestedElement( i )
      if ( element == None or element.GetName() != "Metric" ):
        continue
      metricsDict[ element.GetAttribute( "Name" ) ] = float( element.GetAttribute( "Value" ) )
    
    # Setup the analysis
    peLogic = slicer.modules.perkevaluator.logic()

    peLogic.UpdateToolTrajectories( transformBufferNode )
    peLogic.SetPlaybackTime( peLogic.GetMinTime() )
    
    # Setup the parameters
    peNode = activeScene.CreateNodeByClass( "vtkMRMLPerkEvaluatorNode" )
    
    peNode.SetAnatomyNodeName( "Tissue", tissueModelNode.GetName() )
    peNode.SetTransformRole( needleTransformNode.GetName(), "Needle" )
    
    peNode.SetMarkBegin( 0 )
    peNode.SetMarkEnd( peLogic.GetTotalTime() )
    
    activeScene.AddNode( peNode )
    
    # Calculate the metrics
    pmcLogic = PythonMetricsCalculatorLogic()
    pmcLogic.SetPerkEvaluatorNodeID( peNode.GetID() )
    
    metricStringList = pmcLogic.CalculateAllMetrics()
    if ( len( metricStringList ) == 0 ):
      raise Exception( "No metrics were calculated." )
    if ( len( metricStringList ) % 2 != 0 ):
      raise Exception( "Metric calculation produced an unexpected result." ) 
    
    # Compare the metrics to the expected results
    metricIndex = 0
    metricsFail = False
    precision = 2
    
    while ( metricIndex < len( metricStringList ) ):
      metricName = metricStringList[ metricIndex ]
      metricValue = float( metricStringList[ metricIndex + 1 ] )
      
      if ( metricName not in metricsDict ):
        print "Could not find expected result for metric:", metricName, ". Value:", metricValue, "."
      else:
        if ( round( metricValue, precision ) != round( metricsDict[ metricName ], precision ) ):
          print "Incorrect metric:", metricName, ". Expected:", metricsDict[ metricName ], "but got", metricValue, "!"
          metricsFail = True
        else:
          print "Correct! Metric:", metricName, ". Expected:", metricsDict[ metricName ], "and got", metricValue, "!"
        
      metricIndex = metricIndex + 2
        
    if ( metricsFail == True ):
      self.delayDisplay( "Test failed! Calculated metrics were not consistent with results." )
    else:
      self.delayDisplay( "Test passed! Calculated metrics match results!" )
      
    self.assertFalse( metricsFail )
