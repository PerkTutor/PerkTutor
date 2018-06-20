import os
import unittest
import vtk, qt, ctk, slicer
import numpy
import logging

#
# Linear Combination Skill Assessment
#

BENCHMARK_CRITERION_ZSCORE = "Z-Score"
BENCHMARK_CRITERION_PERCENTILE = "Percentile"

COMPARISON_METHOD_MEAN = "Mean"
COMPARISON_METHOD_MEDIAN = "Median"
COMPARISON_METHOD_MAXIMUM = "Maximum"
COMPARISON_METHOD_MINIMUM = "Minimum"


#
# Linear Combination Parameters Widget
#

class BenchmarkParametersWidget( qt.QFrame ):

  def __init__( self, parent = None ):
    qt.QFrame.__init__( self )
  
    self.parameterNode = None

    #
    # Parameters area
    #    
    self.parametersLayout = qt.QFormLayout( self )
    self.setLayout( self.parametersLayout )
        
    #
    # Benchmark criterion  combo box 
    #    
    self.benchmarkCriterionComboBox = qt.QComboBox()
    self.benchmarkCriterionComboBox.addItem( BENCHMARK_CRITERION_ZSCORE )
    self.benchmarkCriterionComboBox.addItem( BENCHMARK_CRITERION_PERCENTILE )
    self.benchmarkCriterionComboBox.setToolTip( "Choose the benchmark criterion." )
    self.parametersLayout.addRow( "Benchmark criterion ", self.benchmarkCriterionComboBox )
    
    #
    # Numerical level for benchmark spin box
    #
    self.numericalLevelSpinBox = qt.QDoubleSpinBox()
    self.numericalLevelSpinBox.setRange( -100, 100 )
    self.numericalLevelSpinBox.setSingleStep( 0.1 )
    self.numericalLevelSpinBox.setToolTip( "Choose the numerical level for benchmarking." )
    self.parametersLayout.addRow( "Numerical level ", self.numericalLevelSpinBox )
    
    #
    # Comparison method combo box 
    #    
    self.comparisonMethodComboBox = qt.QComboBox()
    self.comparisonMethodComboBox.addItem( COMPARISON_METHOD_MEAN )
    self.comparisonMethodComboBox.addItem( COMPARISON_METHOD_MEDIAN )
    self.comparisonMethodComboBox.addItem( COMPARISON_METHOD_MAXIMUM )
    self.comparisonMethodComboBox.addItem( COMPARISON_METHOD_MINIMUM )
    self.comparisonMethodComboBox.setToolTip( "Choose the comparison method." )
    self.parametersLayout.addRow( "Comparison method ", self.comparisonMethodComboBox )

    
    # connections
    self.benchmarkCriterionComboBox.connect( 'currentIndexChanged(QString)', self.onBenchmarkCriterionChanged )
    self.numericalLevelSpinBox.connect( 'valueChanged(double)', self.onNumericalLevelChanged )
    self.comparisonMethodComboBox.connect( 'currentIndexChanged(QString)', self.onComparisonMethodChanged )
    
    
  def setParameterNode( self, parameterNode ):
    # Replace the old observers
    if ( self.parameterNode is not None ):
      self.parameterNode.RemoveObserver( self.parameterNodeObserverTag )
      
    self.parameterNode = parameterNode

    if ( self.parameterNode.GetAttribute( "BenchmarkCriterion" ) is None ):
      self.parameterNode.SetAttribute( "BenchmarkCriterion", BENCHMARK_CRITERION_ZSCORE )
    if ( self.parameterNode.GetAttribute( "NumericalLevel" ) is None ):
      self.parameterNode.SetAttribute( "NumericalLevel", str( 0 ) )
    if ( self.parameterNode.GetAttribute( "ComparisonMethod" ) is None ):
      self.parameterNode.SetAttribute( "ComparisonMethod", COMPARISON_METHOD_MEAN )

    if ( self.parameterNode is not None ):
      self.parameterNodeObserverTag = self.parameterNode.AddObserver( vtk.vtkCommand.ModifiedEvent, self.updateWidgetFromParameterNode )
    self.updateWidgetFromParameterNode()
    
    
  def getParameterNode( self ):
    return self.parameterNode
    
    
  def onBenchmarkCriterionChanged( self, text ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "BenchmarkCriterion", text )
    
    
  def onNumericalLevelChanged( self, value ):
    if ( self.parameterNode is None ):
      return
    self.parameterNode.SetAttribute( "NumericalLevel", str( value ) )
    
    
  def onComparisonMethodChanged( self, text ):
    if ( self.parameterNode is None ):
      return      
    self.parameterNode.SetAttribute( "ComparisonMethod", text )
    

  def updateWidgetFromParameterNode( self, node = None, eventID = None ):
    if ( self.parameterNode is None ):
      return

    benchmarkCriterionIndex = self.benchmarkCriterionComboBox.findText( self.parameterNode.GetAttribute( "BenchmarkCriterion" ) )
    if ( benchmarkCriterionIndex >= 0 ):
      self.benchmarkCriterionComboBox.setCurrentIndex( benchmarkCriterionIndex )
      
    self.numericalLevelSpinBox.setValue( float( self.parameterNode.GetAttribute( "NumericalLevel" ) ) )
    
    comparisonMethodIndex = self.comparisonMethodComboBox.findText( self.parameterNode.GetAttribute( "ComparisonMethod" ) )
    if ( comparisonMethodIndex >= 0 ):
      self.comparisonMethodComboBox.setCurrentIndex( comparisonMethodIndex )
    

#
# Linear Combination Assessment
#

class BenchmarkAssessment():


  def __init__( self ):
    pass

    
  @staticmethod
  def GetGenericDescription():
    descriptionString = "This assessment method compares the metric values to the benchmarks derived from the training data." + "\n\n"
    return descriptionString
    
    
  @staticmethod
  def GetSpecificDescription( scaledTestRecord, nameRecord ):
    descriptionString = "In this case, the metrics furthest from satisfying the benchmarks were (in descending order): " + "\n\n"
    sortedRecord = sorted( zip( scaledTestRecord, nameRecord ), reverse = True )
    for currScaledTestRecord, currNameRecord in sortedRecord:
      descriptionString = descriptionString + currNameRecord + " (scaled value = " + str( currScaledTestRecord ) + ")" + "\n"
      
    return descriptionString

    
  @staticmethod 
  def ComputeSkill( parameterNode, testRecord, trainingRecords, weights, nameRecord, nameLabels, skillLabels ):
    benchmarkCriterion = parameterNode.GetAttribute( "BenchmarkCriterion" )
    numericalLevel = float( parameterNode.GetAttribute( "NumericalLevel" ) )
    comparisonMethod = parameterNode.GetAttribute( "ComparisonMethod" )
  
    scaledTestRecord = BenchmarkAssessment.GetScaledRecord( testRecord, trainingRecords, benchmarkCriterion )
    benchmarkedTestRecord = BenchmarkAssessment.GetBenchmarkedRecord( scaledTestRecord, numericalLevel )
    combination = BenchmarkAssessment.GetComparedSkillScore( benchmarkedTestRecord, weights, comparisonMethod )
    
    descriptionString = BenchmarkAssessment.GetGenericDescription() + BenchmarkAssessment.GetSpecificDescription( scaledTestRecord, nameRecord )
    
    return combination, descriptionString
    
  @staticmethod
  def GetComparedSkillScore( benchmarkedRecord, weights, comparisonMethod ):
    if ( len( benchmarkedRecord ) != len( weights ) ):
      logging.info( "BenchmarkAssessment::GetComparedSkillScore: Metric values and weights do not correspond." )
      return 0
      
    if ( comparisonMethod == COMPARISON_METHOD_MEAN ):
      return BenchmarkAssessment.GetWeightedMean( benchmarkedRecord, weights )
    if ( comparisonMethod == COMPARISON_METHOD_MEDIAN ):
      return BenchmarkAssessment.GetWeightedMedian( benchmarkedRecord, weights )
    if ( comparisonMethod == COMPARISON_METHOD_MAXIMUM ):
      return BenchmarkAssessment.GetMaximum( benchmarkedRecord, weights )
    if ( comparisonMethod == COMPARISON_METHOD_MINIMUM ):
      return BenchmarkAssessment.GetMinimum( benchmarkedRecord, weights )    
    
    logging.info( "BenchmarkAssessment::GetComparedSkillScore: Metric comparison method improperly specified." )
    return 0
 
 
  @staticmethod
  def GetWeightedMean( benchmarkedRecord, weights ):
    valueSum = 0
    weightSum = 0
  
    for i in range( len( benchmarkedRecord ) ):
      currMetricValue = benchmarkedRecord[ i ]
      currWeight = weights[ i ]
      
      valueSum += currMetricValue * currWeight
      weightSum += currWeight

    if ( weightSum == 0.0 ):
      return 0.0

    return float( valueSum ) / weightSum


  @staticmethod
  def GetWeightedMedian( benchmarkedRecord, weights ):
    sortedPairs = sorted( zip( benchmarkedRecord, weights ) )
  
    totalWeight = sum( weights )
    weightSum = 0
    for pair in sortedPairs:
      weightSum += pair[ 1 ]
      if ( weightSum > 0.5 * totalWeight ):
        return pair[ 0 ]
      
    return benchmarkedRecord[-1]

  @staticmethod
  def GetMaximum( benchmarkedRecord, weights ):
    return max( benchmarkedRecord ) 
    
    
  @staticmethod
  def GetMinimum( benchmarkedRecord, weights ):
    return min( benchmarkedRecord ) 
      
      
  @staticmethod
  def GetBenchmarkedRecord( scaledRecord, numericalLevel ):
    benchmarkedRecord = []
    for i in range( len( scaledRecord ) ):
      benchmarkedRecord.append( int( scaledRecord[ i ] > numericalLevel ) )
      
    return benchmarkedRecord
    
    
  @staticmethod 
  def GetScaledRecord( testRecord, trainingRecords, benchmarkCriterion ):
    # Assume that all the records are of the same length
    scaledRecord = []
    for i in range( len( testRecord ) ):
      currTrainingVector = [] # A list of all values from the same attribute (i.e. metric/task pair)
      for currTrainingRecord in trainingRecords:
        currTrainingVector.append( currTrainingRecord[ i ] )

      currScaled = None
      if ( benchmarkCriterion == BENCHMARK_CRITERION_ZSCORE ):
        currScaled = BenchmarkAssessment.GetZScore( testRecord[ i ], currTrainingVector )
      if ( benchmarkCriterion == BENCHMARK_CRITERION_PERCENTILE ):
        currScaled = BenchmarkAssessment.GetPercentile( testRecord[ i ], currTrainingVector )
      
      if ( currScaled is None ):
        logging.info( "BenchmarkAssessment::GetConvertedMetricValue: Metric benchmark criterion improperly specified." )
        continue
        
      scaledRecord.append( currScaled )
    
    return scaledRecord
    

  # This assumes rank 0 is the smallest value and rank N is the largest value
  @staticmethod
  def GetPercentile( testMetric, trainingMetrics ):
    tiedRank = 0
    for dataPoint in trainingMetrics:
      if ( testMetric > dataPoint ):
        tiedRank = tiedRank + 1
      if ( testMetric == dataPoint ):
        tiedRank = tiedRank + 0.5
        
    return float( tiedRank ) / len( trainingMetrics )


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
    return 0.5 # Good metrics will always return 0, bad metrics always 1