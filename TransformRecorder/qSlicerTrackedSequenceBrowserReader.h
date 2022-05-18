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

#ifndef __qSlicerTrackedSequenceBrowserReader_h
#define __qSlicerTrackedSequenceBrowserReader_h

// SlicerQt includes
#include "qSlicerFileReader.h"
class qSlicerTrackedSequenceBrowserReaderPrivate;

// Slicer includes
#include "vtkAddonMathUtilities.h"
#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkMRMLSequenceNode.h"
#include "vtkSlicerSequencesLogic.h"
#include "vtkMRMLScriptedModuleNode.h"

#include "vtkXMLDataElement.h"

//-----------------------------------------------------------------------------
class qSlicerTrackedSequenceBrowserReader
  : public qSlicerFileReader
{
  Q_OBJECT
public:
  typedef qSlicerFileReader Superclass;
  qSlicerTrackedSequenceBrowserReader( vtkSlicerTransformRecorderLogic* newTransformRecorderLogic = 0, QObject* parent = 0 );
  virtual ~qSlicerTrackedSequenceBrowserReader();

  void setTransformRecorderLogic( vtkSlicerTransformRecorderLogic* newTransformRecorderLogic);
  vtkSlicerTransformRecorderLogic* TransformRecorderLogic() const;

  QString description() const override;
  IOFileType fileType() const override;
  QStringList extensions() const override;
  qSlicerIOOptions* options() const override;

  bool load( const IOProperties& properties ) override;
  
protected:
  QScopedPointer< qSlicerTrackedSequenceBrowserReaderPrivate > d_ptr;

  virtual bool loadXML( vtkMRMLSequenceBrowserNode* trackedSequenceBrowserNode, std::string fileName );
  virtual std::string getTimeStringFromXMLElement( vtkXMLDataElement* element );

  virtual bool loadSQBR( vtkMRMLSequenceBrowserNode* trackedSequenceBrowserNode, std::string fileName, bool useSceneProxyNodes = false );

  void copyNodeAttributes( vtkMRMLNode* sourceNode, vtkMRMLNode* targetNode );

private:
  Q_DECLARE_PRIVATE( qSlicerTrackedSequenceBrowserReader );
  Q_DISABLE_COPY( qSlicerTrackedSequenceBrowserReader );
};

#endif
