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
#include "qSlicerMetricScriptReader.h"

// Logic includes
#include "vtkSlicerPerkEvaluatorLogic.h"

// MRML includes

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerMetricScriptReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerPerkEvaluatorLogic> PerkEvaluatorLogic;
};

//-----------------------------------------------------------------------------
qSlicerMetricScriptReader::qSlicerMetricScriptReader( vtkSlicerPerkEvaluatorLogic* newPerkEvaluatorLogic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMetricScriptReaderPrivate)
{
  this->setPerkEvaluatorLogic( newPerkEvaluatorLogic );
}

//-----------------------------------------------------------------------------
qSlicerMetricScriptReader::~qSlicerMetricScriptReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerMetricScriptReader::setPerkEvaluatorLogic(vtkSlicerPerkEvaluatorLogic* newPerkEvaluatorLogic)
{
  Q_D(qSlicerMetricScriptReader);
  d->PerkEvaluatorLogic = newPerkEvaluatorLogic;
}

//-----------------------------------------------------------------------------
vtkSlicerPerkEvaluatorLogic* qSlicerMetricScriptReader::PerkEvaluatorLogic() const
{
  Q_D(const qSlicerMetricScriptReader);
  return d->PerkEvaluatorLogic;
}

//-----------------------------------------------------------------------------
QString qSlicerMetricScriptReader::description() const
{
  return "Python Metric Script";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerMetricScriptReader::fileType() const
{
  return QString( "Python Metric Script" );
}

//-----------------------------------------------------------------------------
QStringList qSlicerMetricScriptReader::extensions() const
{
  return QStringList() << "Python Metric Script (*.py)";
}

//-----------------------------------------------------------------------------
bool qSlicerMetricScriptReader::load(const IOProperties& properties)
{
  Q_D(qSlicerMetricScriptReader);
  Q_ASSERT( properties.contains("fileName") );
  QString fileName = properties["fileName"].toString();

  QFileInfo fileInfo( fileName );
  QString extension = fileInfo.suffix();
  QString baseName = fileInfo.baseName();
  
  vtkSmartPointer< vtkMRMLMetricScriptNode > msNode;
  msNode.TakeReference( vtkMRMLMetricScriptNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLMetricScriptNode" ) ) );
  
  vtkSmartPointer< vtkMRMLMetricScriptStorageNode > mssNode;
  mssNode.TakeReference( vtkMRMLMetricScriptStorageNode::SafeDownCast( this->mrmlScene()->CreateNodeByClass( "vtkMRMLMetricScriptStorageNode" ) ) );
  mssNode->SetFileName( fileName.toLatin1() );

  int result = mssNode->ReadData( msNode ); // Read
  if ( result == 0 )
  {
    // this->mrmlScene()->RemoveNode( msNode );
    return false;
  }

  msNode->SetName( baseName.toStdString().c_str() );  
  msNode->SetScene( this->mrmlScene() );
  this->mrmlScene()->AddNode( msNode );

  // Indicate that the node was successfully loaded
  this->setLoadedNodes( QStringList( QString( msNode->GetID() ) ) );
  
  return true;
}
