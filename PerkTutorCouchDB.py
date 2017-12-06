import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
import time
import sys
import couchdb
from functools import partial
import re

PERK_TUTOR_DATABASE_NAME = "perk_tutor"
REPLICATOR_DATABASE_NAME = "_replicator"
DESIGN_DOC_VIEW_PATH = "_design/docs/_view/"

LOCAL_SERVER_ADDRESS = "http://127.0.0.1:5984/"

SKILL_LEVEL_SELECTIONS = ( "Trainee", "Novice", "Intermediate", "Advanced", "Expert", "Unknown" )
SESSION_COMPLETION_SELECTIONS = ( "Complete", "Incomplete" )

SEARCH_TABLE_ID_COLUMN_WIDTH = 150
SEARCH_TABLE_NUMBER_OF_COLUMNS = 5

class PerkTutorCouchDB(ScriptedLoadableModule):

  def __init__( self, parent ):
    ScriptedLoadableModule.__init__( self, parent )
    self.parent.title = "Perk Tutor CouchDB"
    self.parent.categories = [ "Perk Tutor" ]
    self.parent.dependencies = []
    self.parent.contributors = [ "Christina Yan (Perk Lab; Queen's University), Matthew S. Holden (Perk Lab; Queen's University)" ]
    self.parent.helpText = """
    This is a scripted loadable module to upload and download Perk Tutor data to/from a distributed database.
    """
    self.parent.acknowledgementText = """
    Acknowledgements.
    """

#
# PerkTutorCouchDBWidget
#

class PerkTutorCouchDBWidget(ScriptedLoadableModuleWidget):

  def setup( self ):
    ScriptedLoadableModuleWidget.setup(self)
        
    #
    # Logic
    #
    self.ptcLogic = PerkTutorCouchDBLogic()
    
    #
    # Save Area
    #
    saveCollapsibleButton = ctk.ctkCollapsibleButton()
    saveCollapsibleButton.text = "Save Session"
    self.layout.addWidget( saveCollapsibleButton )

    # Layout within the dummy collapsible button
    saveFormLayout = qt.QFormLayout( saveCollapsibleButton )

    # User ID input
    self.saveUserIDLineEdit = qt.QLineEdit()
    saveFormLayout.addRow( "User ID", self.saveUserIDLineEdit )

    # Study ID input
    self.saveStudyIDLineEdit = qt.QLineEdit()
    saveFormLayout.addRow("Study ID", self.saveStudyIDLineEdit )

    # Trial ID input
    self.saveTrialIDLineEdit = qt.QLineEdit()
    saveFormLayout.addRow( "Trial ID", self.saveTrialIDLineEdit )

    # skill level selector
    self.saveSkillSelector = qt.QComboBox()
    self.saveSkillSelector.addItems( SKILL_LEVEL_SELECTIONS )
    saveFormLayout.addRow( "Skill level", self.saveSkillSelector )

    # session completed selector
    self.saveSessionCompletionSelector = qt.QComboBox()
    self.saveSessionCompletionSelector.addItems( SESSION_COMPLETION_SELECTIONS )
    saveFormLayout.addRow( "Status", self.saveSessionCompletionSelector )

    # Node to save selector
    self.saveNodeSelector = slicer.qMRMLNodeComboBox()
    self.saveNodeSelector.noneEnabled = True
    self.saveNodeSelector.removeEnabled = False
    self.saveNodeSelector.addEnabled = False
    self.saveNodeSelector.renameEnabled = False
    self.saveNodeSelector.setMRMLScene( slicer.mrmlScene )
    saveFormLayout.addRow( "Data", self.saveNodeSelector )

    # Save Button
    self.saveButton = qt.QPushButton( "Save" )
    self.saveButton.enabled = True
    saveFormLayout.addRow( self.saveButton )
    
    #
    # Search Area
    #
    searchCollapsibleButton = ctk.ctkCollapsibleButton()
    searchCollapsibleButton.text = "Search For Session"
    self.layout.addWidget( searchCollapsibleButton )

    # Layout within the dummy collapsible button
    searchFormLayout = qt.QFormLayout( searchCollapsibleButton )

    # Search user ID field
    self.searchUserIDLineEdit = qt.QLineEdit()
    searchFormLayout.addRow( "User ID", self.searchUserIDLineEdit )

    # Search study ID field
    self.searchStudyIDLineEdit = qt.QLineEdit()
    searchFormLayout.addRow( "Study ID", self.searchStudyIDLineEdit )

    # Search trial ID field
    self.searchTrialIDLineEdit = qt.QLineEdit()
    searchFormLayout.addRow( "Trial ID", self.searchTrialIDLineEdit )

    # Search skill level field
    self.searchSkillSelector = qt.QComboBox()
    self.searchSkillSelector.addItems( ( "", ) + SKILL_LEVEL_SELECTIONS )
    searchFormLayout.addRow( "Skill level", self.searchSkillSelector )

    # Search Button
    self.searchButton = qt.QPushButton( "Search" )
    self.searchButton.enabled = True
    searchFormLayout.addRow( self.searchButton )
    
    #
    # Configuration
    #
    configurationCollapsibleButton = ctk.ctkCollapsibleButton()
    configurationCollapsibleButton.text = "Remote Configuration"
    configurationCollapsibleButton.collapsed = True
    self.layout.addWidget( configurationCollapsibleButton )

    # Layout within the dummy collapsible button
    configurationFormLayout = qt.QFormLayout( configurationCollapsibleButton )
    
    settings = slicer.app.userSettings()

    # Database username field
    self.usernameLineEdit = qt.QLineEdit()
    self.usernameLineEdit.setText( settings.value( self.moduleName + "/Username" ) )
    configurationFormLayout.addRow( "Username", self.usernameLineEdit )

    # Database password field
    self.passwordLineEdit = qt.QLineEdit()
    self.passwordLineEdit.setText( settings.value( self.moduleName + "/Password" ) )
    configurationFormLayout.addRow( "Password", self.passwordLineEdit )

    # Remote database address
    self.addressLineEdit = qt.QLineEdit()
    self.addressLineEdit.setText( settings.value( self.moduleName + "/Address" ) )
    configurationFormLayout.addRow( "Address", self.addressLineEdit )
    
    # Update Button
    self.updateButton = qt.QPushButton( "Update" )
    self.updateButton.enabled = True
    configurationFormLayout.addRow( self.updateButton )
    
    # Initialize the remote database from the settings at outset (if the settings are already specified)
    try:
      self.ptcLogic.initializeDatabaseFromSettings( self.moduleName )
    except Exception as e:
      logging.warning( e )
    
    #
    # Connections
    #
    self.saveButton.connect( "clicked(bool)", self.onSaveButton )
    self.searchButton.connect( "clicked(bool)", self.onSearchButton )
    self.updateButton.connect( "clicked(bool)", self.onUpdateButton )

    
  def onSaveButton( self ):
    userID = ( "UserID", str( self.saveUserIDLineEdit.text ) )
    studyID = ( "StudyID", str( self.saveStudyIDLineEdit.text ) )
    trialID = ( "TrialID", str( self.saveTrialIDLineEdit.text ) )
    skillLevel = ( "SkillLevel", str( self.saveSkillSelector.currentText ) )
    status = ( "Status", str( self.saveSessionCompletionSelector.currentText ) )
    date = ( "Date", time.strftime( "%Y/%m/%d-%H:%M:%S" ) )
    metricsComputed = ( "MetricsComputed", False )
    dataFields = dict( [ userID, studyID, trialID, skillLevel, status, date, metricsComputed ] ) #creates dict from list of tuples, format for saving
    
    self.ptcLogic.uploadSession( dataFields, self.saveNodeSelector.currentNode() )

    
  def onSearchButton( self ):
    searchFieldsDict = dict()
    searchFieldsDict[ "UserID" ] = self.searchUserIDLineEdit.text
    searchFieldsDict[ "StudyID" ] = self.searchStudyIDLineEdit.text
    searchFieldsDict[ "TrialID" ] = self.searchTrialIDLineEdit.text
    searchFieldsDict[ "SkillLevel" ] = self.searchSkillSelector.currentText
    
    searchResults = self.ptcLogic.searchForSession( searchFieldsDict )
    self.searchResultsTable = self.displaySearchResults( searchResults )
    
    
  def onUpdateButton( self ):
    settings = slicer.app.userSettings()
    settings.setValue( self.moduleName + "/Username", self.usernameLineEdit.text )
    settings.setValue( self.moduleName + "/Password", self.passwordLineEdit.text )
    settings.setValue( self.moduleName + "/Address", self.addressLineEdit.text )
    
    try:
      self.ptcLogic.initializeDatabaseFromSettings( self.moduleName )
    except Exception as e:
      logging.warning( e )


  def displaySearchResults( self, searchResults ): # look into highlighting query rows or another option to remove session numbers
    # Add a table with all the search results
    searchResultsTable = qt.QTableWidget()
    searchResultsTable.setWindowTitle( "Sessions" )
    searchResultsTable.resize( 6 * SEARCH_TABLE_ID_COLUMN_WIDTH, 4 * SEARCH_TABLE_ID_COLUMN_WIDTH )
    
    searchResultsTable.setRowCount( len( searchResults ) )
    searchResultsTable.setColumnCount( SEARCH_TABLE_NUMBER_OF_COLUMNS )
    searchResultsTable.setHorizontalHeaderItem( 0, qt.QTableWidgetItem( "User ID" ) )
    searchResultsTable.setColumnWidth( 0, SEARCH_TABLE_ID_COLUMN_WIDTH )
    searchResultsTable.setHorizontalHeaderItem( 1, qt.QTableWidgetItem( "Study ID" ) )
    searchResultsTable.setColumnWidth( 1, SEARCH_TABLE_ID_COLUMN_WIDTH )
    searchResultsTable.setHorizontalHeaderItem( 2, qt.QTableWidgetItem( "Trial ID" ) )
    searchResultsTable.setColumnWidth( 2, SEARCH_TABLE_ID_COLUMN_WIDTH )
    searchResultsTable.setHorizontalHeaderItem( 3, qt.QTableWidgetItem( "Skill Level" ) )
    searchResultsTable.setColumnWidth( 3, SEARCH_TABLE_ID_COLUMN_WIDTH )
    searchResultsTable.setHorizontalHeaderItem( 4, qt.QTableWidgetItem( "Date" ) )
    searchResultsTable.setColumnWidth( 4, 2 * SEARCH_TABLE_ID_COLUMN_WIDTH )
    
    for row in range( len( searchResults ) ):
      for col in range( SEARCH_TABLE_NUMBER_OF_COLUMNS ):
        searchResultsTable.setItem( row, col, qt.QTableWidgetItem( searchResults[ row ][ col ] ) )
    
    searchResultsTable.show() 
    
    searchResultsTable.connect( "cellDoubleClicked(int,int)", partial( self.onLoadSession, searchResultsTable, searchResults ) )           


  def onLoadSession( self, sender, searchResults, row, column ):
    sessionFileName = searchResults[ row ][ -2 ]
    sessionFileType = searchResults[ row ][ -1 ]
    self.ptcLogic.loadSession( sessionFileName, sessionFileType )
    logging.info( "Session " + sessionFileName + " loaded." )

    sender.close()

          
#
# PerkTutorCouchDBLogic
#

class PerkTutorCouchDBLogic(ScriptedLoadableModuleLogic):

  def __init__( self ):
    self.database = None
    
    
  def initializeDatabaseFromSettings( self, moduleName ):
    settings = slicer.app.userSettings()
    username = str( settings.value( moduleName + "/Username" ) )
    password = str( settings.value( moduleName + "/Password" ) )
    address = str( settings.value( moduleName + "/Address" ) )
    
    remoteAddress = username + ":" + password + "@" + address
    self.initializeDatabase( PERK_TUTOR_DATABASE_NAME, remoteAddress )

    
  def initializeDatabase( self, databaseName, remoteAddress ):
    # Create the database
    couchServer = couchdb.Server( LOCAL_SERVER_ADDRESS ) # uploads to localhost, replace with hostname
    if ( REPLICATOR_DATABASE_NAME in couchServer ):
      replicatorDatabase = couchServer[ REPLICATOR_DATABASE_NAME ]
    else:
      replicatorDatabase = couchServer.create( REPLICATOR_DATABASE_NAME )

    if ( databaseName in couchServer ):
      self.database = couchServer[ databaseName ]
    else:
      self.database = couchServer.create( databaseName )
      self.database.save( PerkTutorCouchDBLogic.createDefaultViewDoc() )
      pushDoc, pullDoc = PerkTutorCouchDBLogic.createDefaultReplicatorDocs( databaseName, remoteAddress )
      replicatorDatabase.save( pushDoc )
      replicatorDatabase.save( pullDoc )

    
  @staticmethod  
  def createDefaultViewDoc():
    viewDoc = {
      "_id": "_design/docs",
      "language": "javascript",
      "views": {
        "trial": {
          "map": "function (doc) { emit( [ doc.UserID, doc.StudyID, doc.TrialID ], doc ) }"
        },
        "session": {
          "map": "function (doc) { emit( [ doc.SessionFileName, doc.SessionFileType ], doc ) }"
        }
      }
    }
    
    return viewDoc

    
  @staticmethod  
  def createDefaultReplicatorDocs( databaseName, remoteAddress ):
    pushDoc = {
      "_id": databaseName + "_push",
      "source": LOCAL_SERVER_ADDRESS + databaseName,
      "target": "http://" + remoteAddress + "/" + databaseName,
      "continuous": True,
      "owner": "admin"
    }

    pullDoc = {
      "_id": databaseName + "_pull",
      "target": LOCAL_SERVER_ADDRESS + databaseName,
      "source": "http://" + remoteAddress + "/" + databaseName,
      "continuous": True,
      "owner": "admin"
    }

    return pushDoc, pullDoc

  
  def uploadSession( self, dataFields, sessionFileObject ):
    # Most importantly, save a copy locally (this will also be used to upload attachment)
    if ( sessionFileObject is None ): # We are saving the entire scene
      sessionFileType = "SceneFile"
      sessionFileBaseName = "Scene-" + time.strftime( "%Y-%m-%d-%H-%M-%S" ) + ".mrb"
      tempSessionFileFullPath = slicer.app.temporaryPath + "/" + sessionFileBaseName
      slicer.util.saveScene( tempSessionFileFullPath )
    else: # The session contains a node (e.g. tracked sequence browser)
      sessionFileType, sessionFileExtension = PerkTutorCouchDBLogic.getNodeDefaultWriteTypeExtension( sessionFileObject )
      sessionFileBaseName = sessionFileObject.GetName() + "-" + time.strftime( "%Y-%m-%d-%H-%M-%S" ) + sessionFileExtension
      tempSessionFileFullPath = slicer.app.temporaryPath + "/" + sessionFileBaseName
      slicer.util.saveNode( sessionFileObject, tempSessionFileFullPath )

    if ( self.database is None ):
      logging.warning( "PerkTutorCouchDBLogic::uploadSession: Aborting due to unspecified database." )
      return
    
    # Now save it to the database
    dataFields[ "SessionFileName" ] = sessionFileBaseName
    dataFields[ "SessionFileType" ] = sessionFileType
    self.database.save( dataFields )
    
    with open( tempSessionFileFullPath, "rb" ) as file:
      self.database.put_attachment( dataFields, file )
    logging.info( "Session " + sessionFileBaseName + " saved." )

  
  def searchForSession( self, searchFieldsDict ):
    if ( self.database is None ):
      logging.warning( "PerkTutorCouchDBLogic::searchForSession: Aborting due to unspecified database" )
      return []

    viewPath = DESIGN_DOC_VIEW_PATH + "trial"
    allSessions = self.database.view( viewPath, include_docs = True )
    
    # Convert data in view into list of lists for displaying results
    # This manually filters out sessions that don't match the criteria
    # CouchDB is not really designed to work with queries with different inputs 
    sessionData = []
    for session in allSessions:
    
      invalidSession = False
      for ( key, value ) in searchFieldsDict.iteritems():
        if ( ( key not in session.doc ) or ( value != "" and session.doc[ key ] != value ) ):
          invalidSession = True
      if invalidSession:
        continue
        
      currSessionData = [ session.doc[ "UserID" ], session.doc[ "StudyID" ], session.doc[ "TrialID" ], session.doc[ "SkillLevel" ], session.doc[ "Date" ], session.doc[ "SessionFileName" ], session.doc[ "SessionFileType" ] ]
      sessionData.append( currSessionData )
      
    return sessionData
    

  def loadSession( self, sessionFileName, sessionFileType ):
    if ( self.database is None ):
      logging.warning( "PerkTutorCouchDBLogic::loadSession: Aborting due to unspecified database." )
      return
      
    queryResults = self.database.view( DESIGN_DOC_VIEW_PATH + "session", include_docs = True, key = [ sessionFileName, sessionFileType ] )
    savedSessionDoc = queryResults.rows[ 0 ].doc
    sessionAttachmentFileName = savedSessionDoc[ "SessionFileName" ]
    sessionAttachmentFile = self.database.get_attachment( savedSessionDoc, sessionAttachmentFileName )
    tempSessionLoadFileName = slicer.app.temporaryPath + "/" + sessionAttachmentFileName
    
    with open( tempSessionLoadFileName, "wb" ) as file:
      file.write( sessionAttachmentFile.read() )
    success = slicer.util.loadNodeFromFile( tempSessionLoadFileName, savedSessionDoc[ "SessionFileType" ] )

    return success


  @staticmethod
  # The problem is that some nodes have writers, but do not have storage nodes
  def getNodeDefaultWriteTypeExtension( node ):
    ioManager = slicer.app.coreIOManager()
    fileWriterFileType = ioManager.fileWriterFileType( node )
    fileWriterDefaultExtension = ioManager.fileWriterExtensions( node )[ 0 ]

    extensionRegex = "\.\\w+\\b" # Search for everything after the first . until the next non-alphanumeric character
    extensionSpan = re.search( extensionRegex, fileWriterDefaultExtension ).span()
    extension = fileWriterDefaultExtension[ extensionSpan[ 0 ]:extensionSpan[ 1 ] ]

    return fileWriterFileType, extension



