import os, imp, glob, sys
import unittest
from __main__ import vtk, qt, ctk, slicer
import PythonMetrics

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
  
    self.allMetricModules = dict()
    self.allMetrics = dict()
    
    # By default, grab the instantiated module's logic (though other logics are possible)
    self.SetPerkEvaluatorLogic( slicer.modules.perkevaluator.logic() )
    self.SetMRMLScene( self.peLogic.GetMRMLScene() )
    
    
  @staticmethod
  def AddCoreMetricsToScene():
    # Add the "Core" metrics by default
    coreMetricScriptFiles = glob.glob( os.path.dirname( __file__ ) + "/PythonMetrics/[a-z]*.py" ) # This will ignore any file that doesn't start with a letter # TODO: Is this a good way to ignore __init__.py?
    for script in coreMetricScriptFiles:
      slicer.util.loadNodeFromFile( script, "Python Metric Script" )
    
  
  # We need this in order to determine the tissue model node, etc.  
  def SetMRMLScene( self, newScene ):
    self.mrmlScene = newScene
  

  def SetPerkEvaluatorLogic( self, newPELogic ):
    self.peLogic = newPELogic    
    

  def SetPerkEvaluatorNodeID( self, newPENodeID ):
    self.peNode = self.mrmlScene.GetNodeByID( newPENodeID )
    # Now we can find all of the metrics
    self.allMetricModules = self.GetFreshMetricModules()
 
    
  def SetMetricsTableID( self, newMetricsTableID ):
    self.metricsTable = self.mrmlScene.GetNodeByID( newMetricsTableID )
    
    
  def InitializeMetricsTable( self ):
    self.metricsTable.GetTable().Initialize()
    
    # TODO: Make the more robust (e.g. qSlicerMetricsTableWidget::METRIC_TABLE_COLUMNS) 
    metricsTableColumnNames = [ "MetricName", "MetricRoles", "MetricUnit", "MetricValue" ]
    for columnName in metricsTableColumnNames:
      column = vtk.vtkStringArray()
      column.SetName( columnName )
      self.metricsTable.GetTable().AddColumn( column )
      
      
  def OutputAllMetricsToMetricsTable( self ):
    self.InitializeMetricsTable()

    self.metricsTable.GetTable().SetNumberOfRows( len( self.allMetrics ) )
    insertRow = 0
    for metric in self.allMetrics.values():
      self.metricsTable.GetTable().SetValueByName( insertRow, "MetricName", metric.GetMetricName() )
      self.metricsTable.GetTable().SetValueByName( insertRow, "MetricRoles", metric.CombinedRoleString )
      self.metricsTable.GetTable().SetValueByName( insertRow, "MetricUnit", metric.GetMetricUnit() )
      self.metricsTable.GetTable().SetValueByName( insertRow, "MetricValue", metric.GetMetric() )
      insertRow += 1
    
    
  def GetFreshMetricModules( self ):
    
    # Setup the metrics currently associated with the selected PerkEvaluator node
    execDict = dict()
    metricModuleDict = dict()
    
    # Grab all of the metric script nodes in the scene
    metricScriptNodes = self.mrmlScene.GetNodesByClass( "vtkMRMLMetricScriptNode" )
    
    for i in range( metricScriptNodes.GetNumberOfItems() ):
      currentMetricScriptNode = metricScriptNodes.GetItemAsObject( i )
      exec currentMetricScriptNode.GetPythonSourceCode() in execDict
      metricModuleDict[ currentMetricScriptNode.GetID() ] = execDict[ "PerkEvaluatorMetric" ]
    
    return metricModuleDict
    
    
  def GetFreshMetrics( self ):
    # Get a fresh set of metrics
    newTransformMetricModules = self.GetFreshMetricModules()
    
    # Setup the metrics currently associated with the selected PerkEvaluator node
    execDict = dict()
    metricDict = dict()
    
    # TODO: Make the reference role calling more robust (i.e. vtkMRMLPerkEvaluatorNode::METRIC_INSTANCE_REFERENCE_ROLE)
    for i in range( self.peNode.GetNumberOfNodeReferences( "MetricInstance" ) ):
      metricInstanceNode = self.peNode.GetNthNodeReference( "MetricInstance", i )
      associatedMetricModule = newTransformMetricModules[ metricInstanceNode.GetAssociatedMetricScriptID() ]
      if ( self.AreMetricModuleRolesSatisfied( associatedMetricModule, metricInstanceNode ) ):
        metricDict[ metricInstanceNode.GetID() ] = associatedMetricModule()
        # Add the roles description (to make it easier to distinguish the same metric under different roles)
        metricDict[ metricInstanceNode.GetID() ].CombinedRoleString = metricInstanceNode.GetCombinedRoleString()
        
    # Add the anatomy to the fresh metrics
    self.AddAnatomyNodesToMetrics( metricDict )
   
    return metricDict
    
    
  def AreMetricModuleRolesSatisfied( self, metricModule, metricInstanceNode ):
    # Output whether or not the metric module has its roles completely satisfied by the metricInstance node
     
    rolesSatisfied = True
      
    for role in metricModule.GetRequiredAnatomyRoles():
      if ( metricInstanceNode.GetRoleID( role, metricInstanceNode.AnatomyRole ) == "" ):
        rolesSatisfied = False        
          
    for role in metricModule.GetAcceptedTransformRoles():
      if ( metricInstanceNode.GetRoleID( role, metricInstanceNode.TransformRole ) == "" ):
        rolesSatisfied = False
          
    return rolesSatisfied

    
  # Note: This modifies the inputted dictionary of metrics
  def AddAnatomyNodesToMetrics( self, metrics ):  
    # Keep track of which metrics all anatomies are sucessfully delivered to    
    unfulfilledAnatomies = []    
  
    for metricInstanceID in metrics:
      metricAnatomyRoles = metrics[ metricInstanceID ].GetRequiredAnatomyRoles()
      metricInstanceNode = self.mrmlScene.GetNodeByID( metricInstanceID )
      
      for role in metricAnatomyRoles:
        anatomyNode = metricInstanceNode.GetRoleNode( role, metricInstanceNode.AnatomyRole )
        added = metrics[ metricInstanceID ].AddAnatomyRole( role, anatomyNode )
        
        if ( not added ):
          unfulfilledAnatomies.append( metricInstanceID )
          
    # In practice, the anatomies should always be fulfilled because we already filtered out those that could not be fulfilled
    # However, if the wrong type of node is selected, then this may return false
    for metricInstanceID in unfulfilledAnatomies:
      metrics.pop( metricInstanceID )

        
  # Note: We are returning a dictionary here, not a list
  def GetAllAnatomyRoles( self, metricScriptID ):  
    return self.allMetricModules[ metricScriptID ].GetRequiredAnatomyRoles().keys()

    
  # Note: We are returning a list here
  def GetAllTransformRoles( self, metricScriptID ):
    return self.allMetricModules[ metricScriptID ].GetAcceptedTransformRoles()
    
  # Note: We are returning a string here
  def GetAnatomyRoleClassName( self, metricScriptID, role ):
    return self.allMetricModules[ metricScriptID ].GetRequiredAnatomyRoles()[ role ]

    

  def CalculateAllMetrics( self ):
  
    self.allMetrics = self.GetFreshMetrics()
  
    # Start at the beginning (but remember where we were)
    originalPlaybackTime = self.peNode.GetPlaybackTime()
    
    # Now iterate over all of the trajectories
    combinedTransformBuffer = slicer.modulemrml.vtkLogRecordBuffer()
    self.peNode.GetTransformBufferNode().GetCombinedTransformRecordBuffer( combinedTransformBuffer )
    
    if ( combinedTransformBuffer.GetNumRecords() == 0 ):
      return
      
    self.peNode.SetPlaybackTime( combinedTransformBuffer.GetRecord( 0 ).GetTime(), True )
    minTime = self.peNode.GetTransformBufferNode().GetMinimumTime()
  
    for i in range( combinedTransformBuffer.GetNumRecords() ):
      absTime = combinedTransformBuffer.GetRecord( i ).GetTime()
      self.peNode.SetPlaybackTime( absTime, True )
      self.peLogic.UpdateSceneToPlaybackTime( self.peNode )
      relTime = absTime - minTime # Can't just take the 0th record of the combined buffer, because this doesn't account for the messages
      
      if ( relTime < self.peNode.GetMarkBegin() or relTime > self.peNode.GetMarkEnd() ):
        continue
        
      self.UpdateSelfAndChildMetrics( combinedTransformBuffer.GetRecord( i ).GetDeviceName(), absTime, False )

    self.peNode.SetPlaybackTime( originalPlaybackTime, False ) # Scene automatically updated
    self.OutputAllMetricsToMetricsTable()

    
  def UpdateSelfAndChildMetrics( self, transformName, absTime, updateTable ):
    # Get the recorded transform node
    updatedTransformNode = self.mrmlScene.GetFirstNodeByName( transformName )
    
    # Get all transforms in the scene
    transformCollection = vtk.vtkCollection()
    self.peLogic.GetSceneVisibleTransformNodes( transformCollection )
    
    # Update all metrics associated with children of the recorded transform
    for i in range( transformCollection.GetNumberOfItems() ):
      currentTransformNode = transformCollection.GetItemAsObject( i )
      if ( self.peLogic.IsSelfOrDescendentTransformNode( updatedTransformNode, currentTransformNode ) ):
        self.UpdateTransformMetrics( currentTransformNode, absTime, updateTable )

        
  def UpdateTransformMetrics( self, transformNode, absTime, updateTable ):
      
    # The assumption is that the scene is already appropriately updated
    matrix = vtk.vtkMatrix4x4()
    matrix.Identity()
    transformNode.GetMatrixTransformToWorld( matrix )
    point = [ matrix.GetElement( 0, 3 ), matrix.GetElement( 1, 3 ), matrix.GetElement( 2, 3 ), matrix.GetElement( 3, 3 ) ]
    
    for metricInstanceID in self.allMetrics:
      metric = self.allMetrics[ metricInstanceID ]
      metricInstanceNode = self.mrmlScene.GetNodeByID( metricInstanceID )
      
      for role in metric.GetAcceptedTransformRoles():
        if ( metricInstanceNode.GetRoleID( role, metricInstanceNode.TransformRole ) == transformNode.GetID() ):
          try:
            metric.AddTimestamp( absTime, matrix, point, role )
          except TypeError: # Only look if there is an issue with the number of arguments
            metric.AddTimestamp( absTime, matrix, point ) # TODO: Keep this for backwards compatibility with Python Metrics?
      
    # Output the results to the metrics table node
    # TODO: Do we have to clear it all and re-write it all?
    if ( updateTable ):
      self.OutputAllMetricsToMetricsTable()

    


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
