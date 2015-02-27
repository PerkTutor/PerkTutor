import MembershipFunction
import BinaryFunction

# This script will contain a selection of defuzzification methods

# TODO: Should this should be the only externally called method?
# TODO: Reset compose function for input membership functions
def Defuzzify( membershipFunction, method ):
  if ( method == "COA" ):
    return DefuzzifyCOA( membershipFunction )
  if ( method == "COM" ):
    return DefuzzifyCOM( membershipFunction )
  if ( method == "MOM" ):
    return DefuzzifyMOM( membershipFunction )
  if ( method == "CMCOA" ):
    return DefuzzifyCMCOA( membershipFunction )
  if ( method == "CMCOM" ):
    return DefuzzifyCMCOM( membershipFunction )
  if ( method == "CMMOM" ):
    return DefuzzifyCMMOM( membershipFunction )
    
    
    
def Integrate( function ):
  
  min = -100
  max = 200
  
  loc = min
  step = 1e-2
  integral = 0
  
  while( loc < max ):
    integral += function.Evaluate( loc + step / 2 ) * step
    loc += step
    
  return integral
  
  
def MaxValue( function ):
  
  min = -100
  max = 200
  
  loc = min
  step = 1e-2
  maxVal = 0
  
  while( loc < max ):
    if ( function.Evaluate( loc ) > maxVal ):
      maxVal = function.Evaluate( loc )
    loc += step
    
  return maxVal

  
def DefuzzifyCOA( membershipFunction ):
  
  membershipFunction.SetComposeFunction( BinaryFunction.GodelSNorm() )
  
  numeratorFunction = MembershipFunction.MembershipFunction()
  numeratorFunction.AddBaseFunction( membershipFunction )
  numeratorFunction.AddBaseFunction( XMembershipFunction() )
  numeratorFunction.SetComposeFunction( BinaryFunction.GoguenTNorm() ) # Multiply x * func and integrate
  
  denominatorFunction = MembershipFunction.MembershipFunction()
  denominatorFunction.AddBaseFunction( membershipFunction )
  
  num = Integrate( numeratorFunction )  
  denom = Integrate( denominatorFunction )

  return num / denom


  
def DefuzzifyCOM( membershipFunction ):
  
  membershipFunction.SetComposeFunction( AddBinaryFunction() )
  
  numeratorFunction = MembershipFunction.MembershipFunction()
  numeratorFunction.AddBaseFunction( membershipFunction )
  numeratorFunction.AddBaseFunction( XMembershipFunction() )
  numeratorFunction.SetComposeFunction( BinaryFunction.GoguenTNorm() ) # Multiply x * func and integrate
  
  denominatorFunction = MembershipFunction.MembershipFunction()
  denominatorFunction.AddBaseFunction( membershipFunction )
  
  num = Integrate( numeratorFunction )  
  denom = Integrate( denominatorFunction )

  return num / denom
  
  
def DefuzzifyMOM( membershipFunction ):
  
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
  
  num = Integrate( numeratorFunction )  
  denom = Integrate( denominatorFunction )

  return num / denom
  

def DefuzzifyCMCOA( membershipFunction ):  
  coa = DefuzzifyCOA( membershipFunction )
  return DefuzzifyCM( membershipFunction, coa )
  
  
def DefuzzifyCMCOM( membershipFunction ):  
  com = DefuzzifyCOM( membershipFunction )
  return DefuzzifyCM( membershipFunction, com )
  
  
def DefuzzifyCMMOM( membershipFunction ):  
  mom = DefuzzifyMOM( membershipFunction )
  return DefuzzifyCM( membershipFunction, mom )
   
    
def DefuzzifyCM( membershipFunction, start ):

  maxVal = MaxValue( membershipFunction )
  
  locPlus = start
  locMinus = start
  step = 1e-2
    
  while( True ):
    if ( round( membershipFunction.Evaluate( locPlus ) - maxVal, 4 ) == 0 ):
      return locPlus
    if ( round( membershipFunction.Evaluate( locMinus ) - maxVal, 4 ) == 0 ):
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