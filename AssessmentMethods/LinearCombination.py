import os
import unittest
import vtk, qt, ctk, slicer
import numpy
import logging

#
# Linear Combination Skill Assessment
#

COMBINATION_METHOD_ZSCORE = "Z-Score"
COMBINATION_METHOD_PERCENTILE = "Percentile"
COMBINATION_METHOD_RAW = "Raw"

AGGREGATION_METHOD_MEAN = "Mean"
AGGREGATION_METHOD_MEDIAN = "Median"
AGGREGATION_METHOD_MAXIMUM = "Maximum"


#
# Linear Combination Parameters Widget
#

class LinearCombinationParametersWidget( qt.QFrame ):

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
    self.combinationMethodComboBox = qt.QComboBox()
    self.combinationMethodComboBox.addItem( COMBINATION_METHOD_ZSCORE )
    self.combinationMethodComboBox.addItem( COMBINATION_METHOD_PERCENTILE )
    self.combinationMethodComboBox.addItem( COMBINATION_METHOD_RAW )
    self.combinationMethodComboBox.setToolTip( "Choose the combination method." )
    self.parametersLayout.addRow( "Combination method ", self.combinationMethodComboBox )
    
    #
    # Aggregation method combo box 
    #    
    self.aggregationMethodComboBox = qt.QComboBox()
    self.aggregationMethodComboBox.addItem( AGGREGATION_METHOD_MEAN )
    self.aggregationMethodComboBox.addItem( AGGREGATION_METHOD_MEDIAN )
    self.aggregationMethodComboBox.addItem( AGGREGATION_METHOD_MAXIMUM )
    self.aggregationMethodComboBox.setToolTip( "Choose the aggregation method." )
    self.parametersLayout.addRow( "Aggregation method ", self.aggregationMethodComboBox )

    
    # connections
    self.combinationMethodComboBox.connect( 'currentIndexChanged(QString)', self.onCombinationMethodChanged )
    self.aggregationMethodComboBox.connect( 'currentIndexChanged(QString)', self.onAggregationMethodChanged )
    
    
  def setParameterNode( self, parameterNode ):
    # Replace the old observers
    if ( self.parameterNode is not None ):
      self.parameterNode.RemoveObserver( self.parameterNodeObserverTag )
      
    self.parameterNode = parameterNode
    
    if ( self.parameterNode is not None ):
      self.parameterNodeObserverTag = self.parameterNode.AddObserver( vtk.vtkCommand.ModifiedEvent, self.updateWidgetFromParameterNode )
  
    if ( self.parameterNode.GetAttribute( "CombinationMethod" ) is None ):
      self.parameterNode.SetAttribute( "CombinationMethod", COMBINATION_METHOD_ZSCORE )
    if ( self.parameterNode.GetAttribute( "AggregationMethod" ) is None ):
      self.parameterNode.SetAttribute( "AggregationMethod", AGGREGATION_METHOD_MEAN )

    self.updateWidgetFromParameterNode()
    
    
  def getParameterNode( self ):
    return self.parameterNode
    
    
  def onCombinationMethodChanged( self, text ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "CombinationMethod", text )
    
    
  def onAggregationMethodChanged( self, text ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "AggregationMethod", text )
    
    
  def updateWidgetFromParameterNode( self, node = None, eventID = None ):
    if ( self.parameterNode is None ):
      return

    combinationMethodIndex = self.combinationMethodComboBox.findText( self.parameterNode.GetAttribute( "CombinationMethod" ) )
    if ( combinationMethodIndex >= 0 ):
      self.combinationMethodComboBox.setCurrentIndex( combinationMethodIndex )
    
    aggregationMethodIndex = self.aggregationMethodComboBox.findText( self.parameterNode.GetAttribute( "AggregationMethod" ) )
    if ( aggregationMethodIndex >= 0 ):
      self.aggregationMethodComboBox.setCurrentIndex( aggregationMethodIndex )
    

#
# Linear Combination Assessment
#

class LinearCombinationAssessment():


  def __init__( self ):
    pass

    
  @staticmethod
  def GetAggregatedMetricValue( metricValues, weights, method ):
    if ( len( metricValues ) != len( weights ) ):
      logging.info( "LinearCombinationAssessment::GetAggregatedMetricValue: Metric values and weights do not correspond." )
      return 0
      
    if ( method == AGGREGATION_METHOD_MEAN ):
      return LinearCombinationAssessment.GetWeightedMean( metricValues, weights )
    if ( method == AGGREGATION_METHOD_MEDIAN ):
      return LinearCombinationAssessment.GetWeightedMedian( metricValues, weights )
    if ( method == AGGREGATION_METHOD_MAXIMUM ):
      return LinearCombinationAssessment.GetMaximum( metricValues, weights )
      
    logging.info( "LinearCombinationAssessment::GetAggregatedMetricValue: Metric transformation method improperly specified." )
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
    if ( method == COMBINATION_METHOD_RAW ):
      return LinearCombinationAssessment.GetRaw( testMetric, trainingMetrics )
    if ( method == COMBINATION_METHOD_PERCENTILE ):
      return LinearCombinationAssessment.GetPercentile( testMetric, trainingMetrics )
    if ( method == COMBINATION_METHOD_ZSCORE ):
      return LinearCombinationAssessment.GetZScore( testMetric, trainingMetrics )
      
    logging.info( "LinearCombinationAssessment::GetTransformedMetricValue: Metric transformation method improperly specified." )
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
    
    
  # The critical value for feedback
  @staticmethod
  def GetCriticalValue( combinationMethod ):
    criticalValue = 0 # This is the cutoff between a good metric and a bad metric
    if ( combinationMethod == COMBINATION_METHOD_RAW ):
      criticalValue = 0
    if ( combinationMethod == COMBINATION_METHOD_PERCENTILE ):
      criticalValue = 0.5
    if ( combinationMethod == COMBINATION_METHOD_ZSCORE ):
      criticalValue = 0
      
    return criticalValue