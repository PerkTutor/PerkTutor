from .MembershipFunction import *

# Class for membership functions
class FuzzyRule:

  # Constructor
  def __init__( self ):
    self.OutputMembershipFunction = None # The output membership function for the rule
    self.InputMembershipFunctions = dict() # An dict of input membership functions for the rule
    self.ComposeFunction = None # How to put together input functions. Should be a t-norm or s-norm
    
  def Copy( self, other ):
    if ( other.OutputMembershipFunction != None ):
      copyOutputMembershipFunction = MembershipFunction()
      copyOutputMembershipFunction.Copy( other.OutputMembershipFunction )
      self.OutputMembershipFunction = copyOutputMembershipFunction
    for name in other.InputMembershipFunctions:
      copyInputMembershipFunction = MembershipFunction()
      copyInputMembershipFunction.Copy( other.InputMembershipFunctions[ name ] )
      self.InputMembershipFunctions[ name ] = copyInputMembershipFunction
    if ( other.ComposeFunction != None ):
      copyComposeFunction = MembershipFunction()
      copyComposeFunction.Copy( other.ComposeFunction )
      self.ComposeFunction = copyComposeFunction

      
  # Setting the base function
  # This will be of the form "IF inputName is inputGroup THEN ..."
  def AddInputMembershipFunction( self, newInputMembershipFunction, inputName ):
    if ( inputName not in self.InputMembershipFunctions ):
      self.InputMembershipFunctions[ inputName ] = MembershipFunction()
      self.InputMembershipFunctions[ inputName ].SetComposeFunction( self.ComposeFunction )
      
    self.InputMembershipFunctions[ inputName ].AddBaseFunction( newInputMembershipFunction )
    # No need to deal with the group - it is handled by composing all functions with the same name
    
  # This will be of the form "... THEN output is function"
  def SetOutputMembershipFunction( self, newOutputMembershipFunction ):
    self.OutputMembershipFunction = newOutputMembershipFunction
    
  # Setting the compose function
  def SetComposeFunction( self, newComposeFunction ):
    self.ComposeFunction = newComposeFunction
    
  # Find the output membership function clipped or scaled by input membership function values
  # Input is a dict with keys being function names and values being specific values
  def Evaluate( self, inputValues, transformOutputFunction ):
    if ( self.ComposeFunction == None or self.OutputMembershipFunction == None ):
      emptyMembershipFunction = FlatMembershipFunction()
      emptyMembershipFunction.SetParameters( [ 0 ] )
      return emptyMembershipFunction
      
    # Find the input membership values
    totalMembership = 1 - self.ComposeFunction.Evaluate( 0, 1 ) # This yields 1 for t-norm and 0 for s-norm
    ruleUsed = False
    
    for name in inputValues:
      if ( name not in self.InputMembershipFunctions ):
        continue
      
      currentMembership = self.InputMembershipFunctions[ name ].Evaluate( inputValues[ name ] )
      totalMembership = self.ComposeFunction.Evaluate( totalMembership, currentMembership )
      ruleUsed = True
      
    # Assign the total memership to be zero if the rule is never used
    if ( ruleUsed == False ):
      totalMembership = 0
      
    # Compose the flat membership function
    flatMembershipFunction = FlatMembershipFunction()
    flatMembershipFunction.SetParameters( [ totalMembership ] )
      
    # Apply clipping or scaling to output membership function
    transformedOutputMembershipFunction = MembershipFunction()
    transformedOutputMembershipFunction.AddBaseFunction( self.OutputMembershipFunction )
    transformedOutputMembershipFunction.AddBaseFunction( flatMembershipFunction )
    transformedOutputMembershipFunction.SetComposeFunction( transformOutputFunction )
      
    # Return the transformed output membership function
    return transformedOutputMembershipFunction
