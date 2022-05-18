
// PerkEvaluator Logic includes
#include "vtkSlicerPerkEvaluatorLogic.h"

// MRML includes
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTableNode.h"

// VTK includes
#include <vtkDataArray.h>
#include <vtkIntArray.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkModifiedBSPTree.h>
#include <vtkNew.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSelectEnclosedPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkCollection.h>
#include <vtkCollectionIterator.h>

// STD includes
#include <cassert>
#include <ctime>
#include <iostream>
#include <limits>
#include <sstream>




//----------------------------------------------------------------------------

vtkStandardNewMacro( vtkSlicerPerkEvaluatorLogic );


// Helper for converting QVariants holding string lists to std::vectors of std::strings
std::vector< std::string > QVariantToVector( QVariant variant )
{
  if ( ! variant.canConvert( QVariant::StringList ) )
  {
    return std::vector< std::string >();
  }

  QStringList stringList = variant.toStringList();

  std::vector< std::string > resultVector( stringList.length(), "" );
  for ( int i = 0; i < stringList.length(); i++ )
  {
    resultVector.at( i ) = stringList.at( i ).toStdString();
  }
  return resultVector;
}


// Constructors and Desctructors ----------------------------------------------

vtkSlicerPerkEvaluatorLogic
::vtkSlicerPerkEvaluatorLogic()
{
}



vtkSlicerPerkEvaluatorLogic::
~vtkSlicerPerkEvaluatorLogic()
{
}



// Slicer functions ---------------------------------------------------------------

void vtkSlicerPerkEvaluatorLogic
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  
  os << indent << "vtkSlicerPerkEvaluatorLogic: " << this->GetClassName() << "\n";
}



void vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneEndClose()
{
}


void vtkSlicerPerkEvaluatorLogic
::SetMRMLSceneInternal( vtkMRMLScene * newScene )
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}


void vtkSlicerPerkEvaluatorLogic
::RegisterNodes()
{
  if( ! this->GetMRMLScene() )
  {
    return;
  }

  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::New();
  this->GetMRMLScene()->RegisterNodeClass( peNode );
  peNode->Delete();

  vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::New();
  this->GetMRMLScene()->RegisterNodeClass( msNode );
  msNode->Delete();

  vtkMRMLMetricScriptStorageNode* mssNode = vtkMRMLMetricScriptStorageNode::New();
  this->GetMRMLScene()->RegisterNodeClass( mssNode );
  mssNode->Delete();

  vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::New();
  this->GetMRMLScene()->RegisterNodeClass( miNode );
  miNode->Delete();

  this->PythonManager = qSlicerApplication::application()->pythonManager();
  this->PythonManager->executeString( "import PythonMetricsCalculator" );
  this->PythonManager->executeString( "PythonMetricsCalculator.PythonMetricsCalculatorLogic.Initialize()" );
}


void vtkSlicerPerkEvaluatorLogic
::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}


void vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneNodeAdded( vtkMRMLNode* vtkNotUsed( node ) )
{
}


void vtkSlicerPerkEvaluatorLogic
::OnMRMLSceneNodeRemoved( vtkMRMLNode* vtkNotUsed( node ) )
{
}



// -----------------------------------------------------------------------------------


void vtkSlicerPerkEvaluatorLogic
::ComputeMetrics( vtkMRMLPerkEvaluatorNode* peNode )
{

  // Check conditions
  if ( peNode == NULL || this->GetMRMLScene()->GetNodeByID( peNode->GetID() ) == NULL )
  {
    return;
  }
  if ( peNode->GetMarkBegin() > peNode->GetMarkEnd() ) // TODO: Is a test for to see if the MarkBegin and MarkEnd are within the bounds of the procedure time really necessary?
  {
    return;
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( QString( "PythonMetricsCalculator.PythonMetricsCalculatorLogic.CalculateAllMetrics( '%1' )" ).arg( peNode->GetID() ) );

  peNode->GetMetricsTableNode()->Modified(); // Table has been modified
  peNode->GetMetricsTableNode()->StorableModified(); // Make sure the metrics table is saved by default
}

std::string vtkSlicerPerkEvaluatorLogic
::GetMetricValue( vtkMRMLMetricInstanceNode* miNode, vtkMRMLPerkEvaluatorNode* peNode )
{
  // Verify that the table node exists
  if ( peNode == NULL )
  {
    return "";
  }
  vtkMRMLTableNode* metricsTableNode = peNode->GetMetricsTableNode();
  if ( metricsTableNode == NULL )
  {
    return "";
  }

  std::string miName = this->GetMetricName( miNode->GetAssociatedMetricScriptID() );
  std::string miUnit = this->GetMetricUnit( miNode->GetAssociatedMetricScriptID() );
  std::string miRoles = miNode->GetCombinedRoleString();

  // Simply iterate through all entries in the table, and check if they correspond to the given metric instance node
  for ( int i = 0; i < metricsTableNode->GetTable()->GetNumberOfRows(); i++ )
  {    
    bool namesMatch = miName.compare( metricsTableNode->GetTable()->GetValueByName( i, "MetricName" ).ToString() ) == 0;
    bool unitsMatch = miUnit.compare( metricsTableNode->GetTable()->GetValueByName( i, "MetricUnit" ).ToString() ) == 0;
    bool rolesMatch = miRoles.compare( metricsTableNode->GetTable()->GetValueByName( i, "MetricRoles" ).ToString() ) == 0;

    if ( namesMatch && unitsMatch && rolesMatch )
    {
      return metricsTableNode->GetTable()->GetValueByName( i, "MetricValue" ).ToString(); // Note: returning a string here
    }
  }

  return "";
}


void vtkSlicerPerkEvaluatorLogic
::SetupRealTimeProcessing( vtkMRMLPerkEvaluatorNode* peNode )
{

  // Check conditions
  if ( peNode == NULL || this->GetMRMLScene()->GetNodeByID( peNode->GetID() ) == NULL )
  {
    return;
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( "PythonMetricsCalculatorLogicRealTimeInstance = PythonMetricsCalculator.PythonMetricsCalculatorLogic()" );
  this->PythonManager->executeString( QString( "PythonMetricsCalculatorLogicRealTimeInstance.SetupRealTimeMetricComputation( '%1' )" ).arg( peNode->GetID() ) );
}


// This function is kind of akin to the old "role" system
void vtkSlicerPerkEvaluatorLogic
::SetMetricInstancesRolesToID( vtkMRMLPerkEvaluatorNode* peNode, std::string nodeID, std::string role, /*vtkMRMLMetricInstanceNode::RoleTypeEnum*/ int roleType )
{
  if ( peNode == NULL )
  {
    return;
  }

  // Iterate over all of the metric instance nodes used by the Perk Evaluator node
  std::vector< std::string > metricInstanceIDs = peNode->GetMetricInstanceIDs();
  for ( int i = 0; i < metricInstanceIDs.size(); i++ )
  {
    vtkMRMLMetricInstanceNode* metricInstanceNode = vtkMRMLMetricInstanceNode::SafeDownCast( this->GetMRMLScene()->GetNodeByID( metricInstanceIDs.at( i ) ) );
    if ( metricInstanceNode == NULL )
    {
      continue;
    }

    std::vector< std::string > allRoles = this->GetAllRoles( metricInstanceNode->GetAssociatedMetricScriptID(), roleType );

    for ( int j = 0; j < allRoles.size(); j++ )
    {
      if ( role.compare( allRoles.at( j ) ) == 0 )
      {
        metricInstanceNode->SetRoleID( nodeID, role, roleType ); // Only add if this role is accepted by the metric
      }
    }
    
  }
}


void vtkSlicerPerkEvaluatorLogic
::CreatePervasiveMetric( vtkMRMLMetricScriptNode* msNode, vtkMRMLLinearTransformNode* transformNode, std::string transformRole )
{
  // The assumption is that we have already established that the context of the metric script is "Pervasive"
  if ( msNode == NULL || transformNode == NULL )
  {
    return;
  }

  // Check it doesn't already exist in the scene
  vtkSmartPointer<vtkCollection> metricInstanceNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMetricInstanceNode" ));

  for ( int i = 0; i < metricInstanceNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::SafeDownCast( metricInstanceNodes->GetItemAsObject( i ) );
    bool scriptsEqual = miNode->GetAssociatedMetricScriptNode()->IsEqual( msNode );
    bool oneTransformRole = this->GetAllRoles( msNode->GetID(), vtkMRMLMetricInstanceNode::TransformRole ).size() == 1;
    bool transformRoleIDsEqual = miNode->GetRoleID( transformRole, vtkMRMLMetricInstanceNode::TransformRole ).compare( transformNode->GetID() ) == 0;
    if ( scriptsEqual && oneTransformRole && transformRoleIDsEqual )
    {
      return; // Get out the the function. The metric already exists, so there is nothing to do here.
    }
  }

  // Create it and add it to the scene
  vtkMRMLMetricInstanceNode* newMINode = this->CreateMetricInstance( msNode );
  newMINode->SetRoleID( transformNode->GetID(), transformRole, vtkMRMLMetricInstanceNode::TransformRole );
}


void vtkSlicerPerkEvaluatorLogic
::UpdatePervasiveMetrics( vtkMRMLLinearTransformNode* transformNode )
{
  if ( transformNode == NULL )
  {
    return;
  }

  // Grab all metric scripts
  vtkSmartPointer<vtkCollection> metricScriptNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMetricScriptNode" ));

  for ( int i = 0; i < metricScriptNodes->GetNumberOfItems(); i++ )
  {
    // Assume that we want to add any metric whose only transform role in "Any" and has no anatomy roles
    vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::SafeDownCast( metricScriptNodes->GetItemAsObject( i ) );
    if ( ! this->GetMetricPervasive( msNode->GetID() ) )
    {
      continue;
    }

    // Create a metric instance node, with this transform serving the lone transform role    
    std::vector< std::string > transformRoles = this->GetAllRoles( msNode->GetID(), vtkMRMLMetricInstanceNode::TransformRole );    
    this->CreatePervasiveMetric( msNode, transformNode, transformRoles.at( 0 ) );
  }

}


void vtkSlicerPerkEvaluatorLogic
::UpdatePervasiveMetrics( vtkMRMLMetricScriptNode* msNode )
{
  if ( msNode == NULL || ! this->GetMetricPervasive( msNode->GetID() ) )
  {
    return;
  }
  std::vector< std::string > transformRoles = this->GetAllRoles( msNode->GetID(), vtkMRMLMetricInstanceNode::TransformRole );

  // Grab all transforms
  vtkSmartPointer< vtkCollection > transformNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByClass( "vtkMRMLLinearTransformNode" ));

  for ( int i = 0; i < transformNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( transformNodes->GetItemAsObject( i ) );
    if ( transformNode == NULL || transformNode->GetHideFromEditors() )
    {
      continue;
    }
    // Create a metric instance node, with this transform serving the lone transform role
    this->CreatePervasiveMetric( msNode, transformNode, transformRoles.at( 0 ) ); // Must have precisely one transform role if it is pervasive 
  }

}


vtkMRMLMetricInstanceNode* vtkSlicerPerkEvaluatorLogic
::CreateMetricInstance( vtkMRMLMetricScriptNode* msNode )
{
  if ( msNode == NULL )
  {
    return NULL; // Only work on non-pervasive metrics, the pervasive metrics need a different mechanism
  }

  // Create a metric instance node, with empty roles
  vtkSmartPointer< vtkMRMLMetricInstanceNode > newMINode;
  newMINode.TakeReference( vtkMRMLMetricInstanceNode::SafeDownCast( this->GetMRMLScene()->CreateNodeByClass( "vtkMRMLMetricInstanceNode" ) ) );
  newMINode->SetName( msNode->GetName() );
  newMINode->SetScene( this->GetMRMLScene() );
  newMINode->SetAssociatedMetricScriptID( msNode->GetID() );
	this->GetMRMLScene()->AddNode( newMINode );
  // If it is shared, it will be added automatically to all the perk evaluator nodes
  // If it is not shared, it must be added manually to perk evaluator nodes

  return newMINode;
}


void vtkSlicerPerkEvaluatorLogic
::ShareMetricInstances( vtkMRMLPerkEvaluatorNode* peNode )
{
  if ( peNode == NULL )
  {
    return;
  }

  // Grab all metric instances
  vtkSmartPointer<vtkCollection> metricInstanceNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMetricInstanceNode" ));

  for ( int i = 0; i < metricInstanceNodes->GetNumberOfItems(); i++ )
  {
    // Add all metrics that are "shared"
    vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::SafeDownCast( metricInstanceNodes->GetItemAsObject( i ) );
    if ( ! this->GetMetricShared( miNode->GetAssociatedMetricScriptID() ) )
    {
      continue;
    }
    peNode->AddMetricInstanceID( miNode->GetID() );
  }

}


void vtkSlicerPerkEvaluatorLogic
::ShareMetricInstances( vtkMRMLMetricInstanceNode* miNode )
{
  if ( miNode == NULL || ! this->GetMetricShared( miNode->GetAssociatedMetricScriptID() ) )
  {
    return;
  }

  // Grab all Perk Evaluator nodes
  vtkSmartPointer<vtkCollection> perkEvaluatorNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByClass( "vtkMRMLPerkEvaluatorNode" ));

  for ( int i = 0; i < perkEvaluatorNodes->GetNumberOfItems(); i++ )
  {
    // Add the metric to all of the perk evaluator nodes (since it is shared)
    vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( perkEvaluatorNodes->GetItemAsObject( i ) );
    peNode->AddMetricInstanceID( miNode->GetID() );
  }

}


void vtkSlicerPerkEvaluatorLogic
::MergeMetricScripts( vtkMRMLMetricScriptNode* newMetricScriptNode )
{
  // If any metric script currently in the scene already has the same source code, then move all of its metric instances to the new node
  if ( newMetricScriptNode == NULL )
  {
    return;
  }

  vtkSmartPointer<vtkCollection> metricScriptNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMetricScriptNode" ));

  for ( int i = 0; i < metricScriptNodes->GetNumberOfItems(); i++ )
  {
    // Assume that we want to add any metric whose only transform role in "Any" and has no anatomy roles
    vtkMRMLMetricScriptNode* currMetricScriptNode = vtkMRMLMetricScriptNode::SafeDownCast( metricScriptNodes->GetItemAsObject( i ) );
    bool sameNode = strcmp( currMetricScriptNode->GetID(), newMetricScriptNode->GetID() ) == 0;
    bool equalSource = currMetricScriptNode->IsEqual( newMetricScriptNode );
    bool equalName = strcmp( currMetricScriptNode->GetName(), newMetricScriptNode->GetName() ) == 0;
    bool emptySource = currMetricScriptNode->GetPythonSourceCode().compare( "" ) == 0;
    if ( sameNode || ! equalSource || ! equalName || emptySource )
    {
      continue;
    }

    std::vector< vtkMRMLNode* > referencingNodes;
    this->GetMRMLScene()->GetReferencingNodes( currMetricScriptNode, referencingNodes );
    for ( int j = 0; j < referencingNodes.size(); j++ )
    {
      referencingNodes.at( j )->UpdateReferenceID( currMetricScriptNode->GetID(), newMetricScriptNode->GetID() );
    }

    this->GetMRMLScene()->RemoveNode( currMetricScriptNode );    
  }

}


void vtkSlicerPerkEvaluatorLogic
::MergeAllMetricScripts()
{
  vtkSmartPointer<vtkCollection> metricScriptNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByClass( "vtkMRMLMetricScriptNode" ));

  for ( int i = 0; i < metricScriptNodes->GetNumberOfItems(); i++ )
  {
    vtkMRMLMetricScriptNode* currMetricScriptNode = vtkMRMLMetricScriptNode::SafeDownCast( metricScriptNodes->GetItemAsObject( i ) );
    if ( currMetricScriptNode->GetScene() != NULL ) // Don't look at metrics that have already been removed from the scene
    {
      this->MergeMetricScripts( currMetricScriptNode );
    }
  }

}


void vtkSlicerPerkEvaluatorLogic
::GetProxyRelevantTransformNodes( vtkMRMLSequenceBrowserNode* sequenceBrowser, vtkCollection* relevantTransformNodes )
{
  if ( sequenceBrowser == NULL || relevantTransformNodes == NULL )
  {
    return;
  }

  vtkNew< vtkCollection > proxyNodes;
  sequenceBrowser->GetAllProxyNodes( proxyNodes.GetPointer() );
  this->GetProxyRelevantTransformNodes( proxyNodes.GetPointer(), relevantTransformNodes );
}


void vtkSlicerPerkEvaluatorLogic
::GetProxyRelevantTransformNodes( vtkCollection* proxyNodes, vtkCollection* relevantTransformNodes )
{
  if ( proxyNodes == NULL || relevantTransformNodes == NULL )
  {
    return;
  }

  // Get all transform nodes which are a proxy node or a child of a proxy node
  vtkSmartPointer< vtkCollection > sceneTransformNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByClass( "vtkMRMLLinearTransformNode" ));
  vtkNew< vtkCollectionIterator > sceneTransformNodesIt;
  sceneTransformNodesIt->SetCollection( sceneTransformNodes );

  vtkNew< vtkCollectionIterator > proxyNodesIt;
  proxyNodesIt->SetCollection( proxyNodes );

  for ( sceneTransformNodesIt->InitTraversal(); ! sceneTransformNodesIt->IsDoneWithTraversal(); sceneTransformNodesIt->GoToNextItem() )
  {
    vtkMRMLLinearTransformNode* currSceneTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( sceneTransformNodesIt->GetCurrentObject() );
    if ( currSceneTransformNode == NULL )
    {
      continue;
    }
    for ( proxyNodesIt->InitTraversal(); ! proxyNodesIt->IsDoneWithTraversal(); proxyNodesIt->GoToNextItem() )
    {
      vtkMRMLLinearTransformNode* currProxyTransformNode = vtkMRMLLinearTransformNode::SafeDownCast( proxyNodesIt->GetCurrentObject() );
      if ( currProxyTransformNode == NULL )
      {
        continue;
      }
      if ( currSceneTransformNode == currProxyTransformNode
        || currSceneTransformNode->IsTransformNodeMyParent( currProxyTransformNode ) )
      {
        relevantTransformNodes->AddItem( currSceneTransformNode );
        break;
      }  
    }  
  }
}


std::string vtkSlicerPerkEvaluatorLogic
::GetMetricName( std::string msNodeID )
{
  if ( this->GetMRMLScene()->GetNodeByID( msNodeID ) == NULL )
  {
    return "";
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( QString( "PythonMetricScriptMetricName = PythonMetricsCalculator.PythonMetricsCalculatorLogic.GetMetricName( '%1' )" ).arg( msNodeID.c_str() ) );
  QVariant result = this->PythonManager->getVariable( "PythonMetricScriptMetricName" );
  
  return result.toString().toStdString();
}


std::string vtkSlicerPerkEvaluatorLogic
::GetMetricUnit( std::string msNodeID )
{
  if ( this->GetMRMLScene()->GetNodeByID( msNodeID ) == NULL )
  {
    return "";
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( QString( "PythonMetricScriptMetricUnit = PythonMetricsCalculator.PythonMetricsCalculatorLogic.GetMetricUnit( '%1' )" ).arg( msNodeID.c_str() ) );
  QVariant result = this->PythonManager->getVariable( "PythonMetricScriptMetricUnit" );
  
  return result.toString().toStdString();
}


bool vtkSlicerPerkEvaluatorLogic
::GetMetricShared( std::string msNodeID )
{
  if ( this->GetMRMLScene()->GetNodeByID( msNodeID ) == NULL )
  {
    return false;
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( QString( "PythonMetricScriptMetricShared = PythonMetricsCalculator.PythonMetricsCalculatorLogic.GetMetricShared( '%1' )" ).arg( msNodeID.c_str() ) );
  QVariant result = this->PythonManager->getVariable( "PythonMetricScriptMetricShared" );
  
  return result.toBool();
}


bool vtkSlicerPerkEvaluatorLogic
::GetMetricPervasive( std::string msNodeID )
{
  if ( this->GetMRMLScene()->GetNodeByID( msNodeID ) == NULL )
  {
    return false;
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( QString( "PythonMetricScriptMetricPervasive = PythonMetricsCalculator.PythonMetricsCalculatorLogic.GetMetricPervasive( '%1' )" ).arg( msNodeID.c_str() ) );
  QVariant result = this->PythonManager->getVariable( "PythonMetricScriptMetricPervasive" );
  
  return result.toBool();
}


std::vector< std::string > vtkSlicerPerkEvaluatorLogic
::GetAllRoles( std::string msNodeID, /*vtkMRMLMetricInstanceNode::RoleTypeEnum*/ int roleType )
{
  if ( this->GetMRMLScene()->GetNodeByID( msNodeID ) == NULL )
  {
    return std::vector< std::string >();
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( QString( "PythonMetricScriptRoles = PythonMetricsCalculator.PythonMetricsCalculatorLogic.GetAllRoles( '%1', %2 )" ).arg( msNodeID.c_str() ).arg( roleType ) );
  QVariant result = this->PythonManager->getVariable( "PythonMetricScriptRoles" );

  return QVariantToVector( result );
}


std::string vtkSlicerPerkEvaluatorLogic
::GetAnatomyRoleClassName( std::string msNodeID, std::string role )
{
  if ( this->GetMRMLScene()->GetNodeByID( msNodeID ) == NULL )
  {
    return "";
  }

  // Use the python metrics calculator module
  this->PythonManager->executeString( QString( "PythonMetricScriptAnatomyClassName = PythonMetricsCalculator.PythonMetricsCalculatorLogic.GetAnatomyRoleClassName( '%1', '%2' )" ).arg( msNodeID.c_str() ).arg( role.c_str() ) );
  QVariant result = this->PythonManager->getVariable( "PythonMetricScriptAnatomyClassName" );
  
  return result.toString().toStdString();
}


void vtkSlicerPerkEvaluatorLogic
::DownloadAdditionalMetrics()
{
  // This just call the python metrics calculator's function to download the additional metrics
  // This is because these operations are easier to do in Python
  // TODO: Is there an elegant way to do this in C++
  this->PythonManager->executeString( QString( "PythonMetricsCalculator.PythonMetricsCalculatorLogic.DownloadAdditionalMetrics()" ) );
}


void vtkSlicerPerkEvaluatorLogic
::RestoreDefaultMetrics()
{
  // This just call the python metrics calculator's function to restore the default metrics
  // This is because these operations are easier to do in Python
  // TODO: Is there an elegant way to do this in C++
  this->PythonManager->executeString( QString( "PythonMetricsCalculator.PythonMetricsCalculatorLogic.AddCoreMetricsToScene()" ) );
}


void vtkSlicerPerkEvaluatorLogic
::UpdateSceneToPlaybackTime( vtkMRMLPerkEvaluatorNode* peNode, double playbackTime )
{
  if ( peNode == NULL || peNode->GetTrackedSequenceBrowserNode() == NULL )
  {
    return;
  }

  // Set the correct item number for the master node
  vtkMRMLSequenceNode* masterSequenceNode = peNode->GetTrackedSequenceBrowserNode()->GetMasterSequenceNode();
  if ( masterSequenceNode == NULL )
  {
    return;
  }

  std::stringstream ss;
  ss << playbackTime;
  int itemNumber = masterSequenceNode->GetItemNumberFromIndexValue( ss.str(), false ); // Accept the closest numerical value

  peNode->GetTrackedSequenceBrowserNode()->SetSelectedItemNumber( itemNumber );
}


// Convenience methods for working with sequences --------------------------------
double vtkSlicerPerkEvaluatorLogic
::GetSelectedTime( vtkMRMLSequenceBrowserNode* trackedSequenceBrowserNode )
{
  if ( trackedSequenceBrowserNode == NULL )
  {
    return 0.0;
  }
  vtkMRMLSequenceNode* masterSequenceNode = trackedSequenceBrowserNode->GetMasterSequenceNode();
  if ( masterSequenceNode == NULL )
  {
    return 0.0;
  }

  std::string timeString = masterSequenceNode->GetNthIndexValue( trackedSequenceBrowserNode->GetSelectedItemNumber() );
  std::stringstream timeStream( timeString );
  double time = 0.0; // Default to zero in case conversion goes wrong
  timeStream >> time;

  return time;
}



// Node update methods ----------------------------------------------------------

void vtkSlicerPerkEvaluatorLogic
::ProcessMRMLNodesEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( caller );
  vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::SafeDownCast( caller );


  // Perk Evaluator Node
  // Setup the real-time processing
  if ( peNode != NULL && event == vtkMRMLPerkEvaluatorNode::RealTimeProcessingStartedEvent )
  {
    this->SetupRealTimeProcessing( peNode );
  }

  // Handle an event in the real-time processing
  if ( peNode != NULL && peNode->GetRealTimeProcessing() && event == vtkMRMLPerkEvaluatorNode::TransformRealTimeAddedEvent )
  {
    vtkMRMLSequenceNode* masterSequenceNode = peNode->GetTrackedSequenceBrowserNode()->GetMasterSequenceNode();
    if ( masterSequenceNode == NULL )
    {
      return;
    }
    // Call the metrics update function
    std::string timeString = masterSequenceNode->GetNthIndexValue( masterSequenceNode->GetNumberOfDataNodes() - 1 );
    if ( ! timeString.empty() ) // Time string would be empty if there are no nodes in the sequence
    {
      this->PythonManager->executeString( QString( "PythonMetricsCalculatorLogicRealTimeInstance.UpdateRealTimeMetrics( %1 )" ).arg( timeString.c_str() ) );
    }
    // Make sure the widget is updated to reflect the updated metric values
    peNode->GetMetricsTableNode()->Modified();
  }

}


void vtkSlicerPerkEvaluatorLogic
::ProcessMRMLSceneEvents( vtkObject* caller, unsigned long event, void* callData )
{
  vtkMRMLScene* callerNode = vtkMRMLScene::SafeDownCast( caller );

  // If the added node was a perk evaluator node then observe it
  vtkMRMLNode* addedNode = reinterpret_cast< vtkMRMLNode* >( callData );
  vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( addedNode );
  if ( event == vtkMRMLScene::NodeAddedEvent && peNode != NULL )
  {
    // Observe if a real-time transform event is added
    peNode->AddObserver( vtkMRMLPerkEvaluatorNode::TransformRealTimeAddedEvent, ( vtkCommand* ) this->GetMRMLNodesCallbackCommand() );
    peNode->AddObserver( vtkMRMLPerkEvaluatorNode::RealTimeProcessingStartedEvent, ( vtkCommand* ) this->GetMRMLNodesCallbackCommand() );
  }

  // If a scene is being imported, ignore everything below (because the references should already be set in the scene)
  if ( this->GetMRMLScene() != NULL && this->GetMRMLScene()->IsImporting() )
  {
    return;
  }
  if ( event == vtkMRMLScene::EndImportEvent )
  {
    this->FixOldStyleScene();
    this->MergeAllMetricScripts();
    this->PythonManager->executeString( QString( "PythonMetricsCalculator.PythonMetricsCalculatorLogic.RefreshMetricModules()" ) );
  }

  // If a transform or metric script was added to the scene, make sure all transforms have all pervasive metric instances
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( addedNode );
  if ( event == vtkMRMLScene::NodeAddedEvent && transformNode != NULL )
  {
    this->UpdatePervasiveMetrics( transformNode );
  }
  vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::SafeDownCast( addedNode );
  if ( event == vtkMRMLScene::NodeAddedEvent && msNode != NULL )
  {
    this->MergeMetricScripts( msNode );
    this->PythonManager->executeString( QString( "PythonMetricsCalculator.PythonMetricsCalculatorLogic.RefreshMetricModules()" ) );
    if ( this->GetMetricPervasive( msNode->GetID() ) )
    {
      this->UpdatePervasiveMetrics( msNode );
    }
    else if ( this->GetMetricShared( msNode->GetID() ) )
    {
      this->CreateMetricInstance( msNode );
    }
  }

  // Share any shared metric with all perk evaluator nodes
  vtkMRMLMetricInstanceNode* miNode = vtkMRMLMetricInstanceNode::SafeDownCast( addedNode );
  if ( event == vtkMRMLScene::NodeAddedEvent && miNode != NULL )
  {
    this->ShareMetricInstances( miNode );
  }
  peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( addedNode );
  if ( event == vtkMRMLScene::NodeAddedEvent && peNode != NULL )
  {
    this->ShareMetricInstances( peNode );
  }
}


// This function is only for supporting reading of "old-style" scenes
void vtkSlicerPerkEvaluatorLogic
::FixOldStyleScene()
{
  bool isOldStyleScene = false;
  // Add all metrics from the directories from all Perk Evaluator nodes
  vtkSmartPointer< vtkCollection > peNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByClass( "vtkMRMLPerkEvaluatorNode" ));
  vtkSmartPointer< vtkCollectionIterator > peNodeItr = vtkSmartPointer< vtkCollectionIterator >::New();
  peNodeItr->SetCollection( peNodes );
  for ( peNodeItr->InitTraversal(); ! peNodeItr->IsDoneWithTraversal(); peNodeItr->GoToNextItem() )
  {
    vtkMRMLPerkEvaluatorNode* peNode = vtkMRMLPerkEvaluatorNode::SafeDownCast( peNodeItr->GetCurrentObject() );
    if ( peNode == NULL )
    {
      continue;
    }
    if ( peNode->MetricsDirectory.compare( "" ) != 0 ) // Metrics directory
    {
      this->PythonManager->executeString( QString( "PythonMetricsCalculator.PythonMetricsCalculatorLogic.AddMetricsFromDirectoryToScene( '%1' )" ).arg( peNode->MetricsDirectory.c_str() ) );
      isOldStyleScene = true;
    }
    for ( std::map< std::string, std::string >::iterator itr = peNode->TransformRoleMap.begin(); itr != peNode->TransformRoleMap.end(); itr++ ) // Transform roles
    {
      vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetFirstNodeByName( itr->first.c_str() ) );
      if ( transformNode != NULL )
      {
        this->SetMetricInstancesRolesToID( peNode, transformNode->GetID(), itr->second, vtkMRMLMetricInstanceNode::TransformRole );
        isOldStyleScene = true;
      }
    }
    for ( std::map< std::string, std::string >::iterator itr = peNode->AnatomyNodeMap.begin(); itr != peNode->AnatomyNodeMap.end(); itr++ ) // Transform roles
    {
      vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( this->GetMRMLScene()->GetFirstNodeByName( itr->second.c_str() ) );
      if ( transformNode != NULL )
      {
        this->SetMetricInstancesRolesToID( peNode, transformNode->GetID(), itr->first, vtkMRMLMetricInstanceNode::AnatomyRole );
        isOldStyleScene = true;
      }
    }
  }

  if ( ! isOldStyleScene )
  {
    return;
  }
  // Pervade all the metrics - they are not automatically pervaded during scene load (since the nodes should already exist)
  // But the nodes do not already exist in old-style scenes
  vtkSmartPointer< vtkCollection > transformNodes = vtkSmartPointer<vtkCollection>::Take(this->GetMRMLScene()->GetNodesByClass( "vtkMRMLLinearTransformNode" ));
  vtkSmartPointer< vtkCollectionIterator > transformNodeItr = vtkSmartPointer< vtkCollectionIterator >::New();
  transformNodeItr->SetCollection( transformNodes );
  for ( transformNodeItr->InitTraversal(); ! transformNodeItr->IsDoneWithTraversal(); transformNodeItr->GoToNextItem() )
  {
    vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast( transformNodeItr->GetCurrentObject() );
    if ( transformNode == NULL || transformNode->GetHideFromEditors() )
    {
      continue;
    }
    this->UpdatePervasiveMetrics( transformNode );
  }
}