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

// .NAME vtkSlicerWorkflowSegmentationLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerWorkflowSegmentationLogic_h
#define __vtkSlicerWorkflowSegmentationLogic_h

// Standard Includes
#include <string>
#include <sstream>
#include <vector>
#include <cmath>

// VTK includes
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataElement.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleLogicExport.h"
#include "vtkMRMLWorkflowSegmentationNode.h"
#include "vtkRecordBufferRT.h"
#include "vtkMarkovModelRT.h"
#include "vtkWorkflowToolCollection.h"
#include "vtkWorkflowAlgorithm.h"

#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkMRMLTransformBufferNode.h"




/// \ingroup Slicer_QtModules_WorkflowSegmentation
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_LOGIC_EXPORT vtkSlicerWorkflowSegmentationLogic :
  public vtkSlicerModuleLogic
{
public:
  vtkTypeMacro(vtkSlicerWorkflowSegmentationLogic,vtkSlicerModuleLogic);

  static vtkSlicerWorkflowSegmentationLogic *New();
  
  void PrintSelf(ostream& os, vtkIndent indent);
  void InitializeEventListeners();

protected:
  vtkSlicerWorkflowSegmentationLogic();
  virtual ~vtkSlicerWorkflowSegmentationLogic();

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

private:

  vtkSlicerWorkflowSegmentationLogic(const vtkSlicerWorkflowSegmentationLogic&); // Not implemented
  void operator=(const vtkSlicerWorkflowSegmentationLogic&);               // Not implemented


  // These are methods specific to the Workflow Segmentation logic -------------------------------------------------------
public:

  vtkSlicerTransformRecorderLogic* TransformRecorderLogic;

  vtkMRMLWorkflowSegmentationNode* GetModuleNode();
  void SetModuleNode( vtkMRMLWorkflowSegmentationNode* node );

  void ImportWorkflowProcedure( std::string fileName );
  void ImportWorkflowInput( std::string fileName );
  void ImportWorkflowTraining( std::string fileName );
  void SaveWorkflowTraining( std::string fileName );

  void ResetWorkflowAlgorithms();
  bool GetWorkflowAlgorithmsDefined();
  bool GetWorkflowAlgorithmsInputted();
  bool GetWorkflowAlgorithmsTrained();
  bool Train();

  void AddTrainingBuffer( std::string fileName );
  void SegmentBuffer( std::string fileName );

  void Update();

  std::string GetToolInstructions();

private:

  vtkMRMLWorkflowSegmentationNode* ModuleNode;
  int IndexToProcess;

  vtkXMLDataParser* Parser;
  vtkXMLDataElement* ParseXMLFile( std::string fileName );

private:

  std::vector<vtkWorkflowAlgorithm*> WorkflowAlgorithms;

  vtkWorkflowAlgorithm* GetWorkflowAlgorithmByName( std::string name );

};

#endif

