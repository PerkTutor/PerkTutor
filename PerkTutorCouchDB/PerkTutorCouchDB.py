import os
import unittest
import vtk, qt, ctk, slicer
from slicer.ScriptedLoadableModule import *
import logging
import time
import sys
from functools import partial
import re
import subprocess

# Install couchdb if not available
try:
  import couchdb
except ImportError:
  slicer.util.pip_install('couchdb')
  import couchdb

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
    self.ptcLogic = PerkTutorCouchDBLogic()
    
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
    databaseCollapsibleGroupBox.setTitle( "Remote Database" )
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

    # File server session field
    self.fileServerSessionLineEdit = qt.QLineEdit()
    self.fileServerSessionLineEdit.setText( settings.value( self.moduleName + "/FileServerSession" ) )
    fileServerFormLayout.addRow( "Session", self.fileServerSessionLineEdit )
    
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
      self.ptcLogic.updateDatabase( PERK_TUTOR_DATABASE_NAME )
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

    settings = slicer.app.userSettings()
    serverSessionName = settings.value( self.moduleName + "/FileServerSession" )
    localDirectory = settings.value( self.moduleName + "/FileServerLocalDirectory" )
    serverFtpClient = settings.value( self.moduleName + "/FileServerClient" )
    
    self.ptcLogic.uploadSession( dataFields, self.saveNodeSelector.currentNode(), localDirectory, serverSessionName, serverFtpClient )

    
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
    
    settings.setValue( self.moduleName + "/FileServerSession", self.fileServerSessionLineEdit.text )
    settings.setValue( self.moduleName + "/FileServerLocalDirectory", self.fileServerLocalDirectoryLineEdit.text )
    settings.setValue( self.moduleName + "/FileServerClient", self.ftpClientDirectoryLineEdit.text )

    username = settings.value( self.moduleName + "/DatabaseUsername" )
    password = settings.value( self.moduleName + "/DatabasePassword" )
    address = settings.value( self.moduleName + "/DatabaseAddress" )
    fullRemoteAddress = username + ":" + password + "@" + address
    
    try:
      self.ptcLogic.updateDatabase( PERK_TUTOR_DATABASE_NAME, fullRemoteAddress )
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

    settings = slicer.app.userSettings()
    serverSessionName = settings.value( self.moduleName + "/FileServerSession" )
    localDirectory = settings.value( self.moduleName + "/FileServerLocalDirectory" )
    serverFtpClient = settings.value( self.moduleName + "/FileServerClient" )
    
    success = self.ptcLogic.loadSession( sessionFileName, sessionFileType, localDirectory, serverSessionName, serverFtpClient )
    if ( success ):
      logging.info( "Session " + sessionFileName + " loaded." )

    sender.close()

          
#
# PerkTutorCouchDBLogic
#

class PerkTutorCouchDBLogic(ScriptedLoadableModuleLogic):

  def __init__( self ):
    self.database = None
    self.replicatorDatabase = None

    
  def updateDatabase( self, databaseName, fullRemoteAddress = None ):
    # Create the database
    couchServer = couchdb.Server( LOCAL_SERVER_ADDRESS ) # uploads to localhost, replace with hostname
    if ( REPLICATOR_DATABASE_NAME in couchServer ):
      self.replicatorDatabase = couchServer[ REPLICATOR_DATABASE_NAME ]
    else:
      self.replicatorDatabase = couchServer.create( REPLICATOR_DATABASE_NAME )

    if ( databaseName in couchServer ):
      self.database = couchServer[ databaseName ]
    else:
      self.database = couchServer.create( databaseName )
      viewDoc = PerkTutorCouchDBLogic.createDefaultViewDoc()
      self.database.save( viewDoc )
      pushDoc, pullDoc = PerkTutorCouchDBLogic.createDefaultReplicatorDocs( self.database.name )
      self.replicatorDatabase.save( pushDoc )
      self.replicatorDatabase.save( pullDoc )

    if ( fullRemoteAddress ):
      self.updateReplicatorDocs( databaseName, fullRemoteAddress )


      
    
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
  def createDefaultReplicatorDocs( databaseName ):
    pushDoc = {
      "_id": databaseName + "_push",
      "source": LOCAL_SERVER_ADDRESS + databaseName,
      "target": "",
      "continuous": True,
      "owner": "admin"
    }

    pullDoc = {
      "_id": databaseName + "_pull",
      "target": LOCAL_SERVER_ADDRESS + databaseName,
      "source": "",
      "continuous": True,
      "owner": "admin"
    }

    return pushDoc, pullDoc


  def updateReplicatorDocs( self, databaseName, remoteAddress ):
    pushDocID = self.database.name + "_push"
    pushDoc = self.replicatorDatabase[ pushDocID ]
    pushDoc[ "target" ] = "http://" + remoteAddress + "/" + self.database.name
    self.replicatorDatabase[ pushDocID ] = pushDoc

    pullDocID = self.database.name + "_pull"
    pullDoc = self.replicatorDatabase[ pullDocID ]
    pullDoc[ "source" ] = "http://" + remoteAddress + "/" + self.database.name
    self.replicatorDatabase[ pullDocID ] = pullDoc


  def uploadSession( self, dataFields, sessionFileObject, localDirectory, serverSessionName, serverFtpClient ):
    # Most importantly, save a copy locally (this will also be used to upload attachment)
    settings = slicer.app.userSettings()
    if ( not os.path.exists( localDirectory ) ):
      os.makedirs( localDirectory ) # Make the directory if it doesn't already exist
      
    if ( sessionFileObject is None ): # We are saving the entire scene
      sessionFileType = "SceneFile"
      sessionFileBaseName = "Scene-" + time.strftime( "%Y-%m-%d-%H-%M-%S" ) + ".mrb"
      sessionFileFullName = os.path.join( localDirectory, sessionFileBaseName )
      saveSuccess = slicer.util.saveScene( sessionFileFullName )
    else: # The session contains a node (e.g. tracked sequence browser)
      sessionFileType, sessionFileExtension = PerkTutorCouchDBLogic.getNodeDefaultWriteTypeExtension( sessionFileObject )
      sessionFileBaseName = sessionFileObject.GetName() + "-" + time.strftime( "%Y-%m-%d-%H-%M-%S" ) + sessionFileExtension     
      sessionFileFullName = os.path.join( localDirectory, sessionFileBaseName )
      saveSuccess = slicer.util.saveNode( sessionFileObject, sessionFileFullName )

    # Abort with critical error if scene/node could not be saved at all
    if ( not saveSuccess ):
      logging.error( "PerkTutorCouchDBLogic::uploadSession: Could not save file to specified local path." )
      return

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
    self.syncToFileServer( localDirectory, serverSessionName, serverFtpClient )
    

  
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
      for key, value in searchFieldsDict.items():
        if ( ( key not in session.doc ) or ( value != "" and session.doc[ key ] != value ) ):
          invalidSession = True
      if invalidSession:
        continue
        
      currSessionData = [ session.doc[ "UserID" ], session.doc[ "StudyID" ], session.doc[ "TrialID" ], session.doc[ "SkillLevel" ], session.doc[ "Date" ], session.doc[ "SessionFileName" ], session.doc[ "SessionFileType" ] ]
      sessionData.append( currSessionData )
      
    return sessionData
    

  def loadSession( self, sessionFileName, sessionFileType, localDirectory, serverSessionName, serverFtpClient ):
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
    sessionFileFullName = os.path.join( localDirectory, sessionDoc[ "SessionFileName" ] )
    
    if ( not os.path.isfile( sessionFileFullName ) ):
      self.getFromFileServer( sessionDoc[ "SessionFileName" ], localDirectory, serverSessionName, serverFtpClient )
    
    success = slicer.util.loadNodeFromFile( sessionFileFullName, sessionDoc[ "SessionFileType" ] )
    return success

    
  def getFromFileServer( self, fileBaseName, localDirectory, serverSessionName, serverFtpClient ):
    settings = slicer.app.userSettings()
    
    getCommand = serverFtpClient + " " + serverSessionName + " /command " + "\"lcd \"\"" + localDirectory + "\"\"\"" + " \"get \"\"" + fileBaseName + "\"\"\"" + " \"exit\""
    
    getter = subprocess.Popen( getCommand, shell = True, stderr = subprocess.PIPE )
    logging.info( getter.communicate() )
    
    
  def syncToFileServer( self, localDirectory, serverSessionName, serverFtpClient ):
    settings = slicer.app.userSettings()
    
    syncCommand = serverFtpClient + " " + serverSessionName + " /command " + "\"lcd \"\"" + localDirectory + "\"\"\"" + " \"synchronize remote\"" + " \"exit\""
    
    syncer = subprocess.Popen( syncCommand, shell = True, stderr = subprocess.PIPE )
    logging.info( syncer.communicate() )
    

  @staticmethod
  # The problem is that some nodes have writers, but do not have storage nodes (e.g. Sequence Browser node)
  def getNodeDefaultWriteTypeExtension( node ):
    ioManager = slicer.app.coreIOManager()
    fileWriterFileType = ioManager.fileWriterFileType( node )
    fileWriterDefaultExtension = ioManager.fileWriterExtensions( node )[ 0 ]

    extensionRegex = "\\.\\w+\\b" # Search for everything after the first . until the next non-alphanumeric character
    extensionSpan = re.search( extensionRegex, fileWriterDefaultExtension ).span()
    extension = fileWriterDefaultExtension[ extensionSpan[ 0 ]:extensionSpan[ 1 ] ]

    return fileWriterFileType, extension
    



