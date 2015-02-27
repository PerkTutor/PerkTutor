import sys, math
sys.path.append( "C:\Users\mholden8\Documents\Fuzzy Logic 870\Implementation\Python\FuzzyLogic" )
import MembershipFunction
import BinaryFunction
import FuzzyRule
import Defuzzification


# Use the approach from Riojas et al. 2011
# Use triangular membership functions distributed on 0-100
# Width is such that one reaches zero when the next reaches one
# Assume that class names are sorted: least skilled (0) -> most skilled (100)
def ConstructOutputMemberships( groupNames ):
  
  outputMemberships = dict()
  triangleWidth = 100 / float( len( groupNames ) - 1 )
  
  for i in range( len( groupNames ) ):
    peak = i * triangleWidth
    leftFoot = peak - triangleWidth
    rightFoot = peak + triangleWidth
    
    membershipFunction = MembershipFunction.TriangleMembershipFunction()
    membershipFunction.SetParameters( [ leftFoot, peak, rightFoot ] )
    
    outputMemberships [ groupNames[ i ] ] = membershipFunction
    
  return outputMemberships

    
# Model each input membership with a Gaussian membership function
def AddInputMembership( inputMemberships, metricName, groupName, trainingData ):
  
  if ( metricName not in inputMemberships ):
    inputMemberships[ metricName ] = dict()
  
  mean = 0
  stdev = 0
  for i in range( len( trainingData ) ):
    mean += trainingData[ i ]
    stdev += math.pow( trainingData[ i ], 2 )
    
  mean = mean / len( trainingData )
  stdev = math.sqrt( stdev / len( trainingData ) - math.pow( mean, 2 ) )
  
  membershipFunction = MembershipFunction.GaussianMembershipFunction()
  membershipFunction.SetParameters( [ mean, stdev ] )
  
  inputMemberships[ metricName ][ groupName ] = membershipFunction
  
  
# From the table nodes in the scene, create all of the input membership functions
def CreateAllInputMemberships( scene, classNames ):
  
  inputData = dict()
  
  tableNodes = scene.GetNodesByClass( "vtkMRMLTableNode" )
  
  # Iterate over all table nodes with the metrics in them
  for i in range( tableNodes.GetNumberOfItems() ):
  
    currentTableNode = tableNodes.GetItemAsObject( i )
    
    # Find the group that the metrics belong to
    group = None
    for j in range( len( classNames ) ):
      if ( currentTableNode.GetName().find( classNames[ j ] ) >= 0 ):
        group = classNames[ j ]

    if ( group == None ):
      continue

    # Iterate over all metrics for the particular buffer
    currentTable = currentTableNode.GetTable()
    for j in range( 1, currentTable.GetNumberOfRows() ): #Ignore the header row
      transformName = currentTable.GetValue( j, 0 ).ToString()
      metricName = currentTable.GetValue( j, 1 ).ToString()
      metricValue = currentTable.GetValue( j, 3 ).ToDouble()
      
      totalMetricName = transformName + metricName
      
      # Add dictonaries or lists if necessary
      if ( totalMetricName not in inputData ):
        inputData[ totalMetricName ] = dict()
        
      if ( group not in inputData[ totalMetricName ] ):
        inputData[ totalMetricName ][ group ] = []
        
      inputData[ totalMetricName ][ group ].append( metricValue )
      
  # Now, we can finally iterate through and compose all the membership functions
  inputMemberships = dict()
  for metric in inputData:
    for group in inputData[ metric ]:
      AddInputMembership( inputMemberships, metric, group, inputData[ metric ][ group ] )
        
        
  return inputMemberships
  
  
# Use the approach from Riojas et al. 2011
# Whatever the metric is, a particular group for that metric points to the same group overall
# IF metric is group THEN skill is group
# All metrics will be of this form
def ConstructAllFuzzyRules( inputMemberships, outputMemberships, groupNames ):

  fuzzyRules = []

  for group in groupNames:
    currentOutputMembership = outputMemberships[ group ]
    
    for name in inputMemberships:
      currentRule = FuzzyRule.FuzzyRule()
      currentRule.SetComposeFunction( BinaryFunction.GodelTNorm() )# TODO: Change this to appropriate function
      currentRule.SetOutputMembershipFunction( currentOutputMembership )
      currentRule.AddInputMembershipFunction( inputMemberships[ name ][ group ], name, group )
      
      fuzzyRules.append( currentRule )

  return fuzzyRules
  
  
# This function will, given an array of membership functions, compose all of them, using the appropriate compose function
def ComposeMembershipFunctions( membershipFunctions, composeFunction ):
  
  if ( len( membershipFunctions ) == 0 ):
    return MembershipFunction.MembershipFunction()
  
  if ( len( membershipFunctions ) == 1 ):
    return membershipFunctions[ 0 ]
    
  prevMembershipFunction = MembershipFunction.MembershipFunction()
  prevMembershipFunction.SetComposeFunction( composeFunction )
  
  
  
# Given a crisp input, apply all of the fuzzy rules, and come up with a fuzzy output
def ComputeFuzzyOutput( fuzzyRules, inputValues ):

  # For each fuzzy rule, add evaluate at the input value
  # The function will automatically pick out the required inputs
  consequence = MembershipFunction.MembershipFunction()
  for rule in fuzzyRules:
    consequence.AddBaseFunction( rule.Evaluate( inputValues, "Clip" ) ) # TODO: Allow change to "Scale"
    
  # Note: Still have to set the compose function
  # This is because the compose function will differ depending on which defuzzification technique is used
  return consequence
  
  
# Use a chart node to plot a membership function
# Mostly for debugging
def PlotMembershipFunctions( membershipFunctions ):

  min = -100
  max = 200
  
  loc = min
  step = ( max - min ) / 1e3
  integral = 0

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
    print arrayNode.GetID()
    chartNode.AddArray( "Membership Function " + str( i ), arrayNode.GetID() )
      
  chartNode.SetProperty( 'default', 'title', 'Membership Functions' )
  chartNode.SetProperty( 'default', 'xAxisLabel', 'Membership Value' )
  chartNode.SetProperty( 'default', 'yAxisLabel', 'Element' )
  
  # Set the chart in the chart view node
  chartViewNode = slicer.mrmlScene.GetNthNodeByClass( 0, "vtkMRMLChartViewNode" )
  chartViewNode.SetChartNodeID( chartNode.GetID() )