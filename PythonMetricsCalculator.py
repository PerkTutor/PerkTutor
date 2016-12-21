import os, imp, glob, sys
import urllib, zipfile
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
  
  # We propose two concepts for metric distribution:
  # Sharing: Whether or not to sync the metric with every Perk Evaluator node
  # Ubiquity: Whether or not the metric spreads to every transform
  
  NEEDLE_LENGTH = 300 # 30cm is approximate need length
  
  ADDITIONAL_METRICS_URL = "https://github.com/PerkTutor/PythonMetrics/archive/master.zip"
  METRICS_ZIP_FILE_NAME = "PythonMetrics.zip"
   
  def __init__( self ):    
    self.realTimeMetrics = dict()
    self.realTimeMetricsTable = None
    
    
  @staticmethod
  def Initialize():
    # Static variables (common to all instances of the PythonMetricsCalculatorLogic)
    PythonMetricsCalculatorLogic.AllMetricModules = dict()
    
    PythonMetricsCalculatorLogic.SetMRMLScene( None )
    PythonMetricsCalculatorLogic.SetPerkEvaluatorLogic( None )
    
    PythonMetricsCalculatorLogic.AddCoreMetricsToScene()
      
      
  @staticmethod
  def AddCoreMetricsToScene():
    coreMetricScriptDirectory = os.path.join( os.path.dirname( __file__ ), "PythonMetrics" )
    PythonMetricsCalculatorLogic.AddMetricsFromDirectoryToScene( coreMetricScriptDirectory )

      
  @staticmethod
  def DownloadAdditionalMetrics():
    metricsDownloadDirectory = slicer.mrmlScene.GetCacheManager().GetRemoteCacheDirectory()
    metricsFullZipFileName = os.path.join( metricsDownloadDirectory, PythonMetricsCalculatorLogic.METRICS_ZIP_FILE_NAME )

    # Download the zip file
    urllib.urlretrieve( PythonMetricsCalculatorLogic.ADDITIONAL_METRICS_URL, metricsFullZipFileName )

    # Extract the zip file
    metricsZipFile = zipfile.ZipFile( metricsFullZipFileName )
    metricsZipFile.extractall( metricsDownloadDirectory )
    
    additionalMetricScriptDirectory = os.path.join( metricsDownloadDirectory, "PythonMetrics-master" )
    PythonMetricsCalculatorLogic.AddMetricsFromDirectoryToScene( additionalMetricScriptDirectory )

    
  @staticmethod
  def AddMetricsFromDirectoryToScene( metricsDirectory ):
    metricScriptFiles = glob.glob( os.path.join( metricsDirectory, "[a-z]*.py" ) )
    for script in metricScriptFiles:
      slicer.util.loadNodeFromFile( script, "Python Metric Script" )
    
  
  @staticmethod 
  def SetMRMLScene( newScene ):
    PythonMetricsCalculatorLogic.mrmlScene = newScene
  
  
  @staticmethod
  def GetMRMLScene():
    if ( PythonMetricsCalculatorLogic.mrmlScene != None ):
      return PythonMetricsCalculatorLogic.mrmlScene # Try to return the set scene
      
    try:
      return slicer.mrmlScene # Try to return Slicer's scene
    except:
      return None
  

  @staticmethod
  def SetPerkEvaluatorLogic( newPELogic ):
    PythonMetricsCalculatorLogic.peLogic = newPELogic    
    
  
  @staticmethod
  def GetPerkEvaluatorLogic():
    if ( PythonMetricsCalculatorLogic.peLogic != None ):
      return PythonMetricsCalculatorLogic.peLogic # Try to return the set logic
      
    try:
      return slicer.modules.perkevaluator.logic() # Try to return the module's logic from Python
    except:
      return None
    
    
  @staticmethod
  def InitializeMetricsTable( metricsTable ):
    if ( metricsTable == None ):
      return
  
    metricsTable.GetTable().Initialize()
    
    # TODO: Make the more robust (e.g. qSlicerMetricsTableWidget::METRIC_TABLE_COLUMNS) 
    metricsTableColumnNames = [ "MetricName", "MetricRoles", "MetricUnit", "MetricValue" ]
    for columnName in metricsTableColumnNames:
      column = vtk.vtkStringArray()
      column.SetName( columnName )
      metricsTable.GetTable().AddColumn( column )
      
      
  @staticmethod   
  def OutputAllMetricsToMetricsTable( metricsTable, allMetrics ):
    if ( metricsTable == None ):
      return

    # Hold off on modified events until we are finished modifying
    modifyFlag = metricsTable.StartModify()
  
    PythonMetricsCalculatorLogic.InitializeMetricsTable( metricsTable )

    metricsTable.GetTable().SetNumberOfRows( len( allMetrics ) )
    insertRow = 0
    for metric in allMetrics.values():
      metricsTable.GetTable().SetValueByName( insertRow, "MetricName", metric.GetMetricName() )
      metricsTable.GetTable().SetValueByName( insertRow, "MetricRoles", metric.CombinedRoleString )
      metricsTable.GetTable().SetValueByName( insertRow, "MetricUnit", metric.GetMetricUnit() )
      metricsTable.GetTable().SetValueByName( insertRow, "MetricValue", metric.GetMetric() )
      insertRow += 1

    metricsTable.EndModify( modifyFlag )
    

  @staticmethod
  def RefreshMetricModules():
    PythonMetricsCalculatorLogic.AllMetricModules = PythonMetricsCalculatorLogic.GetFreshMetricModules()
    
  @staticmethod
  def GetFreshMetricModules():
    if ( PythonMetricsCalculatorLogic.GetMRMLScene() == None ):
      return dict()
    
    # Setup the metrics currently associated with the selected PerkEvaluator node
    metricModuleDict = dict()
    
    # Grab all of the metric script nodes in the scene
    metricScriptNodes = PythonMetricsCalculatorLogic.GetMRMLScene().GetNodesByClass( "vtkMRMLMetricScriptNode" )
    
    for i in range( metricScriptNodes.GetNumberOfItems() ):
      execDict = dict()
      currentMetricScriptNode = metricScriptNodes.GetItemAsObject( i )
      exec currentMetricScriptNode.GetPythonSourceCode() in execDict
      metricModuleDict[ currentMetricScriptNode.GetID() ] = execDict[ "PerkEvaluatorMetric" ]
    
    return metricModuleDict
    
  
  @staticmethod
  def GetFreshMetrics( peNodeID ):
    if ( PythonMetricsCalculatorLogic.GetMRMLScene() == None ):
      return dict()
      
    peNode = PythonMetricsCalculatorLogic.GetMRMLScene().GetNodeByID( peNodeID )
    if ( peNode == None ):
      return dict()
  
    # Get a fresh set of metric modules
    newMetricModules = PythonMetricsCalculatorLogic.GetFreshMetricModules()
    
    # Setup the metrics currently associated with the selected PerkEvaluator node
    metricDict = dict()
    
    # TODO: Make the reference role calling more robust (i.e. vtkMRMLPerkEvaluatorNode::METRIC_INSTANCE_REFERENCE_ROLE)
    for i in range( peNode.GetNumberOfNodeReferences( "MetricInstance" ) ):
      metricInstanceNode = peNode.GetNthNodeReference( "MetricInstance", i )
      if ( metricInstanceNode.GetAssociatedMetricScriptID() not in newMetricModules ):
        continue # Ignore metrics whose associated script is not loaded (e.g. if it has been deleted)
      
      associatedMetricModule = newMetricModules[ metricInstanceNode.GetAssociatedMetricScriptID() ]
      if ( PythonMetricsCalculatorLogic.AreMetricModuleRolesSatisfied( associatedMetricModule, metricInstanceNode ) ):
        metricDict[ metricInstanceNode.GetID() ] = associatedMetricModule() # Note: The brackets are important (they instantiate the instance)
        # Add the roles description (to make it easier to distinguish the same metric under different roles)
        metricDict[ metricInstanceNode.GetID() ].CombinedRoleString = metricInstanceNode.GetCombinedRoleString()
        
    # Add the anatomy to the fresh metrics
    PythonMetricsCalculatorLogic.AddAnatomyNodesToMetrics( metricDict )
    PythonMetricsCalculatorLogic.SetNeedleOrientation( metricDict, peNode )
   
    return metricDict
    
    
  @staticmethod
  def AreMetricModuleRolesSatisfied( metricModule, metricInstanceNode ):
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
  @staticmethod
  def AddAnatomyNodesToMetrics( metrics ): 
    if ( PythonMetricsCalculatorLogic.GetMRMLScene() == None ):
      return
  
    # Keep track of which metrics all anatomies are sucessfully delivered to    
    unfulfilledAnatomies = []    
  
    for metricInstanceID in metrics:
      metricAnatomyRoles = metrics[ metricInstanceID ].GetRequiredAnatomyRoles()
      metricInstanceNode = PythonMetricsCalculatorLogic.GetMRMLScene().GetNodeByID( metricInstanceID )
      
      for role in metricAnatomyRoles:
        anatomyNode = metricInstanceNode.GetRoleNode( role, metricInstanceNode.AnatomyRole )
        added = metrics[ metricInstanceID ].AddAnatomyRole( role, anatomyNode )
        
        if ( not added ):
          unfulfilledAnatomies.append( metricInstanceID )
          
    # In practice, the anatomies should always be fulfilled because we already filtered out those that could not be fulfilled
    # However, if the wrong type of node is selected, then this may return false
    for metricInstanceID in unfulfilledAnatomies:
      metrics.pop( metricInstanceID )

  
  @staticmethod
  def SetNeedleOrientation( metrics, peNode ):
    if( peNode == None ):
      return
      
    peNodeNeedleOrientation = [ 0, 0, 0 ]
    peNode.GetNeedleOrientation( peNodeNeedleOrientation )
    
    for metricInstanceID in metrics:
      metrics[ metricInstanceID ].NeedleOrientation = peNodeNeedleOrientation[:] # Element copy
      
        
  # Note: We are returning a list here, not a dictionary
  @staticmethod
  def GetAllRoles( metricScriptID, roleType ):
    if ( metricScriptID not in PythonMetricsCalculatorLogic.AllMetricModules ):
      return []
  
    if ( roleType == slicer.vtkMRMLMetricInstanceNode.TransformRole ):
      return PythonMetricsCalculatorLogic.AllMetricModules[ metricScriptID ].GetAcceptedTransformRoles()
    elif ( roleType == slicer.vtkMRMLMetricInstanceNode.AnatomyRole ):
      return PythonMetricsCalculatorLogic.AllMetricModules[ metricScriptID ].GetRequiredAnatomyRoles().keys()
    else:
      return []
    
    
  # Note: We are returning a string here
  @staticmethod
  def GetAnatomyRoleClassName( metricScriptID, role ):
    if ( metricScriptID not in PythonMetricsCalculatorLogic.AllMetricModules ):
      return "" 
      
    return PythonMetricsCalculatorLogic.AllMetricModules[ metricScriptID ].GetRequiredAnatomyRoles()[ role ]
    
  # Note: We are returning a string here
  @staticmethod
  def GetMetricName( metricScriptID ):
    if ( metricScriptID not in PythonMetricsCalculatorLogic.AllMetricModules ):
      return ""
      
    return PythonMetricsCalculatorLogic.AllMetricModules[ metricScriptID ].GetMetricName()
      
      
  # Note: We are returning a string here
  @staticmethod
  def GetMetricUnit( metricScriptID ):
    if ( metricScriptID not in PythonMetricsCalculatorLogic.AllMetricModules ):
      return ""
            
    return PythonMetricsCalculatorLogic.AllMetricModules[ metricScriptID ].GetMetricUnit()      

   
  # Note: We are returning a bool here
  @staticmethod
  def GetMetricShared( metricScriptID ):
    if ( metricScriptID not in PythonMetricsCalculatorLogic.AllMetricModules ):
      return False
      
    try:
      return PythonMetricsCalculatorLogic.AllMetricModules[ metricScriptID ].GetMetricShared()
    except: # TODO: Keep this for backwards compatibility with Python Metrics?
      return True
      
  
  # Note: We are returning a bool here
  @staticmethod
  def GetMetricPervasive( metricScriptID ):
    if ( metricScriptID not in PythonMetricsCalculatorLogic.AllMetricModules ):
      return False
    
    numTransformRoles = len( PythonMetricsCalculatorLogic.AllMetricModules[ metricScriptID ].GetAcceptedTransformRoles() ) #TODO: Add check for "Any" role?
    numAnatomyRoles = len( PythonMetricsCalculatorLogic.AllMetricModules[ metricScriptID ].GetRequiredAnatomyRoles().keys() )
    if ( numTransformRoles != 1 or numAnatomyRoles != 0 ):
      return False
      
    try:
      return PythonMetricsCalculatorLogic.AllMetricModules[ metricScriptID ].GetMetricPervasive()
    except: # TODO: Keep this for backwards compatibility with Python Metrics?
      return True
      
    
  @staticmethod
  def CalculateAllMetrics( peNodeID ):
    if ( PythonMetricsCalculatorLogic.GetMRMLScene() == None or PythonMetricsCalculatorLogic.GetPerkEvaluatorLogic() == None ):
      return
      
    peNode = PythonMetricsCalculatorLogic.GetMRMLScene().GetNodeByID( peNodeID )
    if ( peNode == None or peNode.GetTransformBufferNode() == None ):
      return dict()
  
    allMetrics = PythonMetricsCalculatorLogic.GetFreshMetrics( peNodeID )
  
    # Start at the beginning (but remember where we were)
    originalPlaybackTime = peNode.GetPlaybackTime()
    
    # Now iterate over all of the trajectories
    combinedTransformBuffer = slicer.vtkLogRecordBuffer()
    peNode.GetTransformBufferNode().GetCombinedTransformRecordBuffer( combinedTransformBuffer )
    
    if ( combinedTransformBuffer.GetNumRecords() == 0 ):
      return
      
    peNode.SetPlaybackTime( combinedTransformBuffer.GetRecord( 0 ).GetTime(), True )
    peNode.SetAnalysisState( 0 )
    minTime = peNode.GetTransformBufferNode().GetMinimumTime()
  
    for i in range( combinedTransformBuffer.GetNumRecords() ):
    
      absTime = combinedTransformBuffer.GetRecord( i ).GetTime()
      relTime = absTime - minTime # Can't just take the 0th record of the combined buffer, because this doesn't account for the messages
      if ( relTime < peNode.GetMarkBegin() or relTime > peNode.GetMarkEnd() ):
        continue
        
      peNode.SetPlaybackTime( absTime, True )
      PythonMetricsCalculatorLogic.GetPerkEvaluatorLogic().UpdateSceneToPlaybackTime( peNode, combinedTransformBuffer.GetRecord( i ).GetDeviceName() )
      PythonMetricsCalculatorLogic.UpdateSelfAndChildMetrics( allMetrics, combinedTransformBuffer.GetRecord( i ).GetDeviceName(), absTime, None )
      
      # Update the progress
      progressPercent = 100 * ( relTime - peNode.GetMarkBegin() ) / ( peNode.GetMarkEnd() - peNode.GetMarkBegin() )
      peNode.SetAnalysisState( int( progressPercent ) )
      
      if ( peNode.GetAnalysisState() < 0 ):
        break

    
    if ( peNode.GetAnalysisState() >= 0 ):
      PythonMetricsCalculatorLogic.OutputAllMetricsToMetricsTable( peNode.GetMetricsTableNode(), allMetrics )
      
    peNode.SetPlaybackTime( originalPlaybackTime, False ) # Scene automatically updated
    peNode.SetAnalysisState( 0 )

  
  @staticmethod  
  def UpdateSelfAndChildMetrics( allMetrics, transformName, absTime, metricsTable ):
    if ( PythonMetricsCalculatorLogic.GetMRMLScene() == None or PythonMetricsCalculatorLogic.GetPerkEvaluatorLogic() == None ):
      return
  
    # Get the recorded transform node
    updatedTransformNode = PythonMetricsCalculatorLogic.GetMRMLScene().GetFirstNode( transformName, "vtkMRMLLinearTransformNode", [ False ] ) # TODO: Is there an error in this function?
    
    # Get all transforms in the scene
    transformCollection = vtk.vtkCollection()
    PythonMetricsCalculatorLogic.GetPerkEvaluatorLogic().GetSceneVisibleTransformNodes( transformCollection )
    
    # Update all metrics associated with children of the recorded transform
    for i in range( transformCollection.GetNumberOfItems() ):
      currentTransformNode = transformCollection.GetItemAsObject( i )
      if ( PythonMetricsCalculatorLogic.GetPerkEvaluatorLogic().IsSelfOrDescendentTransformNode( updatedTransformNode, currentTransformNode ) ):
        PythonMetricsCalculatorLogic.UpdateMetrics( allMetrics, currentTransformNode, absTime, metricsTable )

  
  @staticmethod  
  def UpdateMetrics( allMetrics, transformNode, absTime, metricsTable ):
    if ( PythonMetricsCalculatorLogic.GetMRMLScene() == None ):
      return
      
    # The assumption is that the scene is already appropriately updated
    matrix = vtk.vtkMatrix4x4()
    matrix.Identity()
    transformNode.GetMatrixTransformToWorld( matrix )
    point = [ matrix.GetElement( 0, 3 ), matrix.GetElement( 1, 3 ), matrix.GetElement( 2, 3 ), matrix.GetElement( 3, 3 ) ]
    
    for metricInstanceID in allMetrics:
      metric = allMetrics[ metricInstanceID ]
      metricInstanceNode = PythonMetricsCalculatorLogic.GetMRMLScene().GetNodeByID( metricInstanceID )
      
      for role in metric.GetAcceptedTransformRoles():
        if ( metricInstanceNode.GetRoleID( role, metricInstanceNode.TransformRole ) == transformNode.GetID() ):
          try:
            metric.AddTimestamp( absTime, matrix, point, role )
          except TypeError: # Only look if there is an issue with the number of arguments
            metric.AddTimestamp( absTime, matrix, point ) # TODO: Keep this for backwards compatibility with Python Metrics?
      
    # Output the results to the metrics table node
    # TODO: Do we have to clear it all and re-write it all?
    if ( metricsTable != None ):
      PythonMetricsCalculatorLogic.OutputAllMetricsToMetricsTable( metricsTable, allMetrics )
      
      
  # Instance methods for real-time metric computation
  def SetupRealTimeMetricComputation( self, peNodeID ):
    if ( PythonMetricsCalculatorLogic.GetMRMLScene() == None  ):
      return
      
    peNode = PythonMetricsCalculatorLogic.GetMRMLScene().GetNodeByID( peNodeID )
    if ( peNode == None or peNode.GetMetricsTableNode() == None ):
      return dict()
      
    self.realTimeMetrics = PythonMetricsCalculatorLogic.GetFreshMetrics( peNodeID )
    self.realTimeMetricsTable = peNode.GetMetricsTableNode()
    
    
  def UpdateRealTimeMetrics( self, transformName, absTime ):
    PythonMetricsCalculatorLogic.UpdateSelfAndChildMetrics( self.realTimeMetrics, transformName, absTime, self.realTimeMetricsTable )
    


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
    print( "CTEST_FULL_OUTPUT" )
    
    # These are the IDs of the relevant nodes
    transformBufferID = "vtkMRMLTransformBufferNode1"
    tissueModelID = "vtkMRMLModelNode4"
    needleTransformID = "vtkMRMLLinearTransformNode4"
    trueTableID = "vtkMRMLTableNode1"
    
    sceneFile = os.path.join( os.path.dirname( os.path.abspath( __file__ ) ), "Data", "Lumbar", "Scene_Lumbar.mrml" )
    
    # Load the scene
    activeScene = slicer.mrmlScene
    activeScene.Clear( 0 )
    activeScene.SetURL( sceneFile )
    if ( activeScene.Import() != 1 ):
      raise Exception( "Scene import failed. Scene file: " + sceneFile )   
    
    # Manually add the metric scripts (since they are not saved with the scene, and will be clear with the scene clear)
    coreMetricScriptFiles = glob.glob( os.path.join( os.path.dirname( __file__ ), "PythonMetrics", "[a-z]*.py" ) )
    for script in coreMetricScriptFiles:
      slicer.util.loadNodeFromFile( script, "Python Metric Script" ) # This will load into activeScene, since activeScene == slicer.mrmlScene
    
    transformBufferNode = activeScene.GetNodeByID( transformBufferID )
    if ( transformBufferNode == None ):
      raise Exception( "Bad transform buffer." )
      
    tissueModelNode = activeScene.GetNodeByID( tissueModelID )
    if ( tissueModelNode == None ):
      raise Exception( "Bad tissue model." )
      
    needleTransformNode = activeScene.GetNodeByID( needleTransformID )
    if ( needleTransformNode == None ):
      raise Exception( "Bad needle transform." )
      
    trueTableNode = activeScene.GetNodeByID( trueTableID )
    if ( trueTableNode == None ):
      raise Exception( "Bad true metrics table." )
    
    # Setup the analysis
    peLogic = slicer.modules.perkevaluator.logic()

    # Setup the parameters
    peNode = activeScene.CreateNodeByClass( "vtkMRMLPerkEvaluatorNode" )
    peNode.SetScene( activeScene )
    activeScene.AddNode( peNode )
    
    mtNode = activeScene.CreateNodeByClass( "vtkMRMLTableNode" )
    mtNode.SetScene( activeScene )
    activeScene.AddNode( mtNode )
    
    peNode.SetTransformBufferID( transformBufferNode.GetID() )
    peNode.SetMetricsTableID( mtNode.GetID() )

    # Now propagate the roles
    peLogic.SetMetricInstancesRolesToID( peNode, needleTransformNode.GetID(), "Needle", slicer.vtkMRMLMetricInstanceNode.TransformRole )
    peLogic.SetMetricInstancesRolesToID( peNode, tissueModelNode.GetID(), "Tissue", slicer.vtkMRMLMetricInstanceNode.AnatomyRole )

    # Set the analysis begin and end times
    peNode.SetMarkBegin( 0 )
    peNode.SetMarkEnd( peLogic.GetMaximumRelativePlaybackTime( peNode ) )
    
    # Calculate the metrics
    peLogic.ComputeMetrics( peNode )
    
    # Check both tables to make sure they have the same number of rows    
    if ( trueTableNode.GetTable().GetNumberOfRows() != mtNode.GetTable().GetNumberOfRows() ):
      print "True number of metrics:", trueTableNode.GetTable().GetNumberOfRows(), ", calculated number of metrics:", mtNode.GetTable().GetNumberOfRows()
      raise Exception( "A different number of metrics was computed."  )

    # Compare the metrics to the expected results
    metricsFail = False
    for i in range( mtNode.GetTable().GetNumberOfRows() ):
      
      rowMatch = False # Need to match one row
      for j in range( trueTableNode.GetTable().GetNumberOfRows() ):
        
        colMatch = True # For a given row, need to match every column
        for k in range( trueTableNode.GetTable().GetNumberOfColumns() ):
          columnName = trueTableNode.GetTable().GetColumnName( k )
          trueValue = trueTableNode.GetTable().GetValueByName( j, columnName )
          testValue = mtNode.GetTable().GetValueByName( i, columnName )
          if ( not testValue.IsValid() ):
            raise Exception( "The metrics table was improperly formatted." )
          if ( trueValue != testValue ):
            colMatch = False
        
        if ( colMatch ):
          rowMatch = True
          
      # If we could not find a row in the true table that matches the row in the test table, report an incorrect metric
      if ( not rowMatch ):
        print "Incorrect metric.",
        for k in range( mtNode.GetTable().GetNumberOfColumns() ):
          print mtNode.GetTable().GetColumnName( k ), mtNode.GetTable().GetValue( i, k ),
        print ""          
        metricsFail = True
        
    if ( metricsFail == True ):
      self.delayDisplay( "Test failed! Calculated metrics were not consistent with results." )
    else:
      self.delayDisplay( "Test passed! Calculated metrics match results!" )
      
    self.assertFalse( metricsFail )
