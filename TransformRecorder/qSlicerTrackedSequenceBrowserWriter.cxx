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
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QPixmap>

// CTK includes
#include <ctkMessageBox.h>
#include <ctkUtils.h>

// QtCore includes
#include "qMRMLUtils.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerTrackedSequenceBrowserWriter.h"
#include "vtkSlicerApplicationLogic.h"

// MRML includes
#include <vtkMRMLScene.h>
#include <vtkMRMLStorageNode.h>
#include <vtkMRMLNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>
#include <vtkDirectory.h>

//-----------------------------------------------------------------------------
class qSlicerTrackedSequenceBrowserWriterPrivate
{
public:
  vtkSmartPointer< vtkSlicerTransformRecorderLogic > TransformRecorderLogic;
};

//-----------------------------------------------------------------------------
qSlicerTrackedSequenceBrowserWriter::qSlicerTrackedSequenceBrowserWriter( vtkSlicerTransformRecorderLogic* newTransformRecorderLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTrackedSequenceBrowserWriterPrivate)
{
  this->setTransformRecorderLogic( newTransformRecorderLogic );
}

//----------------------------------------------------------------------------
qSlicerTrackedSequenceBrowserWriter::~qSlicerTrackedSequenceBrowserWriter()
{
}

//-----------------------------------------------------------------------------
void qSlicerTrackedSequenceBrowserWriter::setTransformRecorderLogic(vtkSlicerTransformRecorderLogic* newTransformRecorderLogic)
{
  Q_D(qSlicerTrackedSequenceBrowserWriter );
  d->TransformRecorderLogic = newTransformRecorderLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerTransformRecorderLogic* qSlicerTrackedSequenceBrowserWriter::TransformRecorderLogic() const
{
  Q_D(const qSlicerTrackedSequenceBrowserWriter);
  return d->TransformRecorderLogic;
}

//----------------------------------------------------------------------------
QString qSlicerTrackedSequenceBrowserWriter::description()const
{
  return tr( "Tracked Sequence Browser" );
}

//----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTrackedSequenceBrowserWriter::fileType()const
{
  return QString( "Tracked Sequence Browser" );
}

//----------------------------------------------------------------------------
bool qSlicerTrackedSequenceBrowserWriter::canWriteObject(vtkObject* object)const
{
  return vtkMRMLSequenceBrowserNode::SafeDownCast(object);
}

//----------------------------------------------------------------------------
QStringList qSlicerTrackedSequenceBrowserWriter::extensions(vtkObject* object)const
{
  Q_UNUSED(object);
  return QStringList() << "Tracked Sequence Browser (*.sqbr)" << "Tracked Sequence Browser (*.xml)" << "Tracked Sequence Browser (*)";
}

//----------------------------------------------------------------------------
bool qSlicerTrackedSequenceBrowserWriter::write(const qSlicerIO::IOProperties& properties)
{
  Q_D(qSlicerTrackedSequenceBrowserWriter);
  Q_ASSERT( properties.contains( "fileName" ) );
  QString fileName = properties[ "fileName" ].toString();

  QFileInfo fileInfo( fileName );
  QString extension = fileInfo.suffix();
  QString baseName = fileInfo.baseName();

  Q_ASSERT( properties.contains( "nodeID" ) );
  QString nodeID = properties[ "nodeID" ].toString();

  vtkMRMLSequenceBrowserNode* trackedSequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast( this->mrmlScene()->GetNodeByID( nodeID.toStdString() ) );
  if ( ! this->canWriteObject( trackedSequenceBrowserNode ) )
  {
    return false;
  }

  bool writeSuccess = false;
  if ( extension.compare( "xml" ) == 0 )
  {
    writeSuccess = this->writeXML( trackedSequenceBrowserNode, fileName.toStdString() );
  }
  else
  {
    writeSuccess = this->writeSQBR( trackedSequenceBrowserNode, fileName.toStdString() );
  }

  // Indicate if the node was successfully written
  if ( writeSuccess )
  {
    this->setWrittenNodes( QStringList( QString( trackedSequenceBrowserNode->GetID() ) ) );
  }

  return writeSuccess; // TODO: Check to see read was successful first
}

//----------------------------------------------------------------------------
bool qSlicerTrackedSequenceBrowserWriter
::writeXML( vtkMRMLSequenceBrowserNode* trackedSequenceBrowserNode, std::string fileName )
{
  Q_D(qSlicerTrackedSequenceBrowserWriter);
  // Check whether the file can be opened at all
  std::ofstream output( fileName.c_str() );
  if ( ! output.is_open() )
  {
    return false;
  }

  // Note that the transform recorder log format does not assume sorting
  // This means we can just throw everything into the file without regard for ordering

  vtkIndent indent;
  std::stringstream xmlStream;
  xmlStream << indent << "<TransformRecorderLog>" << std::endl;

  vtkNew< vtkCollection > sequenceNodes;
  trackedSequenceBrowserNode->GetSynchronizedSequenceNodes( sequenceNodes.GetPointer(), true );
  vtkNew< vtkCollectionIterator > sequenceNodesIt; sequenceNodesIt->SetCollection( sequenceNodes.GetPointer() );
  for ( sequenceNodesIt->InitTraversal(); ! sequenceNodesIt->IsDoneWithTraversal(); sequenceNodesIt->GoToNextItem() )
  {
    vtkMRMLSequenceNode* currSequenceNode = vtkMRMLSequenceNode::SafeDownCast( sequenceNodesIt->GetCurrentObject() );
    vtkMRMLLinearTransformNode* currProxyNode = vtkMRMLLinearTransformNode::SafeDownCast( trackedSequenceBrowserNode->GetProxyNode( currSequenceNode ) );
    if ( currSequenceNode == NULL || currProxyNode == NULL )
    {
      continue;
    }
    for ( int i = 0; i < currSequenceNode->GetNumberOfDataNodes(); i++ )
    {
      vtkMRMLLinearTransformNode* currTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( currSequenceNode->GetNthDataNode( i ) );
      if ( currTransformNode == NULL )
      {
        continue;
      }

      // Get the matrix and the time
      vtkNew< vtkMatrix4x4 > transformMatrix;
      currTransformNode->GetMatrixTransformToParent( transformMatrix.GetPointer() );
      std::string transformString = vtkAddonMathUtilities::ToString( transformMatrix.GetPointer() );

      std::string timeXMLString = this->getXMLStringFromTimeString( currSequenceNode->GetNthIndexValue( i ) );

      // Finally write it to XML
      xmlStream << indent.GetNextIndent();
      xmlStream << "<log";
      xmlStream << " " << timeXMLString;
      xmlStream << " type=\"transform\"";
      xmlStream << " DeviceName=\"" << currProxyNode->GetName() << "\"";
      xmlStream << " transform=\"" << transformString << "\"";
      xmlStream << " />" << std::endl;
    }

  }

  // Need a special case for messages because they are written differently than transforms
  vtkMRMLSequenceNode* messagesSequenceNode = d->TransformRecorderLogic->GetMessageSequenceNode( trackedSequenceBrowserNode );
  if ( messagesSequenceNode != NULL )
  {
    for ( int i = 0; i < messagesSequenceNode->GetNumberOfDataNodes(); i++ )
    {
      vtkMRMLNode* currMessageNode = vtkMRMLNode::SafeDownCast( messagesSequenceNode->GetNthDataNode( i ) );
      if ( currMessageNode == NULL )
      {
        continue;
      }

      std::string messageString = currMessageNode->GetAttribute( "Message" );
      if ( messageString.compare( "" ) == 0 )
      {
        continue;
      }

      std::string timeXMLString = this->getXMLStringFromTimeString( messagesSequenceNode->GetNthIndexValue( i ) );

      // Finally write it to XML
      xmlStream << indent.GetNextIndent();
      xmlStream << "<log";
      xmlStream << " " << timeXMLString;
      xmlStream << " type=\"message\"";
      xmlStream << " message=\"" << messageString << "\"";
      xmlStream << " />" << std::endl;
    }
  }

  xmlStream << indent << "</TransformRecorderLog>" << std::endl;

  output << xmlStream.str();
  output.close();

  return true;
}


//----------------------------------------------------------------------------
std::string qSlicerTrackedSequenceBrowserWriter
::getXMLStringFromTimeString( std::string timeString )
{
  std::stringstream timeStream( timeString );
  double time; timeStream >> time;

  long int timestampSec = floor( time );
  long int timestampNSec = 1.0e+9 * ( time - timestampSec );

  std::stringstream timeXMLStream;
  timeXMLStream << "TimeStampSec=\"" << timestampSec << "\"";
  timeXMLStream << " TimeStampNSec=\"" << timestampNSec << "\"";

  return timeXMLStream.str();
}



//----------------------------------------------------------------------------
bool qSlicerTrackedSequenceBrowserWriter
::writeSQBR( vtkMRMLSequenceBrowserNode* trackedSequenceBrowserNode, std::string fileName )
{
  Q_D(qSlicerTrackedSequenceBrowserWriter);

  // The idea is to create a new empty scene, put the sequence browser (and associated sequence and proxy nodes in it)
  // This can be saved and reloaded

  // Create a new scene to house the tracked sequence browser node in
  vtkNew< vtkMRMLScene > tempScene;
  this->mrmlScene()->CopyRegisteredNodesToScene( tempScene.GetPointer() ); // This registers node classes into the sequence browser scene
  tempScene->Clear( true );

  vtkMRMLSequenceBrowserNode* tempTrackedSequenceBrowserNode = vtkMRMLSequenceBrowserNode::SafeDownCast( tempScene->CopyNode( trackedSequenceBrowserNode ) );
  if ( tempTrackedSequenceBrowserNode == NULL )
  {
    qWarning() << "Could not copy node to temporary scene.";
    return false;
  }
  tempTrackedSequenceBrowserNode->SetScene( this->mrmlScene() ); // Allows removing node references properly
  tempTrackedSequenceBrowserNode->RemoveAllSequenceNodes(); // Deletes references to sequence from main scene. We will re-add references to nodes in the saving scene.
  tempTrackedSequenceBrowserNode->SetScene( tempScene.GetPointer() ); // Will allow proxy nodes to be added to the scene

  // Now copy and add the sequence and proxy nodes to the new scene
  vtkNew< vtkCollection > sequenceNodes;
  trackedSequenceBrowserNode->GetSynchronizedSequenceNodes( sequenceNodes.GetPointer(), true );
  vtkNew< vtkCollectionIterator > sequenceNodesIt; sequenceNodesIt->SetCollection( sequenceNodes.GetPointer() );
  for ( sequenceNodesIt->InitTraversal(); ! sequenceNodesIt->IsDoneWithTraversal(); sequenceNodesIt->GoToNextItem() )
  {
    vtkMRMLSequenceNode* currSequenceNode = vtkMRMLSequenceNode::SafeDownCast( sequenceNodesIt->GetCurrentObject() );
    if ( currSequenceNode == NULL )
    {
      continue;
    }
    vtkSmartPointer< vtkMRMLSequenceNode > currTempSequenceNode = vtkSmartPointer< vtkMRMLSequenceNode >::New();
    currTempSequenceNode->Copy( currSequenceNode );
    currTempSequenceNode->SetScene( tempScene.GetPointer() );
    tempScene->AddNode( currTempSequenceNode );
    tempTrackedSequenceBrowserNode->AddSynchronizedSequenceNode( currTempSequenceNode );

    tempTrackedSequenceBrowserNode->SetPlayback( currTempSequenceNode, trackedSequenceBrowserNode->GetPlayback( currSequenceNode ) );
    tempTrackedSequenceBrowserNode->SetRecording( currTempSequenceNode, trackedSequenceBrowserNode->GetRecording( currSequenceNode ) );
    tempTrackedSequenceBrowserNode->SetOverwriteProxyName( currTempSequenceNode, trackedSequenceBrowserNode->GetOverwriteProxyName( currSequenceNode ) );
    tempTrackedSequenceBrowserNode->SetSaveChanges( currTempSequenceNode, trackedSequenceBrowserNode->GetSaveChanges( currSequenceNode ) );

    // Note that if the proxy nodes were copies, they will be removed above, but will be regenerated automatically
    vtkMRMLNode* currProxyNode = trackedSequenceBrowserNode->GetProxyNode( currSequenceNode );
    if ( currProxyNode == NULL )
    {
      continue;
    }
    vtkSmartPointer<vtkMRMLNode> currTempProxyNode = vtkSmartPointer<vtkMRMLNode>::Take(currProxyNode->CreateNodeInstance());
    currTempProxyNode->Copy( currProxyNode );
    currTempProxyNode->SetScene( tempScene.GetPointer() );
    tempScene->AddNode( currTempProxyNode );
    tempTrackedSequenceBrowserNode->AddProxyNode( currTempProxyNode, currTempSequenceNode, false );

    this->copyNodeAttributes( currProxyNode, currTempProxyNode );
    currTempProxyNode->UpdateReferences();

    if (currSequenceNode->GetStorageNode())
    {
      // Use the same class of sequence storage node that is used in the scene
      vtkSmartPointer<vtkMRMLStorageNode> currSequenceStorageNode = currSequenceNode->GetStorageNode();
      vtkSmartPointer<vtkMRMLStorageNode> currTempSequenceStorageNode = vtkSmartPointer<vtkMRMLStorageNode>::Take(vtkMRMLStorageNode::SafeDownCast(currSequenceStorageNode->CreateNodeInstance()));
      currTempSequenceStorageNode->Copy(currSequenceStorageNode);
      tempScene->AddNode(currTempSequenceStorageNode);
      currTempSequenceStorageNode->UpdateReferences();
      currTempSequenceNode->SetAndObserveStorageNodeID(currTempSequenceStorageNode->GetID());
    }
  }

  // Remove references to all other nodes
  tempScene->RemoveUnusedNodeReferences();

  // Save the newly created scene into a scene bundle
  vtkNew< vtkMRMLApplicationLogic > appLogic;
  appLogic->SetAndObserveMRMLScene( tempScene.GetPointer() );

  // TODO: Use QTemporaryDir from Qt5 when available
  QFileInfo fileInfo( QString( fileName.c_str() ) );
  QString tempPath = qSlicerCoreApplication::application()->temporaryPath() + "/" + fileInfo.baseName();

  // TODO: Use Qt mkpath and Qt removeRecursively from Qt5 when available
  // Need to try to create a temporary directory to facilitate saving
  if ( ! vtkDirectory::MakeDirectory( tempPath.toLatin1() ) )
  {
    qWarning() << "Could not make a temporary directory for storing the sequence browser.";
    return false;
  }

  // Saving to temp and zipping
  if ( ! appLogic->SaveSceneToSlicerDataBundleDirectory( tempPath.toStdString().c_str() ) )
  {
    qWarning() << "Could not write the file to the temporary directory.";
    return false;
  }
  if ( ! appLogic->Zip( fileName.c_str(), tempPath.toStdString().c_str() ) )
  {
    qWarning() << "Unable to write the data from the temporary directory to the compressed file.";
    return false;
  }

  // Delete the temporary directory
  if ( ! vtkDirectory::DeleteDirectory( tempPath.toLatin1() ) )
  {
    qWarning() << "Could not remove the temporary directory for storing the sequence browser.";
    return false;
  }

  return true;
}


//----------------------------------------------------------------------------
void qSlicerTrackedSequenceBrowserWriter
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