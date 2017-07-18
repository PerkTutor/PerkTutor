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
    
    if ( self.parameterNode is not None ):
      self.parameterNodeObserverTag = self.parameterNode.AddObserver( vtk.vtkCommand.ModifiedEvent, self.updateWidgetFromParameterNode )
  
    if ( self.parameterNode.GetAttribute( "NeighborWeight" ) is None ):
      self.parameterNode.SetAttribute( "NeighborWeight", NEIGHBOR_WEIGHT_EQUAL )
    if ( self.parameterNode.GetAttribute( "NumberOfNeighbors" ) is None ):
      self.parameterNode.SetAttribute( "NumberOfNeighbors", str( 1 ) )

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
  def ComputeSkill( parameterNode, testRecord, trainingRecords, weights, labels ):
    numberOfNeighbors = parameterNode.GetAttribute( "NumberOfNeighbors" )
    neighborWeight = parameterNode.GetAttribute( "NeighborWeight" )
  
    labels = labels[:] # Deep copy
    distances = NearestNeighborAssessment.ComputeWeightedDistances( testRecord, trainingRecords, weights )
    NearestNeighborAssessment.CutoffNeighbors( distances, labels, numberOfNeighbors )
    vote = NearestNeighborAssessment.CastVotes( distances, labels, neighborWeight )
    
    return vote
    
    
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
  def CutoffNeighbors( distances, labels, numberOfNeighbors ):
    try:
      numberOfNeighbors = int( numberOfNeighbors )
    except:
      logging.warning( "NearestNeighborAssessment::CutoffNeighbors: The number of neighbors specified is a non-integer" )
  
    while ( len( distances ) > numberOfNeighbors ):
      maxIndex = distances.index( max( distances ) )
      distances.pop( maxIndex )
      labels.pop( maxIndex )


  # Cast votes using the specified neighbor weighting scheme
  @staticmethod
  def CastVotes( distances, labels, neighborWeight ):
    if ( neighborWeight == NEIGHBOR_WEIGHT_EQUAL ):
      return NearestNeighborAssessment.CastEqualWeightedVotes( distances, labels )
    if ( neighborWeight == NEIGHBOR_WEIGHT_DISTANCE ):
      return NearestNeighborAssessment.CastDistanceWeightedVotes( distances, labels )
    if ( neighborWeight == NEIGHBOR_WEIGHT_RANK ):
      return NearestNeighborAssessment.CastRankWeightedVotes( distances, labels )
      
    return 0
      
      
  @staticmethod
  def CastEqualWeightedVotes( distances, labels ):
    return float( sum( labels ) ) / len( labels )
    
    
  @staticmethod
  def CastDistanceWeightedVotes( distances, labels ):
    distanceArray = numpy.array( distances )
    labelArray = numpy.array( labels )
    return numpy.dot( 1.0 / ( distanceArray ** 2 ), labelArray )
    
    
  @staticmethod
  def CastRankWeightedVotes( distances, labels ):
    distanceArray = numpy.array( distances )
    labelArray = numpy.array( labels )
    rankedIndices = distanceArray.argsort() # TODO: Deal with ties
    
    return numpy.dot( 1.0 / ( rankedIndices + 1 ), labelArray ) # Add one because ranking starts at ( 0, 1, 2, 3, ... )
      
    
    
  # The critical value for feedback
  @staticmethod
  def GetCriticalValue( parameterNode, skillLabels ):
    # Should be half the range of the labels      
    maxSkill = max( skillLabels )
    minSkill = min( skillLabels )
    criticalValue = minSkill + ( maxSkill - minSkill ) / 2.0

    return criticalValue