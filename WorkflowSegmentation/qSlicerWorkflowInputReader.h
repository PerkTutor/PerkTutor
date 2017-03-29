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

#ifndef __qSlicerWorkflowInputReader_h
#define __qSlicerWorkflowInputReader_h

// SlicerQt includes
#include "qSlicerFileReader.h"
class qSlicerWorkflowInputReaderPrivate;

// Workflow includes
class vtkSlicerWorkflowSegmentationLogic;

//-----------------------------------------------------------------------------
class qSlicerWorkflowInputReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerWorkflowInputReader( vtkSlicerWorkflowSegmentationLogic* newWorkflowSegmentationLogic = 0, QObject* parent = 0 );
  virtual ~qSlicerWorkflowInputReader();

  void setWorkflowSegmentationLogic( vtkSlicerWorkflowSegmentationLogic* newWorkflowSegmentationLogic);
  vtkSlicerWorkflowSegmentationLogic* WorkflowSegmentationLogic() const;

  virtual QString description() const;
  virtual IOFileType fileType() const;
  virtual QStringList extensions() const;

  virtual bool load( const IOProperties& properties );
  
protected:
  QScopedPointer< qSlicerWorkflowInputReaderPrivate > d_ptr;

private:
  Q_DECLARE_PRIVATE( qSlicerWorkflowInputReader );
  Q_DISABLE_COPY( qSlicerWorkflowInputReader );
};

#endif
