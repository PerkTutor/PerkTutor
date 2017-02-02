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
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerTrackedSequenceBrowserReader.h"

// Logic includes
#include "vtkSlicerTransformRecorderLogic.h"

// MRML includes

// VTK includes
#include <vtkSmartPointer.h>

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
  return QStringList() << "Tracked Sequence Browser (*.xml)" << "Tracked Sequence Browser (*)";
}

//-----------------------------------------------------------------------------
bool qSlicerTrackedSequenceBrowserReader::load(const IOProperties& properties)
{
  Q_D(qSlicerTrackedSequenceBrowserReader);
  Q_ASSERT( properties.contains( "fileName" ) );
  QString fileName = properties[ "fileName" ].toString();

  QFileInfo fileInfo( fileName );
  QString extension = fileInfo.suffix();
  QString baseName = fileInfo.baseName();
  
  vtkSmartPointer< vtkMRMLSequenceBrowserNode > trackedSequenceBrowserNode;
  trackedSequenceBrowserNode.TakeReference( vtkMRMLSequenceBrowserNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLSequenceBrowserNode" ) ) );
  trackedSequenceBrowserNode->SetName( baseName.toStdString().c_str() );
  trackedSequenceBrowserNode->SetScene( this->mrmlScene() );
  this->mrmlScene()->AddNode( trackedSequenceBrowserNode );

  int modifyFlag = trackedSequenceBrowserNode->StartModify();
  bool loadSuccess = this->loadXML( trackedSequenceBrowserNode, fileName.toStdString() );
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
  vtkSlicerSequenceBrowserLogic* sbLogic = vtkSlicerSequenceBrowserLogic::SafeDownCast( vtkSlicerTransformRecorderLogic::GetSlicerModuleLogic( "SequenceBrowser" ) );
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
        vtkSmartPointer< vtkMRMLLinearTransformNode > proxyTransformNode;
        proxyTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( this->mrmlScene()->GetFirstNode( deviceName.c_str(), "vtkMRMLLinearTransformNode" ) );
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
      vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::New();
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
