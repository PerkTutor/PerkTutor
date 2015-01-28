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
#include "qSlicerTransformRecorderIO.h"

// Logic includes
#include "vtkSlicerTransformRecorderLogic.h"

// MRML includes

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerTransformRecorderIOPrivate
{
public:
  vtkSmartPointer<vtkSlicerTransformRecorderLogic> TransformRecorderLogic;
};

//-----------------------------------------------------------------------------
qSlicerTransformRecorderIO::qSlicerTransformRecorderIO( vtkSlicerTransformRecorderLogic* newTransformRecorderLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTransformRecorderIOPrivate)
{
  this->setTransformRecorderLogic( newTransformRecorderLogic );
}

//-----------------------------------------------------------------------------
qSlicerTransformRecorderIO::~qSlicerTransformRecorderIO()
{
}

//-----------------------------------------------------------------------------
void qSlicerTransformRecorderIO::setTransformRecorderLogic(vtkSlicerTransformRecorderLogic* newTransformRecorderLogic)
{
  Q_D(qSlicerTransformRecorderIO);
  d->TransformRecorderLogic = newTransformRecorderLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerTransformRecorderLogic* qSlicerTransformRecorderIO::TransformRecorderLogic() const
{
  Q_D(const qSlicerTransformRecorderIO);
  return d->TransformRecorderLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerTransformRecorderIO::description() const
{
  return "Transform Buffer";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTransformRecorderIO::fileType() const
{
  return QString("Transform Buffer");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformRecorderIO::extensions() const
{
  return QStringList() << "Transform Buffer (*.xml)" << "Transform Buffer (*.mha)";
}

//-----------------------------------------------------------------------------
bool qSlicerTransformRecorderIO::load(const IOProperties& properties)
{
  Q_D(qSlicerTransformRecorderIO);
  Q_ASSERT( properties.contains("fileName") );
  QString fileName = properties["fileName"].toString();

  QFileInfo fileInfo( fileName );
  QString extension = fileInfo.suffix();
  QString baseName = fileInfo.baseName();
  
  vtkSmartPointer< vtkMRMLTransformBufferNode > importBufferNode;
  importBufferNode.TakeReference( vtkMRMLTransformBufferNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLTransformBufferNode" ) ) );
  importBufferNode->SetScene( this->mrmlScene() );
  this->mrmlScene()->AddNode( importBufferNode );

  if ( extension.toStdString().compare( "xml" ) == 0 )
  {
    d->TransformRecorderLogic->ImportFromXMLFile( importBufferNode, fileName.toStdString() );
  }  
  if ( extension.toStdString().compare( "mha" ) == 0 )
  {
    d->TransformRecorderLogic->ImportFromMHAFile( importBufferNode, fileName.toStdString() ); 
  }

  importBufferNode->SetActiveTransformsFromBuffer();
  d->TransformRecorderLogic->AddTransformsToScene( importBufferNode );

  std::stringstream importBufferName;
  importBufferName << importBufferNode->GetName() << "_" << baseName.toStdString();
  importBufferNode->SetName( importBufferName.str().c_str() );

  return true; // TODO: Check to see read was successful first
}
