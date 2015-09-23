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
#include "qSlicerWorkflowProcedureReader.h"

// Logic includes
#include "vtkSlicerWorkflowSegmentationLogic.h"

// MRML includes

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerWorkflowProcedureReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerWorkflowSegmentationLogic> WorkflowSegmentationLogic;
};

//-----------------------------------------------------------------------------
qSlicerWorkflowProcedureReader::qSlicerWorkflowProcedureReader( vtkSlicerWorkflowSegmentationLogic* newWorkflowSegmentationLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWorkflowProcedureReaderPrivate)
{
  this->setWorkflowSegmentationLogic( newWorkflowSegmentationLogic );
}

//-----------------------------------------------------------------------------
qSlicerWorkflowProcedureReader::~qSlicerWorkflowProcedureReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerWorkflowProcedureReader::setWorkflowSegmentationLogic(vtkSlicerWorkflowSegmentationLogic* newWorkflowSegmentationLogic)
{
  Q_D(qSlicerWorkflowProcedureReader);
  d->WorkflowSegmentationLogic = newWorkflowSegmentationLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerWorkflowSegmentationLogic* qSlicerWorkflowProcedureReader::WorkflowSegmentationLogic() const
{
  Q_D(const qSlicerWorkflowProcedureReader);
  return d->WorkflowSegmentationLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerWorkflowProcedureReader::description() const
{
  return "Workflow Procedure";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerWorkflowProcedureReader::fileType() const
{
  return QString("Workflow Procedure");
}

//-----------------------------------------------------------------------------
QStringList qSlicerWorkflowProcedureReader::extensions() const
{
  return QStringList() << "Workflow Procedure (*.xml)";
}

//-----------------------------------------------------------------------------
bool qSlicerWorkflowProcedureReader::load(const IOProperties& properties)
{
  Q_D(qSlicerWorkflowProcedureReader);
  Q_ASSERT( properties.contains("fileName") );
  QString fileName = properties["fileName"].toString();

  QFileInfo fileInfo( fileName );
  QString extension = fileInfo.suffix();
  QString baseName = fileInfo.baseName();
  
  vtkSmartPointer< vtkMRMLWorkflowProcedureNode > importProcedureNode;
  importProcedureNode.TakeReference( vtkMRMLWorkflowProcedureNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLWorkflowProcedureNode" ) ) );
  importProcedureNode->SetScene( this->mrmlScene() );
  this->mrmlScene()->AddNode( importProcedureNode );

  if ( extension.toStdString().compare( "xml" ) == 0 )
  {
    vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
    parser->SetFileName( fileName.toStdString().c_str() );
    parser->Parse();
    
    importProcedureNode->FromXMLElement( parser->GetRootElement() );
  }  
  
  importProcedureNode->SetName( baseName.toStdString().c_str() );
  importProcedureNode->Modified();

  return true; // TODO: Check to see read was successful first
}
