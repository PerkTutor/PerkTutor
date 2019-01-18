import os
import unittest
import vtk, qt, ctk, slicer
import numpy
import logging
import math

#
# Decision Tree Skill Assessment
#

#
# Decision Tree Parameters Widget
#

class DecisionTreeParametersWidget( qt.QFrame ):

  def __init__( self, parent = None ):
    qt.QFrame.__init__( self )
  
    self.parameterNode = None

    #
    # Parameters area
    #    
    self.parametersLayout = qt.QFormLayout( self )
    self.setLayout( self.parametersLayout )
        
    #
    # Stop criteria for splitting spin box
    #    
    self.stopCriteriaSpinBox = qt.QDoubleSpinBox()
    self.stopCriteriaSpinBox.setRange( 0, 1 )
    self.stopCriteriaSpinBox.setSingleStep( 0.01 )
    self.stopCriteriaSpinBox.setToolTip( "Choose the stop criteria for splitting. Maximum variance for leaf." )
    self.parametersLayout.addRow( "Stop criteria ", self.stopCriteriaSpinBox )

    # connections
    self.stopCriteriaSpinBox.connect( 'valueChanged(double)', self.onStopCriteriaChanged )
    
    
  def setParameterNode( self, parameterNode ):
    # Replace the old observers
    if ( self.parameterNode is not None ):
      self.parameterNode.RemoveObserver( self.parameterNodeObserverTag )
      
    self.parameterNode = parameterNode
  
    if ( self.parameterNode.GetAttribute( "StopCriteria" ) is None ):
      self.parameterNode.SetAttribute( "StopCriteria", str( 0.05 ) )

    if ( self.parameterNode is not None ):
      self.parameterNodeObserverTag = self.parameterNode.AddObserver( vtk.vtkCommand.ModifiedEvent, self.updateWidgetFromParameterNode )
    self.updateWidgetFromParameterNode()
    
    
  def getParameterNode( self ):
    return self.parameterNode
    
    
  def onStopCriteriaChanged( self, value ):
    if ( self.parameterNode is None ):
      return
    self.parameterNode.SetAttribute( "StopCriteria", str( value ) )
    
    
  def updateWidgetFromParameterNode( self, node = None, eventID = None ):
    if ( self.parameterNode is None ):
      return

    self.stopCriteriaSpinBox.setValue( float( self.parameterNode.GetAttribute( "StopCriteria" ) ) )


#
# Decision Tree Node
#
class DecisionTreeNode():
  
  def __init__( self ):
    self.Center = None
    self.LeftChild = None
    self.RightChild = None
    self.SplitAttribute = None
    self.SplitPoint = None  
    
    

#
# Decision Tree Assessment
#

class DecisionTreeAssessment():


  def __init__( self ):
    pass


  @staticmethod
  def GetGenericDescription():
    descriptionString = "This assessment method makes a series of binary decisions based on metric values to find the most similar group." + "\n\n"
    return descriptionString
    
    
  @staticmethod
  def GetSpecificDescription( path, nameRecord ):
    descriptionString = "In this case, the most positive influential decisions were (in descending order): " + "\n\n"
    centerChanges = [ None ] * ( len( path ) - 1 )
    for nodeIndex in range( len( path ) - 1 ): # Because there is no split on the last node
      centerChanges[ nodeIndex ] = path[ nodeIndex + 1 ].Center - path[ nodeIndex ].Center

    sortedChangesNodes = sorted( zip( centerChanges, path[:-1] ), reverse = True )
    for currChange, currNode in sortedChangesNodes:
      descriptionString = descriptionString + nameRecord[ currNode.SplitAttribute ] + " split at " + str( currNode.SplitPoint ) + " with change in estimated skill level " + str( currChange ) + "\n"
      
    return descriptionString
    
    
  @staticmethod 
  def ComputeSkill( parameterNode, testRecord, trainingRecords, weights, nameRecord, nameLabels, skillLabels ):
    stopCriteria = float( parameterNode.GetAttribute( "StopCriteria" ) )

    # Build the tree
    decisionTree = DecisionTreeAssessment.BuildDecisionTree( trainingRecords, skillLabels, weights, len( trainingRecords ), stopCriteria )
    
    # Run the test record through the decision tree
    path = []
    score = DecisionTreeAssessment.TraverseDecisionTree( testRecord, decisionTree, path )

    descriptionString = DecisionTreeAssessment.GetGenericDescription() + DecisionTreeAssessment.GetSpecificDescription( path, nameRecord )
    
    return score, descriptionString


  @staticmethod
  def FindAttributeToSplitOn( candidateSplits ):
    bestSplitValue = float( 'inf' )
    bestSplitAttribute = None
    for attributeIndex in range( len( candidateSplits ) ):
      if ( candidateSplits[ attributeIndex ][ 1 ] < bestSplitValue ):
        bestSplitValue = candidateSplits[ attributeIndex ][ 1 ]
        bestSplitAttribute = attributeIndex
        
    return bestSplitAttribute
      
    
  @staticmethod
  def BuildDecisionTree( trainingRecords, skillLabels, weights, totalNumRecords, stopCriteria ):
    currentNode = DecisionTreeNode()
    currentNode.Center = numpy.mean( skillLabels )
    # This is the stopping criteria
    if ( numpy.std( skillLabels ) <= stopCriteria ):
      return currentNode

    candidateSplits = [ None ] * len( trainingRecords[ 0 ] )
    for attributeIndex in range( len( trainingRecords[ 0 ] ) ):
      currAttributes = [ None ] * len( trainingRecords )
      for trainingRecordIndex in range( len( trainingRecords ) ):
        currAttributes[ trainingRecordIndex ] = trainingRecords[ trainingRecordIndex ][ attributeIndex ]
        
      # Find the best splitting point for the current attribute
      splitPoint, splitValue = DecisionTreeAssessment.FindAttributeBestSplitPoint( currAttributes, skillLabels )
      p = 1 - len( trainingRecords ) / float( totalNumRecords )
      splitValue = ( 1 - p ) * splitValue + p * ( 1 - weights[ attributeIndex ] ) # Smaller split value is more influential # Based on Al Iqbal et al., ICECE, 2012.
      candidateSplits[ attributeIndex ] = ( splitPoint, splitValue )
    
    # Now find the best attribute to split on
    splitAttribute = DecisionTreeAssessment.FindAttributeToSplitOn( candidateSplits )
    
    # Sanity check
    # Should never be required, because variance would be zero
    if ( splitAttribute is None ):
      return currentNode
    
    currentNode.SplitPoint = candidateSplits[ splitAttribute ][ 0 ]
    currentNode.SplitAttribute = splitAttribute    
    
    # Create the left and right training records
    leftTrainingRecords = []; leftSkillLabels = []
    rightTrainingRecords = []; rightSkillLabels = []
    midTrainingRecords = []; midSkillLabels = []
    for trainingRecordIndex in range( len( trainingRecords ) ):
      currAttributeValue = trainingRecords[ trainingRecordIndex ][ splitAttribute ]
      if ( currAttributeValue < currentNode.SplitPoint ):
        leftTrainingRecords.append( trainingRecords[ trainingRecordIndex ] )
        leftSkillLabels.append( skillLabels[ trainingRecordIndex ] )
      if ( currAttributeValue > currentNode.SplitPoint ):
        rightTrainingRecords.append( trainingRecords[ trainingRecordIndex ] )
        rightSkillLabels.append( skillLabels[ trainingRecordIndex ] )
      if ( currAttributeValue == currentNode.SplitPoint ):
        midTrainingRecords.append( trainingRecords[ trainingRecordIndex ] )
        midSkillLabels.append( skillLabels[ trainingRecordIndex ] )
        
    # Assemble the subtrees
    if ( not leftTrainingRecords or not leftSkillLabels ): # In case the splitting point caused their to be no values in left child
      currentNode.LeftChild = DecisionTreeNode()
      currentNode.LeftChild.Center = currentNode.Center
    else:
      currentNode.LeftChild = DecisionTreeAssessment.BuildDecisionTree( leftTrainingRecords, leftSkillLabels, weights, totalNumRecords, stopCriteria )
    if ( not rightTrainingRecords or not rightSkillLabels ): # In case the splitting point caused their to be no values in right child
      currentNode.RightChild = DecisionTreeNode()
      currentNode.RightChild.Center = currentNode.Center
    else:
      currentNode.RightChild = DecisionTreeAssessment.BuildDecisionTree( rightTrainingRecords, rightSkillLabels, weights, totalNumRecords, stopCriteria )
    
    return currentNode

    
  @staticmethod
  def FindAttributeBestSplitPoint( attributes, skillLabels ):
    attributesSkills = sorted( zip( attributes, skillLabels ) )
    attributes = numpy.array( zip( *attributesSkills )[ 0 ] ) # Clever trick for unzipping
    skillLabels = numpy.array( zip( *attributesSkills )[ 1 ] ) # Clever trick for unzipping
    
    numAll = float( len( skillLabels ) )
    sumAll = numpy.sum( skillLabels )
    sumSquaresAll = numpy.linalg.norm( skillLabels ) ** 2
    
    numLeft = float( 0 )
    sumLeft = 0
    
    numRight = float( numAll )
    sumRight = sumAll
    
    bestSplitValue = float( 'inf' )
    bestSplitPoint = None
    # Assume all records start in the right side and move the split point to include records in the left side until an optimum is reached
    # Note: The best split point will never put everything on one side and nothing on the other, so we do not need to check these cases
    for currIndex in range( len( skillLabels ) - 1 ):
      numLeft = numLeft + 1
      sumLeft = sumLeft + skillLabels[ currIndex ]
      numRight = numRight - 1
      sumRight = sumRight - skillLabels[ currIndex ]
      
      currSplitValue = sumSquaresAll / numAll - ( sumLeft * sumLeft / numLeft + sumRight * sumRight / numRight ) / numAll
      if ( currSplitValue < bestSplitValue ):
        bestSplitValue = currSplitValue
        bestSplitPoint = ( attributes[ currIndex ] + attributes[ currIndex + 1 ] ) / 2
    
    return bestSplitPoint, bestSplitValue
    
    
  @staticmethod
  def TraverseDecisionTree( testRecord, currentNode, path ):
    path.append( currentNode )
    if ( currentNode.SplitAttribute is None ):
      return currentNode.Center
      
    currTestAttributeValue = testRecord[ currentNode.SplitAttribute ]
    if ( currTestAttributeValue < currentNode.SplitPoint ):
      return DecisionTreeAssessment.TraverseDecisionTree( testRecord, currentNode.LeftChild, path )
    if ( currTestAttributeValue > currentNode.SplitPoint ):
      return DecisionTreeAssessment.TraverseDecisionTree( testRecord, currentNode.RightChild, path )
    
    return currentNode.Center # If the value lies directly on the splitting point
          
    
  # The critical value for feedback
  @staticmethod
  def GetCriticalValue( parameterNode, skillLabels ):
    # Should be half the range of the labels      
    maxSkill = max( skillLabels )
    minSkill = min( skillLabels )
    criticalValue = minSkill + ( maxSkill - minSkill ) / 2.0

    return criticalValue