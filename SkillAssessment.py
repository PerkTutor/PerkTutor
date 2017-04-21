import os
import unittest
import vtk, qt, ctk, slicer
import numpy
from slicer.ScriptedLoadableModule import *
import logging

#
# SkillAssessment
#

class SkillAssessment( ScriptedLoadableModule ):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__( self, parent ):
    ScriptedLoadableModule.__init__( self, parent )
    self.parent.title = "Skill Assessment" # TODO make this more human readable by adding spaces
    self.parent.categories = [ "Perk Tutor" ]
    self.parent.dependencies = []
    self.parent.contributors = [ "Matthew S. Holden (Queen's University)" ] # replace with "Firstname Lastname (Organization)"
    self.parent.helpText = """
    This module computes the overall skill level of an operator performing an image-guided intervention. <br>
    Raw Method: Computes a weighted average of metrics. <br>
    Percentile Method: Computes a weighted average of percentile scores (use for non-parametrically distributed data). <br>
    Z-Score Method" Computes a weight average of z-scores (use for normally distributed data).
    """
    self.parent.acknowledgementText = """
    Acknowledgements.
    """ # replace with organization, grant and thanks.

#
# SkillAssessmentWidget
#

class SkillAssessmentWidget( ScriptedLoadableModuleWidget ):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup( self ):
    ScriptedLoadableModuleWidget.setup( self )

    # Instantiate and connect widgets ...
    self.saLogic = SkillAssessmentLogic()

    #
    # Assessment Area
    #
    assessmentCollapsibleButton = ctk.ctkCollapsibleButton()
    assessmentCollapsibleButton.text = "Assessment"
    self.layout.addWidget( assessmentCollapsibleButton )

    # Layout within the dummy collapsible button
    assessmentFormLayout = qt.QFormLayout( assessmentCollapsibleButton )

    #
    # Input metrics selector
    #
    self.metricsSelector = slicer.qMRMLNodeComboBox()
    self.metricsSelector.nodeTypes = [ "vtkMRMLTableNode" ]
    self.metricsSelector.selectNodeUponCreation = True
    self.metricsSelector.addEnabled = False
    self.metricsSelector.removeEnabled = False
    self.metricsSelector.showHidden = False
    self.metricsSelector.showChildNodeTypes = False
    self.metricsSelector.setMRMLScene( slicer.mrmlScene )
    self.metricsSelector.setToolTip( "Choose the metrics table to assess." )
    assessmentFormLayout.addRow( "Metrics: ", self.metricsSelector )
    
    #
    # Weight selector
    #
    self.weightSelector = slicer.qMRMLNodeComboBox()
    self.weightSelector.nodeTypes = [ "vtkMRMLTableNode" ]
    self.weightSelector.selectNodeUponCreation = True
    self.weightSelector.noneEnabled = True
    self.weightSelector.addEnabled = False
    self.weightSelector.removeEnabled = False
    self.weightSelector.showHidden = False
    self.weightSelector.showChildNodeTypes = False
    self.weightSelector.setMRMLScene( slicer.mrmlScene )
    self.weightSelector.setToolTip( "Choose the weights for assessment." )
    assessmentFormLayout.addRow( "Weights: ", self.weightSelector )
    
    #
    # Training set selector
    #
    self.trainingSetSelector = slicer.qMRMLCheckableNodeComboBox()
    self.trainingSetSelector.nodeTypes = [ "vtkMRMLTableNode" ]
    self.trainingSetSelector.addEnabled = False
    self.trainingSetSelector.removeEnabled = False
    self.trainingSetSelector.showHidden = False
    self.trainingSetSelector.showChildNodeTypes = False
    self.trainingSetSelector.setMRMLScene( slicer.mrmlScene )
    self.trainingSetSelector.setToolTip( "Choose the training data for the assessment." )
    assessmentFormLayout.addRow( "Training Data: ", self.trainingSetSelector )
    
    trainingNodes = self.trainingSetSelector.uncheckedNodes()
    for node in trainingNodes:
      self.trainingSetSelector.setCheckState( node, 2 ) # By default, check all of the training data so that we don't have to change the parameters unless we want to exclude training data.
      # TODO: Use qt.Checked instead of 2


    #
    # Assess Button
    #
    self.assessButton = qt.QPushButton( "Assess" )
    self.assessButton.toolTip = "Assess proficiency."
    assessmentFormLayout.addRow( self.assessButton )
    
    #
    # Results Label
    #
    self.resultsLabel = qt.QLabel( "" )
    self.resultsLabel.toolTip = "The overall proficiency score."
    assessmentFormLayout.addRow( "Result: ", self.resultsLabel )

    
    #
    # Options Area
    #
    optionsCollapsibleButton = ctk.ctkCollapsibleButton()
    optionsCollapsibleButton.text = "Options"
    optionsCollapsibleButton.collapsed = True
    self.layout.addWidget(optionsCollapsibleButton)

    # Layout within the dummy collapsible button
    optionsFormLayout = qt.QFormLayout(optionsCollapsibleButton)
    
        
    #
    # Assessment method combo box 
    #    
    self.assessmentMethodComboBox = qt.QComboBox()
    self.assessmentMethodComboBox.addItem( "Raw" )
    self.assessmentMethodComboBox.addItem( "Z-Score" )
    self.assessmentMethodComboBox.addItem( "Percentile" )
    self.assessmentMethodComboBox.setToolTip( "Choose the assessment method." )
    optionsFormLayout.addRow( "Assessment Method: ", self.assessmentMethodComboBox )
    
    #
    # Aggregation method combo box 
    #    
    self.aggregationMethodComboBox = qt.QComboBox()
    self.aggregationMethodComboBox.addItem( "Mean" )
    self.aggregationMethodComboBox.addItem( "Median" )
    self.aggregationMethodComboBox.addItem( "Maximum" )
    self.aggregationMethodComboBox.setToolTip( "Choose the aggregation method." )
    optionsFormLayout.addRow( "Aggregation Method: ", self.aggregationMethodComboBox )




    # connections
    self.metricsSelector.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onMetricsChanged )
    self.weightSelector.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onWeightsChanged )
    self.trainingSetSelector.connect( 'checkedNodesChanged()', self.onTrainingSetChanged )
    
    self.assessButton.connect( 'clicked(bool)', self.onAssessButtonClicked )

    # Add vertical spacer
    self.layout.addStretch(1)


  def cleanup( self ):
    pass
    
    
  def onMetricsChanged( self, metricsNode ):
    self.saLogic.metricsNode = metricsNode
    
  def onWeightsChanged( self, weightNode ):
    self.saLogic.weightNode = weightNode
    
  def onTrainingSetChanged( self ):
    self.saLogic.trainingSet = self.trainingSetSelector.checkedNodes()

  def onAssessButtonClicked( self ):
    result = 0
    
    transformationMethod = self.assessmentMethodComboBox.currentText
    aggregationMethod = self.aggregationMethodComboBox.currentText
      
    result = self.saLogic.Assess( transformationMethod, aggregationMethod )
        
    self.resultsLabel.text = result
    
    # Pop-up the big assessment table in a new window
    
    # TODO
    # HACK
    # This uses an absolute path # This is only for debugging until I can get the widget to be loaded from the additional module paths
    import imp
    saWidgets = imp.load_dynamic( "qSlicerSkillAssessmentModuleWidgetsPythonQt", "d:\PerkTutor\SkillAssessment-0307D\lib\Slicer-4.7\qt-loadable-modules\Debug\qSlicerSkillAssessmentModuleWidgetsPythonQt.pyd" )
    
    self.assessmentTable = saWidgets.qSlicerAssessmentTableWidget()
    self.assessmentTable.setMetricsNode( self.saLogic.metricsNode )
    self.assessmentTable.setMetricsWeightNode( self.saLogic.weightNode )
    self.assessmentTable.setMetricScoreNode( self.saLogic.metricScoreNode )
    self.assessmentTable.setTaskScoreNode( self.saLogic.taskScoreNode )
    self.assessmentTable.setOverallScore( self.saLogic.overallScore )
    self.assessmentTable.show()
    
    

    

#
# SkillAssessmentLogic
#

class SkillAssessmentLogic( ScriptedLoadableModuleLogic ):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """
  def __init__( self ):
    self.metricsNode = None
    self.weightNode = None
    self.metricScoreNode = None
    self.taskScoreNode = None
    
    self.trainingSet = None
    
    self.overallScore = 0
  
  
  def Assess( self, transformationMethod, aggregationMethod ):
  
    if ( transformationMethod == "" or aggregationMethod == "" ):
      logging.info( "SkillAssessmentLogic::Assess: Transformation or aggregation method improperly specified. Please pick on of the pre-defined options." )
      return 0

    if ( self.metricsNode is None ):
      logging.info( "SkillAssessmentLogic::Assess: Metrics table is empty. Could not assess." )
      return 0
      
    if ( self.trainingSet is None or len( self.trainingSet ) == 0 ):
      logging.info( "SkillAssessmentLogic::Assess: Training dataset is empty. Could not assess." )
      return 0
      
    if ( self.weightNode is None ):
      self.weightNode = self.CreateTableNodeFromMetrics( self.metricsNode, 1 )
    
    # TODO: Make this code more modular
    
    # Transform the metric values
    transformedMetricsNode = self.CreateTableNodeFromMetrics( self.metricsNode, 0 )
    transformedMetricsTable = transformedMetricsNode.GetTable()
    
    for rowIndex in range( transformedMetricsTable.GetNumberOfRows() ):
      metricName = transformedMetricsTable.GetValueByName( rowIndex, "MetricName" )
      metricRoles = transformedMetricsTable.GetValueByName( rowIndex, "MetricRoles" ) 
      metricUnit = transformedMetricsTable.GetValueByName( rowIndex, "MetricUnit" )
      
      for columnIndex in range( transformedMetricsTable.GetNumberOfColumns() ):
        columnName = transformedMetricsTable.GetColumnName( columnIndex )
        if ( columnName == "MetricName" or columnName == "MetricRoles" or columnName == "MetricUnit" ):
          continue
          
        trainingMetricValues = self.GetMetricValuesFromNodes( self.trainingSet, metricName, metricRoles, metricUnit, columnName )
        testMetricValue = self.GetMetricValueFromNode( self.metricsNode, metricName, metricRoles, metricUnit, columnName )

        transformedMetricValue = self.GetTransformedMetricValue( testMetricValue, trainingMetricValues, transformationMethod )

        transformedMetricsTable.SetValue( rowIndex, columnIndex, transformedMetricValue )        
    
    # Compute the metric scores
    self.metricScoreNode = self.CreateMetricScoreTableNodeFromMetrics( self.metricsNode, 0 )
    metricScoreTable = self.metricScoreNode.GetTable()
    
    for rowIndex in range( metricScoreTable.GetNumberOfRows() ):
      metricName = metricScoreTable.GetValueByName( rowIndex, "MetricName" )
      metricRoles = metricScoreTable.GetValueByName( rowIndex, "MetricRoles" ) 
      metricUnit = metricScoreTable.GetValueByName( rowIndex, "MetricUnit" )
      
      metricValues = []
      weights = []
      for columnIndex in range( transformedMetricsTable.GetNumberOfColumns() ):
        columnName = transformedMetricsTable.GetColumnName( columnIndex )
        if ( columnName == "MetricName" or columnName == "MetricRoles" or columnName == "MetricUnit" ):
          continue
          
        metricValues.append( self.GetMetricValueFromNode( transformedMetricsNode, metricName, metricRoles, metricUnit, columnName ) )
        weights.append( self.GetMetricValueFromNode( self.weightNode, metricName, metricRoles, metricUnit, columnName ) )

      aggregatedMetricValue = self.GetAggregatedMetricValue( metricValues, weights, aggregationMethod )
      metricScoreTable.SetValueByName( rowIndex, "MetricScore", aggregatedMetricValue ) 
      
      
    # Compute the task scores
    self.taskScoreNode = self.CreateTaskScoreTableNodeFromMetrics( self.metricsNode, 0 )
    taskScoreTable = self.taskScoreNode.GetTable()
    
    for columnIndex in range( taskScoreTable.GetNumberOfColumns() ):
      columnName = taskScoreTable.GetColumnName( columnIndex )
      if ( columnName == "MetricName" or columnName == "MetricRoles" or columnName == "MetricUnit" ):
          continue
       
      metricValues = []
      weights = []       
      for rowIndex in range( transformedMetricsTable.GetNumberOfRows() ):
        metricValues.append( transformedMetricsTable.GetValueByName( rowIndex, columnName ).ToDouble() )
        weights.append( self.weightNode.GetTable().GetValueByName( rowIndex, columnName ).ToDouble() )
        
      aggregatedMetricValue = self.GetAggregatedMetricValue( metricValues, weights, aggregationMethod )
      taskScoreTable.SetValueByName( 0, columnName, aggregatedMetricValue )
      
      
    # Compute the overall score
    metricValues = []
    weights = []
    for rowIndex in range( transformedMetricsTable.GetNumberOfRows() ):
      for columnIndex in range( transformedMetricsTable.GetNumberOfColumns() ):
        columnName = transformedMetricsTable.GetColumnName( columnIndex )
        if ( columnName == "MetricName" or columnName == "MetricRoles" or columnName == "MetricUnit" ):
          continue
          
        metricValues.append( transformedMetricsTable.GetValueByName( rowIndex, columnName ).ToDouble() )
        weights.append( self.weightNode.GetTable().GetValueByName( rowIndex, columnName ).ToDouble() )

    self.overallScore = self.GetAggregatedMetricValue( metricValues, weights, aggregationMethod )

    return self.overallScore
    
    
  @staticmethod
  def CreateTaskScoreTableNodeFromMetrics( metricsNode, value ):
    tableNode = slicer.vtkMRMLTableNode()
    table = tableNode.GetTable()
    
    # Constructive approach
    metricsTable = metricsNode.GetTable()
    
    metricNameColumn = vtk.vtkStringArray()
    metricNameColumn.SetName( "MetricName" )
    metricNameColumn.InsertNextValue( "TaskScore" )
    table.AddColumn( metricNameColumn )
    
    metricRolesColumn = vtk.vtkStringArray()
    metricRolesColumn.SetName( "MetricRoles" )
    metricRolesColumn.InsertNextValue( "[]" )
    table.AddColumn( metricRolesColumn )
    
    metricUnitColumn = vtk.vtkStringArray()
    metricUnitColumn.SetName( "MetricUnit" )
    metricUnitColumn.InsertNextValue( "-" )
    table.AddColumn( metricUnitColumn )
    
    for columnIndex in range( metricsTable.GetNumberOfColumns() ):
      currColumnName = metricsTable.GetColumnName( columnIndex )
      newColumn = vtk.vtkStringArray()
      newColumn.SetName( currColumnName )
      newColumn.InsertNextValue( str( value ) )
      table.AddColumn( newColumn )

    return tableNode
    
 
  @staticmethod
  def CreateMetricScoreTableNodeFromMetrics( metricsNode, value ):
    tableNode = slicer.vtkMRMLTableNode()
    table = tableNode.GetTable()

    # Constructive approach
    metricsTable = metricsNode.GetTable()
    for columnIndex in range( metricsTable.GetNumberOfColumns() ):
      currColumnName = metricsTable.GetColumnName( columnIndex )
      if ( currColumnName != "MetricName" and currColumnName != "MetricRoles" and currColumnName != "MetricUnit" ):
        continue

      newColumn = vtk.vtkStringArray()
      newColumn.SetName( currColumnName )
      newColumn.SetNumberOfValues( metricsTable.GetNumberOfRows() )
      for rowIndex in range( metricsTable.GetNumberOfRows() ):
        newColumn.SetValue( rowIndex, metricsTable.GetValue( rowIndex, columnIndex ).ToString() )
      table.AddColumn( newColumn )

    # Add the score colum
    metricScoreColumn = vtk.vtkStringArray()
    metricScoreColumn.SetName( "MetricScore" )
    metricScoreColumn.SetNumberOfValues( metricsTable.GetNumberOfRows() )
    for rowIndex in range( metricsTable.GetNumberOfRows() ):
      metricScoreColumn.SetValue( rowIndex, str( value ) )
    table.AddColumn( metricScoreColumn )
      
    return tableNode


  @staticmethod
  def CreateTableNodeFromMetrics( metricsNode, value ):
    tableNode = slicer.vtkMRMLTableNode()
    table = tableNode.GetTable()
    table.DeepCopy( metricsNode.GetTable() ) # Make the weight table the same size
    for columnIndex in range( table.GetNumberOfColumns() ):
      columnName = table.GetColumnName( columnIndex )
      if ( columnName == "MetricName" or columnName == "MetricRoles" or columnName == "MetricUnit" ):
        continue
      for rowIndex in range( table.GetNumberOfRows() ):
        table.SetValue( rowIndex, columnIndex, value ) # Set the default value to 1
        
    return tableNode
    
    
  @staticmethod
  def CreateTableNodeFromMetrics( metricsNode, value ):
    tableNode = slicer.vtkMRMLTableNode()
    table = tableNode.GetTable()
    table.DeepCopy( metricsNode.GetTable() ) # Make the weight table the same size
    for columnIndex in range( table.GetNumberOfColumns() ):
      columnName = table.GetColumnName( columnIndex )
      if ( columnName == "MetricName" or columnName == "MetricRoles" or columnName == "MetricUnit" ):
        continue
      for rowIndex in range( table.GetNumberOfRows() ):
        table.SetValue( rowIndex, columnIndex, value ) # Set the default value to 1
        
    return tableNode

    
  @staticmethod
  def GetMetricValuesFromNodes( metricNodes, metricName, metricRoles, metricUnit, taskName ):
    metricValues = []
     
    for currMetricNode in metricNodes:
      currMetricValue = SkillAssessmentLogic.GetMetricValueFromNode( currMetricNode, metricName, metricRoles, metricUnit, taskName )
      metricValues.append( currMetricValue )
      
    return metricValues
      

  @staticmethod
  def GetMetricValueFromNode( metricsNode, metricName, metricRoles, metricUnit, taskName ):
    if ( metricsNode is None or metricsNode.GetTable() is None ):
      logging.info( "SkillAssessmentLogic::GetMetricValueFromNode: Table of metrics is empty." )
      return
    metricsTable = metricsNode.GetTable()  
    
    for rowIndex in range( metricsTable.GetNumberOfRows() ):
      nameMatch = ( metricsTable.GetValueByName( rowIndex, "MetricName" ) == metricName )
      rolesMatch = ( metricsTable.GetValueByName( rowIndex, "MetricRoles" ) == metricRoles )
      unitMatch = ( metricsTable.GetValueByName( rowIndex, "MetricUnit" ) == metricUnit ) # Unit should always match if the names match
      if ( nameMatch and rolesMatch and unitMatch ):
        return metricsTable.GetValueByName( rowIndex, taskName ).ToDouble()
        
    logging.info( "SkillAssessmentLogic::GetMetricValueFromNode: Could not find metric in the table." )
    return

  @staticmethod
  def GetAggregatedMetricValue( metricValues, weights, method ):
    if ( len( metricValues ) != len( weights ) ):
      logging.info( "SkillAssessmentLogic::GetAggregatedMetricValue: Metric values and weights do not correspond." )
      return 0
      
    if ( method == "Mean" ):
      return SkillAssessmentLogic.GetWeightedMean( metricValues, weights )
    if ( method == "Median" ):
      return SkillAssessmentLogic.GetWeightedMedian( metricValues, weights )
    if ( method == "Maximum" ):
      return SkillAssessmentLogic.GetMaximum( metricValues, weights )
      
    logging.info( "SkillAssessmentLogic::GetAggregatedMetricValue: Metric transformation method improperly specified." )
    return 0
  
  @staticmethod
  def GetWeightedMean( metricValues, weights ):
    valueSum = 0
    weightSum = 0
  
    for i in range( len( metricValues ) ):
      currMetricValue = metricValues[ i ]
      currWeight = weights[ i ]
      
      valueSum += currMetricValue * currWeight
      weightSum += currWeight
      
    return valueSum / weightSum


  @staticmethod
  def GetWeightedMedian( metricValues, weights ):
    sortedPairs = sorted( zip( metricValues, weights ) )
  
    totalWeight = sum( weights )
    weightSum = 0
    for pair in sortedPairs:
      weightSum += pair[ 1 ]
      if ( weightSum > 0.5 * totalWeight ):
        return pair[ 0 ]
      
    return metricValues[-1]

  @staticmethod
  def GetMaximum( metricValues, weights ):
    return max( metricValues ) 
      
    
  @staticmethod 
  def GetTransformedMetricValue( testMetric, trainingMetrics, method ):
    if ( method == "Raw" ):
      return SkillAssessmentLogic.GetRaw( testMetric, trainingMetrics )
    if ( method == "Percentile" ):
      return SkillAssessmentLogic.GetPercentile( testMetric, trainingMetrics )
    if ( method == "Z-Score" ):
      return SkillAssessmentLogic.GetZScore( testMetric, trainingMetrics )
      
    logging.info( "SkillAssessmentLogic::GetTransformedMetricValue: Metric transformation method improperly specified." )
    return 0
    
    
  # This assumes rank 0 is the smallest value and rank N is the largest value
  @staticmethod
  def GetRaw( testMetric, trainingMetrics ):
    return testMetric

  # This assumes rank 0 is the smallest value and rank N is the largest value
  @staticmethod
  def GetPercentile( testMetric, trainingMetrics ):
    tiedRank = 0
    for dataPoint in trainingMetrics:
      if ( testMetric > dataPoint ):
        tiedRank = tiedRank + 1
      if ( testMetric == dataPoint ):
        tiedRank = tiedRank + 0.5
        
    return tiedRank / len( trainingMetrics )


  # This computes the z-score
  @staticmethod
  def GetZScore( testMetric, trainingMetrics ):
    trainingMean = numpy.mean( trainingMetrics )
    trainingStd = numpy.std( trainingMetrics )
    
    zscore = 0
    if ( trainingStd is not 0 ):
      zscore = ( testMetric - trainingMean ) / trainingStd
      
    return zscore
    


class SkillAssessmentTest( ScriptedLoadableModuleTest ):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp( self ):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear( 0 )

  def runTest( self ):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_SkillAssessment1()

  def test_SkillAssessment1( self ):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests should exercise the functionality of the logic with different inputs
    (both valid and invalid).  At higher levels your tests should emulate the
    way the user would interact with your code and confirm that it still works
    the way you intended.
    One of the most important features of the tests is that it should alert other
    developers when their changes will have an impact on the behavior of your
    module.  For example, if a developer removes a feature that you depend on,
    your test should break so they know that the feature is needed.
    """

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=5767', 'FA.nrrd', slicer.util.loadVolume),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        logging.info('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        logging.info('Loading %s...' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading')

    volumeNode = slicer.util.getNode(pattern="FA")
    logic = SkillAssessmentLogic()
    self.assertIsNotNone( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
