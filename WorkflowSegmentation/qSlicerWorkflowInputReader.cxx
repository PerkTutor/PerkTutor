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
#include "qSlicerWorkflowInputReader.h"

// Logic includes
#include "vtkSlicerWorkflowSegmentationLogic.h"

// MRML includes

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerWorkflowInputReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerWorkflowSegmentationLogic> WorkflowSegmentationLogic;
};

//-----------------------------------------------------------------------------
qSlicerWorkflowInputReader::qSlicerWorkflowInputReader( vtkSlicerWorkflowSegmentationLogic* newWorkflowSegmentationLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWorkflowInputReaderPrivate)
{
  this->setWorkflowSegmentationLogic( newWorkflowSegmentationLogic );
}

//-----------------------------------------------------------------------------
qSlicerWorkflowInputReader::~qSlicerWorkflowInputReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerWorkflowInputReader::setWorkflowSegmentationLogic(vtkSlicerWorkflowSegmentationLogic* newWorkflowSegmentationLogic)
{
  Q_D(qSlicerWorkflowInputReader);
  d->WorkflowSegmentationLogic = newWorkflowSegmentationLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerWorkflowSegmentationLogic* qSlicerWorkflowInputReader::WorkflowSegmentationLogic() const
{
  Q_D(const qSlicerWorkflowInputReader);
  return d->WorkflowSegmentationLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerWorkflowInputReader::description() const
{
  return "Workflow Input";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerWorkflowInputReader::fileType() const
{
  return QString("Workflow Input");
}

//-----------------------------------------------------------------------------
QStringList qSlicerWorkflowInputReader::extensions() const
{
  return QStringList() << "Workflow Input (*.xml)";
}

//-----------------------------------------------------------------------------
bool qSlicerWorkflowInputReader::load(const IOProperties& properties)
{
  Q_D(qSlicerWorkflowInputReader);
  Q_ASSERT( properties.contains("fileName") );
  QString fileName = properties["fileName"].toString();

  QFileInfo fileInfo( fileName );
  QString extension = fileInfo.suffix();
  QString baseName = fileInfo.baseName();
  
  vtkSmartPointer< vtkMRMLWorkflowInputNode > importInputNode;
  importInputNode.TakeReference( vtkMRMLWorkflowInputNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLWorkflowInputNode" ) ) );
  importInputNode->SetScene( this->mrmlScene() );
  this->mrmlScene()->AddNode( importInputNode );

  if ( extension.toStdString().compare( "xml" ) == 0 )
  {
    vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
    parser->SetFileName( fileName.toStdString().c_str() );
    parser->Parse();
    
    importInputNode->FromXMLElement( parser->GetRootElement() );
  }  
  
  importInputNode->SetName( baseName.toStdString().c_str() );
  importInputNode->Modified();

  return true; // TODO: Check to see read was successful first
}
