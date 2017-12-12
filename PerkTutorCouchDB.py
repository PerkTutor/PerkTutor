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
import subprocess

PERK_TUTOR_DATABASE_NAME = "perk_tutor"
REPLICATOR_DATABASE_NAME = "_replicator"
DESIGN_DOC_VIEW_PATH = "_design/docs/_view/"

LOCAL_SERVER_ADDRESS = "http://127.0.0.1:5984/"

MAX_CONNECT_ATTEMPTS = 5

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
    self.modulePath = os.path.dirname( slicer.modules.perktutorcouchdb.path )
    
    self.ptcLogic = PerkTutorCouchDBLogic()
    self.ptcLogic.moduleName = self.moduleName
    self.ptcLogic.modulePath = self.modulePath
    
    #
    # Save Area
    #
    saveCollapsibleButton = ctk.ctkCollapsibleButton()
    saveCollapsibleButton.text = "Save Session"
    self.layout.addWidget( saveCollapsibleButton )
    # Layout within the collapsible button
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
    # Layout within the collapsible button
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
    configurationCollapsibleButton.text = "Configuration"
    configurationCollapsibleButton.collapsed = True
    self.layout.addWidget( configurationCollapsibleButton )
    # Layout within the collapsible button
    configurationVBoxLayout = qt.QVBoxLayout( configurationCollapsibleButton )
    
    settings = slicer.app.userSettings()
    
    
    #
    # Database
    #
    databaseCollapsibleGroupBox = ctk.ctkCollapsibleGroupBox()
    databaseCollapsibleGroupBox.setTitle( "Database" )
    databaseCollapsibleGroupBox.collapsed = True
    configurationVBoxLayout.addWidget( databaseCollapsibleGroupBox )
    # Layout within the group box
    databaseFormLayout = qt.QFormLayout( databaseCollapsibleGroupBox )    

    # Database username field
    self.databaseUsernameLineEdit = qt.QLineEdit()
    self.databaseUsernameLineEdit.setText( settings.value( self.moduleName + "/DatabaseUsername" ) )
    databaseFormLayout.addRow( "Username", self.databaseUsernameLineEdit )

    # Database password field
    self.databasePasswordLineEdit = qt.QLineEdit()
    self.databasePasswordLineEdit.setEchoMode( qt.QLineEdit.Password )
    self.databasePasswordLineEdit.setText( settings.value( self.moduleName + "/DatabasePassword" ) )    
    databaseFormLayout.addRow( "Password", self.databasePasswordLineEdit )

    # Remote database address
    self.databaseAddressLineEdit = qt.QLineEdit()
    self.databaseAddressLineEdit.setText( settings.value( self.moduleName + "/DatabaseAddress" ) )
    databaseFormLayout.addRow( "Address", self.databaseAddressLineEdit )
    
    
    #
    # File Server
    #
    fileServerCollapsibleGroupBox = ctk.ctkCollapsibleGroupBox()
    fileServerCollapsibleGroupBox.setTitle( "File Server" )
    fileServerCollapsibleGroupBox.collapsed = True
    configurationVBoxLayout.addWidget( fileServerCollapsibleGroupBox )
    # Layout within the group box
    fileServerFormLayout = qt.QFormLayout( fileServerCollapsibleGroupBox )    

    # File server username field
    self.fileServerUsernameLineEdit = qt.QLineEdit()
    self.fileServerUsernameLineEdit.setText( settings.value( self.moduleName + "/FileServerUsername" ) )
    fileServerFormLayout.addRow( "Username", self.fileServerUsernameLineEdit )

    # File server password field
    self.fileServerPasswordLineEdit = qt.QLineEdit()
    self.fileServerPasswordLineEdit.setEchoMode( qt.QLineEdit.Password )
    self.fileServerPasswordLineEdit.setText( settings.value( self.moduleName + "/FileServerPassword" ) )    
    fileServerFormLayout.addRow( "Password", self.fileServerPasswordLineEdit )

    # File server address
    self.fileServerAddressLineEdit = qt.QLineEdit()
    self.fileServerAddressLineEdit.setText( settings.value( self.moduleName + "/FileServerAddress" ) )
    fileServerFormLayout.addRow( "Address", self.fileServerAddressLineEdit )
    
    # Remote storage directory
    self.fileServerRemoteDirectoryLineEdit = qt.QLineEdit()
    self.fileServerRemoteDirectoryLineEdit.setText( settings.value( self.moduleName + "/FileServerRemoteDirectory" ) )
    fileServerFormLayout.addRow( "Remote path", self.fileServerRemoteDirectoryLineEdit )    
    
    # Local storage directory
    self.fileServerLocalDirectoryLineEdit = qt.QLineEdit()
    self.fileServerLocalDirectoryLineEdit.setText( settings.value( self.moduleName + "/FileServerLocalDirectory" ) )
    fileServerFormLayout.addRow( "Local path", self.fileServerLocalDirectoryLineEdit )

    # FTP client
    self.ftpClientDirectoryLineEdit = qt.QLineEdit()
    self.ftpClientDirectoryLineEdit.setText( settings.value( self.moduleName + "/FileServerClient" ) )
    fileServerFormLayout.addRow( "FTP client", self.ftpClientDirectoryLineEdit )
        
    
    # Update Button
    self.updateButton = qt.QPushButton( "Update" )
    self.updateButton.enabled = True
    configurationVBoxLayout.addWidget( self.updateButton )
    
    
    #
    # Stretcher
    #
    self.layout.addStretch( 1 )
    
    #
    # Initialize the remote database from the settings at outset (if the settings are already specified)
    #
    try:
      self.ptcLogic.initializeDatabaseFromSettings()
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
    
    settings.setValue( self.moduleName + "/DatabaseUsername", self.databaseUsernameLineEdit.text )
    settings.setValue( self.moduleName + "/DatabasePassword", self.databasePasswordLineEdit.text )
    settings.setValue( self.moduleName + "/DatabaseAddress", self.databaseAddressLineEdit.text )
    
    settings.setValue( self.moduleName + "/FileServerUsername", self.fileServerUsernameLineEdit.text )
    settings.setValue( self.moduleName + "/FileServerPassword", self.fileServerPasswordLineEdit.text )
    settings.setValue( self.moduleName + "/FileServerAddress", self.fileServerAddressLineEdit.text )
    settings.setValue( self.moduleName + "/FileServerRemoteDirectory", self.fileServerRemoteDirectoryLineEdit.text )
    settings.setValue( self.moduleName + "/FileServerLocalDirectory", self.fileServerLocalDirectoryLineEdit.text )
    settings.setValue( self.moduleName + "/FileServerClient", self.ftpClientDirectoryLineEdit.text )
    
    try:
      self.ptcLogic.initializeDatabaseFromSettings()
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
    
    success = self.ptcLogic.loadSession( sessionFileName, sessionFileType )    
    if ( success ):
      logging.info( "Session " + sessionFileName + " loaded." )

    sender.close()

          
#
# PerkTutorCouchDBLogic
#

class PerkTutorCouchDBLogic(ScriptedLoadableModuleLogic):

  def __init__( self ):
    self.database = None
      

  def initializeDatabaseFromSettings( self ):
    settings = slicer.app.userSettings()
    username = str( settings.value( self.moduleName + "/DatabaseUsername" ) )
    password = str( settings.value( self.moduleName + "/DatabasePassword" ) )
    address = str( settings.value( self.moduleName + "/DatabaseAddress" ) )
    
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
    settings = slicer.app.userSettings()   
    if ( sessionFileObject is None ): # We are saving the entire scene
      sessionFileType = "SceneFile"
      sessionFileBaseName = "Scene-" + time.strftime( "%Y-%m-%d-%H-%M-%S" ) + ".mrb"
      sessionFileFullName = os.path.join( settings.value( self.moduleName + "/FileServerLocalDirectory" ), sessionFileBaseName )
      slicer.util.saveScene( sessionFileFullName )
    else: # The session contains a node (e.g. tracked sequence browser)
      sessionFileType, sessionFileExtension = PerkTutorCouchDBLogic.getNodeDefaultWriteTypeExtension( sessionFileObject )
      sessionFileBaseName = sessionFileObject.GetName() + "-" + time.strftime( "%Y-%m-%d-%H-%M-%S" ) + sessionFileExtension     
      sessionFileFullName = os.path.join( settings.value( self.moduleName + "/FileServerLocalDirectory" ), sessionFileBaseName )
      slicer.util.saveNode( sessionFileObject, sessionFileFullName )

    # Now save it to the database
    if ( self.database is None ):
      logging.warning( "PerkTutorCouchDBLogic::uploadSession: Aborting due to unspecified database." )
      return
        
    dataFields[ "SessionFileName" ] = sessionFileBaseName
    dataFields[ "SessionFileType" ] = sessionFileType
    
    connectAttempts = MAX_CONNECT_ATTEMPTS
    while ( connectAttempts > 0 ):
      try:
        self.database.save( dataFields )
        break
      except:
        logging.warning( "PerkTutorCouchDBLogic::uploadSession: Attempt to connect to database failed. Attempts remaining: " + str( connectAttempts ) )
        connectAttempts = connectAttempts - 1
        
    # Try to sync (copy new local files to remote)
    self.syncToFileServer()
    

  
  def searchForSession( self, searchFieldsDict ):
    if ( self.database is None ):
      logging.warning( "PerkTutorCouchDBLogic::searchForSession: Aborting due to unspecified database" )
      return []

    connectAttempts = MAX_CONNECT_ATTEMPTS
    while ( connectAttempts > 0 ):
      try:
        allSessions = self.database.view( DESIGN_DOC_VIEW_PATH + "trial", include_docs = True )
        break
      except:
        logging.warning( "PerkTutorCouchDBLogic::loadSession: Attempt to connect to database failed. Attempts remaining: " + str( connectAttempts ) )
        connectAttempts = connectAttempts - 1
    
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
    
    connectAttempts = MAX_CONNECT_ATTEMPTS
    while ( connectAttempts > 0 ):
      try:
        queryResults = self.database.view( DESIGN_DOC_VIEW_PATH + "session", include_docs = True, key = [ sessionFileName, sessionFileType ] )
        break
      except:
        logging.warning( "PerkTutorCouchDBLogic::loadSession: Attempt to connect to database failed. Attempts remaining: " + str( connectAttempts ) )
        connectAttempts = connectAttempts - 1
        
    settings = slicer.app.userSettings()   
    sessionDoc = queryResults.rows[ 0 ].doc
    sessionFileFullName = os.path.join( settings.value( self.moduleName + "/FileServerLocalDirectory" ), sessionDoc[ "SessionFileName" ] )
    
    if ( not os.path.isfile( sessionFileFullName ) ):
      self.getFromFileServer( sessionDoc[ "SessionFileName" ] )
    
    success = slicer.util.loadNodeFromFile( sessionFileFullName, sessionDoc[ "SessionFileType" ] )
    return success

    
  def getFromFileServer( self, fileBaseName ):
    settings = slicer.app.userSettings()
    
    getCommand = os.path.join( self.modulePath, "Resources", "GetPerkTutorData.bat" ) + " "
    getCommand += str( settings.value( self.moduleName + "/FileServerUsername" ) ) + " "
    getCommand += str( settings.value( self.moduleName + "/FileServerPassword" ) ) + " "
    getCommand += str( settings.value( self.moduleName + "/FileServerAddress" ) ) + " "
    getCommand += str( settings.value( self.moduleName + "/FileServerRemoteDirectory" ) ) + " "
    getCommand += str( settings.value( self.moduleName + "/FileServerLocalDirectory" ) ) + " "
    getCommand += fileBaseName + " "
    getCommand += str( settings.value( self.moduleName + "/FileServerClient" ) ) + " "
    
    getter = subprocess.Popen( getCommand, shell = True, stderr = subprocess.PIPE )
    logging.info( getter.communicate() )
    
    
  def syncToFileServer( self ):
    settings = slicer.app.userSettings()
    
    syncCommand = os.path.join( self.modulePath, "Resources", "SyncPerkTutorData.bat" ) + " "
    syncCommand += str( settings.value( self.moduleName + "/FileServerUsername" ) ) + " "
    syncCommand += str( settings.value( self.moduleName + "/FileServerPassword" ) ) + " "
    syncCommand += str( settings.value( self.moduleName + "/FileServerAddress" ) ) + " "
    syncCommand += str( settings.value( self.moduleName + "/FileServerRemoteDirectory" ) ) + " "
    syncCommand += str( settings.value( self.moduleName + "/FileServerLocalDirectory" ) ) + " "
    syncCommand += "Placeholder" + " "
    syncCommand += str( settings.value( self.moduleName + "/FileServerClient" ) ) + " "
    
    syncer = subprocess.Popen( syncCommand, shell = True, stderr = subprocess.PIPE )
    logging.info( syncer.communicate() )
    

  @staticmethod
  # The problem is that some nodes have writers, but do not have storage nodes (e.g. Sequence Browser node)
  def getNodeDefaultWriteTypeExtension( node ):
    ioManager = slicer.app.coreIOManager()
    fileWriterFileType = ioManager.fileWriterFileType( node )
    fileWriterDefaultExtension = ioManager.fileWriterExtensions( node )[ 0 ]

    extensionRegex = "\.\\w+\\b" # Search for everything after the first . until the next non-alphanumeric character
    extensionSpan = re.search( extensionRegex, fileWriterDefaultExtension ).span()
    extension = fileWriterDefaultExtension[ extensionSpan[ 0 ]:extensionSpan[ 1 ] ]

    return fileWriterFileType, extension
    



