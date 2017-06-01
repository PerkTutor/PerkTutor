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
    self.parent.dependencies = [ "MarkupsToModel" ]
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
    
    #
    # Display Area
    #
    displayCollapsibleButton = ctk.ctkCollapsibleButton()
    displayCollapsibleButton.text = "Display"
    self.layout.addWidget(displayCollapsibleButton)

    # Layout within the dummy collapsible button
    displayFormLayout = qt.QFormLayout(displayCollapsibleButton)

    #
    # input fiducials selector
    #
    self.markupSelector = slicer.qMRMLNodeComboBox()
    self.markupSelector.nodeTypes = [ "vtkMRMLMarkupsFiducialNode" ]
    self.markupSelector.addEnabled = False
    self.markupSelector.removeEnabled = False
    self.markupSelector.noneEnabled = False
    self.markupSelector.showHidden = False
    self.markupSelector.showChildNodeTypes = False
    self.markupSelector.setMRMLScene( slicer.mrmlScene )
    self.markupSelector.setToolTip( "Select the markup node for the algorithm." )
    displayFormLayout.addRow( "Input Markup ", self.markupSelector )
    
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
    displayFormLayout.addRow( "Output Model ", self.modelSelector )
    
    #
    # Depth slider
    #
    self.depthSlider = ctk.ctkSliderWidget()
    self.depthSlider.maximum = 1000
    self.depthSlider.minimum = 1
    self.depthSlider.value = 100
    self.depthSlider.setToolTip( "Select the depth of the tissue." )
    displayFormLayout.addRow( "Depth (mm) ", self.depthSlider )

    #
    # Flip (ie flip) checkbox
    #
    self.flipCheckBox = qt.QCheckBox()
    self.flipCheckBox.setCheckState( False )
    self.flipCheckBox.setToolTip( "Flip the tissue so it is in the other direction." )
    self.flipCheckBox.setText( "Flip" )
    displayFormLayout.addRow( self.flipCheckBox )

    #
    # Update Button
    #
    self.updateButton = qt.QPushButton( "Update" )
    self.updateButton.toolTip = "Update the tissue model."
    self.updateButton.enabled = True
    displayFormLayout.addRow( self.updateButton )
    
    #
    # Status Label
    #
    self.statusLabel = qt.QLabel( "" )
    self.statusLabel.toolTip = "Status of whether the tissue model was successfully created."
    self.statusLabel.enabled = True
    displayFormLayout.addRow( self.statusLabel )

    
    # connections
    self.updateButton.connect( 'clicked(bool)', self.onUpdateButtonClicked )

  def cleanup( self ):
    pass
    
    
  def onUpdateButtonClicked(self):
    statusText = self.tmcLogic.UpdateTissueModel( self.markupSelector.currentNode(), self.modelSelector.currentNode(), self.depthSlider.value, self.flipCheckBox.checked )

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
    
    
  def UpdateTissueModel( self, markupsNode, modelNode, depth, flip ):
    if ( markupsNode is None or modelNode is None ):
      return "Markups node or model node not properly specified."
      
    # Use the 2D delaunay filter to get a model of the surface
    surfacePolyData = self.ComputeSurfacePolyData( markupsNode )
  
    # Compute the axes from the PCA
    normalAxis = [ 0, 0, 0 ]
    self.ComputeTissueNormal( markupsNode, normalAxis )
    
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
    
    
  def ComputeSurfacePolyData( self, markupsNode ):
    surfacePoints = vtk.vtkPoints()
    for i in range( markupsNode.GetNumberOfFiducials() ):
      point = [ 0, 0, 0 ]
      markupsNode.GetNthFiducialPosition( i, point )
      surfacePoints.InsertNextPoint( point )
  
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
    

  def ComputeTissueNormal( self, markupsNode, normalAxis ):
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
    for i in range( markupsNode.GetNumberOfFiducials() ):   
      currPosition = [ 0, 0, 0 ]
      markupsNode.GetNthFiducialPosition( i, currPosition )      
      arrayX.InsertNextValue( currPosition[ 0 ] )
      arrayY.InsertNextValue( currPosition[ 1 ] ) 
      arrayZ.InsertNextValue( currPosition[ 2 ] )
    
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