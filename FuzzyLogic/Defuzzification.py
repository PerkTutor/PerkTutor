import MembershipFunction
import BinaryFunction

# This script will contain a selection of defuzzification methods

# TODO: Should this should be the only externally called method?
# TODO: Reset compose function for input membership functions
def Defuzzify( membershipFunction, method, min, max, step ):
  if ( method == "COA" ):
    return DefuzzifyCOA( membershipFunction, min, max, step )
  if ( method == "COM" ):
    return DefuzzifyCOM( membershipFunction, min, max, step )
  if ( method == "MOM" ):
    return DefuzzifyMOM( membershipFunction, min, max, step )
  if ( method == "CMCOA" ):
    return DefuzzifyCMCOA( membershipFunction, min, max, step )
  if ( method == "CMCOM" ):
    return DefuzzifyCMCOM( membershipFunction, min, max, step )
  if ( method == "CMMOM" ):
    return DefuzzifyCMMOM( membershipFunction, min, max, step )
    
    
    
def Integrate( function, min, max, step ):
  
  loc = min
  integral = 0
  
  while( loc < max ):
    integral += function.Evaluate( loc + step / 2 ) * step
    loc += step
    
  return integral
  
  
def MaxValue( function, min, max, step ):
  
  loc = min
  maxVal = - float( 'inf' ) # In theory, this can be zero since the max value should always be at least zero (but let's make it more robust in practice)
  
  while( loc < max ):
    if ( function.Evaluate( loc ) > maxVal ):
      maxVal = function.Evaluate( loc )
    loc += step
    
  return maxVal

  
def DefuzzifyCOA( membershipFunction, min, max, step ):
  
  membershipFunction.SetComposeFunction( BinaryFunction.GodelSNorm() )
  
  numeratorFunction = MembershipFunction.MembershipFunction()
  numeratorFunction.AddBaseFunction( membershipFunction )
  numeratorFunction.AddBaseFunction( XMembershipFunction() )
  numeratorFunction.SetComposeFunction( BinaryFunction.GoguenTNorm() ) # Multiply x * func and integrate
  
  denominatorFunction = MembershipFunction.MembershipFunction()
  denominatorFunction.AddBaseFunction( membershipFunction )
  
  num = Integrate( numeratorFunction, min, max, step )  
  denom = Integrate( denominatorFunction, min, max, step )

  return num / denom


  
def DefuzzifyCOM( membershipFunction, min, max, step ):
  
  membershipFunction.SetComposeFunction( AddBinaryFunction() )
  
  numeratorFunction = MembershipFunction.MembershipFunction()
  numeratorFunction.AddBaseFunction( membershipFunction )
  numeratorFunction.AddBaseFunction( XMembershipFunction() )
  numeratorFunction.SetComposeFunction( BinaryFunction.GoguenTNorm() ) # Multiply x * func and integrate
  
  denominatorFunction = MembershipFunction.MembershipFunction()
  denominatorFunction.AddBaseFunction( membershipFunction )
  
  num = Integrate( numeratorFunction, min, max, step )  
  denom = Integrate( denominatorFunction, min, max, step )

  return num / denom
  
  
def DefuzzifyMOM( membershipFunction, min, max, step ):
  
  membershipFunction.SetComposeFunction( BinaryFunction.GodelSNorm() )
  
  # Find the maximum value of the membership function
  flatMaxFunction = MembershipFunction.FlatMembershipFunction()
  flatMaxFunction.SetParameters( [ MaxValue( membershipFunction ) ] )
  
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
  
  num = Integrate( numeratorFunction, min, max, step )  
  denom = Integrate( denominatorFunction, min, max, step )

  return num / denom
  

def DefuzzifyCMCOA( membershipFunction, min, max, step ):  
  coa = DefuzzifyCOA( membershipFunction, min, max, step )
  return DefuzzifyCM( membershipFunction, coa, step )
  
  
def DefuzzifyCMCOM( membershipFunction, min, max, step ):  
  com = DefuzzifyCOM( membershipFunction, min, max, step )
  return DefuzzifyCM( membershipFunction, com, step )
  
  
def DefuzzifyCMMOM( membershipFunction, min, max, step ):  
  mom = DefuzzifyMOM( membershipFunction, min, max, step )
  return DefuzzifyCM( membershipFunction, mom, step )
   
    
def DefuzzifyCM( membershipFunction, start, step ):

  maxVal = MaxValue( membershipFunction )
  
  locPlus = start
  locMinus = start
  precision = - math.floor( math.log10( step ) )
    
  while( True ):
    if ( round( membershipFunction.Evaluate( locPlus ) - maxVal, precision ) == 0 ):
      return locPlus
    if ( round( membershipFunction.Evaluate( locMinus ) - maxVal, precision ) == 0 ):
      return locMinus
      
    locPlus += step
    locMinus -= step

  
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