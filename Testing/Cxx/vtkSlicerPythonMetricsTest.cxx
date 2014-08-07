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

#include "qSlicerPerkEvaluatorModuleWidget.h"

// SlicerQt includes
#include "qSlicerApplication.h"
#include "qSlicerPythonManager.h"

// MRML includes
#include <vtkMRMLTransformBufferNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include "vtkXMLDataParser.h"
#include "vtkSmartPointer.h"
#include "vtkMath.h"

// ITK includes
#include "itkFactoryRegistration.h"

#include <fstream>


// This helper function will check if the metrics are equal to within n decimal places
// We choose two for most applications
bool EqualToNDecimalPlaces( double value1, double value2, int n = 2 )
{
  // Round 100 * the value
  int value1_n = vtkMath::Round( value1 * pow( 10.0, n ) );
  int value2_n = vtkMath::Round( value2 * pow( 10.0, n ) );

  return ( value1_n == value2_n );
}

//-----------------------------------------------------------------------------
int vtkSlicerPythonMetricsTest ( int argc, char * argv[] )
{
  int argIndex = 1;
  std::ostream& outputStream = std::cout;
  std::ostream& errorStream = std::cerr;

  ofstream fileStream;
  fileStream.open( "C:/Devel/PerkTutor/PerkTutorTestLog.txt" );

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

   const char *pythonBinaryDirectory = NULL;
  if (argc > argIndex+1)
  {
    if ( strcmp(argv[argIndex], "-PythonBinaryDirectory") == 0 )
    {
      pythonBinaryDirectory = argv[argIndex+1];
      outputStream << "Python Binary Directory: " << pythonBinaryDirectory << std::endl;
      argIndex += 2;
    }
    else
    {
      pythonBinaryDirectory = "";
    }
  }
  else
  {
    errorStream << "No arguments!" << std::endl;
    return EXIT_FAILURE;
  }

  outputStream << endl << "CTEST_FULL_OUTPUT" << endl;

  // Ensure file reading works
  itk::itkFactoryRegistration();

  // Enusre the resources are loaded (DO NOT DELETE)
  qSlicerApplication* slicerApp = new qSlicerApplication(argc, argv);


  // Load TransformRecorder Python module(s)
  slicerApp->pythonManager()->executeString( QString( "from slicer.util import importVTKClassesFromDirectory;"
      "importVTKClassesFromDirectory('%1', 'slicer.modulelogic', filematch='vtkSlicer%2ModuleLogic.py');"
      "importVTKClassesFromDirectory('%1', 'slicer.modulemrml', filematch='vtkSlicer%2ModuleMRML.py');"
      ).arg( pythonBinaryDirectory ).arg( "TransformRecorder" ) );

  slicerApp->pythonManager()->executeString( QString( "from slicer.util import importVTKClassesFromDirectory;"
      "importVTKClassesFromDirectory('%1', 'slicer.modulelogic', filematch='vtkSlicer%2ModuleLogic.py');"
      "importVTKClassesFromDirectory('%1', 'slicer.modulemrml', filematch='vtkSlicer%2ModuleMRML.py');"
      ).arg( pythonBinaryDirectory ).arg( "PerkEvaluator" ) );


  // Instantiate widget
  qSlicerPerkEvaluatorModuleWidget* peWidget = new qSlicerPerkEvaluatorModuleWidget();


  // Create the scene
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();

  // Create logic classes
  vtkSmartPointer< vtkSlicerTransformRecorderLogic > trLogic = vtkSmartPointer< vtkSlicerTransformRecorderLogic >::New();
  trLogic->SetMRMLScene( mrmlScene ); // We need this to register the transform buffer node class to the scene (DO NOT DELETE)

  vtkSmartPointer< vtkSlicerPerkEvaluatorLogic > peLogic = vtkSmartPointer< vtkSlicerPerkEvaluatorLogic >::New();
  peLogic->SetMRMLScene( mrmlScene );
  peLogic->TransformRecorderLogic = trLogic;


  // Load test scene
  mrmlScene->SetURL( sceneFileName );
  fileStream << "Scene import: " << mrmlScene->Import() << "." << std::endl;

  
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
  peLogic->SetPlaybackTime( peLogic->GetMinTime() );
  peLogic->AddAnalyzeTransform( needleTransformNode );

  peLogic->SetBodyModelNode( tissueModelNode );
  peLogic->SetNeedleTransformNode( needleTransformNode );
  
  peLogic->SetMarkBegin( peLogic->GetMinTime() );
  peLogic->SetMarkEnd( peLogic->GetMaxTime() );
  

  // Calculate the metrics
  std::vector< vtkSlicerPerkEvaluatorLogic::MetricType > calculatedMetrics = peLogic->GetMetrics();
  fileStream << "Number of calculated metrics: " << calculatedMetrics.size() << "." << std::endl;

  if ( calculatedMetrics.size() == 0 )
  {
    fileStream << "No metrics were calculated." << std::endl;
    return EXIT_FAILURE;
  }

 
  // Compare the metrics to the expected results
  std::map< std::string, double > compareMap;
  
  for ( int i = 0; i < calculatedMetrics.size(); i++ )
  {
    //fileStream << calculatedMetrics.at( i ).first << " " << calculatedMetrics.at( i ).second << std::endl;

    if ( metricsMap.find( calculatedMetrics.at( i ).first ) == metricsMap.end() )
    {
      fileStream << "Could not find expected result for metric: " << calculatedMetrics.at( i ).first << ". Value: " << calculatedMetrics.at( i ).second << "." << std::endl;
      continue; // Don't worry about extra metrics for now
    }
    
    if ( ! EqualToNDecimalPlaces( metricsMap[ calculatedMetrics.at( i ).first ], calculatedMetrics.at( i ).second, 2 ) )
    {
      fileStream << "Incorrect python metric: " << calculatedMetrics.at( i ).first << ". Expected: " << metricsMap[ calculatedMetrics.at( i ).first ] << ", but got: " << calculatedMetrics.at( i ).second << "!" << std::endl;
      return EXIT_FAILURE;
    }
    else
    {
      fileStream << "Correct! Python metric: " << calculatedMetrics.at( i ).first << ". Expected: " << metricsMap[ calculatedMetrics.at( i ).first ] << ", and got: " << calculatedMetrics.at( i ).second << "!" << std::endl;
    }
  }
  
  return EXIT_SUCCESS;
}
