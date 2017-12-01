import math
import logging


# Class for membership functions
class MembershipFunction:

  # Constructor
  def __init__( self ):
    self.Parameters = []
    self.BaseFunctions = [] # An array of membership functions on which this can depend
    self.ComposeFunction = None # A function that takes two inputs and produces an output
    
  def Copy( self, other ):
    for base in range( other.BaseFunctions ):
      copyBaseFunction = MembershipFunction()
      copyBaseFunction.Copy( base )
      self.BaseFunctions.append( copyBaseFunctionA )
    if ( other.ComposeFunction != None ):
      copyComposeFunction = MembershipFunction()
      copyComposeFunction.Copy( other.ComposeFunction )
      self.ComposeFunction = copyComposeFunction
      
    self.Parameters = other.Parameters[:]

  # Setting the base function
  def AddBaseFunction( self, newBaseFunction ):
    self.BaseFunctions.append( newBaseFunction )
    
  # Setting the compose function
  def SetComposeFunction( self, newComposeFunction ):
    self.ComposeFunction = newComposeFunction
    
  # Setting the parameters
  def SetParameters( self, newParameters ):
    self.Parameters = newParameters
    
  # "Pure virtual" method to evaluate the function at a particular value
  def Evaluate( self, value, start = 0 ):
    if ( len( self.BaseFunctions ) == start ):
      return 0
      
    if ( len( self.BaseFunctions ) == ( start + 1 ) ):
      return self.BaseFunctions[ start ].Evaluate( value )
      
    if ( self.ComposeFunction == None ):
      return 0
      
    return self.ComposeFunction.Evaluate( self.BaseFunctions[ start ].Evaluate( value ), self.Evaluate( value, start + 1 ) )
    
    
# Now implement some useful membership functions (of course, more can be defined later)
class TriangleMembershipFunction( MembershipFunction ):

  def Evaluate( self, value ):
    if ( len( self.Parameters ) != 3 ):
      logging.warning( "TriangleMembershipFunction::Evaluate: Improperly specified parameters." )
      return 0
    
    if ( value <= self.Parameters[ 0 ] ):
      return 0
    if ( value > self.Parameters[ 0 ] and value <= self.Parameters[ 1 ] ):
      return ( value - self.Parameters[ 0 ] ) / float( self.Parameters[ 1 ] - self.Parameters[ 0 ] )
    if ( value >= self.Parameters[ 1 ] and value < self.Parameters[ 2 ] ):
      return 1 - ( value - self.Parameters[ 1 ] ) / float( self.Parameters[ 2 ] - self.Parameters[ 1 ] )
    if ( value >= self.Parameters[ 2 ] ):
      return 0
      
    return 0
    
class TrapezoidMembershipFunction( MembershipFunction ):

  def Evaluate( self, value ):
    if ( len( self.Parameters ) != 4 ):
      logging.warning( "TrapezoidMembershipFunction::Evaluate: Improperly specified parameters." )
      return 0
    
    if ( value <= self.Parameters[ 0 ] ):
      return 0
    if ( value > self.Parameters[ 0 ] and value < self.Parameters[ 1 ] ):
      return ( value - self.Parameters[ 0 ] ) / float( self.Parameters[ 1 ] - self.Parameters[ 0 ] )
    if ( value > self.Parameters[ 1 ] and value < self.Parameters[ 2 ] ):
      return 1
    if ( value > self.Parameters[ 2 ] and value < self.Parameters[ 3 ] ):
      return 1 - ( value - self.Parameters[ 2 ] ) / float( self.Parameters[ 3 ] - self.Parameters[ 2 ] )
    if ( value >= self.Parameters[ 3 ] ):
      return 0
      
    return 0
    
    
class GaussianMembershipFunction( MembershipFunction ):

  def Evaluate( self, value ):
    if ( len( self.Parameters ) != 2 ):
      logging.warning( "GaussianMembershipFunction::Evaluate: Improperly specified parameters." )
      return 0
    if ( self.Parameters[ 1 ] == 0 ):
      logging.warning( "GaussianMembershipFunction::Evaluate: Gaussian has standard deviation zero." )
      return 0
    
    return math.exp( - math.pow( value - self.Parameters[ 0 ], 2 ) / float( 2 * math.pow( self.Parameters[ 1 ], 2 ) ) )
    
    
class FlatMembershipFunction( MembershipFunction ):

  def Evaluate( self, value ):
    if ( len( self.Parameters ) != 1 ):
      logging.warning( "FlatMembershipFunction::Evaluate: Improperly specified parameters." )
      return 0
    
    return self.Parameters[ 0 ]
    
    
class GaussianKDEMembershipFunction( MembershipFunction ):
  
  def Evaluate( self, value ):
    if ( len( self.Parameters ) % 2 != 1 ):
      logging.warning( "GaussianKDEMembershipFunction::Evaluate: Improperly specified parameters." )
      return 0
    if ( self.Parameters[ 0 ] == 0 ):
      logging.warning( "GaussianKDEMembershipFunction::Evaluate: Bandwidth is zero." )
      return 0    
    # The zeroth parameter should be 'h'
    # All odd number parameters are datapoints, all even number parameters are weights
    
    h = float( self.Parameters[ 0 ] )
    n = float( ( len( self.Parameters ) - 1 ) / 2 ) # number of datapoints
    parameterIndex = 1
    value = 0
    while ( parameterIndex < len( self.Parameters ) ):
      dataPoint = self.Parameters[ parameterIndex ]
      weight = self.Parameters[ parameterIndex + 1 ]
      value = value + weight * math.exp( - math.pow( ( value - dataPoint ) / h, 2 ) )
      parameterIndex = parameterIndex + 2
      
    return value / n