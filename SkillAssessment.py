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
ASSESSMENT_METHOD_REGRESSION = "Regression"

OUTPUT_PRECISION = 3


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
    self.optionsFormLayout.addRow( "Weights ", self.weightsSelector )
    
    #
    # Ignore metric values
    #
    self.ignoreMetricValuesCheckBox = qt.QCheckBox()
    self.ignoreMetricValuesCheckBox.setCheckState( 2 ) # This is checked
    self.ignoreMetricValuesCheckBox.setText( "Ignore metric values" )
    self.ignoreMetricValuesCheckBox.setToolTip( "Ignore the overall metric values when task-spoecific metric values are available." )
    self.optionsFormLayout.addRow( self.ignoreMetricValuesCheckBox )
    
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
    
    self.regressionRadioButton = qt.QRadioButton( self.assessmentMethodGroupBox )
    self.regressionRadioButton.setText( "Regression" )
    self.assessmentMethodLayout.addWidget( self.regressionRadioButton )
    

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
    
    self.fuzzyParametersFrame = AssessmentMethods.FuzzyParametersWidget( self.parametersGroupBox )
    self.fuzzyParametersFrame.hide()
    self.parametersLayout.addWidget( self.fuzzyParametersFrame )
    
    self.regressionParametersFrame = AssessmentMethods.RegressionParametersWidget( self.parametersGroupBox )
    self.regressionParametersFrame.hide()
    self.parametersLayout.addWidget( self.regressionParametersFrame )
    
    
        
    #
    # Translation Area
    #
    translationCollapsibleButton = ctk.ctkCollapsibleButton()
    translationCollapsibleButton.text = "Translation"
    translationCollapsibleButton.collapsed = True
    self.layout.addWidget( translationCollapsibleButton )
    
    # Layout within collapsible buttion
    self.translationVBoxLayout = qt.QVBoxLayout( translationCollapsibleButton )
    
    # Translation table selector
    self.translationTableSelector = slicer.qMRMLNodeComboBox()
    self.translationTableSelector.nodeTypes = [ "vtkMRMLTableNode" ]
    self.translationTableSelector.addEnabled = True
    self.translationTableSelector.removeEnabled = True
    self.translationTableSelector.showHidden = True
    self.translationTableSelector.showChildNodeTypes = False
    self.translationTableSelector.addAttribute( "vtkMRMLTableNode", "Translation" )
    self.translationTableSelector.baseName = "TranslationTable"
    self.translationTableSelector.setMRMLScene( slicer.mrmlScene )
    self.translationTableSelector.setToolTip( "Select the table node with translations for the metrics." )
    self.translationVBoxLayout.addWidget( self.translationTableSelector )
    
    # Translation table
    self.translationTableView = slicer.qMRMLTableView()
    self.translationVBoxLayout.addWidget( self.translationTableView )
    
    # Options for adding/removing translations
    translationManagementHBoxLayout = qt.QHBoxLayout()
    self.translationVBoxLayout.addLayout( translationManagementHBoxLayout )
    
    self.translationPopulateButton = qt.QPushButton( "Populate" )
    self.translationPopulateButton.toolTip = "Add all metrics from the currently selected metrics node."
    translationManagementHBoxLayout.addWidget( self.translationPopulateButton )
    
    self.translationAddButton = qt.QPushButton( "Add translation" )
    self.translationAddButton.toolTip = "Add a metric to be translated."
    translationManagementHBoxLayout.addWidget( self.translationAddButton )
    
    self.translationDeleteButton = qt.QPushButton( "Remove translation" )
    self.translationDeleteButton.toolTip = "Add a metric to be translated."
    translationManagementHBoxLayout.addWidget( self.translationDeleteButton )
    

    
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
    self.metricsSelector.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onMetricsChanged )    
    self.trainingSetSelector.connect( 'checkedNodesChanged()', self.onTrainingSetChanged )
    
    self.parameterNodeSelector.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onParameterNodeChanged )
    self.weightsSelector.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onWeightsChanged )
    
    self.ignoreMetricValuesCheckBox.connect( 'toggled(bool)', self.onIgnoreMetricValuesChanged )
    
    self.linearCombinationRadioButton.connect( 'toggled(bool)', partial( self.onAssessmentMethodRadioButtonToggled, ASSESSMENT_METHOD_LINEARCOMBINATION ) )
    self.nearestNeighborRadioButton.connect( 'toggled(bool)', partial( self.onAssessmentMethodRadioButtonToggled, ASSESSMENT_METHOD_NEARESTNEIGHBOR ) )
    self.fuzzyRadioButton.connect( 'toggled(bool)', partial( self.onAssessmentMethodRadioButtonToggled, ASSESSMENT_METHOD_FUZZY ) )
    self.regressionRadioButton.connect( 'toggled(bool)', partial( self.onAssessmentMethodRadioButtonToggled, ASSESSMENT_METHOD_REGRESSION ) )
    
    self.assessButton.connect( 'clicked(bool)', self.onAssessButtonClicked )
    
    self.translationTableSelector.connect( 'nodeAddedByUser(vtkMRMLNode*)', self.onTranslationTableAdded )
    self.translationTableSelector.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onTranslationTableChanged )
    self.translationPopulateButton.connect( 'clicked(bool)', self.populateTranslationTableFromMetrics )
    self.translationAddButton.connect( 'clicked(bool)', self.addTranslation )
    self.translationDeleteButton.connect( 'clicked(bool)', self.deleteTranslation )

    # Create a default parameter node
    if ( self.parameterNodeSelector.currentNode() is None ):
      self.parameterNodeSelector.addNode( "vtkMRMLScriptedModuleNode" )

    # Add vertical spacer
    self.layout.addStretch(1)


  def cleanup( self ):
    pass
    
    
  def updateAssessmentTable( self, parameterNode ):
    if ( parameterNode is None ):
      return
    
    metricsNode = parameterNode.GetNodeReference( "Metrics" )
    weightsNode = parameterNode.GetNodeReference( "Weights" )      
    
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
      
    ignoreMetricValue = parameterNode.GetAttribute( "IgnoreMetricValue" )  
    translationNode = parameterNode.GetNodeReference( "TranslationTable" )
    translationTable = None
    if ( translationNode is not None ):
      translationTable = translationNode.GetTable()
      
    numMetrics = metricsTable.GetNumberOfRows()
    numTasks = SkillAssessmentLogic.GetNumberOfNonHeaderColumns( metricsTable, ignoreMetricValue ) # Ignore "MetricName", "MetricRoles", "MetricUnit" and optionally "MetricValue"
    
    self.assessmentTable.setRowCount( numMetrics + 4 )
    self.assessmentTable.setColumnCount( numTasks + 4 )
    
    # Task headers
    allTaskNames = SkillAssessmentLogic.GetAllTaskNames( metricsTable, ignoreMetricValue )
    for taskIndex in range( len( allTaskNames ) ):
      taskHeaderLabel = qt.QLabel( allTaskNames[ taskIndex ] )
      taskHeaderLabel.setStyleSheet( "QLabel{ qproperty-alignment: AlignCenter }" )
      self.assessmentTable.setCellWidget( 0, taskIndex + 2, taskHeaderLabel )
        
    # Metric headers
    allMetricTuples = SkillAssessmentLogic.GetAllMetricTuples( metricsTable )
    for metricIndex in range( len( allMetricTuples ) ):
      translatedMetricString = SkillAssessmentLogic.GetTranslatedMetricTaskString( translationTable, allMetricTuples[ metricIndex ], None )      
      metricHeaderLabel = qt.QLabel( translatedMetricString )
      metricHeaderLabel.setStyleSheet( "QLabel{ qproperty-alignment: AlignVCenter }" )
      self.assessmentTable.setCellWidget( metricIndex + 2, 0, metricHeaderLabel )
      
    # Metrics and metric weights
    taskColumnCount = 0
    for taskIndex in range( len( allTaskNames ) ):
      for metricIndex in range( len( allMetricTuples ) ):
        metricTaskValue = SkillAssessmentLogic.GetValueByMetricTask( metricsTable, allMetricTuples[ metricIndex ], allTaskNames[ taskIndex ] )
        metricTaskWeight = SkillAssessmentLogic.GetValueByMetricTask( weightsTable, allMetricTuples[ metricIndex ], allTaskNames[ taskIndex ] )
        
        try:
          metricWeightWidget = self.assessmentTable.cellWidget( metricIndex + 2, taskIndex + 2 )
          metricWeightWidget.findChild( qt.QLabel ).setText( metricTaskValue.ToString() )
          metricWeightWidget.findChild( ctk.ctkSliderWidget ).value = metricTaskWeight.ToDouble()
        except:
          metricWeightWidget = self.createMetricWeightWidget( self.assessmentTable, metricTaskValue.ToString(), metricTaskWeight.ToDouble(), allMetricTuples[ metricIndex ], allTaskNames[ taskIndex ] )
          self.assessmentTable.setCellWidget( metricIndex + 2, taskIndex + 2, metricWeightWidget )
      
    # Get the scores nodes    
    metricScoresNode = parameterNode.GetNodeReference( "MetricScores" )
    taskScoresNode = parameterNode.GetNodeReference( "TaskScores" )
    
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
    for metricIndex in range( len( allMetricTuples ) ):
      metricScoreValue = SkillAssessmentLogic.GetValueByMetricTask( metricScoresTable, allMetricTuples[ metricIndex ], "MetricScore" )
      
      try:
        metricWeightWidget = self.assessmentTable.cellWidget( metricIndex + 2, self.assessmentTable.columnCount - 1 )
        metricWeightWidget.findChild( qt.QLabel ).setText( metricScoreValue.ToString() )
      except:
        metricWeightWidget = self.createMetricWeightWidget( self.assessmentTable, metricScoreValue.ToString(), 1.0, allMetricTuples[ metricIndex ], None )
        self.assessmentTable.setCellWidget( metricIndex + 2, self.assessmentTable.columnCount - 1, metricWeightWidget )
      
    # Task scores
    for taskIndex in range( len( allTaskNames ) ):
      taskScoreValue = SkillAssessmentLogic.GetValueByRowIndexTask( taskScoresTable, 0, allTaskNames[ taskIndex ] )
      
      try:
        metricWeightWidget = self.assessmentTable.cellWidget( self.assessmentTable.rowCount - 1, taskIndex + 2 )
        metricWeightWidget.findChild( qt.QLabel ).setText( taskScoreValue.ToString() )
      except:
        metricWeightWidget = self.createMetricWeightWidget( self.assessmentTable, taskScoreValue.ToString(), 1.0, None, allTaskNames[ taskIndex ] )
        self.assessmentTable.setCellWidget( self.assessmentTable.rowCount - 1, taskIndex + 2, metricWeightWidget )

      
    # Overall score
    overallScore = parameterNode.GetAttribute( "OverallScore" )
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

  
  def createMetricWeightWidget( self, parent, score, weight, metric, task ):
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
    
    weightSlider.connect( 'valueChanged(double)', partial( self.onWeightSliderChanged, metric, task ) )
    
    # Add the metric-task value
    metricLabel = qt.QLabel( score, metricWeightWidget )
    metricLabel.setStyleSheet( "QLabel{ qproperty-alignment: AlignCenter }" )
    metricWeightLayout.addWidget( metricLabel )
    
    return metricWeightWidget
    
    
  def onWeightSliderChanged( self, metric, task, weight ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return
    weightsNode = parameterNode.GetNodeReference( "Weights" )
    if ( weightsNode is None ):
      return
    weightsTable = weightsNode.GetTable()
    if ( weightsTable is None ):
      return
      
    ignoreMetricValue = parameterNode.GetAttribute( "IgnoreMetricValue" )
      
    # If the row or column index is none, that means apply to all rows or columns
    if ( metric is None ):
      setMetrics = SkillAssessmentLogic.GetAllMetricTuples( weightsTable )
    else:
      setMetrics = [ metric ]
    if ( task is None ):
      setTasks = SkillAssessmentLogic.GetAllTaskNames( weightsTable, ignoreMetricValue )
    else:
      setTasks = [ task ]
      
    for currMetric in setMetrics:
      for currTask in setTasks:
        SkillAssessmentLogic.SetValueByMetricTask( weightsTable, currMetric, currTask, weight )
        
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
    self.fuzzyParametersFrame.setParameterNode( parameterNode )
    self.regressionParametersFrame.setParameterNode( parameterNode )

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

    if ( metricsNode is None ):
      parameterNode.RemoveNodeReferenceIDs( "Metrics" )
    else:
      parameterNode.SetNodeReferenceID( "Metrics", metricsNode.GetID() ) # Automatically triggers update
    
    
  def onWeightsChanged( self, weightsNode ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return

    if ( weightsNode is None ): 
      parameterNode.RemoveNodeReferenceIDs( "Weights" )
    else:
      parameterNode.SetNodeReferenceID( "Weights", weightsNode.GetID() ) # Automatically triggers update
    
    
  def onTrainingSetChanged( self ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return

    modifyState = parameterNode.StartModify()

    trainingNodes = self.trainingSetSelector.checkedNodes()
    parameterNode.RemoveNodeReferenceIDs( "Training" )
    for node in trainingNodes:
      parameterNode.AddNodeReferenceID( "Training", node.GetID() )

    parameterNode.EndModify( modifyState ) # Automatically triggers update
    
    
  def onIgnoreMetricValuesChanged( self, toggled ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return
      
    self.assessmentTable.clear()
    if ( toggled ):
      parameterNode.SetAttribute( "IgnoreMetricValue", "" )
    else:
      parameterNode.SetAttribute( "IgnoreMetricValue", "False" )
    
    
  def onAssessmentMethodRadioButtonToggled( self, assessmentMethod, toggled ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return

    self.assessmentTable.clear()
    if ( toggled ):
      parameterNode.SetAttribute( "AssessmentMethod", assessmentMethod )  
    

  def onAssessButtonClicked( self ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return
  
    SkillAssessmentLogic.Assess( parameterNode )
    self.updateWidgetFromParameterNode( parameterNode )
    self.assessmentTable.clear()
    self.updateAssessmentTable( parameterNode )
    self.assessmentTable.show()
    
    
  def onTranslationTableAdded( self, translationTableNode ):
    if ( translationTableNode is None ):
      return
    translationTable = translationTableNode.GetTable()
    if ( translationTable is None ):
      return
  
    translationTable.Initialize()
    translationTableColumnNames = [ "MetricName", "MetricRoles", "MetricUnit", "Translation" ]
    for columnName in translationTableColumnNames:
      column = vtk.vtkStringArray()
      column.SetName( columnName )
      translationTable.AddColumn( column )

    
  def onTranslationTableChanged( self, translationTableNode ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return

    self.translationTableView.setMRMLTableNode( None )
    if ( translationTableNode is None ):
      parameterNode.RemoveNodeReferenceIDs( "TranslationTable" )
      return
      
    # Verify that the translation table has the necessary columns    
    parameterNode.SetNodeReferenceID( "TranslationTable", translationTableNode.GetID() ) # Automatically triggers update
    self.translationTableView.setMRMLTableNode( translationTableNode )
    translationTableNode.SetUseColumnNameAsColumnHeader( True )
    
          
  def addTranslation( self ):
    self.translationTableView.insertRow()

    
  def deleteTranslation( self ):
    self.translationTableView.deleteRow()
    
    
  def populateTranslationTableFromMetrics( self ):
    parameterNode = self.parameterNodeSelector.currentNode()
    if ( parameterNode is None ):
      return
    
    metricsNode = parameterNode.GetNodeReference( "Metrics" )
    translationTableNode = parameterNode.GetNodeReference( "TranslationTable" )
    if ( metricsNode is None or translationTableNode is None ):
      logging.info( "SkillAssessmentWidget::populateTranslationTableFromMetrics: Metrics node and/or translation node are None." )
      return
    
    SkillAssessmentLogic.AddTranslationsFromMetrics( metricsNode.GetTable(), translationTableNode.GetTable() )
    translationTableNode.Modified() # Cue table refresh
    
    
  def updateWidgetFromParameterNode( self, parameterNode ):
    if ( parameterNode is None ):
      return

    blockState = self.metricsSelector.blockSignals( True )
    self.metricsSelector.setCurrentNode( parameterNode.GetNodeReference( "Metrics" ) )
    self.metricsSelector.blockSignals( blockState )

    blockState = self.weightsSelector.blockSignals( True )
    self.weightsSelector.setCurrentNode( parameterNode.GetNodeReference( "Weights" ) )
    self.weightsSelector.blockSignals( blockState )
    
    self.assessButton.setToolTip( parameterNode.GetAttribute( "AssessmentDescription" ) )

    blockState = self.trainingSetSelector.blockSignals( True )
    for node in self.trainingSetSelector.checkedNodes():
      if ( not parameterNode.HasNodeReferenceID( "Training", node.GetID() ) ):
        self.trainingSetSelector.setCheckState( node, 0 )
    for node in self.trainingSetSelector.uncheckedNodes():
      if ( parameterNode.HasNodeReferenceID( "Training", node.GetID() ) ):
        self.trainingSetSelector.setCheckState( node, 2 )
    self.trainingSetSelector.blockSignals( blockState )
    
    ignoreMetricValues = parameterNode.GetAttribute( "IgnoreMetricValue" )
    if ( ignoreMetricValues == None or ignoreMetricValues == "" ):
      self.ignoreMetricValuesCheckBox.setCheckState( 2 )
    else:
      self.ignoreMetricValuesCheckBox.setCheckState( 0 )
      
    assessmentMethod = parameterNode.GetAttribute( "AssessmentMethod" )
    linearCombinationBlockState = self.linearCombinationRadioButton.blockSignals( True )
    nearestNeighborBlockState = self.nearestNeighborRadioButton.blockSignals( True )
    fuzzyBlockState = self.fuzzyRadioButton.blockSignals( True )
    regressionBlockState = self.regressionRadioButton.blockSignals( True )
    self.linearCombinationParametersFrame.hide()
    self.nearestNeighborParametersFrame.hide()
    self.fuzzyParametersFrame.hide()
    self.regressionParametersFrame.hide()
    if ( assessmentMethod == ASSESSMENT_METHOD_LINEARCOMBINATION ):
      self.linearCombinationRadioButton.setChecked( True )
      self.linearCombinationParametersFrame.show()
    if ( assessmentMethod == ASSESSMENT_METHOD_NEARESTNEIGHBOR ):
      self.nearestNeighborRadioButton.setChecked( True )
      self.nearestNeighborParametersFrame.show()
    if ( assessmentMethod == ASSESSMENT_METHOD_FUZZY ):
      self.fuzzyRadioButton.setChecked( True )
      self.fuzzyParametersFrame.show()
    if ( assessmentMethod == ASSESSMENT_METHOD_REGRESSION ):
      self.regressionRadioButton.setChecked( True )
      self.regressionParametersFrame.show()
    self.linearCombinationRadioButton.blockSignals( linearCombinationBlockState)
    self.nearestNeighborRadioButton.blockSignals( nearestNeighborBlockState )
    self.fuzzyRadioButton.blockSignals( fuzzyBlockState )
    self.regressionRadioButton.blockSignals( regressionBlockState )    

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
      
    ignoreMetricValue = parameterNode.GetAttribute( "IgnoreMetricValue" )
    
    metricsNode = parameterNode.GetNodeReference( "Metrics" )
    if ( metricsNode is None ):
      logging.info( "SkillAssessmentLogic::Assess: Metrics table is empty. Could not assess." )
      return 0
    metricsTable = metricsNode.GetTable()
      
    trainingNodes = []
    for i in range( parameterNode.GetNumberOfNodeReferences( "Training" ) ):
      trainingNodes.append( parameterNode.GetNthNodeReference( "Training", i ) )    
    if ( len( trainingNodes ) == 0 ):
      logging.info( "SkillAssessmentLogic::Assess: Training dataset is empty. Could not assess." )
      return 0
      
    translationTable = None
    translationTableNode = parameterNode.GetNodeReference( "TranslationTable" )
    if ( translationTableNode is not None ):
      translationTable = translationTableNode.GetTable()      
      
    modifyState = parameterNode.StartModify()
      
    weightsNode = parameterNode.GetNodeReference( "Weights" )
    if ( weightsNode is None ):      
      weightsNode = slicer.vtkMRMLTableNode()
      weightsTable = SkillAssessmentLogic.CreateBlankMetricsTable( metricsNode, 1 )
      weightsNode.SetAndObserveTable( weightsTable )
      weightsNode.SetName( "Weights" )
      weightsNode.SetScene( slicer.mrmlScene )
      slicer.mrmlScene.AddNode( weightsNode )
      parameterNode.SetNodeReferenceID( "Weights", weightsNode.GetID() )
      
    metricTaskScoresNode = parameterNode.GetNodeReference( "MetricTaskScores" )
    if ( metricTaskScoresNode is None ):
      metricTaskScoresNode = slicer.vtkMRMLTableNode()
      metricTaskScoresNode.SetName( "MetricTaskScores" )
      metricTaskScoresNode.HideFromEditorsOn()
      metricTaskScoresNode.SetScene( slicer.mrmlScene )
      slicer.mrmlScene.AddNode( metricTaskScoresNode )
      parameterNode.SetNodeReferenceID( "MetricTaskScores", metricTaskScoresNode.GetID() )
      
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
    nameLabels, skillLabels = SkillAssessmentLogic.GetNameSkillLabels( trainingNodes )
      
    # Different assessment methods
    if ( assessmentMethod == ASSESSMENT_METHOD_LINEARCOMBINATION ):
      Assessor = AssessmentMethods.LinearCombinationAssessment
    if ( assessmentMethod == ASSESSMENT_METHOD_NEARESTNEIGHBOR ):
      Assessor = AssessmentMethods.NearestNeighborAssessment
    if ( assessmentMethod == ASSESSMENT_METHOD_FUZZY ):
      Assessor = AssessmentMethods.FuzzyAssessment
    if ( assessmentMethod == ASSESSMENT_METHOD_REGRESSION ):
      Assessor = AssessmentMethods.RegressionAssessment
      
    # Get all metrics and tasks
    allMetricTuples = SkillAssessmentLogic.GetAllMetricTuples( metricsTable )
    allTaskNames = SkillAssessmentLogic.GetAllTaskNames( metricsTable, ignoreMetricValue )

    #
    # Compute the metric/task pair skill values
    #
    metricTaskScoresTable = SkillAssessmentLogic.CreateBlankMetricsTable( metricsNode, 0 )
    
    for metricTuple in allMetricTuples:
      for taskName in allTaskNames:       
        metricTaskList = SkillAssessmentLogic.GetMetricTaskList( metricsTable, metricTuple, taskName, ignoreMetricValue )
        metricTaskStringList = SkillAssessmentLogic.TranslateMetricTaskList( translationTable, metricTaskList )
        
        trainingRecords = SkillAssessmentLogic.GetMetricTaskRecordsFromNodes( trainingNodes, metricTaskList )
        testRecord = SkillAssessmentLogic.GetMetricTaskRecordFromNode( metricsNode, metricTaskList )
        weightsRecord = SkillAssessmentLogic.GetMetricTaskRecordFromNode( weightsNode, metricTaskList )

        currMetricTaskScore, _ = Assessor.ComputeSkill( parameterNode, testRecord, trainingRecords, weightsRecord, metricTaskStringList, nameLabels, skillLabels )
        SkillAssessmentLogic.SetValueByMetricTask( metricTaskScoresTable, metricTuple, taskName, round( currMetricTaskScore, OUTPUT_PRECISION ) )

    metricTaskScoresNode = parameterNode.GetNodeReference( "MetricTaskScores" )
    metricTaskScoresNode.SetAndObserveTable( metricTaskScoresTable )    
    
    #
    # Compute the metric scores
    #
    metricScoresTable = SkillAssessmentLogic.CreateMetricScoresTable( metricsNode, 0 )
    
    for metricTuple in allMetricTuples:
      metricTaskList = SkillAssessmentLogic.GetMetricTaskList( metricsTable, metricTuple, None, ignoreMetricValue )
      metricTaskStringList = SkillAssessmentLogic.TranslateMetricTaskList( translationTable, metricTaskList )

      trainingRecords = SkillAssessmentLogic.GetMetricTaskRecordsFromNodes( trainingNodes, metricTaskList )
      testRecord = SkillAssessmentLogic.GetMetricTaskRecordFromNode( metricsNode, metricTaskList )
      weightsRecord = SkillAssessmentLogic.GetMetricTaskRecordFromNode( weightsNode, metricTaskList )

      currMetricScore, _ = Assessor.ComputeSkill( parameterNode, testRecord, trainingRecords, weightsRecord, metricTaskStringList, nameLabels, skillLabels )
      SkillAssessmentLogic.SetValueByMetricTask( metricScoresTable, metricTuple, "MetricScore", round( currMetricScore, OUTPUT_PRECISION ) )
      
    metricScoresNode = parameterNode.GetNodeReference( "MetricScores" )
    metricScoresNode.SetAndObserveTable( metricScoresTable )
    
    #    
    # Compute the task scores
    #
    taskScoresTable = SkillAssessmentLogic.CreateTaskScoresTable( metricsNode, 0 )
    
    for taskName in allTaskNames:      
      metricTaskList = SkillAssessmentLogic.GetMetricTaskList( metricsTable, None, taskName, ignoreMetricValue )
      metricTaskStringList = SkillAssessmentLogic.TranslateMetricTaskList( translationTable, metricTaskList )
      
      trainingRecords = SkillAssessmentLogic.GetMetricTaskRecordsFromNodes( trainingNodes, metricTaskList )
      testRecord = SkillAssessmentLogic.GetMetricTaskRecordFromNode( metricsNode, metricTaskList )
      weightsRecord = SkillAssessmentLogic.GetMetricTaskRecordFromNode( weightsNode, metricTaskList )

      currTaskScore, _ = Assessor.ComputeSkill( parameterNode, testRecord, trainingRecords, weightsRecord, metricTaskStringList, nameLabels, skillLabels )
      SkillAssessmentLogic.SetValueByRowIndexTask( taskScoresTable, 0, taskName, round( currTaskScore, OUTPUT_PRECISION ) )
      
    taskScoresNode = parameterNode.GetNodeReference( "TaskScores" )
    taskScoresNode.SetAndObserveTable( taskScoresTable )
    
    #    
    # Compute the overall score
    #
    metricTaskList = SkillAssessmentLogic.GetMetricTaskList( metricsTable, None, None, ignoreMetricValue )
    metricTaskStringList = SkillAssessmentLogic.TranslateMetricTaskList( translationTable, metricTaskList )
    
    trainingRecords = SkillAssessmentLogic.GetMetricTaskRecordsFromNodes( trainingNodes, metricTaskList )
    testRecord = SkillAssessmentLogic.GetMetricTaskRecordFromNode( metricsNode, metricTaskList )
    weightsRecord = SkillAssessmentLogic.GetMetricTaskRecordFromNode( weightsNode, metricTaskList )

    metricTaskStringList = SkillAssessmentLogic.TranslateMetricTaskList( translationTable, metricTaskList )
    overallScore, description = Assessor.ComputeSkill( parameterNode, testRecord, trainingRecords, weightsRecord, metricTaskStringList, nameLabels, skillLabels )
    parameterNode.SetAttribute( "OverallScore", str( round( overallScore, OUTPUT_PRECISION ) ) )
    parameterNode.SetAttribute( "AssessmentDescription", description )

    #
    # Produce the feedback strings
    #
    criticalValue = Assessor.GetCriticalValue( parameterNode, skillLabels )
    strengthsString = SkillAssessmentLogic.GetFeedbackString( metricTaskScoresTable, translationTable, criticalValue, False, "good", ignoreMetricValue, 3 )
    parameterNode.SetAttribute( "Strengths", strengthsString )
    weaknessesString = SkillAssessmentLogic.GetFeedbackString( metricTaskScoresTable, translationTable, criticalValue, True, "poor", ignoreMetricValue, 3 )
    parameterNode.SetAttribute( "Weaknesses", weaknessesString )

    parameterNode.EndModify( modifyState )
    

  @staticmethod
  def GetFeedbackString( metricTaskScoresTable, translationTable, criticalValue, descendingOrder, noteString, ignoreMetricValue, maxNumberOfFeedbacks = 3 ):
    if ( metricTaskScoresTable is None ):
      logging.info( "SkillAssessmentLogic::GetFeedbackString: Metric-task scores table is empty. Could not provide feedback." )
      return ""
  
    # Create a record from the metricTaskScores
    metricTaskList = SkillAssessmentLogic.GetMetricTaskList( metricTaskScoresTable, None, None, ignoreMetricValue )
    metricTaskRecord = SkillAssessmentLogic.GetMetricTaskRecord( metricTaskScoresTable, metricTaskList )
    
    # Sort based on size of the metricTaskValue
    metricTaskRecordNameSorted = sorted( zip( metricTaskRecord, metricTaskList ), reverse = descendingOrder )
    
    maxNumberOfFeedbacks = min( maxNumberOfFeedbacks, len( metricTaskRecordNameSorted ) )
    feedbackString = ""
    for feedbackIndex in range( maxNumberOfFeedbacks ):
      currMetricTaskScore = metricTaskRecordNameSorted[ feedbackIndex ][ 0 ]
      if ( ( currMetricTaskScore < criticalValue ) == descendingOrder ): # That is, the score is not extreme enough for feedback
        break
        
      currMetricTuple = metricTaskRecordNameSorted[ feedbackIndex ][ 1 ][ 0 ]
      currTaskName = metricTaskRecordNameSorted[ feedbackIndex ][ 1 ][ 1 ]
      
      feedbackString = feedbackString + SkillAssessmentLogic.GetTranslatedMetricTaskString( translationTable, currMetricTuple, currTaskName )

      feedbackString = feedbackString + " is "
      feedbackString = feedbackString + noteString
      feedbackString = feedbackString + "."
      feedbackString = feedbackString + "\n"

    return feedbackString

    
  @staticmethod
  def CreateTaskScoresTable( metricsNode, value ):
    table = vtk.vtkTable()
    
    # Constructive approach    
    metricNameColumn = vtk.vtkStringArray()
    metricNameColumn.SetName( "MetricName" )
    metricNameColumn.InsertNextValue( "TaskScore" )
    table.AddColumn( metricNameColumn )
    
    metricRolesColumn = vtk.vtkStringArray()
    metricRolesColumn.SetName( "MetricRoles" )
    metricRolesColumn.InsertNextValue( "" )
    table.AddColumn( metricRolesColumn )
    
    metricUnitColumn = vtk.vtkStringArray()
    metricUnitColumn.SetName( "MetricUnit" )
    metricUnitColumn.InsertNextValue( "" )
    table.AddColumn( metricUnitColumn )
    
    metricsTable = metricsNode.GetTable()
    allTaskNames = SkillAssessmentLogic.GetAllTaskNames( metricsTable, False )
    for taskName in allTaskNames:
      newColumn = vtk.vtkStringArray()
      newColumn.SetName( taskName )
      newColumn.InsertNextValue( str( value ) )
      table.AddColumn( newColumn )
    
    return table
    
 
  @staticmethod
  def CreateMetricScoresTable( metricsNode, value ):
    table = vtk.vtkTable()

    # Constructive approach
    metricsTable = metricsNode.GetTable()
    allTaskNames = SkillAssessmentLogic.GetAllTaskNames( metricsTable, False )
    for columnIndex in range( metricsTable.GetNumberOfColumns() ):
      # Iterate over only the MetricName, MetricRoles, MetricUnit columns
      if ( not SkillAssessmentLogic.IsHeaderColumn( metricsTable, columnIndex, False ) ):
        continue

      newColumn = vtk.vtkStringArray()
      newColumn.SetName( metricsTable.GetColumnName( columnIndex ) )
      newColumn.SetNumberOfValues( metricsTable.GetNumberOfRows() )
      for rowIndex in range( metricsTable.GetNumberOfRows() ):
        newColumn.SetValue( rowIndex, metricsTable.GetValue( rowIndex, columnIndex ).ToString() ) # Setting the metric name/roles/unit
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
  def CreateBlankMetricsTable( metricsNode, value ):
    table = vtk.vtkTable()
    table.DeepCopy( metricsNode.GetTable() ) # Make the weight table the same size
    for columnIndex in range( table.GetNumberOfColumns() ):
      columnName = table.GetColumnName( columnIndex )
      if ( columnName == "MetricName" or columnName == "MetricRoles" or columnName == "MetricUnit" ):
        continue
      for rowIndex in range( table.GetNumberOfRows() ):
        table.SetValue( rowIndex, columnIndex, value ) # Set the default value
        
    return table

    
  @staticmethod
  def GetMetricTaskList( metricsTable, metricTuple, taskName, ignoreMetricValue ):
    # Note: The outputted record is a list of all the metric/task combination we want for analysis
    if ( metricsTable is None ):
      logging.info( "SkillAssessmentLogic::GetMetricTaskList: Table of metrics is empty." )
      return []
      
    # If any of the inputs are None, that means we want everything
    # That is, if metricName is None, we want all metrics
    # Likewise, if taskName is None, we want all tasks
    if ( metricTuple is None ):
      searchMetrics = SkillAssessmentLogic.GetAllMetricTuples( metricsTable )
    else:
      searchMetrics = [ metricTuple ]
    if ( taskName is None ):
      searchTasks = SkillAssessmentLogic.GetAllTaskNames( metricsTable, ignoreMetricValue )
    else:
      searchTasks = [ taskName ]
    
    metricTaskList = []
    for currMetric in searchMetrics:
      for currTask in searchTasks:
        metricTaskList.append( ( currMetric, currTask ) )
        
    return metricTaskList
    
    
  @staticmethod
  def TranslateMetricTaskList( translationTable, metricTaskList ):
    metricTaskStringList = []
    for currMetricTask in metricTaskList:
      currMetricTaskString = SkillAssessmentLogic.GetTranslatedMetricTaskString( translationTable, currMetricTask[ 0 ], currMetricTask[ 1 ] )
      metricTaskStringList.append( currMetricTaskString )
      
    return metricTaskStringList

 
  @staticmethod
  def GetMetricTaskRecordFromNode( metricsNode, metricTaskList ):
    if ( metricsNode is None ):
      return None
      
    return SkillAssessmentLogic.GetMetricTaskRecord( metricsNode.GetTable(), metricTaskList )
    
 
  @staticmethod
  def GetMetricTaskRecordsFromNodes( metricsNodes, metricTaskList ):
    metricTaskRecords = []
    
    for currMetricsNode in metricsNodes:
      if ( currMetricsNode is None ):
        continue
      currMetricsTable = currMetricsNode.GetTable()
      if ( currMetricsTable is None ):
        continue
      currMetricTaskRecord = SkillAssessmentLogic.GetMetricTaskRecord( currMetricsTable, metricTaskList )
      metricTaskRecords.append( currMetricTaskRecord )

    return metricTaskRecords

    
  @staticmethod
  def GetMetricTaskRecord( metricsTable, metricTaskList ):
    # Note: The outputted record is necessarily ordered in the same way as the metricTaskList
    if ( metricsTable is None ):
      logging.info( "SkillAssessmentLogic::GetMetricTaskRecord: Table of metrics is empty." )
      return
    
    metricTaskRecord = []
    for currMetricTaskName in metricTaskList:
      currMetricTuple = currMetricTaskName[ 0 ]
      currTaskName = currMetricTaskName[ 1 ]
      currMetricTaskValue = SkillAssessmentLogic.GetValueByMetricTask( metricsTable, currMetricTuple, currTaskName )
      metricTaskRecord.append( currMetricTaskValue.ToDouble() )
        
    return metricTaskRecord
    
    
  @staticmethod
  def AddTranslationsFromMetricsTable( metricsTable, translationTable ):
    if ( metricsTable is None or translationTable is None ):
      logging.info( "SkillAssessmentLogic::AddTranslationsFromMetrics: Metrics table and/or translation table is None." )
      return
      
    for rowIndex in range( metricsTable.GetNumberOfRows() ):
      metricTuple = SkillAssessmentLogic.GetMetricByRowIndex( metricsTable, rowIndex )
      rowIndex = SkillAssessmentLogic.GetRowIndexByMetric( translationTable, metricTuple )
      if ( rowIndex is None ):
        newRowIndex = translationTable.InsertNextBlankRow()
        translationTable.SetValueByName( newRowIndex, "MetricName", metricTuple[ 0 ] )
        translationTable.SetValueByName( newRowIndex, "MetricRoles", metricTuple[ 1 ] )
        translationTable.SetValueByName( newRowIndex, "MetricUnit", metricTuple[ 2 ] )
        
        
  @staticmethod
  def GetTranslatedMetricTaskString( translationTable, metricTuple, taskName ):
    # Try to translate the metric
    metricString = SkillAssessmentLogic.GetValueByMetricTask( translationTable, metricTuple, "Translation" )    
    # If we cannot translate the metric, fall back to the regular name in full
    if ( metricString is None or metricString == "" ):
      metricString = SkillAssessmentLogic.GetMetricString( metricTuple )
    
    taskString = SkillAssessmentLogic.GetTaskString( taskName )
    metricTaskString = SkillAssessmentLogic.GetMetricTaskString( metricString, taskString )    
    
    return metricTaskString
    
    
  @staticmethod
  def GetNameSkillLabels( trainingNodes ):
    nameLabels = []
    skillLabels = []
    for currTrainingNode in trainingNodes:
      nameLabels.append( currTrainingNode.GetName() )
      currSkill = currTrainingNode.GetAttribute( "Skill" )
      try:
        skillLabels.append( float( currSkill ) )
      except:
        logging.info( "SkillAssessmentLogic::GetSkillLabels: Training node " + currTrainingNode.GetName() + " has no skill label. Using zero as presumed skill." )
        skillLabels.append( 0 )
        
    return nameLabels, skillLabels
    
    
  # Various convenience methods for working with metrics tables
  @staticmethod
  def IsHeaderColumn( metricsTable, columnIndex, overallMetricValueIsHeader ):
    columnName = metricsTable.GetColumnName( columnIndex )
    if ( columnName == "MetricName"
      or columnName == "MetricRoles"
      or columnName == "MetricUnit" ):
      return True
    if ( columnName == "MetricValue"
      and ( overallMetricValueIsHeader == None or overallMetricValueIsHeader == "" or overallMetricValueIsHeader == True ) ):
      return True
      
    return False
    
    
  @staticmethod
  def GetNumberOfNonHeaderColumns( metricsTable, overallMetricValueIsHeader ):
    nonHeaderColumns = 0
    for columnIndex in range( metricsTable.GetNumberOfColumns() ):
      isHeaderColumn = SkillAssessmentLogic.IsHeaderColumn( metricsTable, columnIndex, overallMetricValueIsHeader )
      if ( not isHeaderColumn ):
        nonHeaderColumns = nonHeaderColumns + 1

    return nonHeaderColumns
    
    
  @staticmethod
  def GetMetricByRowIndex( metricsTable, rowIndex ):
    if ( metricsTable is None ):
      return ( None, None, None )

    metricName = metricsTable.GetValueByName( rowIndex, "MetricName" )
    metricRoles = metricsTable.GetValueByName( rowIndex, "MetricRoles" )
    metricUnit = metricsTable.GetValueByName( rowIndex, "MetricUnit" )

    return ( metricName, metricRoles, metricUnit )
      
      
  @staticmethod
  def GetRowIndexByMetric( metricsTable, metricTuple ):
    if ( metricsTable is None ):
      return None
       
    for rowIndex in range( metricsTable.GetNumberOfRows() ):
      currMetricTuple = SkillAssessmentLogic.GetMetricByRowIndex( metricsTable, rowIndex )
      if ( currMetricTuple == metricTuple ):
        return rowIndex
          
    return None
    
    
  @staticmethod
  def GetTaskByColumIndex( metricsTable, columnIndex, overallMetricValueIsHeader ):
    if ( metricsTable is None ):
      return None
    isHeaderColumn = SkillAssessmentLogic.IsHeaderColumn( metricsTable, columnIndex, overallMetricValueIsHeader )
    if ( isHeaderColumn ):
      return None
        
    return metricsTable.GetColumnName( columnIndex )
      
      
  @staticmethod
  def GetColumnIndexByTask( metricsTable, taskName ):
    if ( metricsTable is None ):
      return None
        
    for columnIndex in range( metricsTable.GetNumberOfColumns() ):
      currTaskName = metricsTable.GetColumnName( columnIndex )
      if ( currTaskName == taskName ):
        return columnIndex
          
    return None
    
    
  @staticmethod
  def GetValueByMetricTask( metricsTable, metricTuple, taskName ):
    if ( metricsTable is None ):
      return None

    rowIndex = SkillAssessmentLogic.GetRowIndexByMetric( metricsTable, metricTuple )
    columnIndex = SkillAssessmentLogic.GetColumnIndexByTask( metricsTable, taskName )
      
    return SkillAssessmentLogic.GetValueByRowIndexColumnIndex( metricsTable, rowIndex, columnIndex )

      
  @staticmethod
  def GetValueByMetricColumnIndex( metricsTable, metricTuple, columnIndex ):
    if ( metricsTable is None ):
      return None

    rowIndex = SkillAssessmentLogic.GetRowIndexByMetric( metricsTable, metricTuple )
      
    return SkillAssessmentLogic.GetValueByRowIndexColumnIndex( metricsTable, rowIndex, columnIndex )
      
      
  @staticmethod
  def GetValueByRowIndexTask( metricsTable, rowIndex, taskName ):
    if ( metricsTable is None ):
      return None

    columnIndex = SkillAssessmentLogic.GetColumnIndexByTask( metricsTable, taskName )
      
    return SkillAssessmentLogic.GetValueByRowIndexColumnIndex( metricsTable, rowIndex, columnIndex )
    
    
  @staticmethod
  def GetValueByRowIndexColumnIndex( metricsTable, rowIndex, columnIndex ):
    if ( metricsTable is None ):
      return None
    if ( rowIndex is None or columnIndex is None ):
      return None
        
    return metricsTable.GetValue( rowIndex, columnIndex )
    
    
  @staticmethod
  def SetValueByMetricTask( metricsTable, metricTuple, taskName, value ):
    if ( metricsTable is None ):
      return

    rowIndex = SkillAssessmentLogic.GetRowIndexByMetric( metricsTable, metricTuple )
    columnIndex = SkillAssessmentLogic.GetColumnIndexByTask( metricsTable, taskName )
      
    SkillAssessmentLogic.SetValueByRowIndexColumnIndex( metricsTable, rowIndex, columnIndex, value )

      
  @staticmethod
  def SetValueByMetricColumnIndex( metricsTable, metricTuple, columnIndex, value ):
    if ( metricsTable is None ):
      return

    rowIndex = SkillAssessmentLogic.GetRowIndexByMetric( metricsTable, metricTuple )
      
    SkillAssessmentLogic.SetValueByRowIndexColumnIndex( metricsTable, rowIndex, columnIndex, value )
      
      
  @staticmethod
  def SetValueByRowIndexTask( metricsTable, rowIndex, taskName, value ):
    if ( metricsTable is None ):
      return

    columnIndex = SkillAssessmentLogic.GetColumnIndexByTask( metricsTable, taskName )
      
    SkillAssessmentLogic.SetValueByRowIndexColumnIndex( metricsTable, rowIndex, columnIndex, value )
    
    
  @staticmethod
  def SetValueByRowIndexColumnIndex( metricsTable, rowIndex, columnIndex, value ):
    if ( metricsTable is None ):
      return None
    if ( rowIndex is None or columnIndex is None ):
      return None
        
    metricsTable.SetValue( rowIndex, columnIndex, value )
    
    
  @staticmethod
  def GetAllMetricTuples( metricsTable ):
    if ( metricsTable is None ):
      return []
      
    allMetricTuples = []
    for rowIndex in range( metricsTable.GetNumberOfRows() ):
      currMetric = SkillAssessmentLogic.GetMetricByRowIndex( metricsTable, rowIndex )
      if ( currMetric is not None ):
        allMetricTuples.append( currMetric )
      
    return allMetricTuples
    
  
  @staticmethod
  def GetAllTaskNames( metricsTable, overallMetricValueIsHeader ):
    if ( metricsTable is None ):
      return []
      
    allTaskNames = []
    for columnIndex in range( metricsTable.GetNumberOfColumns() ):
      currTask = SkillAssessmentLogic.GetTaskByColumIndex( metricsTable, columnIndex, overallMetricValueIsHeader )
      if ( currTask is not None ):
        allTaskNames.append( currTask )
      
    return allTaskNames
    
    
  @staticmethod
  def GetMetricString( metricTuple ):
    if ( metricTuple is None ):
      return ""
    
    metricString = metricTuple[ 0 ].ToString()
    metricString = metricString + " ["
    metricString = metricString + metricTuple[ 1 ].ToString()
    metricString = metricString + "] "
    metricString = metricString + "("
    metricString = metricString + metricTuple[ 2 ].ToString()
    metricString = metricString + ")"
    
    return metricString

  
  @staticmethod
  def GetTaskString( taskName ):
    if ( taskName is None ):
      return ""
      
    return taskName # Do not need ToString method here because we are not picking from the table, we are picking from the column names.
    
    
  @staticmethod
  def GetMetricTaskString( metricString, taskString ):
    if ( metricString is None or metricString == "" ):
      return ""
    if ( taskString is None or taskString == "" ):
      return metricString
      
    return ( metricString + " during " + taskString )
    
    
    


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
