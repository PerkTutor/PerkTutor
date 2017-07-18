import os
import unittest
from __main__ import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
import math

import FuzzyLogic

#
# FuzzySkillEvaluator
#

class FuzzySkillEvaluator(ScriptedLoadableModule):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Fuzzy Skill Evaluator" # TODO make this more human readable by adding spaces
    self.parent.categories = ["Perk Tutor"]
    self.parent.dependencies = []
    self.parent.contributors = ["Matthew Holden (Queen's University)"] # replace with "Firstname Lastname (Org)"
    self.parent.helpText = """
    This module computes a user's overall skill level, given their tool motion metrics.
    """
    self.parent.acknowledgementText = """
    This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).
    """ # replace with organization, grant and thanks.
    self.parent.icon = qt.QIcon( "FuzzySkillEvaluator.png" )

#
# qFuzzySkillEvaluatorWidget
#

class FuzzySkillEvaluatorWidget(ScriptedLoadableModuleWidget):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Instantiate and connect widgets ...
    # Give the widget access to a persistent logic
    self.fseLogic = FuzzySkillEvaluatorLogic()

    #
    # Parameters Area
    #
    parametersCollapsibleButton = ctk.ctkCollapsibleButton()
    parametersCollapsibleButton.text = "Parameters"
    self.layout.addWidget(parametersCollapsibleButton)

    # Layout within the dummy collapsible button
    parametersFormLayout = qt.QFormLayout(parametersCollapsibleButton)
    
    #
    # metrics table selector
    #
    self.metricsTableSelector = slicer.qMRMLNodeComboBox()
    self.metricsTableSelector.nodeTypes = ( ("vtkMRMLTableNode"), "" )
    self.metricsTableSelector.noneEnabled = True
    self.metricsTableSelector.setMRMLScene( slicer.mrmlScene )
    self.metricsTableSelector.setToolTip( "Pick the metrics for which to compute overall skill." )
    parametersFormLayout.addRow( "Input Metrics: ", self.metricsTableSelector )

    #
    # Apply Button
    #
    self.computeButton = qt.QPushButton( "Compute" )
    self.computeButton.toolTip = "Compute the overall skill level"
    self.computeButton.enabled = True
    parametersFormLayout.addRow( self.computeButton )
    
    #
    # Output skill label
    #
    self.skillLabel = qt.QLabel( "" )
    self.skillLabel.toolTip = "Overall skill level"
    self.skillLabel.enabled = True
    parametersFormLayout.addRow( "Skill level: ", self.skillLabel )
    
    #
    # Advanced Area
    #
    advancedCollapsibleButton = ctk.ctkCollapsibleButton()
    advancedCollapsibleButton.text = "Advanced"
    advancedCollapsibleButton.collapsed = True
    self.layout.addWidget( advancedCollapsibleButton )

    # Layout within the dummy collapsible button
    advancedFormLayout = qt.QFormLayout( advancedCollapsibleButton )
    
    #
    # Antecedent compose function selector
    #
    self.antecedentComposeFunctionSelector = qt.QComboBox()
    self.antecedentComposeFunctionSelector.toolTip = "Select the function to be used to compose antecedents in each fuzzy rule."
    self.antecedentComposeFunctionSelector.enabled = True
    
    antecedentComposeFunctionNames = self.fseLogic.GetAllAntecedentComposeFunctionNames()
    defaultIndex = 0
    for i in range( len ( antecedentComposeFunctionNames ) ):
      self.antecedentComposeFunctionSelector.addItem( antecedentComposeFunctionNames[ i ] )
      if ( antecedentComposeFunctionNames[ i ] == self.fseLogic.AntecedentComposeFunctionName ):
        defaultIndex = i
    self.antecedentComposeFunctionSelector.setCurrentIndex( defaultIndex )
    
    advancedFormLayout.addRow( "Antecedent compose function: ", self.antecedentComposeFunctionSelector )
        
    #
    # Output Defuzzifier selector
    #
    self.outputDefuzzifierSelector = qt.QComboBox()
    self.outputDefuzzifierSelector.toolTip = "Select the defuzzification technique for the output membership function."
    self.outputDefuzzifierSelector.enabled = True
    
    outputDefuzzifierNames = self.fseLogic.GetAllOutputDefuzzifierNames()
    defaultIndex = 0
    for i in range( len ( outputDefuzzifierNames ) ):
      self.outputDefuzzifierSelector.addItem( outputDefuzzifierNames[ i ] )
      if ( outputDefuzzifierNames[ i ] == self.fseLogic.OutputDefuzzifierName ):
        defaultIndex = i
    self.outputDefuzzifierSelector.setCurrentIndex( defaultIndex )
    
    advancedFormLayout.addRow( "Output Defuzzifier: ", self.outputDefuzzifierSelector )
    
    #
    # transform technique selector
    #
    self.transformTechniqueSelector = qt.QComboBox()
    self.transformTechniqueSelector.toolTip = "Select the transform technique to apply to consequence functions."
    self.transformTechniqueSelector.enabled = True
    
    transformTechniqueNames = self.fseLogic.GetAllTransformTechniqueNames()
    defaultIndex = 0
    for i in range( len ( transformTechniqueNames ) ):
      self.transformTechniqueSelector.addItem( transformTechniqueNames[ i ] )
      if ( transformTechniqueNames[ i ] == self.fseLogic.TransformTechniqueName ):
        defaultIndex = i
    self.transformTechniqueSelector.setCurrentIndex( defaultIndex )
    
    advancedFormLayout.addRow( "Transform technique: ", self.transformTechniqueSelector )
    
    # connections
    self.computeButton.connect( 'clicked(bool)' , self.onComputeButtonClicked )
    self.antecedentComposeFunctionSelector.connect( 'currentIndexChanged(QString)', self.onAntecedentComposeFunctionSelected )
    self.outputDefuzzifierSelector.connect( 'currentIndexChanged(QString)', self.onOutputDefuzzifierSelected )
    self.transformTechniqueSelector.connect( 'currentIndexChanged(QString)', self.onTransformTechniqueSelected )
    
    # Add vertical spacer
    self.layout.addStretch(1)
    
    



  def cleanup(self):
    pass

  def onAntecedentComposeFunctionSelected( self ):
    selection = self.antecedentComposeFunctionSelector.currentText
    self.fseLogic.SetAntecedentComposeFunction( selection )
    #print self.fseLogic.AntecedentComposeFunction
    
  
  def onOutputDefuzzifierSelected( self ):
    selection = self.outputDefuzzifierSelector.currentText
    self.fseLogic.SetOutputDefuzzifier( selection )
    #print self.fseLogic.OutputDefuzzifier
    
  
  def onTransformTechniqueSelected( self ):
    selection = self.transformTechniqueSelector.currentText
    self.fseLogic.SetTransformTechnique( selection )
    #print self.fseLogic.TransformTechnique

  def onComputeButtonClicked( self ):
    logic = FuzzySkillEvaluatorLogic()
    metricDict = logic.CreateMetricDictionary( self.metricsTableSelector.currentNode() )
    
    skill = logic.ComputeFuzzySkill( metricDict )
    self.skillLabel.setText( str( skill ) )

#
# FuzzySkillEvaluatorLogic
#

class FuzzySkillEvaluatorLogic(ScriptedLoadableModuleLogic):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """
  
  def __init__( self, parent = None ):
    ScriptedLoadableModuleLogic.__init__( self, parent )
    
    # Set the default parameter values
    self.GroupNames = [ "Novice", "Expert" ]
    
    self.MinSkill = 0
    self.MaxSkill = 100
    self.NumSteps = 1e3 # This is constant, and should not depend on max and min values
    
    self.InputMemberships = dict()
    self.OutputMemberships = dict()
    
    self.FuzzyRules = []
    
    self.SetupFunctionDicts()        
    self.SetAntecedentComposeFunction( "GodelTNorm" )
    self.SetOutputDefuzzifier( "COM" )
    self.SetTransformTechnique( "Clip" )
        
        
  def SetupFunctionDicts( self ):
    self.AntecedentComposeFunctionDict = dict()
    self.AntecedentComposeFunctionDict[ "GodelTNorm" ] = FuzzyLogic.BinaryFunction.GodelTNorm()
    self.AntecedentComposeFunctionDict[ "GodelSNorm" ] = FuzzyLogic.BinaryFunction.GodelSNorm()
    self.AntecedentComposeFunctionDict[ "GoguenTNorm" ] = FuzzyLogic.BinaryFunction.GoguenTNorm()
    self.AntecedentComposeFunctionDict[ "GoguenSNorm" ] = FuzzyLogic.BinaryFunction.GoguenSNorm()
    self.AntecedentComposeFunctionDict[ "LukasiewiczTNorm" ] = FuzzyLogic.BinaryFunction.LukasiewiczTNorm()
    self.AntecedentComposeFunctionDict[ "LukasiewiczSNorm" ] = FuzzyLogic.BinaryFunction.LukasiewiczSNorm()
    self.AntecedentComposeFunctionDict[ "NilpotentTNorm" ] = FuzzyLogic.BinaryFunction.NilpotentTNorm()
    self.AntecedentComposeFunctionDict[ "NilpotentSNorm" ] = FuzzyLogic.BinaryFunction.NilpotentSNorm()
    self.AntecedentComposeFunctionDict[ "DrasticTNorm" ] = FuzzyLogic.BinaryFunction.DrasticTNorm()
    self.AntecedentComposeFunctionDict[ "DrasticSNorm" ] = FuzzyLogic.BinaryFunction.DrasticSNorm()
    
    self.OutputDefuzzifierDict = dict()
    self.OutputDefuzzifierDict[ "COA" ] = FuzzyLogic.Defuzzifier.DefuzzifierCOA()
    self.OutputDefuzzifierDict[ "COM" ] = FuzzyLogic.Defuzzifier.DefuzzifierCOM()
    self.OutputDefuzzifierDict[ "MOM" ] = FuzzyLogic.Defuzzifier.DefuzzifierMOM()
    self.OutputDefuzzifierDict[ "CMCOA" ] = FuzzyLogic.Defuzzifier.DefuzzifierCMCOA()
    self.OutputDefuzzifierDict[ "CMCOM" ] = FuzzyLogic.Defuzzifier.DefuzzifierCMCOM()
    self.OutputDefuzzifierDict[ "CMMOM" ] = FuzzyLogic.Defuzzifier.DefuzzifierCMMOM()
    
    self.TransformTechniqueDict = dict()
    self.TransformTechniqueDict[ "Clip" ] = FuzzyLogic.BinaryFunction.GodelTNorm()
    self.TransformTechniqueDict[ "Scale" ] = FuzzyLogic.BinaryFunction.GoguenTNorm()

    
  # Set the appropriaate function values
  def SetAntecedentComposeFunction( self, name ):
    if ( name in self.AntecedentComposeFunctionDict ):
      self.AntecedentComposeFunction = self.AntecedentComposeFunctionDict[ name ]
      self.AntecedentComposeFunctionName = name

  def SetOutputDefuzzifier( self, name ):
    if ( name in self.OutputDefuzzifierDict ):
      self.OutputDefuzzifier = self.OutputDefuzzifierDict[ name ]
      self.OutputDefuzzifierName = name

  def SetTransformTechnique( self, name ):
    if ( name in self.TransformTechniqueDict ):
      self.TransformTechnique = self.TransformTechniqueDict[ name ]
      self.TransformTechniqueName = name
    
  def GetAllAntecedentComposeFunctionNames( self ):
    return self.AntecedentComposeFunctionDict.keys()
    
  def GetAllOutputDefuzzifierNames( self ):
    return self.OutputDefuzzifierDict.keys()
    
  def GetAllTransformTechniqueNames( self ):
    return self.TransformTechniqueDict.keys()
    
  # Use the approach from Riojas et al. 2011
  # Use triangular membership functions distributed on 0-100
  # Width is such that one reaches zero when the next reaches one
  # Assume that class names are sorted: least skilled (0) -> most skilled (100)
  def CreateAllOutputMemberships( self ):
  
    self.OutputMemberships = dict() # Reset
    
    triangleWidth = ( self.MaxSkill - self.MinSkill ) / float( len( self.GroupNames ) - 1 )
  
    for i in range( len( self.GroupNames ) ):
      peak = self.MinSkill + i * triangleWidth
      leftFoot = peak - triangleWidth
      rightFoot = peak + triangleWidth
    
      membershipFunction = FuzzyLogic.MembershipFunction.TriangleMembershipFunction()
      membershipFunction.SetParameters( [ leftFoot, peak, rightFoot ] )
    
      self.OutputMemberships[ self.GroupNames[ i ] ] = membershipFunction
      
      
  # Model each input membership with a Gaussian membership function
  def ComputeInputMembership( self, trainingData ):
  
    mean = 0
    stdev = 0
    for i in range( len( trainingData ) ):
      mean += trainingData[ i ]
      stdev += math.pow( trainingData[ i ], 2 )
    
    mean = mean / len( trainingData )
    stdev = math.sqrt( stdev / len( trainingData ) - math.pow( mean, 2 ) )
  
    membershipFunction = FuzzyLogic.MembershipFunction.GaussianMembershipFunction()
    membershipFunction.SetParameters( [ mean, stdev ] )
    return membershipFunction
  
  
  # From the table nodes in the scene, create all of the input membership functions
  def CreateAllInputMemberships( self, scene ):
  
    # Compose a dictionary of the training data
    trainingDataDict = dict()
  
    tableNodes = scene.GetNodesByClass( "vtkMRMLTableNode" )
  
    # Iterate over all table nodes with the metrics in them
    for i in range( tableNodes.GetNumberOfItems() ):
  
      tableNode = tableNodes.GetItemAsObject( i )
    
      # Find the group that the metrics belong to
      group = None
      for j in range( len( self.GroupNames ) ):
        if ( tableNode.GetName().find( self.GroupNames[ j ] ) >= 0 ):
          group = self.GroupNames[ j ]

      if ( group == None ):
        continue
      if ( group not in trainingDataDict ):
        trainingDataDict[ group ] = []
      
      tableDict = self.CreateMetricDictionary( tableNode )
      
      trainingDataDict[ group ].append( tableDict )

    # Compose a dictionary of input memberships
    self.InputMemberships = dict()
    
    for group in trainingDataDict:
      groupTrainingData = dict()
      
      # Assemble all data for each metric for the group
      for i in range( len( trainingDataDict[ group ] ) ):
        for metric in trainingDataDict[ group ][ i ]:
        
          metricTable = trainingDataDict[ group ][ i ]
          
          if ( metric not in groupTrainingData ):
            groupTrainingData[ metric ] = []
              
          groupTrainingData[ metric ].append( metricTable[ metric ] )
      
      # Compute the 
      self.InputMemberships[ group ] = dict()
      for metric in groupTrainingData:
        self.InputMemberships[ group ][ metric ] = self.ComputeInputMembership( groupTrainingData[ metric ] )
  
  
  # Use the approach from Riojas et al. 2011
  # Whatever the metric is, a particular group for that metric points to the same group overall
  # IF metric is group THEN skill is group
  # All metrics will be of this form
  def ConstructAllFuzzyRules( self ):

    self.FuzzyRules = [] # Reset

    for group in self.InputMemberships:
      outputMembership = self.OutputMemberships[ group ]    
      
      for name in self.InputMemberships[ group ]:
        rule = FuzzyLogic.FuzzyRule.FuzzyRule()
        rule.SetComposeFunction( self.AntecedentComposeFunction )
        rule.SetOutputMembershipFunction( outputMembership )
        rule.AddInputMembershipFunction( self.InputMemberships[ group ][ name ], name )
      
        self.FuzzyRules.append( rule )

        
  # Given a crisp input, apply all of the fuzzy rules, and come up with a fuzzy output
  def ComputeFuzzyOutput( self, inputValues ):

    # For each fuzzy rule, add evaluate at the input value
    # The function will automatically pick out the required inputs
    consequence = FuzzyLogic.MembershipFunction.MembershipFunction()
    for rule in self.FuzzyRules:
      consequence.AddBaseFunction( rule.Evaluate( inputValues, self.TransformTechnique ) ) # TODO: Allow change to "Scale"
    
    # Note: Still have to set the compose function
    # This is because the compose function will differ depending on which defuzzification technique is used
    return consequence
  
  
  # Compute the step size
  def StepSize( self ):
    return ( self.MaxSkillUniverse() - self.MinSkillUniverse() ) / self.NumSteps
    
  # Minimum possible value in the universe of overall "skill"
  def MinSkillUniverse( self ):
    return self.MinSkill - ( self.MaxSkill - self.MinSkill ) / float( len( self.GroupNames ) - 1 )
  
  # Maximum possible value in the universe of overall "skill"  
  def MaxSkillUniverse( self ):
    return self.MaxSkill + ( self.MaxSkill - self.MinSkill ) / float( len( self.GroupNames ) - 1 )
    
    
  # Run the whole thing
  def ComputeFuzzySkill( self, inputValues ):
  
    # Construct the input and output membership functions
    self.CreateAllInputMemberships( slicer.mrmlScene )
    self.CreateAllOutputMemberships()
    
    # Construct all of the fuzzy rules
    self.ConstructAllFuzzyRules()
    
    # Compute the consequence membership
    consequence = self.ComputeFuzzyOutput( inputValues )
    
    # Defuzzify to get crisp skill level
    fuzzySkill = self.OutputDefuzzifier.Evaluate( consequence, self.MinSkillUniverse(), self.MaxSkillUniverse(), self.StepSize() )
    
    # Ensure the output is between the min and max
    fuzzySkill = min( [ fuzzySkill, self.MaxSkill ] )
    fuzzySkill = max( [ fuzzySkill, self.MinSkill ] )
    return int( fuzzySkill )
    
    
    
  # Create a dictionary of metrics from a table node
  def CreateMetricDictionary( self, tableNode ):
  
    metricDict = dict()
    # Iterate over all metrics for the particular buffer
    table = tableNode.GetTable()
      
    for j in range( 1, table.GetNumberOfRows() ): # Ignore the header row
      transformName = table.GetValue( j, 0 ).ToString()
      metricName = table.GetValue( j, 1 ).ToString()
      totalMetricName = transformName + metricName
      metricValue = table.GetValue( j, 3 ).ToDouble()        
      
      # Add to dictionary of all metrics
      metricDict[ totalMetricName ] = metricValue
      
    return metricDict
    
    
  # Use a chart node to plot a membership function
  # Mostly for debugging, but it may be interesting to look at membership functions from the Python interactor
  def PlotMembershipFunctions( membershipFunctions, min, max ):

    step = ( max - min ) / self.NumSteps

    # Setting up the array of values
    chartNode = slicer.mrmlScene.AddNode( slicer.vtkMRMLChartNode() )
  
    for i in range( len( membershipFunctions ) ):
      arrayNode = slicer.mrmlScene.AddNode( slicer.vtkMRMLDoubleArrayNode() )
      array = arrayNode.GetArray()
      array.SetNumberOfTuples( int( ( max - min ) / step ) )
    
      for j in range( array.GetNumberOfTuples() ):
        array.SetComponent( j, 0, min + j * step )
        array.SetComponent( j, 1, membershipFunctions[ i ].Evaluate( min + j * step ) )
        array.SetComponent( j, 2, 0 )
      
      # Add array into a chart node
      chartNode.AddArray( "Membership Function " + str( i ), arrayNode.GetID() )
      
    chartNode.SetProperty( 'default', 'title', 'Membership Functions' )
    chartNode.SetProperty( 'default', 'xAxisLabel', 'Membership Value' )
    chartNode.SetProperty( 'default', 'yAxisLabel', 'Element' )
  
    # Set the chart in the chart view node
    chartViewNode = slicer.mrmlScene.GetNthNodeByClass( 0, "vtkMRMLChartViewNode" )
    chartViewNode.SetChartNodeID( chartNode.GetID() )

        
        


class FuzzySkillEvaluatorTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_FuzzySkillEvaluator1()

  def test_FuzzySkillEvaluator1(self):
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
    logic = FuzzySkillEvaluatorLogic()
    self.assertTrue( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
