import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
import time
import sys
import couchdb
import re

class couchUpload(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "CouchDB Upload" # TODO make this more human readable by adding spaces
    self.parent.categories = ["Perk Tutor"]
    self.parent.dependencies = []
    self.parent.contributors = ["Christina Yan (Perk Lab)"] # replace with "Firstname Lastname (Organization)"
    self.parent.helpText = """
    This is a scripted loadable module to upload PerkTutor data to a distributed database."""
    self.parent.acknowledgementText = ""

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
    self.table = qt.QTableWidget(1, 5)
    self.queryResultsLayout.addRow(self.table)

    #load session button
    self.loadButton = qt.QPushButton("Load Session")
    self.queryResultsLayout.addWidget(self.loadButton)

    # Connections
    self.saveButton.connect('clicked(bool)', self.onSaveButton)
    self.searchButton.connect('clicked(bool)', self.onSearchButton)
    self.loadButton.connect('clicked(bool)', self.onLoadButton)

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
    skillLevel = ('skill', str(self.searchSkill.currentText))
    searchInput = [userID, studyID, trialID, skillLevel]
    logic = couchUploadLogic()
    self.results = logic.queryDB(searchInput)
    self.displayResults()

  def onLoadButton(self):
    logic = couchUploadLogic()
    rowNum = self.table.currentRow()
    sceneName = self.results[rowNum][-1]
    logic.loadScene(sceneName)

  def displayResults(self): # look into highlighting query rows or another option to remove session numbers
    numRows = len(self.results)
    self.table.setRowCount(numRows)
    self.table.setHorizontalHeaderItem(0, qt.QTableWidgetItem("UserID"))
    self.table.setHorizontalHeaderItem(1, qt.QTableWidgetItem("StudyID"))
    self.table.setColumnWidth(1, 150)
    self.table.setHorizontalHeaderItem(2, qt.QTableWidgetItem("TrialID"))
    self.table.setHorizontalHeaderItem(3, qt.QTableWidgetItem("Skill Level"))
    self.table.setColumnWidth(3, 150)
    self.table.setHorizontalHeaderItem(4, qt.QTableWidgetItem("Date"))
    self.table.setColumnWidth(4, 300)
    for row in range(0, numRows):
       for col in range(0, 5):
          self.table.setItem(row, col, qt.QTableWidgetItem(self.results[row][col]))

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
    self.initializeDB('perk_tutor_test') #replace perk_tutor_test with name of db in the host
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
    queryValues = []
    viewName = ''

    #check for non empty userID, studyID, trialID
    for key,value in searchInputs[0:-1]:
      if value != '':
        queryValues.append(value)
        viewName += key
    (skillText, skillLevel) = searchInputs[-1]
    if skillLevel != 'All':
      queryValues.append(skillLevel)
      viewName += skillText

    #if only one value to query, convert one element list of strings into 1 string
    if len(queryValues) == 1:
      queryValues= queryValues[0]

    queryResults = []
    viewPath = '_design/docs/_view/'
    # if search values given to perform query
    if viewName != '':
      viewPath += viewName
      queryResults = self.db.view(viewPath,include_docs=True,key=queryValues)
    # if no query values given, just return the view with all docs in DB (no inputs for the userID, studyID, trialID fields and skill selector on 'All')
    else:
      viewPath+="userIDstudyIDtrialID"
      queryResults = self.db.view(viewPath,include_docs=True)
    rowData = []
    #convert data in view into list of lists for displaying results
    for result in queryResults:
      row = [result.doc['userID'], result.doc['studyID'], result.doc['trialID'], result.doc['skillLevel'], result.doc['date'], result.doc['sceneName']]
      rowData.append(row)
    return rowData

  def loadScene(self, sceneName):
    self.initializeDB('perk_tutor_test')
    sceneView = self.db.view('_design/docs/_view/loadScene', include_docs=True, key=sceneName)
    savedSceneDoc = sceneView.rows[0].doc
    attachmentFilename = savedSceneDoc['sceneName']
    attachmentFile = self.db.get_attachment(savedSceneDoc, attachmentFilename)
    sceneLoadFilename = slicer.app.temporaryPath + '/' + attachmentFilename
    with open(sceneLoadFilename, 'wb') as file:
      file.write(attachmentFile.read())
      slicer.util.loadScene(sceneLoadFilename)
    return


