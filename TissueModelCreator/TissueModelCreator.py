import os, imp, glob, sys
import urllib, zipfile
import unittest
import logging
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *


#
# TissueModelCreator
#

class TissueModelCreator( ScriptedLoadableModule ):
  """Uses ScriptedLoadableModule base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def __init__( self, parent ):
    ScriptedLoadableModule.__init__( self, parent )
    self.parent.title = "Tissue Model Creator"
    self.parent.categories = [ "Perk Tutor" ]
    self.parent.dependencies = []
    self.parent.contributors = [ "Matthew S. Holden (PerkLab; Queen's University)" ]
    self.parent.helpText = """
    The purpose of the Tissue Model Creator module is to create a model based on a set of collected fiducial points. For help on how to use this module visit: <a href='http://www.perktutor.org/'>Perk Tutor</a>.
    """
    self.parent.acknowledgementText = """
    This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).
    """

#
# TissueModelCreatorWidget
#

class TissueModelCreatorWidget( ScriptedLoadableModuleWidget ):
  """Uses ScriptedLoadableModuleWidget base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setup( self ):
    ScriptedLoadableModuleWidget.setup( self )
    
    self.tmcLogic = TissueModelCreatorLogic()
    self.markupsNode = None
    
    #
    # Display Area
    #
    self.displayCollapsibleButton = ctk.ctkCollapsibleButton()
    self.displayCollapsibleButton.text = "Display"
    self.layout.addWidget( self.displayCollapsibleButton )

    # Layout within the dummy collapsible button
    self.displayFormLayout = qt.QFormLayout( self.displayCollapsibleButton )

    #
    # input fiducials selector
    #
    self.markupsSelector = slicer.qMRMLNodeComboBox()
    self.markupsSelector.nodeTypes = [ "vtkMRMLMarkupsFiducialNode" ]
    self.markupsSelector.addEnabled = False
    self.markupsSelector.removeEnabled = False
    self.markupsSelector.noneEnabled = True
    self.markupsSelector.showHidden = False
    self.markupsSelector.showChildNodeTypes = False
    self.markupsSelector.setMRMLScene( slicer.mrmlScene )
    self.markupsSelector.setToolTip( "Select the markup node for the algorithm." )
    self.displayFormLayout.addRow( "Input Markup ", self.markupsSelector )
    
    #
    # Output model selector
    #
    self.modelSelector = slicer.qMRMLNodeComboBox()
    self.modelSelector.nodeTypes = [ "vtkMRMLModelNode" ]
    self.modelSelector.addEnabled = True
    self.modelSelector.removeEnabled = True
    self.modelSelector.renameEnabled = True
    self.modelSelector.noneEnabled = True
    self.modelSelector.showHidden = False
    self.modelSelector.showChildNodeTypes = False
    self.modelSelector.setMRMLScene( slicer.mrmlScene )
    self.modelSelector.setToolTip( "Select the output model for the algorithm." )
    self.displayFormLayout.addRow( "Output Model ", self.modelSelector )
    
    #
    # Depth slider
    #
    self.depthSlider = ctk.ctkSliderWidget()
    self.depthSlider.maximum = 1000
    self.depthSlider.minimum = 1
    self.depthSlider.value = 100
    self.depthSlider.setToolTip( "Select the depth of the tissue." )
    self.displayFormLayout.addRow( "Depth (mm) ", self.depthSlider )

    #
    # Flip (ie flip) checkbox
    #
    self.flipCheckBox = qt.QCheckBox()
    self.flipCheckBox.setCheckState( False )
    self.flipCheckBox.setToolTip( "Flip the tissue so it is in the other direction." )
    self.flipCheckBox.setText( "Flip" )
    self.displayFormLayout.addRow( self.flipCheckBox )
    
    #
    # Fit the surface to a plane checkbox
    #
    self.planeCheckBox = qt.QCheckBox()
    self.planeCheckBox.setCheckState( False )
    self.planeCheckBox.setToolTip( "Force the tissue surface to a plane." )
    self.planeCheckBox.setText( "Plane" )
    self.displayFormLayout.addRow( self.planeCheckBox )

    #
    # Update Button
    #
    self.updateButton = ctk.ctkCheckablePushButton( self.displayCollapsibleButton )
    self.updateButton.setText( "Update" )
    self.updateButton.toolTip = "Update the tissue model."
    self.updateButton.enabled = True
    self.displayFormLayout.addRow( self.updateButton )
    
    #
    # Status Label
    #
    self.statusLabel = qt.QLabel( "" )
    self.statusLabel.toolTip = "Status of whether the tissue model was successfully created."
    self.statusLabel.enabled = True
    self.displayFormLayout.addRow( self.statusLabel )

    # connections
    self.updateButton.connect( 'clicked(bool)', self.updateTissueModel )
    self.updateButton.connect( 'checkBoxToggled(bool)', self.onUpdateButtonToggled )
    
    self.markupsSelector.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onMarkupsNodeChanged )
    
    self.modelSelector.connect( 'currentNodeChanged(vtkMRMLNode*)', self.onInputUpdated )
    self.depthSlider.connect( 'valueChanged(double)', self.onInputUpdated )
    self.flipCheckBox.connect( 'toggled(bool)', self.onInputUpdated )
    self.planeCheckBox.connect( 'toggled(bool)', self.onInputUpdated )
    
    
  def cleanup( self ):
    pass
    
    
  def onMarkupsNodeChanged( self, selectedMarkupsNode ):
    # Remove the old observer (if it exists)
    if ( self.markupsNode is not None ):
      self.markupsNode.RemoveObserver( self.markupsNodeObserverTag )
    
    # Observe the new nodes
    self.markupsNode = selectedMarkupsNode
    if ( self.markupsNode is not None ):
      self.markupsNodeObserverTag = self.markupsNode.AddObserver( vtk.vtkCommand.ModifiedEvent, self.onMarkupsNodeModified )
    
    self.onInputUpdated()
    
    
  def onMarkupsNodeModified( self, node, eventID ):
    self.onInputUpdated()
      
      
  def onInputUpdated( self ):
    if ( self.updateButton.isChecked() == True ):
      self.updateTissueModel()
    
    
  def onUpdateButtonToggled( self, toggled ):
    self.updateButton.setCheckable( toggled )
    self.updateButton.setChecked( toggled )
    
    if toggled:
      self.updateButton.setText( "Auto-Update" )
    else:
      self.updateButton.setText( "Update" )
      
    self.onInputUpdated()
    
    
  def updateTissueModel( self ):
    self.updateButton.setChecked( True ) # Always make it checked if auto-update is on, otherwise this will do nothing
    
    statusText = self.tmcLogic.UpdateTissueModel( self.markupsNode, self.modelSelector.currentNode(), self.depthSlider.value, self.flipCheckBox.checked, self.planeCheckBox.checked )
    self.statusLabel.setText( statusText )

	
#
# TissueModelCreatorLogic
#

class TissueModelCreatorLogic( ScriptedLoadableModuleLogic ):
  """This class should implement all the actual
  computation done by your module.  The interface
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget.
  Uses ScriptedLoadableModuleLogic base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """
   
  def __init__( self ):
    pass
    
    
  def UpdateTissueModel( self, markupsNode, modelNode, depth, flip, plane ):
    if ( markupsNode is None or modelNode is None ):
      return "Markups node or model node not properly specified."
      
    surfacePoints = vtk.vtkPoints()
    self.GetPointsFromMarkups( markupsNode, surfacePoints )
      
    # Use the 2D delaunay filter to get a model of the surface
    if plane:
      surfacePolyData = self.ComputePlanarSurfacePolyData( surfacePoints )
    else:
      surfacePolyData = self.ComputeSurfacePolyData( surfacePoints )
  
    # Compute the axes from the PCA
    normalAxis = [ 0, 0, 0 ]
    self.ComputeTissueNormal( surfacePoints, normalAxis )
    
    # Compute the deep poly data from the surface poly 
    deepPolyData = self.ComputeDeepPolyData( surfacePolyData, normalAxis, depth, flip )
    
    # Compute the side poly data that joins the surface and deep poly data
    surfaceBoundaryPoints = self.GetBoundaryPoints( surfacePolyData )
    deepBoundaryPoints = self.GetBoundaryPoints( deepPolyData )
    
    sidePolyData = self.JoinBoundaryPoints( surfaceBoundaryPoints, deepBoundaryPoints )
    
    # Put it all together
    tissuePolyData = self.JoinTissuePolyData( surfacePolyData, deepPolyData, sidePolyData )
    
    # Put it back in the model
    modelNode.SetAndObservePolyData( tissuePolyData )
    if ( modelNode.GetDisplayNode() is None ):
      modelNode.CreateDefaultDisplayNodes()
      
    if ( not self.IsPolyDataClosed( tissuePolyData ) ):
      return "Could not create closed tissue model."
      
    return "Success!"
    
    
  def ComputePlanarSurfacePolyData( self, surfacePoints ):  
    surfacePolyData = vtk.vtkPolyData()
    surfacePolyData.SetPoints( surfacePoints ) 
  
    comFilter = vtk.vtkCenterOfMass()
    comFilter.SetInputData( surfacePolyData )
    comFilter.SetUseScalarsAsWeights( False )
    comFilter.Update()
    com = comFilter.GetCenter()
    
    # Project all the points onto the plane
    normal = [ 0, 0, 0 ]
    self.ComputeTissueNormal( surfacePoints, normal )    
    
    planePoints = vtk.vtkPoints()
    for i in range( surfacePoints.GetNumberOfPoints() ):
      currPoint = [ 0, 0, 0 ]
      surfacePoints.GetPoint( i, currPoint )
      relativePoint = [ 0, 0, 0 ]
      vtk.vtkMath.Subtract( currPoint, com, relativePoint )
      normalLength = vtk.vtkMath.Dot( relativePoint, normal )
      normalComponent = normal[:]
      vtk.vtkMath.MultiplyScalar( normalComponent, normalLength )
      relativePlanePoint = [ 0, 0, 0 ]
      vtk.vtkMath.Subtract( relativePoint, normalComponent, relativePlanePoint )
      planePoint = [ 0, 0, 0 ]
      vtk.vtkMath.Add( com, relativePlanePoint, planePoint )
      planePoints.InsertNextPoint( planePoint )
      
    return self.ComputeSurfacePolyData( planePoints )
    
    """
    # Use the oriented bounding box
    corner = [ 0, 0, 0 ]
    maxVector = [ 0, 0, 0 ]
    midVector = [ 0, 0, 0 ]
    minVector = [ 0, 0, 0 ]
    size = [ 0, 0, 0 ]    
    obbFilter = vtk.vtkOBBTree()
    obbFilter.ComputeOBB( surfacePoints, corner, maxVector, midVector, minVector, size )
    
    normal = minVector[:]
    vtk.vtkMath().Normalize( normal )
        
    # Get the points defining the plane
    relativeCOM = [ 0, 0, 0 ]
    # Find the projection of the mean point in the minVector direction
    vtk.vtkMath().Subtract( com, corner, relativeCOM )
    minProjectionLength = vtk.vtkMath().Dot( relativeCOM, normal )
    minProjection = normal[:]
    vtk.vtkMath().MultiplyScalar( minProjection, minProjectionLength )
    
    # Find the points on the plane
    origin = [ 0, 0, 0 ]
    point1 = [ 0, 0, 0 ]
    point2 = [ 0, 0, 0 ]
    vtk.vtkMath().Add( corner, minProjection, origin )
    vtk.vtkMath().Add( origin, maxVector, point1 )
    vtk.vtkMath().Add( origin, midVector, point2 )
    
    # Construct the plane
    plane = vtk.vtkPlaneSource()
    plane.SetOrigin( origin )
    plane.SetPoint1( point1 )
    plane.SetPoint2( point2 )
    plane.Update()
    
    return plane.GetOutput()
    """

    
  def ComputeSurfacePolyData( self, surfacePoints ):   
    surfacePolyData = vtk.vtkPolyData()
    surfacePolyData.SetPoints( surfacePoints )  
  
    delaunay = vtk.vtkDelaunay2D()
    delaunay.SetProjectionPlaneMode( vtk.VTK_BEST_FITTING_PLANE )
    delaunay.SetInputData( surfacePolyData )
    delaunay.Update()
    
    surfaceCleaner = vtk.vtkCleanPolyData()
    surfaceCleaner.SetInputData( delaunay.GetOutput() )
    surfaceCleaner.Update()
    
    return surfaceCleaner.GetOutput()
    

  def ComputeTissueNormal( self, surfacePoints, normalAxis ):
    # Create arrays for the dataset
    arrayX = vtk.vtkDoubleArray()
    arrayX.SetNumberOfComponents( 1 )
    arrayX.SetName ( 'X' )
    arrayY = vtk.vtkDoubleArray()
    arrayY.SetNumberOfComponents( 1 )
    arrayY.SetName ( 'Y' )
    arrayZ = vtk.vtkDoubleArray()
    arrayZ.SetNumberOfComponents( 1 )
    arrayZ.SetName ( 'Z' )
    
    # Add the points to the double arrays
    for i in range( surfacePoints.GetNumberOfPoints() ):   
      currPoint = [ 0, 0, 0 ]
      surfacePoints.GetPoint( i, currPoint )      
      arrayX.InsertNextValue( currPoint[ 0 ] )
      arrayY.InsertNextValue( currPoint[ 1 ] ) 
      arrayZ.InsertNextValue( currPoint[ 2 ] )
    
    # Create a table for the dataset
    table = vtk.vtkTable()
    table.AddColumn( arrayX )
    table.AddColumn( arrayY )
    table.AddColumn( arrayZ )
    
    # Setting up the PCA
    pca = vtk.vtkPCAStatistics()
    pca.SetInputData( vtk.vtkStatisticsAlgorithm.INPUT_DATA, table )
    pca.SetColumnStatus( 'X', 1 )
    pca.SetColumnStatus( 'Y', 1 )
    pca.SetColumnStatus( 'Z', 1 )
    pca.RequestSelectedColumns()
    pca.SetDeriveOption( True )
    pca.Update()
    
    eigenvectors = vtk.vtkDoubleArray()
    pca.GetEigenvectors( eigenvectors )  
    
    # The eigenvectors are arrange from largest to smallest
    #eigenvectors.GetTuple( 0, majorAxis )
    #eigenvectors.GetTuple( 1, minorAxis )
    eigenvectors.GetTuple( 2, normalAxis )

      
  def ComputeDeepPolyData( self, surfacePolyData, normalAxis, depth, flip ):
    translationVector = normalAxis[:]
    if ( flip ):
      vtk.vtkMath.MultiplyScalar( translationVector, depth )
    else:
      vtk.vtkMath.MultiplyScalar( translationVector, -depth )
    
    deepTransform = vtk.vtkTransform()
    deepTransform.Translate( translationVector[ 0 ], translationVector[ 1 ], translationVector[ 2 ] )
    
    deepTransformFilter = vtk.vtkTransformPolyDataFilter()
    deepTransformFilter.SetInputData( surfacePolyData ) # TODO: May need deep copy
    deepTransformFilter.SetTransform( deepTransform )
    deepTransformFilter.Update()
    
    return deepTransformFilter.GetOutput()
    
    
  def GetBoundaryPoints( self, inputPolyData ):  
    featureEdges = vtk.vtkFeatureEdges()
    featureEdges.FeatureEdgesOff()
    featureEdges.NonManifoldEdgesOff()
    featureEdges.ManifoldEdgesOff()
    featureEdges.BoundaryEdgesOn()
    featureEdges.SetInputData( inputPolyData )
    featureEdges.Update()
    
    stripper = vtk.vtkStripper()
    stripper.SetInputData( featureEdges.GetOutput() )
    stripper.Update()
    
    cleaner = vtk.vtkCleanPolyData()
    cleaner.SetInputData( stripper.GetOutput() )
    cleaner.Update()
       
    return cleaner.GetOutput().GetPoints()
    
    
  def JoinBoundaryPoints( self, surfaceBoundaryPoints, deepBoundaryPoints ):
    if ( surfaceBoundaryPoints.GetNumberOfPoints() != deepBoundaryPoints.GetNumberOfPoints() ):
      logging.error( "TissueModelCreatorLogic::JoinBoundaryPoints: Top and deep surfaces have different number of boundary points." )
      return
    
    joiningAppend = vtk.vtkAppendPolyData()
    numPoints = surfaceBoundaryPoints.GetNumberOfPoints()
  
    for i in range( 0, numPoints ):    
      currPointsForSurface = vtk.vtkPoints()
    
      point1 = [ 0, 0, 0 ]
      surfaceBoundaryPoints.GetPoint( i % numPoints, point1 )
      currPointsForSurface.InsertNextPoint( point1[ 0 ], point1[ 1 ], point1[ 2 ] )
    
      point2 = [ 0, 0, 0 ]
      deepBoundaryPoints.GetPoint( i % numPoints, point2 )
      currPointsForSurface.InsertNextPoint( point2[ 0 ], point2[ 1 ], point2[ 2 ] )
    
      # Observe that the deep is flipped from the surface
      # Must proceed in opposite orders
      point3 = [ 0, 0, 0 ]
      surfaceBoundaryPoints.GetPoint( ( i + 1 ) % numPoints, point3 )
      currPointsForSurface.InsertNextPoint( point3[ 0 ], point3[ 1 ], point3[ 2 ] )
    
      point4 = [ 0, 0, 0 ]
      deepBoundaryPoints.GetPoint( ( i + 1 ) % numPoints, point4 )
      currPointsForSurface.InsertNextPoint( point4[ 0 ], point4[ 1 ], point4[ 2 ] )
      
      # We know what the triangles should look like, so create them manually
      # Note: The order here is important - ensure the triangles face the correct way
      triangle1 = vtk.vtkTriangle()
      triangle1.GetPointIds().SetId( 0, 0 )
      triangle1.GetPointIds().SetId( 1, 1 )
      triangle1.GetPointIds().SetId( 2, 2 )
      
      triangle2 = vtk.vtkTriangle()
      triangle2.GetPointIds().SetId( 0, 3 )
      triangle2.GetPointIds().SetId( 1, 2 )
      triangle2.GetPointIds().SetId( 2, 1 )
      
      triangles = vtk.vtkCellArray()
      triangles.InsertNextCell( triangle1 )
      triangles.InsertNextCell( triangle2 )
      
      currPolyDataForSurface = vtk.vtkPolyData()
      currPolyDataForSurface.SetPoints( currPointsForSurface )
      currPolyDataForSurface.SetPolys( triangles )
         
      joiningAppend.AddInputData( currPolyDataForSurface )
    
    joiningAppend.Update()
    return joiningAppend.GetOutput()

    
  def JoinTissuePolyData( self, surfacePolyData, deepPolyData, sidePolyData ):
    # Put the poly data together
    tissuePolyDataAppend = vtk.vtkAppendPolyData()
    tissuePolyDataAppend.AddInputData( surfacePolyData )
    tissuePolyDataAppend.AddInputData( deepPolyData )
    tissuePolyDataAppend.AddInputData( sidePolyData )   
    tissuePolyDataAppend.Update()
    
    # Clean up so the surface is closed and the normal face inward
    preNormalCleaner = vtk.vtkCleanPolyData()
    preNormalCleaner.SetInputData( tissuePolyDataAppend.GetOutput() )
    preNormalCleaner.Update()
    
    # Make sure the normals all face inward
    normalFilter = vtk.vtkPolyDataNormals()
    normalFilter.SetInputData( preNormalCleaner.GetOutput() )
    normalFilter.AutoOrientNormalsOn()
    normalFilter.Update()
    
    # Clean up so the surface is closed and the normal face inward
    postNormalCleaner = vtk.vtkCleanPolyData()
    postNormalCleaner.SetInputData( normalFilter.GetOutput() )
    postNormalCleaner.Update()
    
    return postNormalCleaner.GetOutput()
    
    
  def IsPolyDataClosed( self, inputPolyData ):
    edgesFilter = vtk.vtkFeatureEdges()
    edgesFilter.FeatureEdgesOff()
    edgesFilter.BoundaryEdgesOn()
    edgesFilter.NonManifoldEdgesOn()
    edgesFilter.SetInputData( inputPolyData )
    edgesFilter.Update()
    
    return ( edgesFilter.GetOutput().GetNumberOfCells() == 0 )
    
    
  def GetPointsFromMarkups( self, markupsNode, points ):
    for i in range( markupsNode.GetNumberOfFiducials() ):
      currPoint = [ 0, 0, 0 ]
      markupsNode.GetNthFiducialPosition( i, currPoint )
      points.InsertNextPoint( currPoint )
 
      
class TissueModelCreatorTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_TissueModelCreator1()

  def test_TissueModelCreator1(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests sould exercise the functionality of the logic with different inputs
    (both valid and invalid).  At higher levels your tests should emulate the
    way the user would interact with your code and confirm that it still works
    the way you intended.
    One of the most important features of the tests is that it should alert other
    developers when their changes will have an impact on the behavior of your
    module.  For example, if a developer removes a feature that you depend on,
    your test should break so they know that the feature is needed.
    """

    self.delayDisplay('Tests are not implemented for TissueModelCreator')