import os
import unittest
import vtk, qt, ctk, slicer
import numpy
import logging

#
# Linear Combination Skill Assessment
#

SCALING_METHOD_ZSCORE = "Z-Score"
SCALING_METHOD_PERCENTILE = "Percentile"
SCALING_METHOD_RAW = "Raw"

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
    # Scaling method combo box 
    #    
    self.scalingMethodComboBox = qt.QComboBox()
    self.scalingMethodComboBox.addItem( SCALING_METHOD_ZSCORE )
    self.scalingMethodComboBox.addItem( SCALING_METHOD_PERCENTILE )
    self.scalingMethodComboBox.addItem( SCALING_METHOD_RAW )
    self.scalingMethodComboBox.setToolTip( "Choose the scaling method." )
    self.parametersLayout.addRow( "Scaling method ", self.scalingMethodComboBox )
    
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
    self.scalingMethodComboBox.connect( 'currentIndexChanged(QString)', self.onScalingMethodChanged )
    self.aggregationMethodComboBox.connect( 'currentIndexChanged(QString)', self.onAggregationMethodChanged )
    
    
  def setParameterNode( self, parameterNode ):
    # Replace the old observers
    if ( self.parameterNode is not None ):
      self.parameterNode.RemoveObserver( self.parameterNodeObserverTag )
      
    self.parameterNode = parameterNode

    if ( self.parameterNode.GetAttribute( "ScalingMethod" ) is None ):
      self.parameterNode.SetAttribute( "ScalingMethod", SCALING_METHOD_ZSCORE )
    if ( self.parameterNode.GetAttribute( "AggregationMethod" ) is None ):
      self.parameterNode.SetAttribute( "AggregationMethod", AGGREGATION_METHOD_MEAN )

    if ( self.parameterNode is not None ):
      self.parameterNodeObserverTag = self.parameterNode.AddObserver( vtk.vtkCommand.ModifiedEvent, self.updateWidgetFromParameterNode )
    self.updateWidgetFromParameterNode()
    
    
  def getParameterNode( self ):
    return self.parameterNode
    
    
  def onScalingMethodChanged( self, text ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "ScalingMethod", text )
    
    
  def onAggregationMethodChanged( self, text ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "AggregationMethod", text )
    
    
  def updateWidgetFromParameterNode( self, node = None, eventID = None ):
    if ( self.parameterNode is None ):
      return

    scalingMethodIndex = self.scalingMethodComboBox.findText( self.parameterNode.GetAttribute( "ScalingMethod" ) )
    if ( scalingMethodIndex >= 0 ):
      self.scalingMethodComboBox.setCurrentIndex( scalingMethodIndex )
    
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
  def GetGenericDescription():
    descriptionString = "This assessment method scales the metric values relative to the training data and takes a weighted sum of these scaled metric values." + "\n\n"
    return descriptionString
    
    
  @staticmethod
  def GetSpecificDescription( scaledTestRecord, nameRecord ):
    descriptionString = "In this case, most outlying metrics were (in descending order): " + "\n\n"
    sortedRecord = sorted( zip( scaledTestRecord, nameRecord ), reverse = True )
    for currScaledTestRecord, currNameRecord in sortedRecord:
      descriptionString = descriptionString + currNameRecord + " (scaled value = " + str( currScaledTestRecord ) + ")" + "\n"
      
    return descriptionString

    
  @staticmethod 
  def ComputeSkill( parameterNode, testRecord, trainingRecords, weights, nameRecord, nameLabels, skillLabels ):
    scalingMethod = parameterNode.GetAttribute( "ScalingMethod" )
    aggregationMethod = parameterNode.GetAttribute( "AggregationMethod" )
  
    scaledTestRecord = LinearCombinationAssessment.GetScaledRecord( testRecord, trainingRecords, scalingMethod )
    combination = LinearCombinationAssessment.GetAggregatedSkillScore( scaledTestRecord, weights, aggregationMethod )
    
    descriptionString = LinearCombinationAssessment.GetGenericDescription() + LinearCombinationAssessment.GetSpecificDescription( scaledTestRecord, nameRecord )
    
    return combination, descriptionString
    
  @staticmethod
  def GetAggregatedSkillScore( scaledRecord, weights, method ):
    if ( len( scaledRecord ) != len( weights ) ):
      logging.info( "LinearCombinationAssessment::GetAggregatedMetricValue: Metric values and weights do not correspond." )
      return 0
      
    if ( method == AGGREGATION_METHOD_MEAN ):
      return LinearCombinationAssessment.GetWeightedMean( scaledRecord, weights )
    if ( method == AGGREGATION_METHOD_MEDIAN ):
      return LinearCombinationAssessment.GetWeightedMedian( scaledRecord, weights )
    if ( method == AGGREGATION_METHOD_MAXIMUM ):
      return LinearCombinationAssessment.GetMaximum( scaledRecord, weights )
      
    logging.info( "LinearCombinationAssessment::GetAggregatedMetricValue: Metric transformation method improperly specified." )
    return 0
 
 
  @staticmethod
  def GetWeightedMean( scaledRecord, weights ):
    valueSum = 0
    weightSum = 0
  
    for i in range( len( scaledRecord ) ):
      currMetricValue = scaledRecord[ i ]
      currWeight = weights[ i ]
      
      valueSum += currMetricValue * currWeight
      weightSum += currWeight

    if ( weightSum == 0.0 ):
      return 0.0

    return float( valueSum ) / weightSum


  @staticmethod
  def GetWeightedMedian( scaledRecord, weights ):
    sortedPairs = sorted( zip( scaledRecord, weights ) )
  
    totalWeight = sum( weights )
    weightSum = 0
    for pair in sortedPairs:
      weightSum += pair[ 1 ]
      if ( weightSum > 0.5 * totalWeight ):
        return pair[ 0 ]
      
    return scaledRecord[-1]

  @staticmethod
  def GetMaximum( scaledRecord, weights ):
    return max( scaledRecord ) 
      
    
  @staticmethod 
  def GetScaledRecord( testRecord, trainingRecords, method ):
    # Assume that all the records are of the same length
    scaledRecord = []
    for i in range( len( testRecord ) ):
      currTrainingVector = [] # A list of all values from the same attribute (i.e. metric/task pair)
      for currTrainingRecord in trainingRecords:
        currTrainingVector.append( currTrainingRecord[ i ] )

      currScaled = None
      if ( method == SCALING_METHOD_RAW ):
        currScaled = LinearCombinationAssessment.GetRaw( testRecord[ i ], currTrainingVector )
      if ( method == SCALING_METHOD_PERCENTILE ):
        currScaled = LinearCombinationAssessment.GetPercentile( testRecord[ i ], currTrainingVector )
      if ( method == SCALING_METHOD_ZSCORE ):
        currScaled = LinearCombinationAssessment.GetZScore( testRecord[ i ], currTrainingVector )
      
      if ( currScaled is None ):
        logging.info( "LinearCombinationAssessment::GetConvertedMetricValue: Metric scaling method improperly specified." )
        continue
        
      scaledRecord.append( currScaled )
    
    return scaledRecord
    
    
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
  def GetCriticalValue( parameterNode, skillLabels ):
    scalingMethod = parameterNode.GetAttribute( "ScalingMethod" )
    
    criticalValue = 0 # This is the cutoff between a good metric and a bad metric
    if ( scalingMethod == SCALING_METHOD_RAW ):
      criticalValue = 0
    if ( scalingMethod == SCALING_METHOD_PERCENTILE ):
      criticalValue = 0.5
    if ( scalingMethod == SCALING_METHOD_ZSCORE ):
      criticalValue = 0
      
    return criticalValue