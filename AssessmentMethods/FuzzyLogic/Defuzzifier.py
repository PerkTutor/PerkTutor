import MembershipFunction
import BinaryFunction
import math


# Class for defuzzifiers
# TODO: Reset compose function for input membership functions
class Defuzzifier:

  # Constructor
  def __init__( self ):
    pass
    
    
  def Copy( self, other ):
    pass
    
    
  # Helper function for calculating defuzzification
  def Integrate( self, function, min, max, step ):
    loc = min
    integral = 0
  
    while( loc < max ):
      integral += function.Evaluate( loc + step / 2 ) * step
      loc += step
    
    return integral
    
    
  def MaximumValue( self, function, min, max, step ):
    loc = min
    maxVal = - float( 'inf' ) # In theory, this can be zero since the max value should always be at least zero (but let's make it more robust in practice)
  
    while( loc < max ):
      if ( function.Evaluate( loc ) > maxVal ):
        maxVal = function.Evaluate( loc )
      loc += step
    
    return maxVal
  
  def ClosestMaximum( self, membershipFunction, start, min, max, step ):

    maxVal = self.MaximumValue( membershipFunction, min, max, step )
  
    locPlus = start
    locMinus = start
    precision = - math.floor( math.log10( step ) )
    
    # This is guaranteed to terminate
    while( True ):
      if ( round( membershipFunction.Evaluate( locPlus ) - maxVal, precision ) == 0 ):
        return locPlus
      if ( round( membershipFunction.Evaluate( locMinus ) - maxVal, precision ) == 0 ):
        return locMinus
      
      locPlus += step
      locMinus -= step
  

  # "Pure virtual" method to evaluate a particular defuzzification of the function
  def Evaluate( self, membershipFunction, min, max, step ):
    raise NotImplementedError( "Evaluate function not implemented in base class." )


  
class DefuzzifierCOA( Defuzzifier ):
  
  def Evaluate( self, membershipFunction, min, max, step ):
  
    membershipFunction.SetComposeFunction( BinaryFunction.GodelSNorm() )
  
    numeratorFunction = MembershipFunction.MembershipFunction()
    numeratorFunction.AddBaseFunction( membershipFunction )
    numeratorFunction.AddBaseFunction( XMembershipFunction() )
    numeratorFunction.SetComposeFunction( BinaryFunction.GoguenTNorm() ) # Multiply x * func and integrate
  
    denominatorFunction = MembershipFunction.MembershipFunction()
    denominatorFunction.AddBaseFunction( membershipFunction )
  
    num = self.Integrate( numeratorFunction, min, max, step )  
    denom = self.Integrate( denominatorFunction, min, max, step )

    return num / denom


  
class DefuzzifierCOM( Defuzzifier ):
  
  def Evaluate( self, membershipFunction, min, max, step ):
  
    membershipFunction.SetComposeFunction( AddBinaryFunction() )
  
    numeratorFunction = MembershipFunction.MembershipFunction()
    numeratorFunction.AddBaseFunction( membershipFunction )
    numeratorFunction.AddBaseFunction( XMembershipFunction() )
    numeratorFunction.SetComposeFunction( BinaryFunction.GoguenTNorm() ) # Multiply x * func and integrate
  
    denominatorFunction = MembershipFunction.MembershipFunction()
    denominatorFunction.AddBaseFunction( membershipFunction )
  
    num = self.Integrate( numeratorFunction, min, max, step )  
    denom = self.Integrate( denominatorFunction, min, max, step )

    return num / denom
  
  
class DefuzzifierMOM( Defuzzifier ):
  
  def Evaluate( self, membershipFunction, min, max, step ):
  
    membershipFunction.SetComposeFunction( BinaryFunction.GodelSNorm() )
  
    # Find the maximum value of the membership function
    flatMaxFunction = MembershipFunction.FlatMembershipFunction()
    flatMaxFunction.SetParameters( [ self.MaximumValue( membershipFunction, min, max, step ) ] )
  
    # This function is the max value when the original membership function achieves its max, and is zero otherwise
    maxFunction = MembershipFunction.MembershipFunction()
    maxFunction.AddBaseFunction( flatMaxFunction )
    maxFunction.AddBaseFunction( membershipFunction )
    maxFunction.SetComposeFunction( EqualBinaryFunction() )
  
    numeratorFunction = MembershipFunction.MembershipFunction()
    numeratorFunction.AddBaseFunction( maxFunction )
    numeratorFunction.AddBaseFunction( XMembershipFunction() )
    numeratorFunction.SetComposeFunction( BinaryFunction.GoguenTNorm() ) # Multiply x * func and integrate
  
    denominatorFunction = MembershipFunction.MembershipFunction()
    denominatorFunction.AddBaseFunction( maxFunction )
  
    num = self.Integrate( numeratorFunction, min, max, step )  
    denom = self.Integrate( denominatorFunction, min, max, step )

    return num / denom
  

class DefuzzifierCMCOA( Defuzzifier ):
  
  def Evaluate( self, membershipFunction, min, max, step ):
  
    coaDefuzzifier = DefuzzifierCOA()
    coa = coaDefuzzifier.Evaluate( membershipFunction, min, max, step )
    
    return self.ClosestMaximum( membershipFunction, coa, min, max, step )
  

class DefuzzifierCMCOM( Defuzzifier ):
  
  def Evaluate( self, membershipFunction, min, max, step ):
  
    comDefuzzifier = DefuzzifierCOM()
    com = comDefuzzifier.Evaluate( membershipFunction, min, max, step )
    
    return self.ClosestMaximum( membershipFunction, com, min, max, step )
    
    
class DefuzzifierCMMOM( Defuzzifier ):
  
  def Evaluate( self, membershipFunction, min, max, step ):
  
    momDefuzzifier = DefuzzifierMOM()
    mom = momDefuzzifier.Evaluate( membershipFunction, min, max, step )
    
    return self.ClosestMaximum( membershipFunction, mom, min, max, step )
    

  
# Create some new membership functions specifically for defuzzification purposes
class XMembershipFunction( MembershipFunction.MembershipFunction ):

  def Evaluate( self, value ):
    if ( len( self.Parameters ) != 0 ):
      raise Exception( "Improperly specified parameters" )
    
    return value
    
    
# Create some new binary function for centre of area calculation
# Note: This is not a t-norm nor s-norm
class AddBinaryFunction( BinaryFunction.BinaryFunction ):
  def Evaluate( self, x, y ):
    return x + y
    
# Create some new binary function for mean of max
# Note: This is not a t-norm nor s-norm
class EqualBinaryFunction( BinaryFunction.BinaryFunction ):
  def Evaluate( self, x, y ):
    if ( round( x - y, 4 ) == 0 ):
      return ( x + y ) / float( 2 )
    else:
      return 0