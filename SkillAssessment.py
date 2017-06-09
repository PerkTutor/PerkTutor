import os
import unittest
import vtk, qt, ctk, slicer
import numpy
from slicer.ScriptedLoadableModule import *
import logging
from functools import partial

#
# SkillAssessment
#

ASSESSMENT_METHOD_ZSCORE = "Z-Score"
ASSESSMENT_METHOD_PERCENTILE = "Percentile"
ASSESSMENT_METHOD_RAW = "Raw"

AGGREGATION_METHOD_MEAN = "Mean"
AGGREGATION_METHOD_MEDIAN = "Median"
AGGREGATION_METHOD_MAXIMUM = "Maximum"

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
    Z-Score Method: Computes a weight average of z-scores (use for normally distributed data).
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

    self.parameterNodeObserverTags = []

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
    self.metricsSelector.noneEnabled = True
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
    self.weightsSelector = slicer.qMRMLNodeComboBox()
    self.weightsSelector.nodeTypes = [ "vtkMRMLTableNode" ]
    self.weightsSelector.selectNodeUponCreation = True
    self.weightsSelector.noneEnabled = True
    self.weightsSelector.addEnabled = False
    self.weightsSelector.removeEnabled = False
    self.weightsSelector.showHidden = False
    self.weightsSelector.showChildNodeTypes = False
    self.weightsSelector.setMRMLScene( slicer.mrmlScene )
    self.weightsSelector.setToolTip( "Choose the weights for assessment." )
    assessmentFormLayout.addRow( "Weights: ", self.weightsSelector )
    
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
    # Skill assessment module node selector
    #    
    self.parameterNodeSelector = slicer.qMRMLNodeComboBox()
    self.parameterNodeSelector.nodeTypes = [ "vtkMRMLScriptedModuleNode" ]
    self.parameterNodeSelector.addEnabled = True
    self.parameterNodeSelector.removeEnabled = True
    self.parameterNodeSelector.showHidden = True
    self.parameterNodeSelector.showChildNodeTypes = False
    self.parameterNodeSelector.addAttribute( "vtkMRMLScriptedModuleNode", "SkillAssessment" )
    self.parameterNodeSelector.baseName = "SkillAssessment"
    self.parameterNodeSelector.setMRMLScene( slicer.mrmlScene )
    self.parameterNodeSelector.setToolTip( "Select the module parameters node." )
    optionsFormLayout.addRow( "Parameter node: ", self.parameterNodeSelector )
    
        
    #
    # Assessment method combo box 
    #    
    self.assessmentMethodComboBox = qt.QComboBox()
    self.assessmentMethodComboBox.addItem( ASSESSMENT_METHOD_ZSCORE )
    self.assessmentMethodComboBox.addItem( ASSESSMENT_METHOD_PERCENTILE )
    self.assessmentMethodComboBox.addItem( ASSESSMENT_METHOD_RAW )
    self.assessmentMethodComboBox.setToolTip( "Choose the assessment method." )
    optionsFormLayout.addRow( "Assessment Method: ", self.assessmentMethodComboBox )
    
    #
    # Aggregation method combo box 
    #    
    self.aggregationMethodComboBox = qt.QComboBox()
    self.aggregationMethodComboBox.addItem( AGGREGATION_METHOD_MEAN )
    self.aggregationMethodComboBox.addItem( AGGREGATION_METHOD_MEDIAN )
    self.aggregationMethodComboBox.addItem( AGGREGATION_METHOD_MAXIMUM )
    self.aggregationMethodComboBox.setToolTip( "Choose the aggregation method." )
    optionsFormLayout.addRow( "Aggregation Method: ", self.aggregationMethodComboBox )
    
    #
    # Showing weight sliders
    #
    self.metricWeightSlidersCheckBox = qt.QCheckBox()
    self.metricWeightSlidersCheckBox.setChecked( True )
    self.metricWeightSlidersCheckBox.setText( "Show metric weights" )
    self.metricWeightSlidersCheckBox.setToolTip( "Allow weights for individual metrics to be adjusted from the assessment table." )
    optionsFormLayout.addWidget( self.metricWeightSlidersCheckBox )
    
    self.scoreWeightSlidersCheckBox = qt.QCheckBox()
    self.scoreWeightSlidersCheckBox.setChecked( True )
    self.scoreWeightSlidersCheckBox.setText( "Show score weights" )
    self.scoreWeightSlidersCheckBox.setToolTip( "Allow weights for an entire row or column to be adjusted from the assessment table." )
    optionsFormLayout.addWidget( self.scoreWeightSlidersCheckBox )
    
    #
    # Whether to show the transformed metric values or the raw values
    #
    self.showTransformedMetricValuesCheckBox = qt.QCheckBox()
    self.showTransformedMetricValuesCheckBox.setChecked( False )
    self.showTransformedMetricValuesCheckBox.setText( "Show transformed metrics" )
    self.showTransformedMetricValuesCheckBox.setToolTip( "Show the transformed metric values in the assessment table." )
    optionsFormLayout.addWidget( self.showTransformedMetricValuesCheckBox )
    
    #
    # The assessment table itself
    #
    self.assessmentTable = qt.QTableWidget()
    self.assessmentTable.horizontalHeader().hide()
    self.assessmentTable.verticalHeader().hide()
    self.assessmentTable.setShowGrid( False )
    self.assessmentTable.setAlternatingRowColors( True )
    self.assessmentTable.resize( 600, 400 ) # Reasonable starting size - to be adjusted by the user

    # connections
    self.parameterNodeSelector.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onParameterNodeChanged )
    self.metricsSelector.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onMetricsChanged )
    self.weightsSelector.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onWeightsChanged )
    self.trainingSetSelector.connect( 'checkedNodesChanged()', self.onTrainingSetChanged )
    
    self.assessmentMethodComboBox.connect( 'currentIndexChanged(QString)', self.onAssessmentMethodChanged )
    self.aggregationMethodComboBox.connect( 'currentIndexChanged(QString)', self.onAggregationMethodChanged )
    
    self.assessButton.connect( 'clicked(bool)', self.onAssessButtonClicked )

    # Create a default parameter node
    if ( self.parameterNodeSelector.currentNode() is None ):
      self.parameterNodeSelector.addNode( "vtkMRMLScriptedModuleNode" )

    # Add vertical spacer
    self.layout.addStretch(1)


  def cleanup( self ):
    pass
    
    
  def updateAssessmentTable( self, node ):
    if ( node is None ):
      return
    
    metricsNode = node.GetNodeReference( "Metrics" )
    weightsNode = node.GetNodeReference( "Weights" )
    
    if ( metricsNode is None or weightsNode is None ):
      logging.info( "SkillAssessmentWidget::updateAssessmentTableWidget: Empty metrics or weights node." )
      return
      
    metricsTable = metricsNode.GetTable()
    weightsTable = weightsNode.GetTable()
      
    if ( metricsTable is None or weightsTable is None ):
      logging.info( "SkillAssessmentWidget::updateAssessmentTableWidget: Metrics or weights table not specified." )
      return
      
    if ( metricsTable.GetNumberOfRows() == 0 or metricsTable.GetNumberOfColumns() == 0
      or metricsTable.GetNumberOfRows() != weightsTable.GetNumberOfRows()
      or metricsTable.GetNumberOfColumns() != weightsTable.GetNumberOfColumns() ):
      logging.info( "SkillAssessmentWidget::updateAssessmentTableWidget: Metrics and weights tables incompatible." )
      return
      
    numMetrics = metricsTable.GetNumberOfRows()
    numTasks = metricsTable.GetNumberOfColumns() - 3 # Ignore "MetricName", "MetricRoles", "MetricUnit"
    
    self.assessmentTable.setRowCount( numMetrics + 4 )
    self.assessmentTable.setColumnCount( numTasks + 4 )
    
    # Task headers
    taskColumnCount = 0
    for columnIndex in range( metricsTable.GetNumberOfColumns() ):
      if ( SkillAssessmentLogic.IsHeaderColumn( metricsTable, columnIndex ) ):
        continue
      
      columnName = metricsTable.GetColumnName( columnIndex )
      taskHeaderLabel = qt.QLabel( columnName )
      taskHeaderLabel.setStyleSheet( "QLabel{ qproperty-alignment: AlignCenter }" )
      self.assessmentTable.setCellWidget( 0, taskColumnCount + 2, taskHeaderLabel )
      taskColumnCount = taskColumnCount + 1
        
    # Metric headers
    for rowIndex in range( metricsTable.GetNumberOfRows() ):
      metricHeaderString = metricsTable.GetValueByName( rowIndex, "MetricName" ).ToString()
      
      metricHeaderString = metricHeaderString + " ["
      metricHeaderString = metricHeaderString + metricsTable.GetValueByName( rowIndex, "MetricRoles" ).ToString()
      metricHeaderString = metricHeaderString + "] "
      
      metricHeaderString = metricHeaderString + "("
      metricHeaderString = metricHeaderString +  metricsTable.GetValueByName( rowIndex, "MetricUnit" ).ToString()
      metricHeaderString = metricHeaderString + ")"
      
      metricHeaderLabel = qt.QLabel( metricHeaderString )
      metricHeaderLabel.setStyleSheet( "QLabel{ qproperty-alignment: AlignVCenter }" )
      self.assessmentTable.setCellWidget( rowIndex + 2, 0, metricHeaderLabel )
      
    # Metrics and metric weights
    taskColumnCount = 0
    for columnIndex in range( metricsTable.GetNumberOfColumns() ):
      if ( SkillAssessmentLogic.IsHeaderColumn( metricsTable, columnIndex ) ):
        continue
    
      for rowIndex in range( metricsTable.GetNumberOfRows() ):
        metric = metricsTable.GetValue( rowIndex, columnIndex )
        weight = weightsTable.GetValue( rowIndex, columnIndex )
        
        try:
          metricWeightWidget = self.assessmentTable.cellWidget( rowIndex + 2, taskColumnCount + 2 )
          metricWeightWidget.findChild( qt.QLabel ).setText( metric.ToString() )
          metricWeightWidget.findChild( ctk.ctkSliderWidget ).value = weight.ToDouble()
        except:
          metricWeightWidget = self.createMetricWeightWidget( self.assessmentTable, metric.ToString(), weight.ToDouble(), rowIndex, columnIndex )
          self.assessmentTable.setCellWidget( rowIndex + 2, taskColumnCount + 2, metricWeightWidget )
        
      taskColumnCount = taskColumnCount + 1
      
    # Get the scores nodes    
    metricScoresNode = node.GetNodeReference( "MetricScores" )
    taskScoresNode = node.GetNodeReference( "TaskScores" )
    
    if ( metricScoresNode is None or taskScoresNode is None ):
      logging.info( "SkillAssessmentWidget::updateAssessmentTableWidget: Empty metric scores or task scores node." )
      return
      
    metricScoresTable = metricScoresNode.GetTable()
    taskScoresTable = taskScoresNode.GetTable()
      
    if ( metricScoresTable is None or taskScoresTable is None ):
      logging.info( "SkillAssessmentWidget::updateAssessmentTableWidget: Metric scores or task scores table not specified." )
      return
      
    if ( metricScoresTable.GetNumberOfColumns() != 4
      or metricScoresTable.GetNumberOfRows() != metricsTable.GetNumberOfRows()
      or taskScoresTable.GetNumberOfRows() != 1
      or taskScoresTable.GetNumberOfColumns() != metricsTable.GetNumberOfColumns() ):
      logging.info( "SkillAssessmentWidget::updateAssessmentTableWidget: Metric scores or task scores tables incompatible with metrics table." )
      return
      
    # Metric scores
    for rowIndex in range( metricScoresTable.GetNumberOfRows() ):
      metric = metricScoresTable.GetValueByName( rowIndex, "MetricScore" )
      try:
        metricWeightWidget = self.assessmentTable.cellWidget( rowIndex + 2, self.assessmentTable.columnCount - 1 )
        metricWeightWidget.findChild( qt.QLabel ).setText( metric.ToString() )
      except:
        metricWeightWidget = self.createMetricWeightWidget( self.assessmentTable, metric.ToString(), 1.0, rowIndex, None )
        self.assessmentTable.setCellWidget( rowIndex + 2, self.assessmentTable.columnCount - 1, metricWeightWidget )
      
    # Task scores
    taskColumnCount = 0
    for columnIndex in range( taskScoresTable.GetNumberOfColumns() ):
      if ( SkillAssessmentLogic.IsHeaderColumn( taskScoresTable, columnIndex ) ):
        continue
        
      metric = taskScoresTable.GetValue( 0, columnIndex )
      try:
        metricWeightWidget = self.assessmentTable.cellWidget( self.assessmentTable.rowCount - 1, taskColumnCount + 2 )
        metricWeightWidget.findChild( qt.QLabel ).setText( metric.ToString() )
      except:
        metricWeightWidget = self.createMetricWeightWidget( self.assessmentTable, metric.ToString(), 1.0, None, columnIndex )
        self.assessmentTable.setCellWidget( self.assessmentTable.rowCount - 1, taskColumnCount + 2, metricWeightWidget )
        
      taskColumnCount = taskColumnCount + 1
      
    # Overall score
    overallScore = node.GetAttribute( "OverallScore" )
    overallScoreLabel = qt.QLabel( overallScore )
    overallScoreLabel.setStyleSheet( "QLabel{ qproperty-alignment: AlignCenter }" )
    self.assessmentTable.setCellWidget( self.assessmentTable.rowCount - 1, self.assessmentTable.columnCount - 1, overallScoreLabel )
    
    # Do some strechting to make the table look nice
    self.assessmentTable.horizontalHeader().setResizeMode( qt.QHeaderView.Stretch )
    self.assessmentTable.verticalHeader().setResizeMode( qt.QHeaderView.Stretch )
    
    if ( self.assessmentTable.columnCount > 1 ):
      self.assessmentTable.horizontalHeader().setResizeMode( 0, qt.QHeaderView.ResizeToContents )
      self.assessmentTable.horizontalHeader().setResizeMode( 1, qt.QHeaderView.ResizeToContents )
      self.assessmentTable.horizontalHeader().setResizeMode( self.assessmentTable.columnCount - 2, qt.QHeaderView.ResizeToContents )
    
    if ( self.assessmentTable.rowCount > 1 ):
      self.assessmentTable.verticalHeader().setResizeMode( 0, qt.QHeaderView.ResizeToContents )
      self.assessmentTable.verticalHeader().setResizeMode( 1, qt.QHeaderView.ResizeToContents )
      self.assessmentTable.verticalHeader().setResizeMode( self.assessmentTable.rowCount - 2, qt.QHeaderView.ResizeToContents )
      
  
  
  def createMetricWeightWidget( self, parent, metric, weight, rowIndex = None, columnIndex = None ):
    # Establish the container widget and layout
    metricWeightWidget = slicer.qSlicerWidget( parent )
    metricWeightLayout = qt.QVBoxLayout( metricWeightWidget )
    
    # Add the weight slider
    weightSlider = ctk.ctkSliderWidget( metricWeightWidget )
    weightSlider.maximum = 1
    weightSlider.minimum = 0
    weightSlider.singleStep = 0.01
    weightSlider.pageStep = 0.1
    weightSlider.decimals = 2
    weightSlider.value = weight
    metricWeightLayout.addWidget( weightSlider )
    
    weightSlider.connect( 'valueChanged(double)', partial( self.onWeightSliderChanged, rowIndex, columnIndex ) )
    
    # Add the metric value
    metricLabel = qt.QLabel( metric, metricWeightWidget )
    metricLabel.setStyleSheet( "QLabel{ qproperty-alignment: AlignCenter }" )
    metricWeightLayout.addWidget( metricLabel )
    
    return metricWeightWidget
    
    
  def onWeightSliderChanged( self, rowIndex, columnIndex, weight ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return
    weightsNode = parameterNode.GetNodeReference( "Weights" )
    if ( weightsNode is None ):
      return
    weightsTable = weightsNode.GetTable()
    if ( weightsTable is None ):
      return      
      
    # If the row or column index is none, that means apply to all rows or columns
    if ( rowIndex is None ):
      rowIndex = range( weightsTable.GetNumberOfRows() )
    else:
      rowIndex = [ rowIndex ]
    if ( columnIndex is None ):
      columnIndex = range( weightsTable.GetNumberOfColumns() ) # Ignore header columns
    else:
      columnIndex = [ columnIndex ]
      
    for row in rowIndex:
      for column in columnIndex:
        if ( not SkillAssessmentLogic.IsHeaderColumn( weightsTable, column ) ):
          weightsTable.SetValue( row, column, weight )
        
    SkillAssessmentLogic.Assess( parameterNode )
    self.updateAssessmentTable( parameterNode )

    
  def onParameterNodeChanged( self, parameterNode ):
    if ( parameterNode is None ):
      return
      
    # Set the default values
    parameterNode.SetAttribute( "SkillAssessment", "True" )

    if ( parameterNode.GetAttribute( "AssessmentMethod" ) is None ):
      parameterNode.SetAttribute( "AssessmentMethod", ASSESSMENT_METHOD_ZSCORE )
    if ( parameterNode.GetAttribute( "AggregationMethod" ) is None ):
      parameterNode.SetAttribute( "AggregationMethod", AGGREGATION_METHOD_MEAN )
      
    if ( parameterNode.GetAttribute( "OverallScore" ) is None ):
      parameterNode.SetAttribute( "OverallScore", "0" )

    self.updateWidgetFromParameterNode( parameterNode )

    # Deal with observing the parameter node
    for tag in self.parameterNodeObserverTags:
      parameterNode.RemoveObserver( tag )
    self.parameterNodeObserverTags = []

    self.parameterNodeObserverTags.append( parameterNode.AddObserver( vtk.vtkCommand.ModifiedEvent, self.onParameterNodeModified ) )
    self.parameterNodeObserverTags.append( parameterNode.AddObserver( slicer.vtkMRMLNode.ReferencedNodeModifiedEvent, self.onParameterNodeModified ) )
  
    SkillAssessmentLogic.Assess( parameterNode )
    self.updateAssessmentTable( parameterNode )
    
    
  def onParameterNodeModified( self, parameterNode, eventid ):
    if ( parameterNode is None ):
      return
    
    self.updateWidgetFromParameterNode( parameterNode )
    SkillAssessmentLogic.Assess( parameterNode )
    self.updateAssessmentTable( parameterNode )
    

  def onMetricsChanged( self, metricsNode ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return
    
    parameterNode.SetNodeReferenceID( "Metrics", metricsNode.GetID() )
    SkillAssessmentLogic.Assess( parameterNode )
    self.updateAssessmentTable( parameterNode )
    
    
  def onWeightsChanged( self, weightNode ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return
    
    parameterNode.SetNodeReferenceID( "Weights", weightNode.GetID() )
    SkillAssessmentLogic.Assess( parameterNode )
    self.updateAssessmentTable( parameterNode )
    
    
  def onTrainingSetChanged( self ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return
      
    trainingNodes = self.trainingSetSelector.checkedNodes()
    parameterNode.RemoveNodeReferenceIDs( "Training" )
    for node in trainingNodes:
      parameterNode.AddNodeReferenceID( "Training", node.GetID() )
    
    SkillAssessmentLogic.Assess( parameterNode )
    self.updateAssessmentTable( parameterNode )
    
    
  def onAssessmentMethodChanged( self, text ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return
      
    parameterNode.SetAttribute( "AssessmentMethod", text )
    SkillAssessmentLogic.Assess( parameterNode )
    self.updateAssessmentTable( parameterNode )
    
    
  def onAggregationMethodChanged( self, text ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return
      
    parameterNode.SetAttribute( "AggregationMethod", text )
    SkillAssessmentLogic.Assess( parameterNode )
    self.updateAssessmentTable( parameterNode )
    

  def onAssessButtonClicked( self ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return
  
    SkillAssessmentLogic.Assess( parameterNode )
    self.updateAssessmentTable( parameterNode )
    self.assessmentTable.show()
    
    
  def updateWidgetFromParameterNode( self, parameterNode ):
    if ( parameterNode is None ):
      return
      
    self.metricsSelector.setCurrentNode( parameterNode.GetNodeReference( "Metrics" ) )
    self.weightsSelector.setCurrentNode( parameterNode.GetNodeReference( "Weights" ) )
    
    for nodeIndex in range( parameterNode.GetNumberOfNodeReferences( "Training" ) ):
      trainingNode = parameterNode.GetNthNodeReference( "Training", nodeIndex )
      self.trainingSetSelector.setCheckState( trainingNode, 2 )
      
    assessmentMethodIndex = self.assessmentMethodComboBox.findText( parameterNode.GetAttribute( "AssessmentMethod" ) )
    if ( assessmentMethodIndex >= 0 ):
      self.assessmentMethodComboBox.setCurrentIndex( assessmentMethodIndex )
    
    aggregationMethodIndex = self.aggregationMethodComboBox.findText( parameterNode.GetAttribute( "AggregationMethod" ) )
    if ( aggregationMethodIndex >= 0 ):
      self.aggregationMethodComboBox.setCurrentIndex( aggregationMethodIndex )

    self.resultsLabel.setText( parameterNode.GetAttribute( "OverallScore" ) )
    

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
    pass
    
  
  @staticmethod
  def Assess( parameterNode ):
  
    assessmentMethod = parameterNode.GetAttribute( "AssessmentMethod" )
    aggregationMethod = parameterNode.GetAttribute( "AggregationMethod" )
    if ( assessmentMethod == "" or aggregationMethod == "" ):
      logging.info( "SkillAssessmentLogic::Assess: Assessment or aggregation method improperly specified. Please pick on of the pre-defined options." )
      return 0

    metricsNode = parameterNode.GetNodeReference( "Metrics" )
    if ( metricsNode is None ):
      logging.info( "SkillAssessmentLogic::Assess: Metrics table is empty. Could not assess." )
      return 0
      
    trainingNodes = []
    for i in range( parameterNode.GetNumberOfNodeReferences( "Training" ) ):
      trainingNodes.append( parameterNode.GetNthNodeReference( "Training", i ) )    
    if ( len( trainingNodes ) == 0 ):
      logging.info( "SkillAssessmentLogic::Assess: Training dataset is empty. Could not assess." )
      return 0
      
    weightsNode = parameterNode.GetNodeReference( "Weights" )
    if ( weightsNode is None ):      
      weightsNode = slicer.vtkMRMLTableNode()
      weightsTable = SkillAssessmentLogic.CreateTableFromMetrics( metricsNode, 1 )
      weightsNode.SetAndObserveTable( weightsTable )
      weightsNode.SetName( "Weights" )
      weightsNode.SetScene( slicer.mrmlScene )
      slicer.mrmlScene.AddNode( weightsNode )
      parameterNode.SetNodeReferenceID( "Weights", weightsNode.GetID() )
    
    # TODO: Make this code more modular
    
    # Transform the metric values
    transformedMetricsTable = SkillAssessmentLogic.CreateTableFromMetrics( metricsNode, 0 )
    
    for rowIndex in range( transformedMetricsTable.GetNumberOfRows() ):
      metricName = transformedMetricsTable.GetValueByName( rowIndex, "MetricName" )
      metricRoles = transformedMetricsTable.GetValueByName( rowIndex, "MetricRoles" ) 
      metricUnit = transformedMetricsTable.GetValueByName( rowIndex, "MetricUnit" )
      
      for columnIndex in range( transformedMetricsTable.GetNumberOfColumns() ):
        if ( SkillAssessmentLogic.IsHeaderColumn( transformedMetricsTable, columnIndex ) ):
          continue
        
        columnName = transformedMetricsTable.GetColumnName( columnIndex )
        trainingMetricValues = SkillAssessmentLogic.GetMetricValuesFromNodes( trainingNodes, metricName, metricRoles, metricUnit, columnName )
        testMetricValue = SkillAssessmentLogic.GetMetricValueFromNode( metricsNode, metricName, metricRoles, metricUnit, columnName )

        transformedMetricValue = SkillAssessmentLogic.GetTransformedMetricValue( testMetricValue, trainingMetricValues, assessmentMethod )

        transformedMetricsTable.SetValue( rowIndex, columnIndex, transformedMetricValue )        
    
    # Compute the metric scores
    metricScoresTable = SkillAssessmentLogic.CreateMetricScoresTableFromMetrics( metricsNode, 0 )
    
    for rowIndex in range( metricScoresTable.GetNumberOfRows() ):
      metricName = metricScoresTable.GetValueByName( rowIndex, "MetricName" )
      metricRoles = metricScoresTable.GetValueByName( rowIndex, "MetricRoles" ) 
      metricUnit = metricScoresTable.GetValueByName( rowIndex, "MetricUnit" )
      
      metrics = []
      weights = []
      for columnIndex in range( transformedMetricsTable.GetNumberOfColumns() ):
        if ( SkillAssessmentLogic.IsHeaderColumn( transformedMetricsTable, columnIndex ) ):
          continue
        
        columnName = transformedMetricsTable.GetColumnName( columnIndex )
        metrics.append( SkillAssessmentLogic.GetMetricValueFromTable( transformedMetricsTable, metricName, metricRoles, metricUnit, columnName ) )
        weights.append( SkillAssessmentLogic.GetMetricValueFromNode( weightsNode, metricName, metricRoles, metricUnit, columnName ) )

      aggregatedMetricValue = SkillAssessmentLogic.GetAggregatedMetricValue( metrics, weights, aggregationMethod )
      metricScoresTable.SetValueByName( rowIndex, "MetricScore", aggregatedMetricValue )
      
    metricScoresNode = parameterNode.GetNodeReference( "MetricScores" )
    if ( metricScoresNode is None ):
      metricScoresNode = slicer.vtkMRMLTableNode()
      metricScoresNode.SetName( "MetricScores" )
      metricScoresNode.HideFromEditorsOn()
      metricScoresNode.SetScene( slicer.mrmlScene )
      slicer.mrmlScene.AddNode( metricScoresNode )
      parameterNode.SetNodeReferenceID( "MetricScores", metricScoresNode.GetID() )
    metricScoresNode.SetAndObserveTable( metricScoresTable )
      
      
    # Compute the task scores
    taskScoresTable = SkillAssessmentLogic.CreateTaskScoresTableFromMetrics( metricsNode, 0 )
    
    for columnIndex in range( taskScoresTable.GetNumberOfColumns() ):
      if ( SkillAssessmentLogic.IsHeaderColumn( taskScoresTable, columnIndex ) ):
        continue
      columnName = taskScoresTable.GetColumnName( columnIndex )

      metrics = []
      weights = []       
      for rowIndex in range( transformedMetricsTable.GetNumberOfRows() ):
        metrics.append( transformedMetricsTable.GetValueByName( rowIndex, columnName ).ToDouble() )
        weights.append( weightsNode.GetTable().GetValueByName( rowIndex, columnName ).ToDouble() )
        
      aggregatedMetricValue = SkillAssessmentLogic.GetAggregatedMetricValue( metrics, weights, aggregationMethod )
      taskScoresTable.SetValueByName( 0, columnName, aggregatedMetricValue )
      
    taskScoresNode = parameterNode.GetNodeReference( "TaskScores" )
    if ( taskScoresNode is None ):
      taskScoresNode = slicer.vtkMRMLTableNode()
      taskScoresNode.SetName( "TaskScores" )
      taskScoresNode.HideFromEditorsOn()
      taskScoresNode.SetScene( slicer.mrmlScene )
      slicer.mrmlScene.AddNode( taskScoresNode )
      parameterNode.SetNodeReferenceID( "TaskScores", taskScoresNode.GetID() )
    taskScoresNode.SetAndObserveTable( taskScoresTable )
      
      
    # Compute the overall score
    metrics = []
    weights = []
    for rowIndex in range( transformedMetricsTable.GetNumberOfRows() ):
      for columnIndex in range( transformedMetricsTable.GetNumberOfColumns() ):
        if ( SkillAssessmentLogic.IsHeaderColumn( transformedMetricsTable, columnIndex ) ):
          continue
          
        columnName = transformedMetricsTable.GetColumnName( columnIndex )
        metrics.append( transformedMetricsTable.GetValueByName( rowIndex, columnName ).ToDouble() )
        weights.append( weightsNode.GetTable().GetValueByName( rowIndex, columnName ).ToDouble() )

    overallScore = SkillAssessmentLogic.GetAggregatedMetricValue( metrics, weights, aggregationMethod )
    parameterNode.SetAttribute( "OverallScore", str( overallScore ) )
      
    
  @staticmethod
  def CreateTaskScoresTableFromMetrics( metricsNode, value ):
    table = vtk.vtkTable()
    
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

    return table
    
 
  @staticmethod
  def CreateMetricScoresTableFromMetrics( metricsNode, value ):
    table = vtk.vtkTable()

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
      
    return table


  @staticmethod
  def CreateTableFromMetrics( metricsNode, value ):
    table = vtk.vtkTable()
    table.DeepCopy( metricsNode.GetTable() ) # Make the weight table the same size
    for columnIndex in range( table.GetNumberOfColumns() ):
      columnName = table.GetColumnName( columnIndex )
      if ( columnName == "MetricName" or columnName == "MetricRoles" or columnName == "MetricUnit" ):
        continue
      for rowIndex in range( table.GetNumberOfRows() ):
        table.SetValue( rowIndex, columnIndex, value ) # Set the default value to 1
        
    return table

    
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

    return SkillAssessmentLogic.GetMetricValueFromTable( metricsNode.GetTable(), metricName, metricRoles, metricUnit, taskName ) 
    
    
  @staticmethod
  def GetMetricValueFromTable( metricsTable, metricName, metricRoles, metricUnit, taskName ):
    if ( metricsTable is None ):
      logging.info( "SkillAssessmentLogic::GetMetricValueFromTable: Table of metrics is empty." )
      return
    
    for rowIndex in range( metricsTable.GetNumberOfRows() ):
      nameMatch = ( metricsTable.GetValueByName( rowIndex, "MetricName" ) == metricName )
      rolesMatch = ( metricsTable.GetValueByName( rowIndex, "MetricRoles" ) == metricRoles )
      unitMatch = ( metricsTable.GetValueByName( rowIndex, "MetricUnit" ) == metricUnit ) # Unit should always match if the names match
      if ( nameMatch and rolesMatch and unitMatch ):
        return metricsTable.GetValueByName( rowIndex, taskName ).ToDouble()
        
    logging.info( "SkillAssessmentLogic::GetMetricValueFromTable: Could not find metric in the table." )
    return
    

  @staticmethod
  def GetAggregatedMetricValue( metricValues, weights, method ):
    if ( len( metricValues ) != len( weights ) ):
      logging.info( "SkillAssessmentLogic::GetAggregatedMetricValue: Metric values and weights do not correspond." )
      return 0
      
    if ( method == AGGREGATION_METHOD_MEAN ):
      return SkillAssessmentLogic.GetWeightedMean( metricValues, weights )
    if ( method == AGGREGATION_METHOD_MEDIAN ):
      return SkillAssessmentLogic.GetWeightedMedian( metricValues, weights )
    if ( method == AGGREGATION_METHOD_MAXIMUM ):
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

    if ( weightSum == 0.0 ):
      return 0.0

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
    if ( method == ASSESSMENT_METHOD_RAW ):
      return SkillAssessmentLogic.GetRaw( testMetric, trainingMetrics )
    if ( method == ASSESSMENT_METHOD_PERCENTILE ):
      return SkillAssessmentLogic.GetPercentile( testMetric, trainingMetrics )
    if ( method == ASSESSMENT_METHOD_ZSCORE ):
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

    if ( trainingStd == 0.0 ):
      return 0.0

    return ( testMetric - trainingMean ) / trainingStd

    
  # Convenience method for checking if a column in a metrics table is a header columns
  @staticmethod
  def IsHeaderColumn( metricsTable, index ):
    columnName = metricsTable.GetColumnName( index )
    if ( columnName == "MetricName"
      or columnName == "MetricRoles"
      or columnName == "MetricUnit" ):
      return True
      
    return False
    


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
