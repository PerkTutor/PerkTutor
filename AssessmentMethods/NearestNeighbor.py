import os
import unittest
import vtk, qt, ctk, slicer
import numpy
import logging
import math

#
# Nearest Neighbor Skill Assessment
#

NEIGHBOR_WEIGHT_EQUAL = "Equal"
NEIGHBOR_WEIGHT_DISTANCE = "Distance"
NEIGHBOR_WEIGHT_RANK = "Rank"


#
# Nearest Neighbor Parameters Widget
#

class NearestNeighborParametersWidget( qt.QFrame ):

  def __init__( self, parent = None ):
    qt.QFrame.__init__( self )
  
    self.parameterNode = None

    #
    # Parameters area
    #    
    self.parametersLayout = qt.QFormLayout( self )
    self.setLayout( self.parametersLayout )
        
    #
    # Combination method combo box 
    #    
    self.neighborWeightComboBox = qt.QComboBox()
    self.neighborWeightComboBox.addItem( NEIGHBOR_WEIGHT_EQUAL )
    self.neighborWeightComboBox.addItem( NEIGHBOR_WEIGHT_DISTANCE )
    self.neighborWeightComboBox.addItem( NEIGHBOR_WEIGHT_RANK )
    self.neighborWeightComboBox.setToolTip( "Choose the weighting scheme for neighbors." )
    self.parametersLayout.addRow( "Weighting ", self.neighborWeightComboBox )
    
    #
    # Number of neighbors
    #    
    self.numberOfNeighborsSpinBox = qt.QSpinBox()
    self.numberOfNeighborsSpinBox.setRange( 1, 100 )
    self.numberOfNeighborsSpinBox.setSingleStep( 1 )
    self.numberOfNeighborsSpinBox.setToolTip( "Choose the number of neighbors (k)." )
    self.parametersLayout.addRow( "Neighbors  ", self.numberOfNeighborsSpinBox )

    
    # connections
    self.neighborWeightComboBox.connect( 'currentIndexChanged(QString)', self.onNeighborWeightChanged )
    self.numberOfNeighborsSpinBox.connect( 'valueChanged(int)', self.onNumberOfNeighborsChanged )
    
    
  def setParameterNode( self, parameterNode ):
    # Replace the old observers
    if ( self.parameterNode is not None ):
      self.parameterNode.RemoveObserver( self.parameterNodeObserverTag )
      
    self.parameterNode = parameterNode
  
    if ( self.parameterNode.GetAttribute( "NeighborWeight" ) is None ):
      self.parameterNode.SetAttribute( "NeighborWeight", NEIGHBOR_WEIGHT_EQUAL )
    if ( self.parameterNode.GetAttribute( "NumberOfNeighbors" ) is None ):
      self.parameterNode.SetAttribute( "NumberOfNeighbors", str( 1 ) )

    if ( self.parameterNode is not None ):
      self.parameterNodeObserverTag = self.parameterNode.AddObserver( vtk.vtkCommand.ModifiedEvent, self.updateWidgetFromParameterNode )
    self.updateWidgetFromParameterNode()
    
    
  def getParameterNode( self ):
    return self.parameterNode
    
    
  def onNeighborWeightChanged( self, text ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "NeighborWeight", text )
    
    
  def onNumberOfNeighborsChanged( self, number ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "NumberOfNeighbors", str( number ) )
    
    
  def updateWidgetFromParameterNode( self, node = None, eventID = None ):
    if ( self.parameterNode is None ):
      return

    neighborWeightIndex = self.neighborWeightComboBox.findText( self.parameterNode.GetAttribute( "NeighborWeight" ) )
    if ( neighborWeightIndex >= 0 ):
      self.neighborWeightComboBox.setCurrentIndex( neighborWeightIndex )
    
    self.numberOfNeighborsSpinBox.setValue( int( self.parameterNode.GetAttribute( "NumberOfNeighbors" ) ) )
    

#
# Nearest Neighbor Assessment
#

class NearestNeighborAssessment():


  def __init__( self ):
    pass
      
      
  @staticmethod
  def GetGenericDescription():
    descriptionString = "This assessment method looks at the most similar metrics table to the input. Each of the similar metrics tables votes on what skill level they think it is. The result of the vote is taken as the overall skill level." + "\n\n"
    return descriptionString
    
    
  @staticmethod
  def GetSpecificDescription( nameLabels, skillLabels ):
    descriptionString = "In this case, the most similar metrics tables were: " + "\n\n"
    for labelIndex in range( len( nameLabels ) ):
      descriptionString = descriptionString + nameLabels[ labelIndex ] +  " (skill level = " + str( skillLabels[ labelIndex ] ) + ")" + "\n"
      
    return descriptionString
      
    
  @staticmethod 
  def ComputeSkill( parameterNode, testRecord, trainingRecords, weights, nameRecord, nameLabels, skillLabels ):
    numberOfNeighbors = parameterNode.GetAttribute( "NumberOfNeighbors" )
    neighborWeight = parameterNode.GetAttribute( "NeighborWeight" )
  
    skillLabels = skillLabels[:] # Deep copy
    nameLabels = nameLabels[:] # Deep copy
    
    distances = NearestNeighborAssessment.ComputeWeightedDistances( testRecord, trainingRecords, weights )
    NearestNeighborAssessment.CutoffNeighbors( distances, skillLabels, nameLabels, numberOfNeighbors )
    vote = NearestNeighborAssessment.CastVotes( distances, skillLabels, neighborWeight )
    
    descriptionString = NearestNeighborAssessment.GetGenericDescription() + NearestNeighborAssessment.GetSpecificDescription( nameLabels, skillLabels )
    
    return vote, descriptionString
    
    
  # Computed the weighted distance from the record values in the test metric to the record values in the training set
  @staticmethod
  def ComputeWeightedDistances( testRecord, trainingRecords, weights ):
    testArray = numpy.array( testRecord )
    distances = []    
    for currTrainingRecord in trainingRecords:
      currTrainingArray = numpy.array( currTrainingRecord )
      currDistance = math.sqrt( numpy.dot( weights, ( testArray - currTrainingArray ) ** 2 ) ) # Computes the weighted distance between the two record arrays
      distances.append( currDistance )
      
    return distances


  # Remove the neighbors with largest distance until we have only the selected number of neighbors left
  @staticmethod
  def CutoffNeighbors( distances, skillLabels, nameLabels, numberOfNeighbors ):
    try:
      numberOfNeighbors = int( numberOfNeighbors )
    except:
      logging.warning( "NearestNeighborAssessment::CutoffNeighbors: The number of neighbors specified is a non-integer" )
  
    while ( len( distances ) > numberOfNeighbors ):
      maxIndex = distances.index( max( distances ) )
      distances.pop( maxIndex )
      skillLabels.pop( maxIndex )
      nameLabels.pop( maxIndex )


  # Cast votes using the specified neighbor weighting scheme
  @staticmethod
  def CastVotes( distances, skillLabels, neighborWeight ):
    if ( neighborWeight == NEIGHBOR_WEIGHT_EQUAL ):
      return NearestNeighborAssessment.CastEqualWeightedVotes( distances, skillLabels )
    if ( neighborWeight == NEIGHBOR_WEIGHT_DISTANCE ):
      return NearestNeighborAssessment.CastDistanceWeightedVotes( distances, skillLabels )
    if ( neighborWeight == NEIGHBOR_WEIGHT_RANK ):
      return NearestNeighborAssessment.CastRankWeightedVotes( distances, skillLabels )
      
    return 0
      
      
  @staticmethod
  def CastEqualWeightedVotes( distances, skillLabels ):
    labelArray = numpy.array( skillLabels )
    equalWeights = numpy.ones( len( distances ) )
    equalWeights = equalWeights / sum( equalWeights )
    return numpy.dot( equalWeights, labelArray )
    
    
  @staticmethod
  def CastDistanceWeightedVotes( distances, skillLabels ):
    distanceArray = numpy.array( distances )
    labelArray = numpy.array( skillLabels )
    distanceWeights = 1.0 / ( distanceArray ** 2 )
    distanceWeights = distanceWeights / sum( distanceWeights )
    return numpy.dot( distanceWeights, labelArray )
    
    
  @staticmethod
  def CastRankWeightedVotes( distances, skillLabels ):
    distanceArray = numpy.array( distances )
    labelArray = numpy.array( skillLabels )
    rankedIndices = distanceArray.argsort() # TODO: Deal with ties

    rankWeights = 1.0 / ( rankedIndices + 1 )
    rankWeights = rankWeights / sum( rankWeights )
    return numpy.dot( rankWeights, labelArray ) # Add one because ranking starts at ( 0, 1, 2, 3, ... )
      
    
    
  # The critical value for feedback
  @staticmethod
  def GetCriticalValue( parameterNode, skillLabels ):
    # Should be half the range of the labels      
    maxSkill = max( skillLabels )
    minSkill = min( skillLabels )
    criticalValue = minSkill + ( maxSkill - minSkill ) / 2.0

    return criticalValue