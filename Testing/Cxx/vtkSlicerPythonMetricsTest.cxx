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
#include "qSlicerLoadableModule.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerApplication.h"
#include "qSlicerApplicationHelper.h"
#include "qSlicerStyle.h"
#include "qSlicerPythonManager.h"

#include "ctkPythonConsole.h"

// MRML includes
#include <vtkMRMLTransformBufferNode.h>
#include <vtkMRMLModelNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include "vtkXMLDataParser.h"
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"
#include "vtkMath.h"

// ITK includes
#include "itkFactoryRegistration.h"

// QT includes
#include "PythonQt.h"
#include "QDir.h"
#include "QDirIterator.h"
#include "QResource.h"
#include "QStringList.h"

// CTK includes
#include <ctkScopedCurrentDir.h>

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

   const char *binaryDirectory = NULL;
  if (argc > argIndex+1)
  {
    if ( strcmp(argv[argIndex], "-BinaryDirectory") == 0 )
    {
      binaryDirectory = argv[argIndex+1];
      outputStream << "Binary Directory: " << binaryDirectory << std::endl;
      argIndex += 2;
    }
    else
    {
      binaryDirectory = "";
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
  //QApplication app(argc, argv);
  qSlicerApplication* slicerApp = new qSlicerApplication(argc, argv);

  // Load EMSegment Python module(s)
  fileStream << argv[1] << std::endl;
  fileStream << vtkSlicerApplicationLogic::GetModuleSlicerXYLibDirectory( argv[1] ) << std::endl;

  QString TransformRecorderModulePath = QString::fromStdString( vtkSlicerApplicationLogic::GetModuleSlicerXYLibDirectory( argv[1] ) );
  TransformRecorderModulePath.append("/" Slicer_QTLOADABLEMODULES_SUBDIR);
  slicerApp->pythonManager()->appendPythonPath( TransformRecorderModulePath );
  // Update current application directory, so that *PythonD modules can be loaded
  ctkScopedCurrentDir scopedCurrentDirTR( TransformRecorderModulePath );

  QString TransformRecorderModulePythonPath = TransformRecorderModulePath + "/Python";
  fileStream << "TransformRecorderModulePath:" << qPrintable( TransformRecorderModulePythonPath ) << std::endl;
  slicerApp->pythonManager()->executeString( QString( "from slicer.util import importVTKClassesFromDirectory;"
      "importVTKClassesFromDirectory('%1', 'slicer.modulelogic', filematch='vtkSlicer%2ModuleLogic.py');"
      "importVTKClassesFromDirectory('%1', 'slicer.modulemrml', filematch='vtkSlicer%2ModuleMRML.py');"
      ).arg( binaryDirectory ).arg( "TransformRecorder" ) );

  QString PerkEvaluatorModulePath = QString::fromStdString( vtkSlicerApplicationLogic::GetModuleSlicerXYLibDirectory( argv[1] ) );
  PerkEvaluatorModulePath.append("/" Slicer_QTLOADABLEMODULES_SUBDIR);
  slicerApp->pythonManager()->appendPythonPath( PerkEvaluatorModulePath );
  // Update current application directory, so that *PythonD modules can be loaded
  ctkScopedCurrentDir scopedCurrentDirPE( PerkEvaluatorModulePath );

  QString PerkEvaluatorModulePythonPath = PerkEvaluatorModulePath + "/Python";
  fileStream << "PerkEvaluatorModulePath:" << qPrintable( PerkEvaluatorModulePythonPath ) << std::endl;
  slicerApp->pythonManager()->executeString( QString( "from slicer.util import importVTKClassesFromDirectory;"
      "importVTKClassesFromDirectory('%1', 'slicer.modulelogic', filematch='vtkSlicer%2ModuleLogic.py');"
      "importVTKClassesFromDirectory('%1', 'slicer.modulemrml', filematch='vtkSlicer%2ModuleMRML.py');"
      ).arg( "c:/Devel/PerkTutor/PerkTutor-Debug/inner-build/lib/Slicer-4.3/qt-loadable-modules/Python" ).arg( "PerkEvaluator" ) );


  QString LinearTransformModulePath = QString::fromStdString( vtkSlicerApplicationLogic::GetModuleSlicerXYLibDirectory( argv[1] ) );
  LinearTransformModulePath.append("/" Slicer_QTLOADABLEMODULES_SUBDIR);
  slicerApp->pythonManager()->appendPythonPath( LinearTransformModulePath );
  // Update current application directory, so that *PythonD modules can be loaded
  ctkScopedCurrentDir scopedCurrentDirLT( LinearTransformModulePath );

  QString LinearTransformModulePythonPath = LinearTransformModulePath + "/Python";
  fileStream << "LinearTransformModulePath:" << qPrintable( LinearTransformModulePythonPath ) << std::endl;
  slicerApp->pythonManager()->executeString( QString( "from slicer.util import importVTKClassesFromDirectory;"
      "importVTKClassesFromDirectory('%1', 'slicer', filematch='mrml.py');"
      ).arg( "c:/Devel/Slicer_Apr16/Debug/Slicer-build/bin/Python" ) );

  

      // Load EMSegment Python module(s)
    QString emsegmentModulePath = QString::fromStdString(
          vtkSlicerApplicationLogic::GetModuleSlicerXYLibDirectory(argv[1]));
    emsegmentModulePath.append("/" Slicer_QTLOADABLEMODULES_SUBDIR);
    slicerApp->pythonManager()->appendPythonPath(emsegmentModulePath);
    // Update current application directory, so that *PythonD modules can be loaded
    ctkScopedCurrentDir scopedCurrentDir(emsegmentModulePath);

    QString emsegmentModulePythonPath = emsegmentModulePath + "/Python";
    fileStream << "emsegmentModulePythonPath:" << qPrintable(emsegmentModulePythonPath) << std::endl;
    slicerApp->pythonManager()->executeString(QString(
      "from slicer.util import importVTKClassesFromDirectory;"
      "importVTKClassesFromDirectory('%1', 'slicer.modulelogic', filematch='vtkSlicer%2ModuleLogic.py');"
      "importVTKClassesFromDirectory('%1', 'slicer.modulemrml', filematch='vtkSlicer%2ModuleMRML.py');"
      ).arg(emsegmentModulePythonPath).arg("EMSegment"));

  //vtkSlicerCommonInterface* common = vtkSlicerCommonInterface::New();
  //qSlicerApplication* slicerApp = new qSlicerApplication(argc, argv);
  //slicerApp.exec();

  //qSlicerPythonManager* pyManager = new qSlicerPythonManager();
  //pyManager->executeFile( slicerHome + "/bin/Python/slicer/slicerqt.py" );
  //ctkPythonConsole* pyConsole = new ctkPythonConsole();
  
  //pyConsole->initialize( pyManager );
  //pyConsole->initialize( qSlicerApplication::application()->pythonManager() );

  qSlicerPerkEvaluatorModuleWidget* peWidget = new qSlicerPerkEvaluatorModuleWidget();

  //qSlicerPerkEvaluatorModuleWidget* peWidget = new qSlicerPerkEvaluatorModuleWidget();
  //qSlicerPythonManager* pyManager = slicerApp->pythonManager();
  //pyManager->executeString( "from __main__ import vtk, qt, ctk, slicer" );

  errorStream << "So far so good" << endl;

  
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  QString slicerHome = env.value("Slicer_HOME");

  fileStream << "Slicer home: " << slicerHome.toStdString() << std::endl;

  /*
  std::string appArgv1String = ( slicerHome + "/bin/Debug/SlicerApp-real.exe" ).toStdString();
  char* appArgv1 = new char[ appArgv1String.size() + 1 ];
  strcpy( appArgv1, appArgv1String.c_str() ); 

  char* appArgv[] = { appArgv1, NULL };
  int appArgc = sizeof( appArgv )/sizeof( appArgv[0] ) - 1;

  std::cout << "Application executable: " << appArgv1String << std::endl;

  qSlicerApplication slicerApp( appArgc, appArgv );

  */
  /*
  //pyConsole->show();

  //qSlicerApplicationHelper::initializePythonConsole( pyConsole );

  pyManager->executeFile( slicerHome + "/bin/Python/slicer/__init__.py" );
  pyManager->executeFile( slicerHome + "/bin/Python/slicer/util.py" );
  
  pyManager->executeFile( slicerHome + "/bin/Python/vtkITK.py" );
  */


  // Create the scene
  vtkSmartPointer<vtkMRMLScene> mrmlScene = vtkSmartPointer<vtkMRMLScene>::New();

  // Create logic classes
  vtkSmartPointer< vtkSlicerTransformRecorderLogic > trLogic = vtkSmartPointer< vtkSlicerTransformRecorderLogic >::New();
  //vtkSmartPointer< vtkSlicerTransformRecorderLogic > trLogic = vtkSlicerTransformRecorderLogic::SafeDownCast( qSlicerApplication::application()->moduleManager()->module( "TransformRecorder" )->logic() );
  trLogic->SetMRMLScene( mrmlScene ); // We need this to register the transform buffer node class to the scene (DO NOT DELETE)

  vtkSmartPointer< vtkSlicerPerkEvaluatorLogic > peLogic = vtkSmartPointer< vtkSlicerPerkEvaluatorLogic >::New();
  //vtkSmartPointer< vtkSlicerPerkEvaluatorLogic > peLogic = vtkSlicerPerkEvaluatorLogic::SafeDownCast( qSlicerApplication::application()->moduleManager()->module( "PerkEvaluator" )->logic() );
  peLogic->SetMRMLScene( mrmlScene );
  peLogic->TransformRecorderLogic = trLogic;

  // Load test scene
  mrmlScene->SetURL( sceneFileName );
  fileStream << "Scene import: " << mrmlScene->Import() << "." << std::endl;

  for ( int i = 0; i < mrmlScene->GetNumberOfNodes(); i++ )
  {
    fileStream << "Scene node: " << mrmlScene->GetNthNode( i )->GetName() << " " << mrmlScene->GetNthNode( i )->GetID() << std::endl;
  }
  
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
    fileStream << element->GetAttribute( "Name" ) << " " << element->GetAttribute( "Value" ) << std::endl;
  }

  // Print out all the metrics we should calculate


  // Check python console can be called at all
  PythonQt::init();
  PythonQtObjectPtr context = PythonQt::self()->getMainModule();
  if ( context == NULL )
  {
    errorStream << "Python console cannot be initialized!" << std::endl;
    return EXIT_FAILURE;
  }
  int resultInt = 0;
  context.evalScript("def multiply(a,b):\n  return a*b;\n");
  QVariantList args;
  args << 7 << 3;
  resultInt = context.call("multiply", args).toInt();
  context.evalScript( "print 'dfdfgsdgfsd'" );
  if ( resultInt == 21 )
  {
    fileStream << "Successful calculation using Python console!" << std::endl;
  }
  else
  {
    fileStream << "Could not perform calculation using Python console!" << std::endl;
  }
  //context.evalScript( "execfile( 'C:/Devel/Slicer_Apr16/Debug/VTK-build/Wrapping/Python/setup.py' )" );
  //context.evalScript( "execfile( 'C:/Devel/Slicer_Apr16/Debug/VTK-build/Wrapping/Python/setup.py' )" );

  //context.evalScript( "print os.environ['PythonMetricsTestDataPath']" );

  int resultSize = -1;
  context.evalScript( "import vtk" );
  context.evalScript( "coll = vtk.vtkCollection()" );
  context.evalScript( "coll.AddItem( vtk.vtkObject() )" );
  resultSize = context.call( "coll.GetNumberOfItems" ).toInt();
  if ( resultSize == 1 )
  {
    fileStream << "Successful calculation using Python console for vtk objects!" << std::endl;
  }
  else
  {
    fileStream << "Could not perform calculation using Python console for vtk objects!" << std::endl;
  }
  
  if ( resultSize == 1 )
  {
    //return EXIT_FAILURE;
  }

  QVariantList elementIndex;
  elementIndex << 0;
  elementIndex << 0;

  double resultElement = -1;
  context.evalScript( "from __main__ import vtk" );
  context.evalScript( "mat = vtk.vtkMatrix4x4()" );
  context.evalScript( "mat.SetElement( 0, 0, 100 )" );
  resultElement = context.call( "mat.GetElement", elementIndex ).toDouble();
  if ( resultElement == 100 )
  {
    fileStream << "Successful calculation using Python console for vtk matrices!" << std::endl;
  }
  else
  {
    fileStream << "Could not perform calculation using Python console for vtk matrices!" << std::endl;
  }
  
  if ( resultElement > 90 && resultElement < 110 )
  {
    //return EXIT_FAILURE;
  }


  // Check that the resources are all available
  QStringList pythonFilter;
  pythonFilter << "*.py";
  QDirIterator resourceIterator( ":", pythonFilter, QDir::NoFilter, QDirIterator::NoIteratorFlags );
  int resourceCount = 0;
  while ( resourceIterator.hasNext() ) {
    std::string currentResource = resourceIterator.next().toStdString();
    fileStream << "Current resource: " << currentResource << "." << std::endl;
    resourceCount++;
  }
  fileStream << "Number of resources: " << resourceCount << "." << std::endl;
  
  // Set up to analyze
  peLogic->UpdateToolTrajectories( transformBufferNode );
  peLogic->SetPlaybackTime( peLogic->GetMinTime() );
  peLogic->AddAnalyzeTransform( needleTransformNode );

  fileStream << "Needle rotation parent: " << needleTransformNode->GetParentTransformNode()->GetName() << " " << needleTransformNode->GetParentTransformNode()->GetID() <<std::endl;
  
  peLogic->SetBodyModelNode( tissueModelNode );
  peLogic->SetNeedleTransformNode( needleTransformNode );
  
  peLogic->SetMarkBegin( peLogic->GetMinTime() );
  peLogic->SetMarkEnd( peLogic->GetMaxTime() );
  
  // Calculate the metrics
  std::vector< vtkSlicerPerkEvaluatorLogic::MetricType > calculatedMetrics = peLogic->GetMetrics();
  fileStream << "Number of calculated metrics: " << calculatedMetrics.size() << "." << std::endl;

  if ( calculatedMetrics.size() > 3 )
  {
    //return EXIT_FAILURE;
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
      //return EXIT_FAILURE;
    }
    else
    {
      fileStream << "Correct! Python metric: " << calculatedMetrics.at( i ).first << ". Expected: " << metricsMap[ calculatedMetrics.at( i ).first ] << ", and got: " << calculatedMetrics.at( i ).second << "!" << std::endl;
      //return EXIT_FAILURE;
    }
  }
  
  fileStream << "SOMETHING MUST APPEAR!!!";
  return EXIT_SUCCESS;
}
