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
#include "qSlicerTransformBufferReader.h"

// Logic includes
#include "vtkSlicerTransformRecorderLogic.h"

// MRML includes

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerTransformBufferReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerTransformRecorderLogic> TransformRecorderLogic;
};

//-----------------------------------------------------------------------------
qSlicerTransformBufferReader::qSlicerTransformBufferReader( vtkSlicerTransformRecorderLogic* newTransformRecorderLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTransformBufferReaderPrivate)
{
  this->setTransformRecorderLogic( newTransformRecorderLogic );
}

//-----------------------------------------------------------------------------
qSlicerTransformBufferReader::~qSlicerTransformBufferReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerTransformBufferReader::setTransformRecorderLogic(vtkSlicerTransformRecorderLogic* newTransformRecorderLogic)
{
  Q_D(qSlicerTransformBufferReader);
  d->TransformRecorderLogic = newTransformRecorderLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerTransformRecorderLogic* qSlicerTransformBufferReader::TransformRecorderLogic() const
{
  Q_D(const qSlicerTransformBufferReader);
  return d->TransformRecorderLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerTransformBufferReader::description() const
{
  return "Transform Buffer";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTransformBufferReader::fileType() const
{
  return QString("Transform Buffer");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTransformBufferReader::extensions() const
{
  return QStringList() << "Transform Buffer (*.xml)" << "Transform Buffer (*.mha)" << "Transform Buffer (*)";
}

//-----------------------------------------------------------------------------
bool qSlicerTransformBufferReader::load(const IOProperties& properties)
{
  Q_D(qSlicerTransformBufferReader);
  Q_ASSERT( properties.contains("fileName") );
  QString fileName = properties["fileName"].toString();

  QFileInfo fileInfo( fileName );
  QString extension = fileInfo.suffix();
  QString baseName = fileInfo.baseName();
  
  vtkSmartPointer< vtkMRMLTransformBufferNode > importBufferNode;
  importBufferNode.TakeReference( vtkMRMLTransformBufferNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLTransformBufferNode" ) ) );
  importBufferNode->SetName( baseName.toStdString().c_str() );
  importBufferNode->SetScene( this->mrmlScene() );
  this->mrmlScene()->AddNode( importBufferNode );

  // Unless it is an mha, assume that it is in xml format
  if ( extension.toStdString().compare( "mha" ) == 0 )
  {
    d->TransformRecorderLogic->ImportFromMHAFile( importBufferNode, fileName.toStdString() ); 
  }
  else // if ( extension.toStdString().compare( "xml" ) == 0 )
  {
    d->TransformRecorderLogic->ImportFromXMLFile( importBufferNode, fileName.toStdString() );
  }  

  d->TransformRecorderLogic->ObserveAllRecordedTransforms( importBufferNode ); // Automatically adds all transforms to the scene

  return true; // TODO: Check to see read was successful first
}
