import os
import unittest
import vtk, qt, ctk, slicer
import math
import numpy
import logging
from .FuzzyLogic import *

#
# Fuzzy Skill Assessment
#

# Note: With current methods, the antecedents are never composed
NORM_GODEL_T = "GodelTNorm"
NORM_GODEL_S = "GodelSNorm"
NORM_GOGUEN_T = "GoguenTNorm"
NORM_GOGUEN_S = "GoguenSNorm"
NORM_LUKASIEWICZ_T = "LukasiewiczTNorm"
NORM_LUKASIEWICZ_S = "LukasiewiczSNorm"
NORM_NILPOTENT_T = "NilpotentTNorm"
NORM_NILPOTENT_S = "NilpotentSNorm"
NORM_DRASTIC_T = "DrasticTNorm"
NORM_DRASTIC_S = "DrasticSNorm"

DEFUZZIFIER_COA = "CenterOfArea"
DEFUZZIFIER_COM = "CenterOfMass"
DEFUZZIFIER_MOM = "MeanOfMax"
DEFUZZIFIER_CMCOA = "ClosestMaxToCenterOfArea"
DEFUZZIFIER_CMCOM = "ClosestMaxToCenterOfMass"
DEFUZZIFIER_CMMOM = "ClosestMaxToMeanOfMax"

SHRINK_SCALE = "Scale"
SHRINK_CLIP = "Clip"

METRIC_MEMBERSHIP_GAUSSIANKDE = "Non-parametric"
METRIC_MEMBERSHIP_GAUSSIAN = "Gaussian"

NUMBER_OF_STEPS = 1e3


#
# Fuzzy Parameters Widget
#

class FuzzyParametersWidget( qt.QFrame ):

  def __init__( self, parent = None ):
    qt.QFrame.__init__( self )
  
    self.parameterNode = None

    #
    # Parameters area
    #    
    self.parametersLayout = qt.QFormLayout( self )
    self.setLayout( self.parametersLayout )
    
    # No need for antecedent composition with current methods
    
    #
    # Defuzzifier combo box 
    #    
    self.defuzzifierComboBox = qt.QComboBox()
    self.defuzzifierComboBox.addItem( DEFUZZIFIER_COA )
    self.defuzzifierComboBox.addItem( DEFUZZIFIER_COM )
    self.defuzzifierComboBox.addItem( DEFUZZIFIER_MOM )
    self.defuzzifierComboBox.addItem( DEFUZZIFIER_CMCOA )
    self.defuzzifierComboBox.addItem( DEFUZZIFIER_CMCOM )
    self.defuzzifierComboBox.addItem( DEFUZZIFIER_CMMOM )
    self.defuzzifierComboBox.setToolTip( "Choose the method for defuzzifying the output." )
    self.parametersLayout.addRow( "Defuzzifier ", self.defuzzifierComboBox )
    
    #
    # Shrink combo box 
    #    
    self.shrinkComboBox = qt.QComboBox()
    self.shrinkComboBox.addItem( SHRINK_SCALE )
    self.shrinkComboBox.addItem( SHRINK_CLIP )
    self.shrinkComboBox.setToolTip( "Choose the aggregation method." )
    self.parametersLayout.addRow( "Shrink ", self.shrinkComboBox )
    
    #
    # Metric membership combo box
    #    
    self.metricMembershipComboBox = qt.QComboBox()
    self.metricMembershipComboBox.addItem( METRIC_MEMBERSHIP_GAUSSIANKDE )
    self.metricMembershipComboBox.addItem( METRIC_MEMBERSHIP_GAUSSIAN )
    self.metricMembershipComboBox.setToolTip( "Choose the aggregation method." )
    self.parametersLayout.addRow( "Metric membership ", self.metricMembershipComboBox )
        
    #
    # Number of skill classes
    #    
    self.skillClassesSpinBox = qt.QSpinBox()
    self.skillClassesSpinBox.setRange( 1, 100 )
    self.skillClassesSpinBox.setSingleStep( 1 )
    self.skillClassesSpinBox.setToolTip( "Choose the number of skill classes." )
    self.parametersLayout.addRow( "Skill classes  ", self.skillClassesSpinBox )
    
    # connections
    self.defuzzifierComboBox.connect( 'currentIndexChanged(QString)', self.onDefuzzifierChanged )
    self.shrinkComboBox.connect( 'currentIndexChanged(QString)', self.onShrinkChanged )
    self.metricMembershipComboBox.connect( 'currentIndexChanged(QString)', self.onMetricMembershipChanged )
    self.skillClassesSpinBox.connect( 'valueChanged(int)', self.onSkillClassesChanged )
    
    
  def setParameterNode( self, parameterNode ):
    # Replace the old observers
    if ( self.parameterNode is not None ):
      self.parameterNode.RemoveObserver( self.parameterNodeObserverTag )
      
    self.parameterNode = parameterNode

    if ( self.parameterNode.GetAttribute( "Defuzzifier" ) is None ):
      self.parameterNode.SetAttribute( "Defuzzifier", DEFUZZIFIER_COM )
    if ( self.parameterNode.GetAttribute( "Shrink" ) is None ):
      self.parameterNode.SetAttribute( "Shrink", SHRINK_SCALE )
    if ( self.parameterNode.GetAttribute( "MetricMembership" ) is None ):
      self.parameterNode.SetAttribute( "MetricMembership", METRIC_MEMBERSHIP_GAUSSIANKDE )
    if ( self.parameterNode.GetAttribute( "SkillClasses" ) is None ):
      self.parameterNode.SetAttribute( "SkillClasses", str( 2 ) ) # Default 2 (i.e. Novice and Expert)

    if ( self.parameterNode is not None ):
      self.parameterNodeObserverTag = self.parameterNode.AddObserver( vtk.vtkCommand.ModifiedEvent, self.updateWidgetFromParameterNode )
    self.updateWidgetFromParameterNode()
    
    
  def getParameterNode( self ):
    return self.parameterNode
    
    
  def onDefuzzifierChanged( self, text ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "Defuzzifier", text )
    
    
  def onShrinkChanged( self, text ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "Shrink", text )
    

  def onMetricMembershipChanged( self, text ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "MetricMembership", text )
    
    
  def onSkillClassesChanged( self, number ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "SkillClasses", str( number ) )
    
    
  def updateWidgetFromParameterNode( self, node = None, eventID = None ):
    if ( self.parameterNode is None ):
      return

    defuzzifierIndex = self.defuzzifierComboBox.findText( self.parameterNode.GetAttribute( "Defuzzifier" ) )
    if ( defuzzifierIndex >= 0 ):
      self.defuzzifierComboBox.setCurrentIndex( defuzzifierIndex )
    
    shrinkIndex = self.shrinkComboBox.findText( self.parameterNode.GetAttribute( "Shrink" ) )
    if ( shrinkIndex >= 0 ):
      self.shrinkComboBox.setCurrentIndex( shrinkIndex )
      
    metricMembershipIndex = self.metricMembershipComboBox.findText( self.parameterNode.GetAttribute( "MetricMembership" ) )
    if ( metricMembershipIndex >= 0 ):
      self.metricMembershipComboBox.setCurrentIndex( metricMembershipIndex )
      
    self.skillClassesSpinBox.setValue( int( self.parameterNode.GetAttribute( "SkillClasses" ) ) )
    

#
# Linear Combination Assessment
#

class FuzzyAssessment():


  def __init__( self ):
    pass

    
  @staticmethod
  def GetGenericDescription():
    descriptionString = "This assessment method uses rules of the form: IF the metric value looks like it is of a particular skill class THEN operator is from that skill class. These rules are combined to come up with an overall assessment." + "\n\n"
    return descriptionString
    
    
  @staticmethod
  def GetSpecificDescription( testRecord, nameRecord, metricMembershipFunctions, skillLabels ):
    descriptionString = "In this case, the rules with the most influence were (in descending order): " + "\n\n"
    ruleSkillClasses = []
    ruleMetricNames = []
    ruleMemberships = []
    for skillClassIndex in metricMembershipFunctions:
      for metricIndex in metricMembershipFunctions[ skillClassIndex ]:
        ruleSkillClasses.append( skillClassIndex )
        ruleMetricNames.append( nameRecord[ metricIndex ] )
        ruleMemberships.append( metricMembershipFunctions[ skillClassIndex ][ metricIndex ].Evaluate( testRecord[ metricIndex ] ) )
        
    ruleMetricNameSkillClass = zip( ruleMetricNames, ruleSkillClasses )
    sortedRules = sorted( zip( ruleMemberships, ruleMetricNameSkillClass ), reverse = True )
    for currRuleMembership, currRuleMetricNameSkillClass in sortedRules:
      descriptionString = descriptionString + "IF " + currRuleMetricNameSkillClass[ 0 ] + " looks like skill class " + str( currRuleMetricNameSkillClass[ 1 ] ) + " THEN operator is from skill class " + str( currRuleMetricNameSkillClass[ 1 ] ) +" (membership = " + str( currRuleMembership ) + ")" + "\n"
      
    return descriptionString
    
    
  @staticmethod 
  def ComputeSkill( parameterNode, testRecord, trainingRecords, weights, nameRecord, nameLabels, skillLabels ):
    defuzzifier = FuzzyAssessment.GetDefuzzifier( parameterNode.GetAttribute( "Defuzzifier" ) )
    shrinker = FuzzyAssessment.GetShrinker( parameterNode.GetAttribute( "Shrink" ) )
    metricMembershipDistribution = parameterNode.GetAttribute( "MetricMembership" )
    
    skillClasses = int( parameterNode.GetAttribute( "SkillClasses" ) )
    minSkill = 0 #min( skillLabels )
    maxSkill = 1 #max( skillLabels )

    skillMembershipFunctions, triangleWidth = FuzzyAssessment.CreateAllSkillMembershipFunctions( skillClasses, minSkill, maxSkill )
    metricMembershipFunctions = FuzzyAssessment.CreateAllMetricMembershipFunctions( testRecord, trainingRecords, skillLabels, skillMembershipFunctions, metricMembershipDistribution )
    
    fuzzyRules = FuzzyAssessment.CreateAllFuzzyRules( metricMembershipFunctions, skillMembershipFunctions )
    
    consequence = FuzzyAssessment.ComputeFuzzyOutput( fuzzyRules, testRecord, weights, shrinker )

    minSkillMembership = minSkill - triangleWidth
    maxSkillMembership = maxSkill + triangleWidth
    stepSize = ( maxSkillMembership - minSkillMembership ) / NUMBER_OF_STEPS
    fuzzySkill = defuzzifier.Evaluate( consequence, minSkillMembership, maxSkillMembership, stepSize )

    if ( len( consequence.BaseFunctions ) > 2 ):
      FuzzyAssessment.PlotMembershipFunctions( [ consequence ], minSkillMembership, maxSkillMembership, NUMBER_OF_STEPS ) # For visualization
    descriptionString = FuzzyAssessment.GetGenericDescription() + FuzzyAssessment.GetSpecificDescription( testRecord, nameRecord, metricMembershipFunctions, skillLabels )
    
    return fuzzySkill, descriptionString
    
    
  @staticmethod
  def GetDefuzzifier( defuzzifierName ):
    if ( defuzzifierName == DEFUZZIFIER_COA ):
      return DefuzzifierCOA()
    if ( defuzzifierName == DEFUZZIFIER_COM ):
      return DefuzzifierCOM()
    if ( defuzzifierName == DEFUZZIFIER_MOM ):
      return DefuzzifierMOM()
    if ( defuzzifierName == DEFUZZIFIER_CMCOA ):
      return DefuzzifierCMCOA()
    if ( defuzzifierName == DEFUZZIFIER_CMCOM ):
      return DefuzzifierCMCOM()
    if ( defuzzifierName == DEFUZZIFIER_CMMOM ):
      return DefuzzifierCMMOM()      
    
    return None

    
  @staticmethod
  def GetShrinker( shrinkerName ):
    if ( shrinkerName == SHRINK_CLIP ):
      return GodelTNorm()
    if ( shrinkerName == SHRINK_SCALE ):
      return GoguenTNorm()
      
    return None
    
    
  # Use the approach from Riojas et al. 2011
  # Use triangular membership functions distributed on minSkill - maxSkill
  # Width is such that one reaches zero when the next reaches one
  # Assume that class names are sorted: most skilled (0) -> least skilled (100)
  @staticmethod
  def CreateAllSkillMembershipFunctions( skillClasses, minSkill, maxSkill ):
    skillMembershipFunctions = dict()
    triangleWidth = float( maxSkill - minSkill ) / float( skillClasses - 1 )
  
    for skillClassIndex in range( skillClasses ):
      peak = minSkill + skillClassIndex * triangleWidth
      leftFoot = peak - triangleWidth
      rightFoot = peak + triangleWidth
    
      membershipFunction = TriangleMembershipFunction()
      membershipFunction.SetParameters( [ leftFoot, peak, rightFoot ] )
    
      skillMembershipFunctions[ skillClassIndex ] = membershipFunction
      
    return skillMembershipFunctions, triangleWidth
    
    
  # Compute the membership functions
  # Weight based on the training data's degree of membership
  @staticmethod
  def CreateAllMetricMembershipFunctions( testRecord, trainingRecords, skillLabels, skillMembershipFunctions, metricMembershipDistribution ):
    # Assume that all records are the same length
    # Need an input membership function for each skill class for each metric
    metricMembershipFunctions = dict()
    metricMembershipMax = 0
    for skillClassIndex in skillMembershipFunctions:
      memberships = []
      for currSkillLabel in skillLabels:
        memberships.append( skillMembershipFunctions[ skillClassIndex ].Evaluate( currSkillLabel ) )
    
      metricMembershipFunctions[ skillClassIndex ] = dict()
      for metricIndex in range( len( testRecord ) ):
        currTrainingVector = []
        for currTrainingRecord in trainingRecords:
          currTrainingVector.append( currTrainingRecord[ metricIndex ] )
          
        currMetricMembershipFunction = FuzzyAssessment.CreateMetricMembershipFunction( currTrainingVector, memberships , metricMembershipDistribution )
        metricMembershipFunctions[ skillClassIndex ][ metricIndex ] = currMetricMembershipFunction

        currMetricMembershipMax = Defuzzifier().MaximumValue( currMetricMembershipFunction, min( currTrainingVector ), max( currTrainingVector ), NUMBER_OF_STEPS )
        metricMembershipMax = max( metricMembershipMax, currMetricMembershipMax )

    # Scale the metric membership functions such that for each metric, the AUC is the same for all skill classes and the maximum value is one
    for skillClassIndex in metricMembershipFunctions:
      for metricIndex in metricMembershipFunctions[ skillClassIndex ]:
        if metricMembershipFunctions[ skillClassIndex ][ metricIndex ].Parameters: # Make sure parameters exist (their is some membership)
          metricMembershipFunctions[ skillClassIndex ][ metricIndex ].Parameters[ 0 ] = 1 / float( metricMembershipMax )
        
    return metricMembershipFunctions




  # Create a Gaussian membership function, using the weighted inputs to estimate the mean and stdev
  @staticmethod
  def CreateMetricMembershipFunction( trainingData, memberships, metricMembershipDistribution ):
    # Make sure the memberships add to one
    totalMembership = sum( memberships )
    if ( totalMembership == 0 ):
      logging.warning( "FuzzyAssessment::CreateMetricMembershipFunction: No membership in skill class." )
      return MembershipFunction()

    for i in range( len( memberships ) ):
      memberships[ i ] = memberships[ i ] / float( totalMembership )

    mean = 0
    stdev = 0
    for i in range( len( trainingData ) ):
      mean += memberships[ i ] * trainingData[ i ]
      stdev += memberships[ i ] * math.pow( trainingData[ i ], 2 )
    
    stdev = math.sqrt( stdev - math.pow( mean, 2 ) )
    
    if ( stdev == 0 ):
      logging.warning( "FuzzyAssessment::CreateMetricMembershipFunction: Cannot create a Gaussian KDE membership function from the provided input. Input requires more instances or more variance." )
    
    if ( metricMembershipDistribution == METRIC_MEMBERSHIP_GAUSSIANKDE ):
      return FuzzyAssessment.CreateGaussianKDEMetricMembershipFunction( trainingData, memberships, stdev )
    if ( metricMembershipDistribution == METRIC_MEMBERSHIP_GAUSSIAN ):
      return FuzzyAssessment.CreateGaussianMetricMembershipFunction( mean, stdev )
      
    logging.info( "FuzzyAssessment::CreateMetricMembershipFunction: Metric membership distribution improperly specified." )    
    return MembershipFunction()

    
  @staticmethod
  def CreateGaussianKDEMetricMembershipFunction( trainingData, memberships, stdev ):
    parameters = [ 1 ] # default scaling one
    parameters.append( math.pow( 4 * math.pow( stdev, 5 ) / ( 3 * sum( memberships ) ), 0.2 ) ) # Use the stdev to compute the Silverman rule of thumb for the bandwith
    for i in range( len( trainingData ) ):
      parameters.append( trainingData[ i ] )
      parameters.append( memberships[ i ] )
      
    membershipFunction = GaussianKDEMembershipFunction()
    membershipFunction.SetParameters( parameters )

    return membershipFunction
    
    
  @staticmethod
  def CreateGaussianMetricMembershipFunction( mean, stdev ):
    membershipFunction = GaussianMembershipFunction()
    membershipFunction.SetParameters( [ 1, mean, stdev ] ) # default scaling one
    return membershipFunction

    
  # Use the approach from Riojas et al. 2011
  # Whatever the metric is, a particular group for that metric points to the same group overall
  # IF metric is group THEN skill is group
  # All metrics will be of this form
  @staticmethod
  def CreateAllFuzzyRules( metricMembershipFunctions, skillMembershipFunctions ):
    fuzzyRules = dict() # Reset

    for skillClassIndex in metricMembershipFunctions:
      skillMembership = skillMembershipFunctions[ skillClassIndex ]
      fuzzyRules[ skillClassIndex ] = dict()
      for metricIndex in metricMembershipFunctions[ skillClassIndex ]:
        currFuzzyRule = FuzzyRule()
        currFuzzyRule.SetComposeFunction( GodelTNorm() ) #TODO: This compose function is not actually used. In the future, the rule should not require a compose function unless multiple input membership functions are specified for the rule.
        currFuzzyRule.SetOutputMembershipFunction( skillMembership )
        currFuzzyRule.AddInputMembershipFunction( metricMembershipFunctions[ skillClassIndex ][ metricIndex ], "Metric" )
      
        fuzzyRules[ skillClassIndex ][ metricIndex ] = currFuzzyRule
        
    return fuzzyRules
    
    
  # Given a crisp input, apply all of the fuzzy rules, and come up with a fuzzy output
  @staticmethod
  def ComputeFuzzyOutput( fuzzyRules, testRecord, weights, shrinker ):
    # Turn the test record into a dictionary
    totalConsequence = MembershipFunction()
    for skillClassIndex in fuzzyRules:
      for metricIndex in fuzzyRules[ skillClassIndex ]:
        currFuzzyRule = fuzzyRules[ skillClassIndex ][ metricIndex ]
        currInputValue = { "Metric": testRecord[ metricIndex ] }
        currConsequence = currFuzzyRule.Evaluate( currInputValue, shrinker )
        # Use the weighting (with the same shrink method)
        weightMembershipFunction = FlatMembershipFunction()
        weightMembershipFunction.SetParameters( [ weights[ metricIndex ] ] )
        currConsequence.AddBaseFunction( weightMembershipFunction )
        # Add the current consequence function to the total consequence function
        totalConsequence.AddBaseFunction( currConsequence )

    # Note: Still have to set the compose function
    # This is because the compose function will differ depending on which defuzzification technique is used
    return totalConsequence
    
    
  # The critical value for feedback
  # The critical value for feedback
  @staticmethod
  def GetCriticalValue( parameterNode, skillLabels ):
    # Should be half the range of the labels      
    maxSkill = 1 #max( skillLabels )
    minSkill = 0 #min( skillLabels )
    criticalValue = minSkill + ( maxSkill - minSkill ) / 2.0

    return criticalValue


  # Use a chart node to plot a membership function
  # Mostly for debugging, but it may be interesting to look at membership functions from the Python interactor
  @staticmethod
  def PlotMembershipFunctions( membershipFunctions, min, max, steps ):

    step = float( max - min ) / steps
    # Setting up the array of values
    plotChartNode = slicer.vtkMRMLPlotChartNode()
    plotChartNode.SetScene( slicer.mrmlScene )
    slicer.mrmlScene.AddNode( plotChartNode )

    tableNode = slicer.vtkMRMLTableNode()
    tableNode.SetScene( slicer.mrmlScene )
    slicer.mrmlScene.AddNode( tableNode )

    # Array of x values
    xArray = vtk.vtkDoubleArray()
    xArray.SetName( "x" )
    xArray.SetNumberOfComponents( 1 )
    xArray.SetNumberOfTuples( int( ( max - min ) / step ) )
    for j in range( xArray.GetNumberOfTuples() ):
      xArray.SetComponent( j, 0, min + j * step )
    tableNode.AddColumn( xArray )

    # Array of y values
    for i in range( len( membershipFunctions ) ):
      yArray = vtk.vtkDoubleArray()
      yArray.SetName( str( i ) )
      yArray.SetNumberOfComponents( 1 )
      yArray.SetNumberOfTuples( int( ( max - min ) / step ) )
      for j in range( yArray.GetNumberOfTuples() ):
        yArray.SetComponent( j, 0, membershipFunctions[ i ].Evaluate( min + j * step ) )
      tableNode.AddColumn( yArray )

    # Setup the plot series
    for i in range( len( membershipFunctions ) ):        
      plotSeriesNode = slicer.vtkMRMLPlotSeriesNode()
      plotSeriesNode.SetScene( slicer.mrmlScene )
      slicer.mrmlScene.AddNode( plotSeriesNode )
      plotSeriesNode.SetAndObserveTableNodeID(tableNode.GetID())
      plotSeriesNode.SetXColumnName( str( 0 ) )
      plotSeriesNode.SetYColumnName( str( i ) )
      plotChartNode.AddAndObservePlotSeriesNodeID(plotSeriesNode.GetID())

    # Set the chart in the chart view node
    plotViewNode = slicer.mrmlScene.GetNthNodeByClass( 0, "vtkMRMLPlotViewNode" )
    if ( plotViewNode is None ):
      plotViewNode = slicer.vtkMRMLPlotViewNode()
      plotViewNode.SetScene( slicer.mrmlScene )
      slicer.mrmlScene.AddNode( plotViewNode )
    plotViewNode.SetPlotChartNodeID( plotChartNode.GetID() )