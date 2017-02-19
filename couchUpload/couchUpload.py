import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
import time
import sys
import couchdb

class couchUpload(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "couchUpload" # TODO make this more human readable by adding spaces
    self.parent.categories = ["Perk Tutor"]
    self.parent.dependencies = []
    self.parent.contributors = ["Christina Yan (Perk Lab)"] # replace with "Firstname Lastname (Organization)"
    self.parent.helpText = """
    This is a scripted loadable module to upload PerkTutor data to a distributed database."""
    self.parent.acknowledgementText = """organization, grant and thanks."""

#
# couchUploadWidget
#

class couchUploadWidget(ScriptedLoadableModuleWidget):

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

    # Metadata Area
    metadataCollapsibleButton = ctk.ctkCollapsibleButton()
    metadataCollapsibleButton.text = "Metadata"
    self.layout.addWidget(metadataCollapsibleButton)

    # Layout within the dummy collapsible button
    metadataFormLayout = qt.QFormLayout(metadataCollapsibleButton)

    # Name input
    self.nameField = qt.QLineEdit()
    metadataFormLayout.addRow("Participant Name: ", self.nameField)

    # userID input
    self.userIDField = qt.QLineEdit()
    metadataFormLayout.addRow("userID: ", self.userIDField)

    # studyID input
    self.studyIDField = qt.QLineEdit()
    metadataFormLayout.addRow("studyID: ", self.studyIDField)

    # trialID input
    self.trialIDField = qt.QLineEdit()
    metadataFormLayout.addRow("trialID: ", self.trialIDField)

    # skill level selector
    self.skillOptions = ("Novice", "Trainee", "Expert")
    self.skillSelector = qt.QComboBox()
    self.skillSelector.addItems(self.skillOptions)
    metadataFormLayout.addRow("Select skill level: ", self.skillSelector)

    # procedure input -- should this be replaced as a combobox?
    self.procedureField = qt.QLineEdit()
    metadataFormLayout.addRow("Procedure: ", self.procedureField)

    # session completed selector
    self.completedCheckBox = qt.QCheckBox("Session completed")
    self.incompletedCheckBox = qt.QCheckBox("Session incomplete")
    metadataFormLayout.addRow(self.completedCheckBox, self.incompletedCheckBox)

    # Save Button
    self.saveButton = qt.QPushButton("Save session")
    self.saveButton.toolTip = "Uploading data to database."
    self.saveButton.enabled = True
    metadataFormLayout.addRow(self.saveButton)

    # Connections
    self.saveButton.connect('clicked(bool)', self.onSaveButton)

    # Add vertical spacer
    self.layout.addStretch(1)

  def onSaveButton(self):
    logic = couchUploadLogic()
    name = ('name', str(self.nameField.text))
    userID = ('userID', str(self.userIDField.text))
    studyID = ('studyID', str(self.studyIDField.text))
    procedure = ('procedure', str(self.procedureField.text))
    date = ('date', time.strftime("%m/%d/%Y-%H:%M:%S"))
    metricsComputed = ('metrics computed', False)
    dataFields = dict([name, userID, studyID, procedure, date, metricsComputed]) #creates dict from list of tuples, format for saving
    logic.uploadSession(dataFields)

#couchUploadLogic
class couchUploadLogic(ScriptedLoadableModuleLogic):

  def uploadSession(self, dataFields):
    couch = couchdb.Server() #uploads to localhost, replace with hostname
    db = couch['pt'] #replace test with name of db in the host
    db.save(dataFields)

    # save scene to db
    sceneName = "Scene-" + time.strftime("%Y%m%d-%H%M%S")
    sceneSaveFilename = slicer.app.temporaryPath + "/" + sceneName + ".mrb"
    slicer.util.saveScene(sceneSaveFilename)
    with open(sceneSaveFilename,'rb') as f:
      db.put_attachment(dataFields, f)

class couchUploadTest(ScriptedLoadableModuleTest):
  """
  This is the test case for your scripted module.
  Uses ScriptedLoadableModuleTest base class, available at:
  https://github.com/Slicer/Slicer/blob/master/Base/Python/slicer/ScriptedLoadableModule.py
  """

  def setUp(self):
    """ Do whatever is needed to reset the state - typically a scene clear will be enough.
    """
    slicer.mrmlScene.Clear(0)

  def runTest(self):
    """Run as few or as many tests as needed here.
    """
    self.setUp()
    self.test_couchUpload1()

  def test_couchUpload1(self):
    """ Ideally you should have several levels of tests.  At the lowest level
    tests should exercise the functionality of the logic with different inputs
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
        logging.info('Requesting download %s from %s...\n' % (name, url))
        urllib.urlretrieve(url, filePath)
      if loader:
        logging.info('Loading %s...' % (name,))
        loader(filePath)
    self.delayDisplay('Finished with download and loading')

    volumeNode = slicer.util.getNode(pattern="FA")
    logic = couchUploadLogic()
    self.assertIsNotNone( logic.hasImageData(volumeNode) )
    self.delayDisplay('Test passed!')
