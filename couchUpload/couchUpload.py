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
    loadSceneCollapsibleButton.text = "Search For Session"
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

    # Query Results Area
    self.queryResultsCollapsibleButton = ctk.ctkCollapsibleButton()
    self.queryResultsCollapsibleButton.text = "Query Results"
    self.queryResultsCollapsibleButton.setVisible(False)
    self.layout.addWidget(self.queryResultsCollapsibleButton)

    self.queryResultsLayout = qt.QFormLayout(self.queryResultsCollapsibleButton)
    self.table = qt.QTableWidget(1, 6)
    self.queryResultsLayout.addRow(self.table)

    #session selector in query results
    self.selectLabel = qt.QLabel("Select session number: ")
    self.selectSession = qt.QComboBox()
    self.queryResultsLayout.addRow(self.selectLabel, self.selectSession)

    self.loadButton = qt.QPushButton("Load Session")
    self.queryResultsLayout.addWidget(self.loadButton)
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
    searchInput = [userID, studyID, trialID, skillLevel]
    logic = couchUploadLogic()
    results = logic.queryDB(searchInput)
    self.displayResults(results)

  def displayResults(self, results):
    numRows = len(results)
    self.table.setRowCount(numRows)
    self.table.setHorizontalHeaderItem(0, qt.QTableWidgetItem("UserID"))
    self.table.setHorizontalHeaderItem(1, qt.QTableWidgetItem("StudyID"))
    self.table.setColumnWidth(1, 150)
    self.table.setHorizontalHeaderItem(2, qt.QTableWidgetItem("TrialID"))
    self.table.setHorizontalHeaderItem(3, qt.QTableWidgetItem("Skill Level"))
    self.table.setColumnWidth(3, 150)
    self.table.setHorizontalHeaderItem(4, qt.QTableWidgetItem("Date"))
    self.table.setColumnWidth(4, 300)
    self.table.setHorizontalHeaderItem(5, qt.QTableWidgetItem("Session Number"))
    self.table.setColumnWidth(5, 200)
    for row in range(0, numRows):
      for col in range(0, 5):
        self.table.setItem(row, col, qt.QTableWidgetItem(results[row][col]))
      self.table.setItem(row, 5, qt.QTableWidgetItem(str(row+1)))
    self.layout.addStretch(1)
    self.sessionNumbers = tuple(range(1, len(results) + 1))
    self.selectSession.addItems(self.sessionNumbers)
    self.queryResultsCollapsibleButton.setVisible(True)

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
    #replace perk_tutor_test with name of db in the host
    #testhost = couch['host_test']

    #db.replicate('http://127.0.0.1:5984/perk_tutor_test/', 'http://127.0.0.1:5984/host_test/', continuous=True)
    # save scene to db
    self.sceneName = "Scene-" + time.strftime("%Y%m%d-%H%M%S") + ".mrb"
    self.sceneSaveFilename = slicer.app.temporaryPath + "/" + self.sceneName
    dataFields['sceneName'] = self.sceneName
    self.db.save(dataFields)
    slicer.util.saveScene(self.sceneSaveFilename)
    with open(self.sceneSaveFilename,'rb') as f:
      self.db.put_attachment(dataFields, f)
    self.delayDisplay("Session saved.")

  def queryDB(self, searchInputs):
    self.initializeDB('perk_tutor_test')
    searchInputs = [[field, value] if value != '' else ["Null Field", value] for (field, value) in searchInputs]
    savedScenesView = self.db.view('_design/queryDB/_view/loadAllAttributes', include_docs=True)
    queryResults = []
    for row in savedScenesView:
      flag = True
      rowData = [row.doc['userID'], row.doc['studyID'], row.doc['trialID'], row.doc['skillLevel'], row.doc['date'], row.doc['sceneName']]
      for i in range(0, 3):
        if searchInputs[i][0] != "Null Field" and searchInputs[i][1] != rowData[i]:
          flag = False
          continue
      if searchInputs[3][1] != rowData[3] and searchInputs[3][1] != 'All':
        flag = False
      if flag:
        queryResults.append(rowData) #each row should be a new line on a popup window with button to load scene
    return queryResults
