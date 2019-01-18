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
    

class FlatMembershipFunction( MembershipFunction ):

  def Evaluate( self, value ):
    if ( len( self.Parameters ) != 1 ):
      logging.warning( "FlatMembershipFunction::Evaluate: Improperly specified parameters." )
      return 0
    
    return self.Parameters[ 0 ]


class GaussianMembershipFunction( MembershipFunction ):

  def Evaluate( self, value ):
    if ( len( self.Parameters ) != 3 ):
      logging.warning( "GaussianMembershipFunction::Evaluate: Improperly specified parameters." )
      return 0
    if ( self.Parameters[ 2 ] == 0 ):
      logging.warning( "GaussianMembershipFunction::Evaluate: Gaussian has standard deviation zero." )
      return 0
    # The zeroth parameter is 'scale'
    # The first parameter is mean
    # The second parameter is standard deviation

    result = math.exp( - math.pow( value - self.Parameters[ 1 ], 2 ) / float( 2 * math.pow( self.Parameters[ 2 ], 2 ) ) )
    result = result / ( self.Parameters[ 2 ] * math.sqrt( 2 * math.pi ) ) # Necessarily integrates to one
    result = result * scale
    return result


class GaussianKDEMembershipFunction( MembershipFunction ):
  
  def Evaluate( self, value ):
    if ( len( self.Parameters ) % 2 != 0 ):
      logging.warning( "GaussianKDEMembershipFunction::Evaluate: Improperly specified parameters." )
      return 0
    if ( self.Parameters[ 1 ] == 0 ):
      logging.warning( "GaussianKDEMembershipFunction::Evaluate: Bandwidth is zero." )
      return 0
    # The zeroth parameter should be 'scale'
    # The first parameter should be 'h'
    # All even number parameters are datapoints, all odd number parameters are weights
    
    scale = float( self.Parameters[ 0 ] )
    h = float( self.Parameters[ 1 ] )
    dataPoints = self.Parameters[2::2] # every other parameter, skipping the 0th element (because it is bandwidth h)
    weights = self.Parameters[3::2] # every other parameter, skipping the 0th element (because it is bandwidth h)
    result = 0
    for i in range( len( dataPoints ) ):
      currDataPoint = dataPoints[ i ]
      currWeight = weights[ i ]
      result = result + currWeight * math.exp( - math.pow( ( value - currDataPoint ) / h, 2 ) )

    result = result / ( h * math.sqrt( math.pi ) ) # This makes it integrate to one. Note that the sum of weights is normalized to one already
    result = result * scale # Scale it
    return result