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

  This file was originally developed by Kevin Wang, PMH.
  and was partially funded by OCAIRO and Sparkit.

==============================================================================*/

// Qt includes
#include <QFileInfo>

// SlicerQt includes
#include "qSlicerTablesReader.h"

// Logic includes
#include "vtkSlicerPerkEvaluatorLogic.h"

// MRML includes
#include <vtkMRMLTableNode.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerTablesReaderPrivate
{
public:
  vtkSmartPointer<vtkSlicerPerkEvaluatorLogic> Logic;
};

//-----------------------------------------------------------------------------
qSlicerTablesReader::qSlicerTablesReader(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTablesReaderPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTablesReader
::qSlicerTablesReader(vtkSlicerPerkEvaluatorLogic* logic, QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTablesReaderPrivate)
{
  this->setLogic(logic);
}

//-----------------------------------------------------------------------------
qSlicerTablesReader::~qSlicerTablesReader()
{
}

//-----------------------------------------------------------------------------
void qSlicerTablesReader::setLogic(vtkSlicerPerkEvaluatorLogic* logic)
{
  Q_D(qSlicerTablesReader);
  d->Logic = logic;
}

//-----------------------------------------------------------------------------
vtkSlicerPerkEvaluatorLogic* qSlicerTablesReader::logic()const
{
  Q_D(const qSlicerTablesReader);
  return d->Logic.GetPointer();
}

//-----------------------------------------------------------------------------
QString qSlicerTablesReader::description()const
{
  return "Table";
}

//-----------------------------------------------------------------------------
qSlicerIO::IOFileType qSlicerTablesReader::fileType()const
{
  return QString("TableFile");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTablesReader::extensions()const
{
  return QStringList()
    << "Table (*.csv)"
    ;
}

//-----------------------------------------------------------------------------
bool qSlicerTablesReader::load(const IOProperties& properties)
{
  Q_D(qSlicerTablesReader);
  Q_ASSERT(properties.contains("fileName"));
  QString fileName = properties["fileName"].toString();

  QString name = QFileInfo(fileName).baseName();
  if (properties.contains("name"))
    {
    name = properties["name"].toString();
    }
  Q_ASSERT(d->Logic);
  vtkMRMLTableNode* node = d->Logic->AddTable(
    fileName.toLatin1(),
    name.toLatin1());
  if (node)
    {
    this->setLoadedNodes(QStringList(QString(node->GetID())));
    }
  else
    {
    this->setLoadedNodes(QStringList());
    }
  return node != 0;
}
