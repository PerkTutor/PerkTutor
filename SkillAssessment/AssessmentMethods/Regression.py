import os
import unittest
import vtk, qt, ctk, slicer
import numpy
import logging
import math

#
# Polynomial Regression Skill Assessment
#

#
# Polynomial Regression Parameters Widget
#

class RegressionParametersWidget( qt.QFrame ):

  def __init__( self, parent = None ):
    qt.QFrame.__init__( self )
  
    self.parameterNode = None

    #
    # Parameters area
    #    
    self.parametersLayout = qt.QFormLayout( self )
    self.setLayout( self.parametersLayout )
        
    #
    # Polynomial regression order spin box
    #    
    self.regressionOrderSpinBox = qt.QSpinBox()
    self.regressionOrderSpinBox.setRange( 1, 10 )
    self.regressionOrderSpinBox.setSingleStep( 1 )
    self.regressionOrderSpinBox.setToolTip( "Choose the order of polynomial fit for the regression model." )
    self.parametersLayout.addRow( "Order ", self.regressionOrderSpinBox )
    
    # connections
    self.regressionOrderSpinBox.connect( 'valueChanged(int)', self.onRegressionOrderChanged )
    
    
  def setParameterNode( self, parameterNode ):
    # Replace the old observers
    if ( self.parameterNode is not None ):
      self.parameterNode.RemoveObserver( self.parameterNodeObserverTag )
      
    self.parameterNode = parameterNode
  
    if ( self.parameterNode.GetAttribute( "RegressionOrder" ) is None ):
      self.parameterNode.SetAttribute( "RegressionOrder", str( 1 ) )

    if ( self.parameterNode is not None ):
      self.parameterNodeObserverTag = self.parameterNode.AddObserver( vtk.vtkCommand.ModifiedEvent, self.updateWidgetFromParameterNode )
    self.updateWidgetFromParameterNode()
    
    
  def getParameterNode( self ):
    return self.parameterNode
    
    
  def onRegressionOrderChanged( self, value ):
    if ( self.parameterNode is None ):
      return
    self.parameterNode.SetAttribute( "RegressionOrder", str( value ) )
    
    
  def updateWidgetFromParameterNode( self, node = None, eventID = None ):
    if ( self.parameterNode is None ):
      return

    self.regressionOrderSpinBox.setValue( int( self.parameterNode.GetAttribute( "RegressionOrder" ) ) )
    

#
# Nearest Neighbor Assessment
#

class RegressionAssessment():


  def __init__( self ):
    pass


  @staticmethod
  def GetGenericDescription():
    descriptionString = "This assessment method finds the optimal linear or polynomial regression to best fit the data." + "\n\n"
    return descriptionString
    
    
  @staticmethod
  def GetSpecificDescription( coeff, regressionOrder, nameRecord ):
    descriptionString = "In this case, the largest coefficients in the regression were (in descending order): " + "\n\n"
    coeffOrders = list( range( regressionOrder + 1 ) ) * len( nameRecord )
    coeffNames = [ name for name in nameRecord for repetitions in range( regressionOrder + 1 ) ]
    coeffNamesOrders = zip( coeffNames, coeffOrders )
    sortedCoeff = sorted( zip( coeff, coeffNamesOrders ), reverse = True )
    for currCoeff, currCoeffNameOrder in sortedCoeff:
      descriptionString = descriptionString + currCoeffNameOrder[ 0 ] + " (order = " + str( currCoeffNameOrder[ 1 ] ) +  "; coefficient = " + str( currCoeff ) + ")" + "\n"
      
    return descriptionString
    
    
  @staticmethod 
  def ComputeSkill( parameterNode, testRecord, trainingRecords, weights, nameRecord, nameLabels, skillLabels ):
    regressionOrder = int( parameterNode.GetAttribute( "RegressionOrder" ) )
  
    skillLabels = skillLabels[:] # Deep copy    
    vandermondeMatrix = RegressionAssessment.ComputeVandermondeMatrix( testRecord, trainingRecords, regressionOrder )
    # The inputted weights are ignored, as the individual metric weights have no effect on the coefficients
    coeff = RegressionAssessment.ComputeLeastSquaresCoefficients( vandermondeMatrix, skillLabels )
    
    testVandermondeMatrix = RegressionAssessment.ComputeVandermondeMatrix( testRecord, [ testRecord ], regressionOrder )
    score = numpy.dot( testVandermondeMatrix.squeeze(), coeff )
    
    # Enforce the score to be between 0 and 1
    score = max( 0, score )
    score = min( 1, score )
    
    descriptionString = RegressionAssessment.GetGenericDescription() + RegressionAssessment.GetSpecificDescription( coeff, regressionOrder, nameRecord )

    return score, descriptionString

    
  @staticmethod
  def ComputeVandermondeMatrix( testRecord, trainingRecords, regressionOrder ):
    vandermondeMatrix = numpy.zeros( ( len( trainingRecords ), len( testRecord ) * ( regressionOrder + 1 ) ) )
    for metricIndex in range( len( testRecord ) ):
      for recordIndex in range( len( trainingRecords ) ):
        for orderIndex in range( regressionOrder + 1 ):
          vandermondeColumnIndex = metricIndex * ( regressionOrder + 1 ) + orderIndex
          vandermondeMatrix[ recordIndex ][ vandermondeColumnIndex ] = math.pow( trainingRecords[ recordIndex ][ metricIndex ], orderIndex ) # [ 1, x, x^2, ..., 1, y, y^2, ..., 1, z, z^2, ... ]
      
    return vandermondeMatrix

    
  @staticmethod
  def ComputeLeastSquaresCoefficients( vandermondeMatrix, skillLabels ):
    leftMatrix = numpy.dot( numpy.transpose( vandermondeMatrix ), vandermondeMatrix )
    rightVector = numpy.dot( numpy.transpose( vandermondeMatrix ), skillLabels )
    coeff, _, _, _ = numpy.linalg.lstsq( leftMatrix, rightVector ) # Returns: solution, error, rank, singular values
    return coeff
    
    
    
  # The critical value for feedback
  @staticmethod
  def GetCriticalValue( parameterNode, skillLabels ):
    # Should be half the range of the labels      
    maxSkill = max( skillLabels )
    minSkill = min( skillLabels )
    criticalValue = minSkill + ( maxSkill - minSkill ) / 2.0

    return criticalValue