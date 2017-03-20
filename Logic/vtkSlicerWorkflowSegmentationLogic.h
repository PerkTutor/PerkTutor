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

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// VTK includes
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataElement.h"

#include "vtkMRMLWorkflowSegmentationNode.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleLogicExport.h"

// Transform Recorder includes
#include "vtkSlicerTransformRecorderLogic.h"





/// \ingroup Slicer_QtModules_WorkflowSegmentation
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_LOGIC_EXPORT 
vtkSlicerWorkflowSegmentationLogic : public vtkSlicerModuleLogic
{
public:
  vtkTypeMacro(vtkSlicerWorkflowSegmentationLogic,vtkSlicerModuleLogic);
  static vtkSlicerWorkflowSegmentationLogic *New();  
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSlicerWorkflowSegmentationLogic();
  virtual ~vtkSlicerWorkflowSegmentationLogic();

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void SetMRMLSceneInternal( vtkMRMLScene* newScene );
  virtual void RegisterNodes();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

private:

  vtkSlicerWorkflowSegmentationLogic(const vtkSlicerWorkflowSegmentationLogic&); // Not implemented
  void operator=(const vtkSlicerWorkflowSegmentationLogic&);               // Not implemented


  // These are methods specific to the Workflow Segmentation logic -------------------------------------------------------
public:

  vtkMRMLWorkflowToolNode* GetToolByProxyNodeID( vtkMRMLWorkflowSegmentationNode* workflowNode, std::string proxyNodeID );
 
  void ResetAllToolSequences( vtkMRMLWorkflowSegmentationNode* workflowNode );
  void TrainAllTools( vtkMRMLWorkflowSegmentationNode* workflowNode, vtkCollection* trainingTrackedSequenceBrowserNodes );
 
  static bool GetAllToolsInputted( vtkMRMLWorkflowSegmentationNode* workflowNode );
  static bool GetAllToolsTrained( vtkMRMLWorkflowSegmentationNode* workflowNode );
  
  static std::vector< std::string > GetToolStatusStrings( vtkMRMLWorkflowSegmentationNode* workflowNode );
  static std::vector< std::string > GetInstructionStrings( vtkMRMLWorkflowSegmentationNode* workflowNode );
  static std::vector< std::string > GetOrderedWorkflowTaskStrings( vtkMRMLWorkflowToolNode* toolNode );

  void SetupRealTimeProcessing( vtkMRMLWorkflowSegmentationNode* wsNode );

  void ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData );
  void ProcessMRMLSceneEvents( vtkObject* caller, unsigned long event, void* callData );

protected:

};

#endif

