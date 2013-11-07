import os
import unittest
from __main__ import vtk, qt, ctk, slicer

#
# TissueModelCreator
#

class TissueModelCreator:
  def __init__(self, parent):
    parent.title = "Tissue Model Creator" # TODO make this more human readable by adding spaces
    parent.categories = ["Perk Tutor"]
    parent.dependencies = []
    parent.contributors = ["Matthew Holden (Queen's University)"] # replace with "Firstname Lastname (Org)"
    parent.helpText = """
    The purpose of the Tissue Model Creator module is to create a model based on a set of collected fiducial points. For help on how to use this module visit: <a href='http://www.github.com/PerkTutor/TissueModelCreator/wiki'>Tissue Model Creator</a>.
    """
    parent.acknowledgementText = """
    This work was was funded by Cancer Care Ontario and the Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).
    """ # replace with organization, grant and thanks.
    parent.icon = qt.QIcon( "TissueModelCreator.png" )
    self.parent = parent

    # Add this test to the SelfTest module's list for discovery when the module
    # is created.  Since this module may be discovered before SelfTests itself,
    # create the list if it doesn't already exist.
    try:
      slicer.selfTests
    except AttributeError:
      slicer.selfTests = {}
    slicer.selfTests['TissueModelCreator'] = self.runTest

  def runTest(self):
    tester = TissueModelCreatorTest()
    tester.runTest()

#
# qTissueModelCreatorWidget
#

class TissueModelCreatorWidget:
  def __init__(self, parent = None):
    if not parent:
      self.parent = slicer.qMRMLWidget()
      self.parent.setLayout(qt.QVBoxLayout())
      self.parent.setMRMLScene(slicer.mrmlScene)
    else:
      self.parent = parent
    self.layout = self.parent.layout()
    if not parent:
      self.setup()
      self.parent.show()

  def setup(self):
    # Instantiate and connect widgets ...

    # # Comment these out when not debugging
    # #
    # # Reload and Test area
    # #
    # reloadCollapsibleButton = ctk.ctkCollapsibleButton()
    # reloadCollapsibleButton.text = "Reload && Test"
    # self.layout.addWidget(reloadCollapsibleButton)
    # reloadFormLayout = qt.QFormLayout(reloadCollapsibleButton)

    # # reload button
    # # (use this during development, but remove it when delivering
    # #  your module to users)
    # self.reloadButton = qt.QPushButton("Reload")
    # self.reloadButton.toolTip = "Reload this module."
    # self.reloadButton.name = "TissueModelCreator Reload"
    # reloadFormLayout.addWidget(self.reloadButton)
    # self.reloadButton.connect('clicked()', self.onReload)

    # # reload and test button
    # # (use this during development, but remove it when delivering
    # #  your module to users)
    # self.reloadAndTestButton = qt.QPushButton("Reload and Test")
    # self.reloadAndTestButton.toolTip = "Reload this module and then run the self tests."
    # reloadFormLayout.addWidget(self.reloadAndTestButton)
    # self.reloadAndTestButton.connect('clicked()', self.onReloadAndTest)

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
    self.markupSelector.nodeTypes = ( ("vtkMRMLMarkupsFiducialNode"), "" )
    self.markupSelector.addEnabled = False
    self.markupSelector.removeEnabled = False
    self.markupSelector.noneEnabled = False
    self.markupSelector.showHidden = False
    self.markupSelector.showChildNodeTypes = False
    self.markupSelector.setMRMLScene( slicer.mrmlScene )
    self.markupSelector.setToolTip( "Pick the markup node for the algorithm." )
    displayFormLayout.addRow( "Select Markup Node: ", self.markupSelector )
    
    #
    # Depth slider
    #
    self.depthSlider = ctk.ctkSliderWidget()
    self.depthSlider.maximum = 1000
    self.depthSlider.minimum = 0
    self.depthSlider.value = 100
    self.depthSlider.setToolTip( "Select the depth of the tissue." )
    displayFormLayout.addRow( "Depth (mm): ", self.depthSlider )

    #
    # Flip (ie flip) checkbox
    #
    self.flipCheckBox = qt.QCheckBox()
    self.flipCheckBox.setCheckState( False )
    self.flipCheckBox.setToolTip( "Flip the tissue so it is in the other direction." )
    self.flipCheckBox.setText( "Flip" )
    displayFormLayout.addRow( self.flipCheckBox )

    #
    # Create Button
    #
    self.createButton = qt.QPushButton( "Create" )
    self.createButton.toolTip = "Create a tissue model."
    self.createButton.enabled = True
    displayFormLayout.addRow( self.createButton )
    
    #
    # Status Label
    #
    self.statusLabel = qt.QLabel( "Status:" )
    self.statusLabel.toolTip = "Status of whether the tissue model was successfully created."
    self.statusLabel.enabled = True
    displayFormLayout.addRow( self.statusLabel )
    
    

    # connections
    self.createButton.connect( 'clicked(bool)', self.onCreateButtonClicked )

    # Add vertical spacer
    self.layout.addStretch(1)

  def cleanup(self):
    pass

  def onCreateButtonClicked(self):
    logic = TissueModelCreatorLogic()
    surfaceClosed = logic.run( self.markupSelector.currentNode(), self.depthSlider.value, self.flipCheckBox.checked )
    
    if ( surfaceClosed == True ):
      self.statusLabel.setText( "Status: Success!" )
    else:
      self.statusLabel.setText( "Status: Failed!" )

  def onReload(self,moduleName="TissueModelCreator"):
    """Generic reload method for any scripted module.
    ModuleWizard will subsitute correct default moduleName.
    """
    import imp, sys, os, slicer

    widgetName = moduleName + "Widget"

    # reload the source code
    # - set source file path
    # - load the module to the global space
    filePath = eval('slicer.modules.%s.path' % moduleName.lower())
    p = os.path.dirname(filePath)
    if not sys.path.__contains__(p):
      sys.path.insert(0,p)
    fp = open(filePath, "r")
    globals()[moduleName] = imp.load_module(
        moduleName, fp, filePath, ('.py', 'r', imp.PY_SOURCE))
    fp.close()

    # rebuild the widget
    # - find and hide the existing widget
    # - create a new widget in the existing parent
    parent = slicer.util.findChildren(name='%s Reload' % moduleName)[0].parent().parent()
    for child in parent.children():
      try:
        child.hide()
      except AttributeError:
        pass
    # Remove spacer items
    item = parent.layout().itemAt(0)
    while item:
      parent.layout().removeItem(item)
      item = parent.layout().itemAt(0)

    # delete the old widget instance
    if hasattr(globals()['slicer'].modules, widgetName):
      getattr(globals()['slicer'].modules, widgetName).cleanup()

    # create new widget inside existing parent
    globals()[widgetName.lower()] = eval(
        'globals()["%s"].%s(parent)' % (moduleName, widgetName))
    globals()[widgetName.lower()].setup()
    setattr(globals()['slicer'].modules, widgetName, globals()[widgetName.lower()])

  def onReloadAndTest(self,moduleName="TissueModelCreator"):
    try:
      self.onReload()
      evalString = 'globals()["%s"].%sTest()' % (moduleName, moduleName)
      tester = eval(evalString)
      tester.runTest()
    except Exception, e:
      import traceback
      traceback.print_exc()
      qt.QMessageBox.warning(slicer.util.mainWindow(), 
          "Reload and Test", 'Exception!\n\n' + str(e) + "\n\nSee Python Console for Stack Trace")


#
# TissueModelCreatorLogic
#

class TissueModelCreatorLogic:
  """This class should implement all the actual 
  computation done by your module.  The interface 
  should be such that other python code can import
  this class and make use of the functionality without
  requiring an instance of the Widget
  """
  def __init__(self):
    pass


  def run( self, markupNode, depth, flip ):
    """
    Run the actual algorithm
    """
    
    # Get all of the fiducial nodes and convert to vtkPoints
    points = vtk.vtkPoints()
    
    for i in range( 0, markupNode.GetNumberOfFiducials() ):
      currentCoordinates = [ 0, 0, 0 ]
      markupNode.GetNthFiducialPosition( i, currentCoordinates )
      points.InsertNextPoint( currentCoordinates )
      
    # Check that there is non-zero range in all coordinate directions
    pointsBounds = [ 0, 0, 0, 0, 0, 0 ]
    points.GetBounds( pointsBounds )
    if ( pointsBounds[0] == pointsBounds [1] or pointsBounds[2] == pointsBounds [3] or pointsBounds[4] == pointsBounds [5] ):
      print "Tissue Model Creator: Points have no extent in one or more coordinate directions."
      return False
      
    # Create a polydata object from the points
    # The reversiness doesn't matter - we will fix it later if it os wrong
    surfacePolyData = self.PointsToSurfacePolyData( points, True )
    
    mean = self.CalculateMean( points )
    
    surfaceBase = [ 0, 0, 0 ]
    surfaceDir1 = [ 0, 0, 0 ]
    surfaceDir2 = [ 0, 0, 0 ]
    surfaceNormal = [ 0, 0, 0 ]
    self.CalculatePlane( surfacePolyData.GetPoints(), surfaceBase, surfaceDir1, surfaceDir2, surfaceNormal )
    
    if ( flip == True ):
      surfaceNormal[ 0 ] = - surfaceNormal[ 0 ]
      surfaceNormal[ 1 ] = - surfaceNormal[ 1 ]
      surfaceNormal[ 2 ] = - surfaceNormal[ 2 ]
    
    extremePointIndex = self.FindExtremePoint( surfacePolyData.GetPoints(), surfaceBase, surfaceNormal )
    reverse = self.ReverseNormals( extremePointIndex, surfacePolyData, surfaceNormal )
    
    # Reverse the normals if necessary
    reverseFilter = vtk.vtkReverseSense()
    reverseFilter.SetInput( surfacePolyData )
    reverseFilter.SetReverseCells( reverse )
    reverseFilter.SetReverseNormals( reverse )
    reverseFilter.Update()
    surfacePolyData = reverseFilter.GetOutput()
    
    untransDeepPolyData = vtk.vtkPolyData()
    untransDeepPolyData.DeepCopy( surfacePolyData )
    
    # Make the normals opposite the surface's normals
    reverseFilter = vtk.vtkReverseSense()
    reverseFilter.SetInput( untransDeepPolyData )
    reverseFilter.SetReverseCells( True )
    reverseFilter.SetReverseNormals( True )
    reverseFilter.Update()
    untransDeepPolyData = reverseFilter.GetOutput()
    
    deepTransform = vtk.vtkTransform()
    deepTransform.Translate( depth * surfaceNormal[0], depth * surfaceNormal[1], depth * surfaceNormal[2] )  
    deepTransformFilter = vtk.vtkTransformPolyDataFilter()
    deepTransformFilter.SetInput( untransDeepPolyData )
    deepTransformFilter.SetTransform( deepTransform )
    deepTransformFilter.Update()
    
    deepPolyData = deepTransformFilter.GetOutput()

    
    surfaceHullPoints = self.GetBoundaryPoints( surfacePolyData )
    
    deepHullPoints = self.GetBoundaryPoints( deepPolyData )
    
    jointHullPolyData = self.JoinBoundaryPoints( surfaceHullPoints, deepHullPoints )
    
    # Append all of the polydata together
    tissuePolyDataAppend = vtk.vtkAppendPolyData()
    tissuePolyDataAppend.AddInput( surfacePolyData )
    tissuePolyDataAppend.AddInput( deepPolyData )
    tissuePolyDataAppend.AddInput( jointHullPolyData )    

    # Clean up so the surface is closed
    tissueCleaner = vtk.vtkCleanPolyData()
    tissueCleaner.SetInput( tissuePolyDataAppend.GetOutput() )
    tissueCleaner.Update()
    
    tissueModelPolyData = tissueCleaner.GetOutput()
    
    # Add the data to a model 
    tissueModel = slicer.mrmlScene.CreateNodeByClass( "vtkMRMLModelNode" )
    tissueModel.SetAndObservePolyData( tissueModelPolyData )
    tissueModel.SetName( "TissueModel" )
    tissueModel.SetScene( slicer.mrmlScene )
    
    # Finally display the model
    tissueModelDisplay = slicer.mrmlScene.CreateNodeByClass( "vtkMRMLModelDisplayNode" )
    tissueModelDisplay.SetScene( slicer.mrmlScene )
    tissueModelDisplay.SetInputPolyData( tissueModel.GetPolyData() )
    
    slicer.mrmlScene.AddNode( tissueModelDisplay )
    slicer.mrmlScene.AddNode( tissueModel )
    
    tissueModel.SetAndObserveDisplayNodeID( tissueModelDisplay.GetID() )

    # Check to make sure the model is a closed surface
    edgesFilter = vtk.vtkFeatureEdges()
    edgesFilter.FeatureEdgesOff()
    edgesFilter.BoundaryEdgesOn()
    edgesFilter.NonManifoldEdgesOn()
    edgesFilter.SetInput( tissueModel.GetPolyData() )
    edgesFilter.Update()
    
    if ( edgesFilter.GetOutput().GetNumberOfCells() != 0 ):
      print "Tissue Model Creator: Surface is not closed."
      return False
      
    return True
      
    
  def PointsToSurfacePolyData( self, inPoints, reverse ):

    # Create a polydata object from the points
    pointsPolyData = vtk.vtkPolyData()
    pointsPolyData.SetPoints( inPoints )
  
    # Create the surface filter from the polydata
    surfaceFilter = vtk.vtkSurfaceReconstructionFilter()
    surfaceFilter.SetInput( pointsPolyData )
    
    # Do the contouring filter, and reverse to ensure it works properly
    contourFilter = vtk.vtkContourFilter()
    contourFilter.SetValue( 0, 0.0 )
    contourFilter.SetInput( surfaceFilter.GetOutput() )
    
    # Reverse the normals if necessary
    reverseFilter = vtk.vtkReverseSense()
    reverseFilter.SetInput( contourFilter.GetOutput() )
    reverseFilter.SetReverseCells( reverse )
    reverseFilter.SetReverseNormals( reverse )
    reverseFilter.Update()
   
    # Reset the scaling to let the surface match the points
    fiducialBounds = [ 0, 0, 0, 0, 0, 0 ]
    pointsPolyData.GetBounds( fiducialBounds )
    
    tissueBounds = [ 0, 0, 0, 0, 0, 0 ]
    reverseFilter.GetOutput().GetBounds( tissueBounds )
    
    scaleX = ( fiducialBounds[1] - fiducialBounds[0] ) / ( tissueBounds[1] - tissueBounds[0] )
    scaleY = ( fiducialBounds[3] - fiducialBounds[2] ) / ( tissueBounds[3] - tissueBounds[2] )
    scaleZ = ( fiducialBounds[5] - fiducialBounds[4] ) / ( tissueBounds[5] - tissueBounds[4] )
    
    transform = vtk.vtkTransform()
    transform.Translate( fiducialBounds[0], fiducialBounds[2], fiducialBounds[4] )
    transform.Scale( scaleX, scaleY, scaleZ )
    transform.Translate( - tissueBounds[0], - tissueBounds[2], - tissueBounds[4] )
  
    transformFilter = vtk.vtkTransformPolyDataFilter()
    transformFilter.SetInput( reverseFilter.GetOutput() )
    transformFilter.SetTransform( transform )
    transformFilter.Update()
  
    return transformFilter.GetOutput()
    

  def CalculateMean( self, inPoints ):

    mean = [ 0, 0, 0 ]
  
    for i in range( 0, inPoints.GetNumberOfPoints() ):
    
      currPoint = [ 0, 0, 0 ]
      inPoints.GetPoint( i, currPoint )
    
      mean[ 0 ] = mean[ 0 ] + currPoint[ 0 ]
      mean[ 1 ] = mean[ 1 ] + currPoint[ 1 ]
      mean[ 2 ] = mean[ 2 ] + currPoint[ 2 ]
    
    if ( inPoints.GetNumberOfPoints() > 0 ):
      mean[ 0 ] = mean[ 0 ] / inPoints.GetNumberOfPoints()
      mean[ 1 ] = mean[ 1 ] / inPoints.GetNumberOfPoints()
      mean[ 2 ] = mean[ 2 ] / inPoints.GetNumberOfPoints()
    
    return mean
    

  def CalculatePlane( self, inPoints, base, dir1, dir2, normal ):

    # Create arrays for the dataset
    points2D = vtk.vtkPoints()
  
    arrayX = vtk.vtkDoubleArray()
    arrayX.SetNumberOfComponents( 1 )
    arrayX.SetName ( 'X' )
    arrayY = vtk.vtkDoubleArray()
    arrayY.SetNumberOfComponents( 1 )
    arrayY.SetName ( 'Y' )
    arrayZ = vtk.vtkDoubleArray()
    arrayZ.SetNumberOfComponents( 1 )
    arrayZ.SetName ( 'Z' )
    
    # Add the points to the table
    for i in range( 0, inPoints.GetNumberOfPoints() ):
    
      currPoint = [ 0, 0, 0 ]
      inPoints.GetPoint( i, currPoint )   
      
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
    pca.SetInput( vtk.vtkStatisticsAlgorithm.INPUT_DATA, table )
    pca.SetColumnStatus( 'X', 1 )
    pca.SetColumnStatus( 'Y', 1 )
    pca.SetColumnStatus( 'Z', 1 )
    pca.RequestSelectedColumns()
    pca.SetDeriveOption( True )
    pca.Update()
    
    eigvec = vtk.vtkDoubleArray()
    pca.GetEigenvectors( eigvec )
  
    
    eigvec.GetTuple( 0, dir1 )
    eigvec.GetTuple( 1, dir2 )
    eigvec.GetTuple( 2, normal )
  
    mean = self.CalculateMean( inPoints )
    base[0] = mean[0]
    base[1] = mean[1]
    base[2] = mean[2]
    
    
  def GetBoundaryPoints( self, inPolyData ):

    featureEdges = vtk.vtkFeatureEdges()
    featureEdges.FeatureEdgesOff()
    featureEdges.NonManifoldEdgesOff()
    featureEdges.ManifoldEdgesOff()
    featureEdges.BoundaryEdgesOn()
    featureEdges.SetInput( inPolyData )
    featureEdges.Update()
    
    stripper = vtk.vtkStripper()
    stripper.SetInput( featureEdges.GetOutput() )
    stripper.Update()
    
    cleaner = vtk.vtkCleanPolyData()
    cleaner.SetInput( stripper.GetOutput() )
    cleaner.Update()
       
    return cleaner.GetOutput().GetPoints()

  
  def JoinBoundaryPoints( self, hullPoints1, hullPoints2 ):

    if ( hullPoints1.GetNumberOfPoints() != hullPoints2.GetNumberOfPoints() ):
      return
    
    joiningAppend = vtk.vtkAppendPolyData()
    numPoints = hullPoints1.GetNumberOfPoints()
  
    for i in range( 0, numPoints ):
    
      currPointsForSurface = vtk.vtkPoints()
    
      point1 = [ 0, 0, 0 ]
      hullPoints1.GetPoint( i % numPoints, point1 )
      currPointsForSurface.InsertNextPoint( point1[ 0 ], point1[ 1 ], point1[ 2 ] )
    
      point2 = [ 0, 0, 0 ]
      hullPoints2.GetPoint( ( - i ) % numPoints, point2 )
      currPointsForSurface.InsertNextPoint( point2[ 0 ], point2[ 1 ], point2[ 2 ] )
    
      # Observe that the deep is flipped from the surface
      # Must proceed in opposite orders
      point3 = [ 0, 0, 0 ]
      hullPoints1.GetPoint( ( i + 1 ) % numPoints, point3 )
      currPointsForSurface.InsertNextPoint( point3[ 0 ], point3[ 1 ], point3[ 2 ] )
    
      point4 = [ 0, 0, 0 ]
      hullPoints2.GetPoint( ( - ( i + 1 ) ) % numPoints, point4 )
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
         
      joiningAppend.AddInput( currPolyDataForSurface )
    
    return joiningAppend.GetOutput()
    
  
  def FindExtremePoint( self, surfacePoints, surfaceBase, surfaceNormal ):
  
    extremePointIndex = 1
    extremePointProjection = 0
    # Find the point with the smallest projection (ie most negative) onto the surface normal
    for i in range( surfacePoints.GetNumberOfPoints() ):
    
      currPoint = [ 0, 0, 0 ]
      surfacePoints.GetPoint( i, currPoint )
          
      currProjection = ( currPoint[ 0 ] - surfaceBase [ 0 ] ) * surfaceNormal [ 0 ] + ( currPoint[ 1 ] - surfaceBase [ 1 ] ) * surfaceNormal [ 1 ] + ( currPoint[ 2 ] - surfaceBase [ 2 ] ) * surfaceNormal [ 2 ]
      
      if ( currProjection < extremePointProjection ):
        extremePointProjection = currProjection
        extremePointIndex = i
    
    return extremePointIndex


  def ReverseNormals( self, extremePointIndex, surfacePolyData, surfaceNormal ):
   
    normalArray = surfacePolyData.GetPointData().GetNormals()
    
    extremePointNormal = [ 0, 0, 0 ]
    normalArray.GetTuple( extremePointIndex, extremePointNormal )
    
    if ( vtk.vtkMath.Dot( extremePointNormal, surfaceNormal ) > 0 ):
      return True
      
    return False
    




    


class TissueModelCreatorTest(unittest.TestCase):
  """
  This is the test case for your scripted module.
  """

  def delayDisplay(self,message,msec=1000):
    """This utility method displays a small dialog and waits.
    This does two things: 1) it lets the event loop catch up
    to the state of the test so that rendering and widget updates
    have all taken place before the test continues and 2) it
    shows the user/developer/tester the state of the test
    so that we'll know when it breaks.
    """
    print(message)
    self.info = qt.QDialog()
    self.infoLayout = qt.QVBoxLayout()
    self.info.setLayout(self.infoLayout)
    self.label = qt.QLabel(message,self.info)
    self.infoLayout.addWidget(self.label)
    qt.QTimer.singleShot(msec, self.info.close)
    self.info.exec_()

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

    self.delayDisplay("Starting the test")
    #
    # first, get some data
    #
    import urllib
    downloads = (
        ('http://slicer.kitware.com/midas3/download?items=5767', 'FA.nrrd', slicer.util.loadVolume),
        )

    for url,name,loader in downloads:
      filePath = slicer.app.temporaryPath + '/' + name
      if not os.path.exists(filePath) or os.stat(filePath).st_size == 0:
        print('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        print('Loading %s...\n' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading\n')

    volumeNode = slicer.util.getNode(pattern="FA")
    logic = TissueModelCreatorLogic()
    self.assertTrue( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
