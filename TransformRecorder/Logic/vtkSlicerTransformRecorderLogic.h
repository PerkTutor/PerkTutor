/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// .NAME vtkSlicerTransformRecorderLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerTransformRecorderLogic_h
#define __vtkSlicerTransformRecorderLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"
#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkObjectFactory.h"

#include "vtkSmartPointer.h"

// MRML includes
#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceBrowserNode.h"

// STD includes
#include <cstdlib>

#include "vtkSlicerTransformRecorderModuleLogicExport.h"



/// \ingroup Slicer_QtModules_TransformRecorder
class VTK_SLICER_TRANSFORMRECORDER_MODULE_LOGIC_EXPORT vtkSlicerTransformRecorderLogic :
  public vtkSlicerModuleLogic
{
public:
  vtkTypeMacro(vtkSlicerTransformRecorderLogic,vtkSlicerModuleLogic);

  static vtkSlicerTransformRecorderLogic *New();

  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSlicerTransformRecorderLogic();
  virtual ~vtkSlicerTransformRecorderLogic();

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  
public:
  /// Initialize listening to MRML events
  void InitializeEventListeners();

  // Convenience methods for intersection between widget and MRML
  vtkMRMLSequenceNode* GetMessageSequenceNode( vtkMRMLSequenceBrowserNode* browserNode );
  void AddMessage( vtkMRMLSequenceBrowserNode* browserNode, std::string messageString, std::string indexValue );
  void UpdateMessage( vtkMRMLSequenceBrowserNode* browserNode, std::string messageString, int itemNumber );
  void RemoveMessage( vtkMRMLSequenceBrowserNode* browserNode, int itemNumber );
  void ClearMessages( vtkMRMLSequenceBrowserNode* browserNode );
  std::string GetPriorMessageString( vtkMRMLSequenceBrowserNode* browserNode, std::string indexValue );

  double GetMaximumIndexValue( vtkMRMLSequenceBrowserNode* browserNode );
  int GetMaximumNumberOfDataNodes( vtkMRMLSequenceBrowserNode* browserNode );

  // Grab module logic
  static vtkMRMLAbstractLogic* GetSlicerModuleLogic( std::string moduleName );
  
private:

  vtkSlicerTransformRecorderLogic(const vtkSlicerTransformRecorderLogic&); // Not implemented
  void operator=(const vtkSlicerTransformRecorderLogic&);               // Not implemented
  // Reference to the module MRML node.

};

#endif

