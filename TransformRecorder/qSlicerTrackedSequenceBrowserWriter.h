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

#ifndef __qSlicerTrackedSequenceBrowserWriter_h
#define __qSlicerTrackedSequenceBrowserWriter_h

// QtCore includes
#include "qSlicerFileWriter.h"
class qSlicerTrackedSequenceBrowserWriterPrivate;

// Slicer includes
#include "vtkAddonMathUtilities.h"
#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLScriptedModuleNode.h"


class qSlicerTrackedSequenceBrowserWriter
  : public qSlicerFileWriter
{
  Q_OBJECT
public:
  typedef qSlicerFileWriter Superclass;
  qSlicerTrackedSequenceBrowserWriter( vtkSlicerTransformRecorderLogic* newTransformRecorderLogic = 0, QObject* parent = 0);
  virtual ~qSlicerTrackedSequenceBrowserWriter();

  void setTransformRecorderLogic( vtkSlicerTransformRecorderLogic* newTransformRecorderLogic );
  vtkSlicerTransformRecorderLogic* TransformRecorderLogic() const;

  virtual QString description()const;
  virtual IOFileType fileType()const;

  /// Return true if the object is handled by the writer.
  virtual bool canWriteObject(vtkObject* object)const;

  /// Return  a list of the supported extensions for a particuliar object.
  /// Please read QFileDialog::nameFilters for the allowed formats
  /// Example: "Image (*.jpg *.png *.tiff)", "Model (*.vtk)"
  virtual QStringList extensions(vtkObject* object)const;

  /// Write the node identified by nodeID into the fileName file.
  /// Returns true on success.
  virtual bool write(const qSlicerIO::IOProperties& properties);

protected:
  QScopedPointer< qSlicerTrackedSequenceBrowserWriterPrivate > d_ptr;

  /// Write the node to an XML file
  virtual bool writeXML( vtkMRMLSequenceBrowserNode* trackedSequenceBrowserNode, std::string fileName );

  virtual std::string getXMLStringFromTimeString( std::string timeString );

  /// Write the node to an SQBR file (sequence browser file)
  /// This is just a slicer scene bundle with all irrelevant nodes removed and a fancy extension
  virtual bool writeSQBR( vtkMRMLSequenceBrowserNode* trackedSequenceBrowserNode, std::string fileName );

  void copyNodeAttributes( vtkMRMLNode* sourceNode, vtkMRMLNode* targetNode );

private:
  Q_DECLARE_PRIVATE( qSlicerTrackedSequenceBrowserWriter );
  Q_DISABLE_COPY( qSlicerTrackedSequenceBrowserWriter );

};

#endif
