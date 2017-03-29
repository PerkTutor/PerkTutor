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
#include "qSlicerWorkflowTrainingReader.h"

// Logic includes
#include "vtkSlicerWorkflowSegmentationLogic.h"

// MRML includes

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerWorkflowTrainingReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerWorkflowSegmentationLogic> WorkflowSegmentationLogic;
};

//-----------------------------------------------------------------------------
qSlicerWorkflowTrainingReader::qSlicerWorkflowTrainingReader( vtkSlicerWorkflowSegmentationLogic* newWorkflowSegmentationLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWorkflowTrainingReaderPrivate)
{
  this->setWorkflowSegmentationLogic( newWorkflowSegmentationLogic );
}

//-----------------------------------------------------------------------------
qSlicerWorkflowTrainingReader::~qSlicerWorkflowTrainingReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerWorkflowTrainingReader::setWorkflowSegmentationLogic(vtkSlicerWorkflowSegmentationLogic* newWorkflowSegmentationLogic)
{
  Q_D(qSlicerWorkflowTrainingReader);
  d->WorkflowSegmentationLogic = newWorkflowSegmentationLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerWorkflowSegmentationLogic* qSlicerWorkflowTrainingReader::WorkflowSegmentationLogic() const
{
  Q_D(const qSlicerWorkflowTrainingReader);
  return d->WorkflowSegmentationLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerWorkflowTrainingReader::description() const
{
  return "Workflow Training";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerWorkflowTrainingReader::fileType() const
{
  return QString("Workflow Training");
}

//-----------------------------------------------------------------------------
QStringList qSlicerWorkflowTrainingReader::extensions() const
{
  return QStringList() << "Workflow Training (*.xml)";
}

//-----------------------------------------------------------------------------
bool qSlicerWorkflowTrainingReader::load(const IOProperties& properties)
{
  Q_D(qSlicerWorkflowTrainingReader);
  Q_ASSERT( properties.contains("fileName") );
  QString fileName = properties["fileName"].toString();

  QFileInfo fileInfo( fileName );
  QString extension = fileInfo.suffix();
  QString baseName = fileInfo.baseName();
  
  vtkSmartPointer< vtkMRMLWorkflowTrainingNode > importTrainingNode;
  importTrainingNode.TakeReference( vtkMRMLWorkflowTrainingNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLWorkflowTrainingNode" ) ) );
  importTrainingNode->SetScene( this->mrmlScene() );
  this->mrmlScene()->AddNode( importTrainingNode );

  if ( extension.toStdString().compare( "xml" ) == 0 )
  {
    vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
    parser->SetFileName( fileName.toStdString().c_str() );
    parser->Parse();
    
    importTrainingNode->FromXMLElement( parser->GetRootElement() );
  }  
  
  importTrainingNode->SetName( baseName.toStdString().c_str() );
  importTrainingNode->Modified();

  return true; // TODO: Check to see read was successful first
}
