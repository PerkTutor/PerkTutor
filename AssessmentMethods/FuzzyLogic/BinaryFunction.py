import math

# Class for membership functions
class BinaryFunction:

  # Constructor
  def __init__( self ):
    pass
    
  def Copy( self, other ):
    pass

  # "Pure virtual" method to evaluate the function at a particular value
  def Evaluate( self, x, y ):
    raise NotImplementedError( "Evaluate function not implemented in base class." )
    
    
# Now implement some useful t-norms and s-norms (of course, more can be defined later)
class GodelTNorm( BinaryFunction ):
  def Evaluate( self, x, y ):
    return min( x, y )

    
class GodelSNorm( BinaryFunction ):
  def Evaluate( self, x, y ):
    return max( x, y )
    
    
class GoguenTNorm( BinaryFunction ):
  def Evaluate( self, x, y ):
    return x * y

    
class GoguenSNorm( BinaryFunction ):
  def Evaluate( self, x, y ):
    return x + y - x * y
    
    
class LukasiewiczTNorm( BinaryFunction ):
  def Evaluate( self, x, y ):
    return max( 0, x + y - 1 )

    
class LukasiewiczSNorm( BinaryFunction ):
  def Evaluate( self, x, y ):
    return min( 1, x + y )
    
    
class NilpotentTNorm( BinaryFunction ):
  def Evaluate( self, x, y ):
    if ( x + y > 1 ):
      return min( x, y )
    else:
      return 0

    
class NilpotentSNorm( BinaryFunction ):
  def Evaluate( self, x, y ):
    if ( x + y < 1 ):
      return max( x, y )
    else:
      return 1
      
      
class DrasticTNorm( BinaryFunction ):
  def Evaluate( self, x, y ):
    if ( max( x, y ) == 1 ):
      return min( x, y )
    else:
      return 0

    
class DrasticSNorm( BinaryFunction ):
  def Evaluate( self, x, y ):
    if ( min( x, y ) == 0 ):
      return max( x, y )
    else:
      return 1