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

  This file was originally developed by Adam Rankin, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// PerkTutor includes
#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkSlicerPerkEvaluatorLogic.h"

// SlicerQt includes
#include "qSlicerLoadableModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"

// MRML includes
#include <vtkMRMLTransformBufferNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include "vtkXMLDataParser.h"
#include "vtkSmartPointer.h"

// ITK includes
#include "itkFactoryRegistration.h"


//-----------------------------------------------------------------------------
int vtkSlicerPythonMetricsTest ( int argc, char * argv[] )
{
  int argIndex = 1;
  std::ostream& outputStream = std::cout;
  std::ostream& errorStream = std::cerr;

  // TestSceneFile
  const char *sceneFileName  = NULL;
  if (argc > argIndex+1)
  {
    if ( strcmp(argv[argIndex], "-SceneFile") == 0 )
    {
      sceneFileName = argv[argIndex+1];
      outputStream << "Test MRML scene file name: " << sceneFileName << std::endl;
      argIndex += 2;
    }
    else
    {
      sceneFileName = "";
    }
  }
  else
  {
    errorStream << "Invalid arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  const char *transformBufferID = NULL;
  if (argc > argIndex+1)
  {
    if ( strcmp(argv[argIndex], "-TransformBufferID") == 0 )
    {
      transformBufferID = argv[argIndex+1];
      outputStream << "Transform buffer ID: " << transformBufferID << std::endl;
      argIndex += 2;
    }
    else
    {
      transformBufferID = "";
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }
  
  const char *tissueModelID = NULL;
  if (argc > argIndex+1)
  {
    if ( strcmp(argv[argIndex], "-TissueModelID") == 0 )
    {
      tissueModelID = argv[argIndex+1];
      outputStream << "Tissue model ID: " << tissueModelID << std::endl;
      argIndex += 2;
    }
    else
    {
      tissueModelID = "";
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }
  
  const char *needleTransformID = NULL;
  if (argc > argIndex+1)
  {
    if ( strcmp(argv[argIndex], "-NeedleTransformID") == 0 )
    {
      needleTransformID = argv[argIndex+1];
      outputStream << "Needle transform ID: " << needleTransformID << std::endl;
      argIndex += 2;
    }
    else
    {
      needleTransformID = "";
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }
  
  const char *metricsDirectory = NULL;
  if (argc > argIndex+1)
  {
    if ( strcmp(argv[argIndex], "-ScriptedMetricsDirectory") == 0 )
    {
      metricsDirectory = argv[argIndex+1];
      outputStream << "Metrics directory: " << metricsDirectory << std::endl;
      argIndex += 2;
    }
    else
    {
      metricsDirectory = "";
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }
  
    const char *resultsFile = NULL;
  if (argc > argIndex+1)
  {
    if ( strcmp(argv[argIndex], "-ExpectedResultsFile") == 0 )
    {
      resultsFile = argv[argIndex+1];
      outputStream << "Results file: " << resultsFile << std::endl;
      argIndex += 2;
    }
    else
    {
      resultsFile = "";
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  // Ensure file reading works
  itk::itkFactoryRegistration();

  // Create the scene
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();

  // Create logic classes
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  if ( app == NULL )
  {
    outputStream << "Core application is null!" << std::endl;
  }

  vtkSlicerTransformRecorderLogic* trLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( qSlicerCoreApplication::application()->moduleManager()->module("TransformRecorder")->logic() );


  //vtkSmartPointer< vtkSlicerTransformRecorderLogic > trLogic = vtkSmartPointer< vtkSlicerTransformRecorderLogic >::New();
  trLogic->SetMRMLScene( mrmlScene ); // We need this to register the transform buffer node class to the scene (DO NOT DELETE)

  vtkSmartPointer< vtkSlicerPerkEvaluatorLogic > peLogic = vtkSmartPointer< vtkSlicerPerkEvaluatorLogic >::New();
  peLogic->SetMRMLScene( mrmlScene );


  // Load test scene
  mrmlScene->SetURL( sceneFileName );
  outputStream << "Scene import: " << mrmlScene->Import() << "." << std::endl;
  
  // Get references to the relevant nodes
  vtkMRMLTransformBufferNode* transformBufferNode = vtkMRMLTransformBufferNode::SafeDownCast( mrmlScene->GetNodeByID( transformBufferID ) );
  if ( transformBufferNode == NULL )
  {
    errorStream << "Bad transform buffer!" << std::endl;
    return EXIT_FAILURE;
  }
  
  vtkMRMLModelNode* tissueModelNode = vtkMRMLModelNode::SafeDownCast( mrmlScene->GetNodeByID( tissueModelID ) );
  if ( tissueModelNode == NULL )
  {
    errorStream << "Bad tissue model!" << std::endl;
    return EXIT_FAILURE;
  }
  
  vtkMRMLLinearTransformNode* needleTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( mrmlScene->GetNodeByID( needleTransformID ) );
  if ( needleTransformID == NULL )
  {
    errorStream << "Bad needle transform!" << std::endl;
    return EXIT_FAILURE;
  }
  
  // Read the results file
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( resultsFile );
  parser->Parse();
  vtkSmartPointer< vtkXMLDataElement > rootElement = parser->GetRootElement();
  
  // Create a map of metric names to values
  std::map< std::string, double > metricsMap; 
  
  if ( ! rootElement || strcmp( rootElement->GetName(), "PythonMetricsResults" ) != 0 ) 
  {
    errorStream << "Bad python metrics results file!" << std::endl;
    return EXIT_FAILURE;
  }

  int numElements = rootElement->GetNumberOfNestedElements();  // Number of metrics 
  for ( int i = 0; i < numElements; i++ )
  {
    vtkXMLDataElement* element = rootElement->GetNestedElement( i );
	  if ( element == NULL || strcmp( element->GetName(), "Metric" ) != 0 )
	  {
      continue;
	  }    
    metricsMap[ element->GetAttribute( "Name" ) ] = atof( element->GetAttribute( "Value" ) ); 
  }
  
  // Set up to analyze
  peLogic->UpdateToolTrajectories( transformBufferNode );
  peLogic->AddAnalyzeTransform( needleTransformNode );
  
  peLogic->SetBodyModelNode( tissueModelNode );
  peLogic->SetNeedleTransformNode( needleTransformNode );
  
  peLogic->SetMarkBegin( peLogic->GetMinTime() );
  peLogic->SetMarkEnd( peLogic->GetMaxTime() );
  
  // Calculate the metrics
  std::vector< vtkSlicerPerkEvaluatorLogic::MetricType > calculatedMetrics = peLogic->GetMetrics();
  outputStream << "Number of calculated metrics: " << calculatedMetrics.size() << "." << std::endl;
  
  // Compare the metrics to the expected results
  std::map< std::string, double > compareMap;
  
  for ( int i = 0; i < calculatedMetrics.size(); i++ )
  {
    if ( metricsMap.find( calculatedMetrics.at( i ).first ) == metricsMap.end() )
    {
      outputStream << "Could not find metric: " << calculatedMetrics.at( i ).first << "." << std::endl;
      continue; // Don't worry about extra metrics for now
    }
    
    if ( metricsMap[ calculatedMetrics.at( i ).first ] != calculatedMetrics.at( i ).second )
    {
      errorStream << "Incorrect python metric: " << calculatedMetrics.at( i ).first << ". Expected: " << metricsMap[ calculatedMetrics.at( i ).first ] << ", but got: " << calculatedMetrics.at( i ).second << "!" << std::endl;
      return EXIT_FAILURE;
    }
    else
    {
      outputStream << "Correct! Python metric: " << calculatedMetrics.at( i ).first << ". Expected: " << metricsMap[ calculatedMetrics.at( i ).first ] << ", and got: " << calculatedMetrics.at( i ).second << "!" << std::endl;
    }
  }
  
  return EXIT_SUCCESS;
}
