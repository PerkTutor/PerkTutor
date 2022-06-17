/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDir>
#include <QDebug>
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerCoreApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "qSlicerTrackedSequenceBrowserReader.h"
#include "qSlicerTrackedSequenceBrowserReaderOptionsWidget.h"

// Logic includes
#include "vtkSlicerTransformRecorderLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkCollectionIterator.h>
#include <vtksys/SystemTools.hxx>
#include <vtkXMLDataParser.h>
#include <vtkMatrix4x4.h>

//-----------------------------------------------------------------------------
class qSlicerTrackedSequenceBrowserReaderPrivate
{
public:
  vtkSmartPointer< vtkSlicerTransformRecorderLogic > TransformRecorderLogic;
};

//-----------------------------------------------------------------------------
qSlicerTrackedSequenceBrowserReader::qSlicerTrackedSequenceBrowserReader( vtkSlicerTransformRecorderLogic* newTransformRecorderLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTrackedSequenceBrowserReaderPrivate)
{
  this->setTransformRecorderLogic( newTransformRecorderLogic );
}

//-----------------------------------------------------------------------------
qSlicerTrackedSequenceBrowserReader::~qSlicerTrackedSequenceBrowserReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerTrackedSequenceBrowserReader::setTransformRecorderLogic(vtkSlicerTransformRecorderLogic* newTransformRecorderLogic)
{
  Q_D(qSlicerTrackedSequenceBrowserReader);
  d->TransformRecorderLogic = newTransformRecorderLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerTransformRecorderLogic* qSlicerTrackedSequenceBrowserReader::TransformRecorderLogic() const
{
  Q_D(const qSlicerTrackedSequenceBrowserReader);
  return d->TransformRecorderLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerTrackedSequenceBrowserReader::description() const
{
  return "Tracked Sequence Browser";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTrackedSequenceBrowserReader::fileType() const
{
  return QString( "Tracked Sequence Browser" );
}

//-----------------------------------------------------------------------------
QStringList qSlicerTrackedSequenceBrowserReader::extensions() const
{
  return QStringList() << "Tracked Sequence Browser (*.sqbr)" << "Tracked Sequence Browser (*.xml)" << "Tracked Sequence Browser (*)";
}

//-----------------------------------------------------------------------------
qSlicerIOOptions* qSlicerTrackedSequenceBrowserReader::options()const
{
  // set the mrml scene on the options widget to allow selecting a color node
  qSlicerTrackedSequenceBrowserReaderOptionsWidget* options = new qSlicerTrackedSequenceBrowserReaderOptionsWidget;
  options->setMRMLScene(this->mrmlScene());
  return options;
}

//-----------------------------------------------------------------------------
bool qSlicerTrackedSequenceBrowserReader::load(const IOProperties& properties)
{
  Q_D(qSlicerTrackedSequenceBrowserReader);
  Q_ASSERT( properties.contains( "fileName" ) );
  QString fileName = properties[ "fileName" ].toString();
  bool useSceneProxyNodes = true;
  if ( properties.contains( "UseSceneProxyNodes" ) )
  {
    useSceneProxyNodes = properties[ "UseSceneProxyNodes" ].toBool();
  }

  QFileInfo fileInfo( fileName );
  QString extension = fileInfo.suffix();
  QString baseName = fileInfo.baseName();
  
  vtkSmartPointer< vtkMRMLSequenceBrowserNode > trackedSequenceBrowserNode;
  trackedSequenceBrowserNode.TakeReference( vtkMRMLSequenceBrowserNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLSequenceBrowserNode" ) ) );
  trackedSequenceBrowserNode->SetName( baseName.toStdString().c_str() );
  trackedSequenceBrowserNode->SetScene( this->mrmlScene() );
  this->mrmlScene()->AddNode( trackedSequenceBrowserNode );

  int modifyFlag = trackedSequenceBrowserNode->StartModify();
  bool loadSuccess = false;
  if ( extension.compare( "xml" ) == 0 )
  {
    loadSuccess = this->loadXML( trackedSequenceBrowserNode, fileName.toStdString() );
  }
  else
  {
    loadSuccess = this->loadSQBR( trackedSequenceBrowserNode, fileName.toStdString(), useSceneProxyNodes );
  }
  trackedSequenceBrowserNode->EndModify( modifyFlag );

  // Indicate if the node was successfully loaded
  if ( loadSuccess )
  {
    this->setLoadedNodes( QStringList( QString( trackedSequenceBrowserNode->GetID() ) ) );
  }

  return loadSuccess; // TODO: Check to see read was successful first
}


bool qSlicerTrackedSequenceBrowserReader
::loadXML( vtkMRMLSequenceBrowserNode* trackedSequenceBrowserNode, std::string fileName )
{
  Q_D(qSlicerTrackedSequenceBrowserReader);

  // Nedd access to the sequence browser logic for putting the sequences in the sequence browser
  vtkSlicerSequencesLogic* sbLogic = vtkSlicerSequencesLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "Sequences" ) );
  if ( sbLogic == NULL )
  {
    return NULL;
  }

  // Parse the XML file
  vtkNew< vtkXMLDataParser > parser;
  parser->SetFileName( fileName.c_str() );
  parser->Parse();
  vtkXMLDataElement* rootElement = parser->GetRootElement();

  // Verify that this is a legitimate XML file
  if ( rootElement == NULL || strcmp( rootElement->GetName(), "TransformRecorderLog" ) != 0 ) 
  {
    return false;
  }

  std::map< std::string, vtkMRMLSequenceNode* > deviceSequenceMap;
  int numElements = rootElement->GetNumberOfNestedElements();  // Number of saved records (including transforms and messages).
  
  for ( int i = 0; i < numElements; i++ )
  {
    vtkXMLDataElement* element = rootElement->GetNestedElement( i );

	  if ( element == NULL || strcmp( element->GetName(), "log" ) != 0 || element->GetAttribute( "type" ) == NULL )
	  {
      continue;
	  }

	  if ( strcmp( element->GetAttribute( "type" ), "transform" ) == 0 )
    {
      std::string transformString = element->GetAttribute( "transform" );
      std::string deviceName = element->GetAttribute( "DeviceName" );
      if ( transformString.compare( "" ) == 0 || deviceName.compare( "" ) == 0 )
      {
        continue;
      }

      // If the device name has never been previously encountered, create a sequence and proxy for it
      if ( deviceSequenceMap.find( deviceName ) == deviceSequenceMap.end() )
      {
        vtkSmartPointer< vtkMRMLLinearTransformNode > proxyTransformNode =
          vtkMRMLLinearTransformNode::SafeDownCast( this->mrmlScene()->GetFirstNode( deviceName.c_str(), "vtkMRMLLinearTransformNode" ) );
        if ( proxyTransformNode == NULL )
        {
          proxyTransformNode.TakeReference( vtkMRMLLinearTransformNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLLinearTransformNode" ) ) );
          proxyTransformNode->SetName( deviceName.c_str() );
          proxyTransformNode->SetScene( this->mrmlScene() );
	        this->mrmlScene()->AddNode( proxyTransformNode );
        }

        vtkMRMLSequenceNode* transformSequenceNode = sbLogic->AddSynchronizedNode( NULL, proxyTransformNode, trackedSequenceBrowserNode );
        trackedSequenceBrowserNode->SetRecording( transformSequenceNode, false );
        trackedSequenceBrowserNode->SetOverwriteProxyName( NULL, false );
        trackedSequenceBrowserNode->SetSaveChanges( NULL, false );

        deviceSequenceMap[ deviceName ] = transformSequenceNode;
      }

      // Set the transform in the sequence node
      vtkNew<vtkMRMLLinearTransformNode> transformNode;
      vtkNew< vtkMatrix4x4 > transformMatrix;
      vtkAddonMathUtilities::FromString( transformMatrix.GetPointer(), transformString );
      transformNode->SetMatrixTransformToParent( transformMatrix.GetPointer() );

      std::string timeString = this->getTimeStringFromXMLElement( element );

      deviceSequenceMap[ deviceName ]->SetDataNodeAtValue( transformNode, timeString );
    }

    if ( strcmp( element->GetAttribute( "type" ), "message" ) == 0 )
    {
      std::string messageString = element->GetAttribute( "message" );
      if ( messageString.compare( "" ) == 0 )
      {
        continue;
      }

      vtkMRMLSequenceNode* messagesSequenceNode = d->TransformRecorderLogic->GetMessageSequenceNode( trackedSequenceBrowserNode );
      if ( messagesSequenceNode == NULL )
      {
        continue;
      }

      vtkMRMLNode* messageNode = vtkMRMLScriptedModuleNode::New();
      messageNode->SetName( "Message" );
      messageNode->SetAttribute( "Message", messageString.c_str() );

      std::string timeString = this->getTimeStringFromXMLElement( element );

      messagesSequenceNode->SetDataNodeAtValue( messageNode, timeString );
    }
   
  }

  return true;
}


std::string qSlicerTrackedSequenceBrowserReader
::getTimeStringFromXMLElement( vtkXMLDataElement* element )
{
  Q_D(qSlicerTrackedSequenceBrowserReader);

  std::string timestampSecString = element->GetAttribute( "TimeStampSec" );
  std::string timestampNSecString = element->GetAttribute( "TimeStampNSec" );
  if ( timestampSecString.compare( "" ) == 0 || timestampNSecString.compare( "" ) == 0 )
  {
    return "";
  }

  // We can construct the timestamp
  std::stringstream timestampSecStream( timestampSecString );
  double timestampSec; timestampSecStream >> timestampSec;
  std::stringstream timestampNSecStream( timestampNSecString );
  double timestampNSec; timestampNSecStream >> timestampNSec;

  double time = ( timestampSec + 1.0e-9 * timestampNSec );

  std::stringstream timeStream;
  timeStream << time;

  return timeStream.str();
}



//-----------------------------------------------------------------------------
bool qSlicerTrackedSequenceBrowserReader
::loadSQBR( vtkMRMLSequenceBrowserNode* trackedSequenceBrowserNode, std::string fileName, bool useSceneProxyNodes )
{
  Q_D(qSlicerTrackedSequenceBrowserReader);

  // Create a new scene to house the tracked sequence browser node in 
  vtkNew< vtkMRMLScene > tempScene;
  tempScene->SetDataIOManager( vtkNew< vtkDataIOManager >() ); // Needed to read sequences saved in mrb format
  tempScene->GetDataIOManager()->SetCacheManager( vtkNew< vtkCacheManager >() ); // Needed to read sequences saved in mrb format
  this->mrmlScene()->CopyRegisteredNodesToScene( tempScene.GetPointer() ); // This registers node classes into the sequence browser scene
  tempScene->Clear( true );

  // Load into the temp scene
  vtkNew< vtkMRMLApplicationLogic > appLogic;
  appLogic->SetAndObserveMRMLScene( tempScene.GetPointer() );

  // TODO: Use QTemporaryDir from Qt5 when available
  QFileInfo fileInfo( QString( fileName.c_str() ) );
  QString tempPath = qSlicerCoreApplication::application()->temporaryPath() + "/" + fileInfo.baseName();

  // Need to try to create a temporary directory to facilitate saving
  if ( ! vtksys::SystemTools::MakeDirectory( tempPath.toStdString() ) )
  {
    qWarning() << "Could not make a temporary directory for unpacking the sequence browser.";
    return false;
  }

  // Unpack into temp and open
  if ( ! appLogic->OpenSlicerDataBundle( fileName.c_str(), tempPath.toStdString().c_str() ) ) // This adds the saved scene file into Slicer (does not overwrite existing nodes)
  {
    qWarning() << "Could not open file from the temporary directory.";
    return false;
  }

  // Delete the temporary directory
  if ( ! vtksys::SystemTools::RemoveADirectory( tempPath.toStdString() ) )
  {
    qWarning() << "Could not remove the temporary directory for unpacking the sequence browser.";
    return false;
  }


  vtkMRMLSequenceBrowserNode* tempTrackedSequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast( tempScene->GetFirstNode( NULL, "vtkMRMLSequenceBrowserNode" ) );
  if ( tempTrackedSequenceBrowserNode == NULL )
  {
    qWarning() << "Could not find sequence browser node in temporary scene.";
    return false;
  }
  trackedSequenceBrowserNode->Copy( tempTrackedSequenceBrowserNode );
  trackedSequenceBrowserNode->SetScene( tempScene.GetPointer() ); // Allows removing node references properly
  trackedSequenceBrowserNode->RemoveAllSequenceNodes(); // Deletes references to sequences from temp scene. We will re-add references to sequence nodes in the main scene.
  trackedSequenceBrowserNode->SetScene( this->mrmlScene() ); // Will allow proxy nodes to be added to the scene
  

  // Now copy and add the sequence and proxy nodes to the main scene
  vtkNew< vtkCollection > tempSequenceNodes;
  tempTrackedSequenceBrowserNode->GetSynchronizedSequenceNodes( tempSequenceNodes.GetPointer(), true );
  vtkNew< vtkCollection > tempProxyNodes;
  tempTrackedSequenceBrowserNode->GetAllProxyNodes( tempProxyNodes.GetPointer() );
  vtkNew< vtkCollectionIterator > tempSequenceNodesIt; tempSequenceNodesIt->SetCollection( tempSequenceNodes.GetPointer() );
  for ( tempSequenceNodesIt->InitTraversal(); ! tempSequenceNodesIt->IsDoneWithTraversal(); tempSequenceNodesIt->GoToNextItem() )
  {
    vtkMRMLSequenceNode* currTempSequenceNode = vtkMRMLSequenceNode::SafeDownCast( tempSequenceNodesIt->GetCurrentObject() );
    if ( currTempSequenceNode == NULL )
    {
      continue;
    }
    vtkSmartPointer< vtkMRMLSequenceNode > currSequenceNode = vtkSmartPointer< vtkMRMLSequenceNode >::New();
    currSequenceNode->Copy( currTempSequenceNode );
    currSequenceNode->SetScene( this->mrmlScene() );
    this->mrmlScene()->AddNode( currSequenceNode );
    trackedSequenceBrowserNode->AddSynchronizedSequenceNode( currSequenceNode );

    trackedSequenceBrowserNode->SetPlayback( currSequenceNode, tempTrackedSequenceBrowserNode->GetPlayback( currTempSequenceNode ) );
    trackedSequenceBrowserNode->SetRecording( currSequenceNode, tempTrackedSequenceBrowserNode->GetRecording( currTempSequenceNode ) );
    trackedSequenceBrowserNode->SetOverwriteProxyName( currSequenceNode, tempTrackedSequenceBrowserNode->GetOverwriteProxyName( currTempSequenceNode ) );
    trackedSequenceBrowserNode->SetSaveChanges( currSequenceNode, tempTrackedSequenceBrowserNode->GetSaveChanges( currTempSequenceNode ) );

    // Note that if the proxy nodes were copies, they will be removed above, but will be regenerated automatically
    vtkMRMLNode* currTempProxyNode = tempTrackedSequenceBrowserNode->GetProxyNode( currTempSequenceNode );
    if ( currTempProxyNode == NULL )
    {
      continue;
    }

    vtkSmartPointer< vtkMRMLNode > currProxyNode = this->mrmlScene()->GetFirstNode( currTempProxyNode->GetName(), currTempProxyNode->GetClassName() );
    if ( ! useSceneProxyNodes || currProxyNode == NULL )
    {
      currProxyNode = currTempProxyNode->CreateNodeInstance();
      currProxyNode->Copy( currTempProxyNode );
      currProxyNode->SetScene( this->mrmlScene() );
      this->mrmlScene()->AddNode( currProxyNode );
    }
    trackedSequenceBrowserNode->AddProxyNode( currProxyNode, currSequenceNode, false );

    this->copyNodeAttributes( currTempProxyNode, currProxyNode );

    // Create default display nodes for any displayable nodes
    vtkMRMLDisplayableNode* currDisplayableProxyNode = vtkMRMLDisplayableNode::SafeDownCast( currProxyNode );
    if ( currDisplayableProxyNode != NULL )
    {
      currDisplayableProxyNode->CreateDefaultDisplayNodes();
    }
  }

  return true;
}


//----------------------------------------------------------------------------
void qSlicerTrackedSequenceBrowserReader
::copyNodeAttributes( vtkMRMLNode* sourceNode, vtkMRMLNode* targetNode )
{
  std::vector< std::string > attributeNames = sourceNode->GetAttributeNames();
  std::vector< std::string >::iterator attributeNamesIt;
  for ( attributeNamesIt = attributeNames.begin(); attributeNamesIt != attributeNames.end(); attributeNamesIt++ )
  {
    const char* currAttributeName = ( *attributeNamesIt ).c_str();
    const char* currAttributeValue = sourceNode->GetAttribute( ( *attributeNamesIt ).c_str() );
    if ( currAttributeValue == NULL )
    {
      continue;
    }
    targetNode->SetAttribute( currAttributeName, currAttributeValue );
  }
}
