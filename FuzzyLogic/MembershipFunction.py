import math


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
      raise Exception( "Improperly specified parameters" )
    
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
      raise Exception( "Improperly specified parameters" )
    
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
      raise Exception( "Improperly specified parameters" )
    
    return math.exp( - math.pow( value - self.Parameters[ 0 ], 2 ) / float( 2 * math.pow( self.Parameters[ 1 ], 2 ) ) )
    
    
class FlatMembershipFunction( MembershipFunction ):

  def Evaluate( self, value ):
    if ( len( self.Parameters ) != 1 ):
      raise Exception( "Improperly specified parameters" )
    
    return self.Parameters[ 0 ]