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

    # userID input
    self.userIDField = qt.QLineEdit()
    metadataFormLayout.addRow("UserID: ", self.userIDField)

    # studyID input
    self.studyIDField = qt.QLineEdit()
    metadataFormLayout.addRow("StudyID: ", self.studyIDField)

    # trialID input
    self.trialIDField = qt.QLineEdit()
    metadataFormLayout.addRow("TrialID: ", self.trialIDField)

    # skill level selector
    self.skillOptions = ("Novice", "Trainee", "Expert")
    self.skillSelector = qt.QComboBox()
    self.skillSelector.addItems(self.skillOptions)
    metadataFormLayout.addRow("Select skill level: ", self.skillSelector)

    # session completed selector
    self.sessionCompletionOptions = ("Complete", "Incomplete")
    self.sessionCompletionSelector = qt.QComboBox()
    self.sessionCompletionSelector.addItems(self.sessionCompletionOptions)
    metadataFormLayout.addRow("Session status: ", self.sessionCompletionSelector)

    # Save Button
    self.saveButton = qt.QPushButton("Save session")
    self.saveButton.toolTip = "Uploading data to database."
    self.saveButton.enabled = True
    metadataFormLayout.addRow(self.saveButton)

    # Add vertical spacer
    self.layout.addStretch(1)

     # Load Scene Area
    loadSceneCollapsibleButton = ctk.ctkCollapsibleButton()
    loadSceneCollapsibleButton.text = "Load Scene"
    self.layout.addWidget(loadSceneCollapsibleButton)

    # Layout within the dummy collapsible button
    loadSceneFormLayout = qt.QFormLayout(loadSceneCollapsibleButton)

    # search fields text
    self.searchText = qt.QLabel()
    self.searchText.text = "Search by fields:"
    loadSceneFormLayout.addRow(self.searchText)
    self.layout.addStretch(1)

    # Search user ID field
    self.searchUserID = qt.QLineEdit()
    loadSceneFormLayout.addRow("UserID: ", self.searchUserID)

    # Search study ID field
    self.searchStudyID = qt.QLineEdit()
    loadSceneFormLayout.addRow("StudyID: ", self.searchStudyID)

    # Search trial ID field
    self.searchTrialID = qt.QLineEdit()
    loadSceneFormLayout.addRow("TrialID: ", self.searchTrialID)

    # Search skill level field
    self.skillOptions = ("All", "Novice", "Trainee", "Expert")
    self.searchSkill = qt.QComboBox()
    self.searchSkill.addItems(self.skillOptions)
    loadSceneFormLayout.addRow("Select skill level: ", self.searchSkill)

    # Search Button
    self.searchButton = qt.QPushButton("Search for session")
    self.searchButton.enabled = True
    loadSceneFormLayout.addRow(self.searchButton)

    # Connections
    self.saveButton.connect('clicked(bool)', self.onSaveButton)
    self.searchButton.connect('clicked(bool)', self.onSearchButton)

  def onSaveButton(self):
    userID = ('userID', str(self.userIDField.text))
    studyID = ('studyID', str(self.studyIDField.text))
    trialID = ('trialID', str(self.trialIDField.text))
    skillLevel = ('skillLevel', str(self.skillSelector.currentText))
    status = ('status', str(self.sessionCompletionSelector.currentText))
    date = ('date', time.strftime("%m/%d/%Y-%H:%M:%S"))
    metricsComputed = ('metrics computed', False)
    dataFields = dict([userID, studyID, trialID, skillLevel, status, date, metricsComputed]) #creates dict from list of tuples, format for saving
    logic = couchUploadLogic()
    logic.uploadSession(dataFields)

  def onSearchButton(self):
    userID = ('userID', str(self.searchUserID.text))
    studyID = ('studyID', str(self.searchStudyID.text))
    trialID = ('trialID', str(self.searchTrialID.text))
    skillLevel = ('skill level', str(self.searchSkill.currentText))
    logic = couchUploadLogic()
    logic.loadSession([userID, studyID, trialID, skillLevel])

#couchUploadLogic
class couchUploadLogic(ScriptedLoadableModuleLogic):

  def initializeDB(self, dbName):
    couch = couchdb.Server() #uploads to localhost, replace with hostname
    try:
      self.db = couch[dbName]
    except:
      self.db = couch.create(dbName)

  def uploadSession(self, dataFields):
    self.initializeDB('perk_tutor_test')
    print 'db now', self.db
    #replace perk_tutor_test with name of db in the host
    #testhost = couch['host_test']
    self.db.save(dataFields)
    #db.replicate('http://127.0.0.1:5984/perk_tutor_test/', 'http://127.0.0.1:5984/host_test/', continuous=True)
    # save scene to db
    self.sceneName = "Scene-" + time.strftime("%Y%m%d-%H%M%S")
    self.sceneSaveFilename = slicer.app.temporaryPath + "/" + self.sceneName + ".mrb"
    slicer.util.saveScene(self.sceneSaveFilename)
    with open(self.sceneSaveFilename,'rb') as f:
      self.db.put_attachment(dataFields, f)
    self.delayDisplay("Session saved.")

  def loadSession(self, searchInputs):
    self.initializeDB('perk_tutor_test')
    searchInputs = [[field, value] if value != '' else ["Null Field", value] for (field, value) in searchInputs]
    searchFields = ['userID', 'studyID', 'trialID', 'skillLevel']
    savedScenesView = self.db.view('_design/queryDB/_view/loadAllAttributes', include_docs=True)
    for row in savedScenesView:
      flag = True
      rowData = [row.doc['userID'], row.doc['studyID'], row.doc['trialID'], row.doc['skillLevel']]
      for i in range(0, 3):
        if searchInputs[i][0] != "Null Field" and searchInputs[i][1] != rowData[i]:
          flag = False
          continue
      if searchInputs[3][1] != rowData[3] and searchInputs[3][1] != 'All':
        flag = False
      if flag:
        print rowData

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
