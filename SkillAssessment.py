import os
import unittest
import vtk, qt, ctk, slicer
import numpy
from slicer.ScriptedLoadableModule import *
import logging
from functools import partial
import operator
import AssessmentMethods

#
# SkillAssessment
#
ASSESSMENT_METHOD_LINEARCOMBINATION = "LinearCombination"
ASSESSMENT_METHOD_NEARESTNEIGHBOR = "NearestNeighbor"
ASSESSMENT_METHOD_FUZZY = "Fuzzy"


class SkillAssessment( ScriptedLoadableModule ):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__( self, parent ):
    ScriptedLoadableModule.__init__( self, parent )
    self.parent.title = "Skill Assessment"
    self.parent.categories = [ "Perk Tutor" ]
    self.parent.dependencies = []
    self.parent.contributors = [ "Matthew S. Holden (Perk Lab; Queen's University)" ]
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
    assessmentFormLayout.addRow( "Metrics ", self.metricsSelector )
    
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
    assessmentFormLayout.addRow( "Weights ", self.weightsSelector )
    
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
    assessmentFormLayout.addRow( "Training data ", self.trainingSetSelector )
    
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
    # Feedback Area
    #
    feedbackCollapsibleButton = ctk.ctkCollapsibleButton()
    feedbackCollapsibleButton.text = "Feedback"
    feedbackCollapsibleButton.collapsed = False
    self.layout.addWidget(feedbackCollapsibleButton)

    # Layout within the dummy collapsible button
    feedbackFormLayout = qt.QVBoxLayout( feedbackCollapsibleButton )

    # Strengths label
    self.strengthsLabel = qt.QLabel( "Strengths" )
    feedbackFormLayout.addWidget( self.strengthsLabel )

    # Strengths text box
    self.strengthsTextBox = qt.QTextBrowser()
    self.strengthsTextBox.toolTip = "A list of the top strengths during the performance."
    feedbackFormLayout.addWidget( self.strengthsTextBox )

    # Weaknesses label
    self.weaknessesLabel = qt.QLabel( "Weaknesses" )
    feedbackFormLayout.addWidget( self.weaknessesLabel )

    # Strengths text box
    self.weaknessesTextBox = qt.QTextBrowser()
    self.weaknessesTextBox.toolTip = "A list of the top weaknesses during the performance."
    feedbackFormLayout.addWidget( self.weaknessesTextBox )

    
    #
    # Options Area
    #
    optionsCollapsibleButton = ctk.ctkCollapsibleButton()
    optionsCollapsibleButton.text = "Options"
    optionsCollapsibleButton.collapsed = True
    self.layout.addWidget( optionsCollapsibleButton )

    # Layout within the dummy collapsible button
    self.optionsFormLayout = qt.QFormLayout( optionsCollapsibleButton )
    
    
    #
    # Skill assessment module node selector
    #    
    self.parameterNodeSelector = slicer.qMRMLNodeComboBox()
    self.parameterNodeSelector.nodeTypes = [ "vtkMRMLScriptedModuleNode" ]
    self.parameterNodeSelector.addEnabled = True
    self.parameterNodeSelector.removeEnabled = True
    self.parameterNodeSelector.showHidden = True
    self.parameterNodeSelector.showChildNodeTypes = False
    self.parameterNodeSelector.addAttribute( "vtkMRMLScriptedModuleNode", "AssessmentMethod" )
    self.parameterNodeSelector.baseName = "SkillAssessment"
    self.parameterNodeSelector.setMRMLScene( slicer.mrmlScene )
    self.parameterNodeSelector.setToolTip( "Select the module parameters node." )
    self.optionsFormLayout.addRow( "Parameter node ", self.parameterNodeSelector )
    
    # Assessment method selection
    self.assessmentMethodGroupBox = qt.QGroupBox( "Assessment Method" )
    self.assessmentMethodLayout = qt.QVBoxLayout( self.assessmentMethodGroupBox )
    self.assessmentMethodGroupBox.setLayout( self.assessmentMethodLayout )
    self.optionsFormLayout.addRow( self.assessmentMethodGroupBox )
    
    self.linearCombinationRadioButton = qt.QRadioButton( self.assessmentMethodGroupBox )
    self.linearCombinationRadioButton.setText( "Linear combination" )
    self.assessmentMethodLayout.addWidget( self.linearCombinationRadioButton )
    
    self.nearestNeighborRadioButton = qt.QRadioButton( self.assessmentMethodGroupBox )
    self.nearestNeighborRadioButton.setText( "Nearest neighbor" )
    self.assessmentMethodLayout.addWidget( self.nearestNeighborRadioButton )
    
    self.fuzzyRadioButton = qt.QRadioButton( self.assessmentMethodGroupBox )
    self.fuzzyRadioButton.setText( "Fuzzy" )
    self.assessmentMethodLayout.addWidget( self.fuzzyRadioButton )
    

    #
    # Parameters area
    #   
    self.parametersGroupBox = qt.QGroupBox( "Parameters" )
    self.parametersLayout = qt.QVBoxLayout( self.parametersGroupBox )
    self.parametersGroupBox.setLayout( self.parametersLayout )
    self.optionsFormLayout.addRow( self.parametersGroupBox )
    
    self.linearCombinationParametersFrame = AssessmentMethods.LinearCombinationParametersWidget( self.parametersGroupBox )
    self.linearCombinationParametersFrame.hide()
    self.parametersLayout.addWidget( self.linearCombinationParametersFrame )
    
    self.nearestNeighborParametersFrame = AssessmentMethods.NearestNeighborParametersWidget( self.parametersGroupBox )
    self.nearestNeighborParametersFrame.hide()
    self.parametersLayout.addWidget( self.nearestNeighborParametersFrame )

    
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
    
    self.linearCombinationRadioButton.connect( 'toggled(bool)', partial( self.onAssessmentMethodRadioButtonToggled, ASSESSMENT_METHOD_LINEARCOMBINATION, self.linearCombinationParametersFrame ) )
    self.nearestNeighborRadioButton.connect( 'toggled(bool)', partial( self.onAssessmentMethodRadioButtonToggled, ASSESSMENT_METHOD_NEARESTNEIGHBOR, self.nearestNeighborParametersFrame ) )
    # self.fuzzyRadioButton.connect( 'toggled(bool)', partial( self.onAssessmentMethodRadioButtonToggled, ASSESSMENT_METHOD_FUZZY, self.fuzzyParametersFrame ) )
    
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

    # More options
    self.addOptionsMenuToAssessmentTable()
    
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
      

  def addOptionsMenuToAssessmentTable( self ):
    moreOptionsWidget = slicer.qSlicerWidget( self.assessmentTable )
    moreOptionsLayout = qt.QVBoxLayout( moreOptionsWidget )
    moreOptionsLayout.setAlignment( 0x0002 ) # TODO: This is right alignment, but the enum is not accessible in Python. Is there a better way to do this?

    # Set up the more options button
    moreOptionsButton = qt.QToolButton( moreOptionsWidget )
    moreOptionsButton.setText( "..." )
    moreOptionsButton.setPopupMode( qt.QToolButton.InstantPopup )
    moreOptionsButton.setSizePolicy( qt.QSizePolicy.Fixed, qt.QSizePolicy.Preferred )
    moreOptionsLayout.addWidget( moreOptionsButton )

    # Create the menu
    moreOptionsMenu = qt.QMenu( "More options", moreOptionsButton )
    moreOptionsMenu.setObjectName( "MoreOptionsMenu" )

    # Create all the actions
    metricsWeightsVisibilityAction = qt.QAction( qt.QIcon( ":/Icons/Small/SlicerVisible.png" ), "Metrics Weights Visibility", moreOptionsMenu )
    scoreWeightsVisibilityAction = qt.QAction( qt.QIcon( ":/Icons/Small/SlicerVisible.png" ), "Score Weights Visibility", moreOptionsMenu )

    # Add the actions to the menu and menu to the button
    moreOptionsMenu.addAction( metricsWeightsVisibilityAction )
    metricsWeightsVisibilityAction.connect( 'triggered()', self.toggleMetricsWeightsVisibility )
    moreOptionsMenu.addAction( scoreWeightsVisibilityAction )
    scoreWeightsVisibilityAction.connect( 'triggered()', self.toggleScoreWeightsVisibility )

    moreOptionsButton.setMenu( moreOptionsMenu )

    self.assessmentTable.setCellWidget( 0, self.assessmentTable.columnCount - 1, moreOptionsWidget )


  def toggleMetricsWeightsVisibility( self ):
    # Iterate over all non-last rows/columns and toggle visibility on all of the slider widgets
    for rowIndex in range( self.assessmentTable.rowCount - 1 ):
      for columnIndex in range( self.assessmentTable.columnCount - 1 ):
        try:
          metricWeightWidget = self.assessmentTable.cellWidget( rowIndex, columnIndex )
          descendentSliderWidget = metricWeightWidget.findChild( ctk.ctkSliderWidget )
          descendentSliderWidget.setVisible( not descendentSliderWidget.isVisible() )
        except:
          continue


  def toggleScoreWeightsVisibility( self ):
    # Iterate over last column and last row and toogle visibility of all those sliders
    for rowIndex in range( self.assessmentTable.rowCount ):
      try:
        metricWeightWidget = self.assessmentTable.cellWidget( rowIndex, self.assessmentTable.columnCount - 1 )
        descendentSliderWidget = metricWeightWidget.findChild( ctk.ctkSliderWidget )
        descendentSliderWidget.setVisible( not descendentSliderWidget.isVisible() )
      except:
        continue

    for columnIndex in range( self.assessmentTable.columnCount ):
      try:
        metricWeightWidget = self.assessmentTable.cellWidget( self.assessmentTable.rowCount - 1, columnIndex )
        descendentSliderWidget = metricWeightWidget.findChild( ctk.ctkSliderWidget )
        descendentSliderWidget.setVisible( not descendentSliderWidget.isVisible() )
      except:
        continue

  
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
    parameterNode.SetAttribute( "AssessmentMethod", "LinearCombination" )
      
    if ( parameterNode.GetAttribute( "OverallScore" ) is None ):
      parameterNode.SetAttribute( "OverallScore", "0" )

    self.updateWidgetFromParameterNode( parameterNode )
    
    self.linearCombinationParametersFrame.setParameterNode( parameterNode )
    self.nearestNeighborParametersFrame.setParameterNode( parameterNode )    

    # Deal with observing the parameter node
    for tag in self.parameterNodeObserverTags:
      parameterNode.RemoveObserver( tag )
    self.parameterNodeObserverTags = []

    self.parameterNodeObserverTags.append( parameterNode.AddObserver( vtk.vtkCommand.ModifiedEvent, self.onParameterNodeModified ) )
    self.parameterNodeObserverTags.append( parameterNode.AddObserver( slicer.vtkMRMLNode.ReferencedNodeModifiedEvent, self.onParameterNodeModified ) )
  
    self.updateWidgetFromParameterNode( parameterNode )
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
    
    
  def onAssessmentMethodRadioButtonToggled( self, assessmentMethod, frame, toggled ):    
    frame.setVisible( toggled )
    
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return

    if ( toggled ):
      parameterNode.SetAttribute( "AssessmentMethod", assessmentMethod )  
    

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
      
    assessmentMethod = parameterNode.GetAttribute( "AssessmentMethod" )
    if ( assessmentMethod == ASSESSMENT_METHOD_LINEARCOMBINATION ):
      self.linearCombinationRadioButton.setChecked( True )
    if ( assessmentMethod == ASSESSMENT_METHOD_NEARESTNEIGHBOR ):
      self.nearestNeighborRadioButton.setChecked( True )
    if ( assessmentMethod == ASSESSMENT_METHOD_FUZZY ):
      self.fuzzyRadioButton.setChecked( True )

    self.resultsLabel.setText( parameterNode.GetAttribute( "OverallScore" ) )

    self.strengthsTextBox.setText( parameterNode.GetAttribute( "Strengths" ) )
    self.weaknessesTextBox.setText( parameterNode.GetAttribute( "Weaknesses" ) )
    

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
    if ( parameterNode is None ):
      logging.info( "SkillAssessmentLogic::Assess: Parameter node is None." )
      return 0
  
    assessmentMethod = parameterNode.GetAttribute( "AssessmentMethod" )
    if ( assessmentMethod == "" ):
      logging.info( "SkillAssessmentLogic::Assess: Assessment method improperly specified. Please pick one of the pre-defined options." )
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
      weightsTable = SkillAssessmentLogic.CreateTableFromMetricsNode( metricsNode, 1 )
      weightsNode.SetAndObserveTable( weightsTable )
      weightsNode.SetName( "Weights" )
      weightsNode.SetScene( slicer.mrmlScene )
      slicer.mrmlScene.AddNode( weightsNode )
      parameterNode.SetNodeReferenceID( "Weights", weightsNode.GetID() )
      
    convertedMetricsNode = parameterNode.GetNodeReference( "ConvertedMetrics" )
    if ( convertedMetricsNode is None ):
      convertedMetricsNode = slicer.vtkMRMLTableNode()
      convertedMetricsNode.SetName( "ConvertedMetrics" )
      convertedMetricsNode.HideFromEditorsOn()
      convertedMetricsNode.SetScene( slicer.mrmlScene )
      slicer.mrmlScene.AddNode( convertedMetricsNode )
      parameterNode.SetNodeReferenceID( "ConvertedMetrics", convertedMetricsNode.GetID() )
      
    metricScoresNode = parameterNode.GetNodeReference( "MetricScores" )
    if ( metricScoresNode is None ):
      metricScoresNode = slicer.vtkMRMLTableNode()
      metricScoresNode.SetName( "MetricScores" )
      metricScoresNode.HideFromEditorsOn()
      metricScoresNode.SetScene( slicer.mrmlScene )
      slicer.mrmlScene.AddNode( metricScoresNode )
      parameterNode.SetNodeReferenceID( "MetricScores", metricScoresNode.GetID() )
      
    taskScoresNode = parameterNode.GetNodeReference( "TaskScores" )
    if ( taskScoresNode is None ):
      taskScoresNode = slicer.vtkMRMLTableNode()
      taskScoresNode.SetName( "TaskScores" )
      taskScoresNode.HideFromEditorsOn()
      taskScoresNode.SetScene( slicer.mrmlScene )
      slicer.mrmlScene.AddNode( taskScoresNode )
      parameterNode.SetNodeReferenceID( "TaskScores", taskScoresNode.GetID() )
      
    # Grab the skill labels (even though they are not necessary for all methods)
    skillLabels = SkillAssessmentLogic.GetSkillLabels( trainingNodes )
      
    # Different assessment methods
    if ( assessmentMethod == ASSESSMENT_METHOD_LINEARCOMBINATION ):
      Assessor = AssessmentMethods.LinearCombinationAssessment
    if ( assessmentMethod == ASSESSMENT_METHOD_NEARESTNEIGHBOR ):
      Assessor = AssessmentMethods.NearestNeighborAssessment
      
    print Assessor

    # Compute the metric/task pair skill values
    convertedMetricsTable = SkillAssessmentLogic.CreateTableFromMetricsNode( metricsNode, 0 )
    
    for rowIndex in range( convertedMetricsTable.GetNumberOfRows() ):
      metricName = convertedMetricsTable.GetValueByName( rowIndex, "MetricName" )
      metricRoles = convertedMetricsTable.GetValueByName( rowIndex, "MetricRoles" ) 
      metricUnit = convertedMetricsTable.GetValueByName( rowIndex, "MetricUnit" )
      
      for columnIndex in range( convertedMetricsTable.GetNumberOfColumns() ):
        if ( SkillAssessmentLogic.IsHeaderColumn( convertedMetricsTable, columnIndex ) ):
          continue        
        columnName = convertedMetricsTable.GetColumnName( columnIndex )
        
        trainingMetricValues = SkillAssessmentLogic.GetMetricValuesFromNodes( trainingNodes, metricName, metricRoles, metricUnit, columnName )
        testMetricValue = SkillAssessmentLogic.GetMetricValuesFromNode( metricsNode, metricName, metricRoles, metricUnit, columnName )
        weights = SkillAssessmentLogic.GetMetricValuesFromNode( weightsNode, metricName, metricRoles, metricUnit, columnName )

        convertedMetricValue = Assessor.ComputeSkill( parameterNode, testMetricValue, trainingMetricValues, weights, skillLabels )
        convertedMetricsTable.SetValue( rowIndex, columnIndex, convertedMetricValue )

    convertedMetricsNode = parameterNode.GetNodeReference( "ConvertedMetrics" )
    convertedMetricsNode.SetAndObserveTable( convertedMetricsTable )    
    
    # Compute the metric scores
    metricScoresTable = SkillAssessmentLogic.CreateMetricScoresTableFromMetricsNode( metricsNode, 0 )
    
    for rowIndex in range( metricScoresTable.GetNumberOfRows() ):
      metricName = metricScoresTable.GetValueByName( rowIndex, "MetricName" )
      metricRoles = metricScoresTable.GetValueByName( rowIndex, "MetricRoles" ) 
      metricUnit = metricScoresTable.GetValueByName( rowIndex, "MetricUnit" )

      trainingMetricValues = SkillAssessmentLogic.GetMetricValuesFromNodes( trainingNodes, metricName, metricRoles, metricUnit, None )
      testMetricValue = SkillAssessmentLogic.GetMetricValuesFromNode( metricsNode, metricName, metricRoles, metricUnit, None )
      weights = SkillAssessmentLogic.GetMetricValuesFromNode( weightsNode, metricName, metricRoles, metricUnit, None )

      currMetricScore = Assessor.ComputeSkill( parameterNode, testMetricValue, trainingMetricValues, weights, skillLabels )
      metricScoresTable.SetValueByName( rowIndex, "MetricScore", currMetricScore )
      
    metricScoresNode = parameterNode.GetNodeReference( "MetricScores" )
    metricScoresNode.SetAndObserveTable( metricScoresTable )
      
    # Compute the task scores
    taskScoresTable = SkillAssessmentLogic.CreateTaskScoresTableFromMetricsNode( metricsNode, 0 )
    
    for columnIndex in range( taskScoresTable.GetNumberOfColumns() ):
      if ( SkillAssessmentLogic.IsHeaderColumn( taskScoresTable, columnIndex ) ):
        continue
      columnName = taskScoresTable.GetColumnName( columnIndex )
      
      trainingMetricValues = SkillAssessmentLogic.GetMetricValuesFromNodes( trainingNodes, None, None, None, columnName )
      testMetricValue = SkillAssessmentLogic.GetMetricValuesFromNode( metricsNode, None, None, None, columnName )
      weights = SkillAssessmentLogic.GetMetricValuesFromNode( weightsNode, None, None, None, columnName )

      currTaskScore = Assessor.ComputeSkill( parameterNode, testMetricValue, trainingMetricValues, weights, skillLabels )
      taskScoresTable.SetValueByName( 0, columnName, currTaskScore )
      
    taskScoresNode = parameterNode.GetNodeReference( "TaskScores" )
    taskScoresNode.SetAndObserveTable( taskScoresTable )
      
    # Compute the overall score
    trainingMetricValues = SkillAssessmentLogic.GetMetricValuesFromNodes( trainingNodes, None, None, None, None )
    testMetricValue = SkillAssessmentLogic.GetMetricValuesFromNode( metricsNode, None, None, None, None )
    weights = SkillAssessmentLogic.GetMetricValuesFromNode( weightsNode, None, None, None, None )

    overallScore = Assessor.ComputeSkill( parameterNode, testMetricValue, trainingMetricValues, weights, skillLabels )
    parameterNode.SetAttribute( "OverallScore", str( overallScore ) )

    # Produce the feedback strings
    criticalValue = Assessor.GetCriticalValue( parameterNode, skillLabels )
    strengthsString = SkillAssessmentLogic.GetFeedbackString( convertedMetricsTable, criticalValue, operator.lt, "good", 3 )
    parameterNode.SetAttribute( "Strengths", strengthsString )
    weaknessesString = SkillAssessmentLogic.GetFeedbackString( convertedMetricsTable, criticalValue, operator.gt, "too high", 3 )
    parameterNode.SetAttribute( "Weaknesses", weaknessesString )
    

  @staticmethod
  def GetFeedbackString( convertedMetricsTable, criticalValue, operator, noteString, maxNumberOfFeedbacks = 3 ):
    if ( convertedMetricsTable is None ):
      logging.info( "SkillAssessmentLogic::GetFeedbackString: Converted metrics table is empty. Could not provide feedback." )
      return ""
  
    # Get a list of row/column indices sorted by the metric value
    feedbackElements = []
    for rowIndex in range( convertedMetricsTable.GetNumberOfRows() ):
      for columnIndex in range( convertedMetricsTable.GetNumberOfColumns() ):
        if ( SkillAssessmentLogic.IsHeaderColumn( convertedMetricsTable, columnIndex ) ):
          continue

        currMetricValue = convertedMetricsTable.GetValue( rowIndex, columnIndex ).ToDouble()
        if ( operator( criticalValue, currMetricValue ) ):
          continue # The metric did not satisfy the cutoff

        newElementInserted = False
        for feedbackIndex in range( len( feedbackElements ) ):
          feedbackValue = convertedMetricsTable.GetValue( feedbackElements[ feedbackIndex ][ 0 ], feedbackElements[ feedbackIndex ][ 1 ] ).ToDouble()
          if ( operator( currMetricValue, feedbackValue ) ):
            feedbackElements.insert( feedbackIndex, [ rowIndex, columnIndex ] )
            newElementInserted = True
            break
        if ( not newElementInserted ):
          feedbackElements.append( [ rowIndex, columnIndex ] )

    # Reconstruct the list of row/column indices into feedback strings
    feedbackString = ""
    for feedbackIndex in range( min( maxNumberOfFeedbacks, len( feedbackElements ) ) ):
      feedbackString = feedbackString + convertedMetricsTable.GetValueByName( feedbackElements[ feedbackIndex ][ 0 ], "MetricName" ).ToString()

      feedbackString = feedbackString + " ["
      feedbackString = feedbackString + convertedMetricsTable.GetValueByName( feedbackElements[ feedbackIndex ][ 0 ], "MetricRoles" ).ToString()
      feedbackString = feedbackString + "] "

      feedbackString = feedbackString + "("
      feedbackString = feedbackString + convertedMetricsTable.GetValueByName( feedbackElements[ feedbackIndex ][ 0 ], "MetricUnit" ).ToString()
      feedbackString = feedbackString + ")"

      feedbackString = feedbackString + " during "
      feedbackString = feedbackString + convertedMetricsTable.GetColumnName( feedbackElements[ feedbackIndex ][ 1 ] )
      feedbackString = feedbackString + " is "
      feedbackString = feedbackString + noteString
      feedbackString = feedbackString + "."
      feedbackString = feedbackString + "\n"

    return feedbackString


    
  @staticmethod
  def CreateTaskScoresTableFromMetricsNode( metricsNode, value ):
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
  def CreateMetricScoresTableFromMetricsNode( metricsNode, value ):
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
  def CreateTableFromMetricsNode( metricsNode, value ):
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
      currMetricValue = SkillAssessmentLogic.GetMetricValuesFromNode( currMetricNode, metricName, metricRoles, metricUnit, taskName )
      metricValues.append( currMetricValue )
      
    return metricValues
      

  @staticmethod
  def GetMetricValuesFromNode( metricsNode, metricName, metricRoles, metricUnit, taskName ):
    if ( metricsNode is None or metricsNode.GetTable() is None ):
      logging.info( "SkillAssessmentLogic::GetMetricValueFromNode: Table of metrics is empty." )
      return

    return SkillAssessmentLogic.GetMetricValuesFromTable( metricsNode.GetTable(), metricName, metricRoles, metricUnit, taskName ) 
    
    
  @staticmethod
  def GetMetricValuesFromTable( metricsTable, metricName, metricRoles, metricUnit, taskName ):
    if ( metricsTable is None ):
      logging.info( "SkillAssessmentLogic::GetMetricValueFromTable: Table of metrics is empty." )
      return
    
    metricValues = []
    for rowIndex in range( metricsTable.GetNumberOfRows() ):
      nameMatch = ( metricsTable.GetValueByName( rowIndex, "MetricName" ) == metricName or metricName is None )
      rolesMatch = ( metricsTable.GetValueByName( rowIndex, "MetricRoles" ) == metricRoles or metricRoles is None )
      unitMatch = ( metricsTable.GetValueByName( rowIndex, "MetricUnit" ) == metricUnit or metricUnit is None ) # Unit should always match if the names match
      if ( not nameMatch or not rolesMatch or not unitMatch ):
        continue
      for columnIndex in range( metricsTable.GetNumberOfColumns() ):
        if ( SkillAssessmentLogic.IsHeaderColumn( metricsTable, columnIndex ) ):
          continue
        taskMatch = ( metricsTable.GetColumnName( columnIndex ) == taskName or taskName is None )
        if ( not taskMatch ):
          continue
        
        metricValues.append( metricsTable.GetValue( rowIndex, columnIndex ).ToDouble() ) 
        
    return metricValues
    
    
  # Convenience method for checking if a column in a metrics table is a header columns
  @staticmethod
  def IsHeaderColumn( metricsTable, index ):
    columnName = metricsTable.GetColumnName( index )
    if ( columnName == "MetricName"
      or columnName == "MetricRoles"
      or columnName == "MetricUnit" ):
      return True
      
    return False
    
    
  @staticmethod
  def GetSkillLabels( trainingNodes ):
    skillLabels = []
    for currTrainingNode in trainingNodes:
      currLabel = currTrainingNode.GetAttribute( "Skill" )
      try:
        skillLabels.append( float( currLabel ) )
      except:
        logging.info( "SkillAssessmentLogic::GetSkillLabels: Training node " + currTrainingNode.GetName() + " has no skill label. Using zero as presumed skill." )
        skillLabels.append( 0 )
        
    return skillLabels
    
    


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

    self.delayDisplay("No test available")
