

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
#include <iostream>
// Qt includes

// SlicerQt includes
#include "qSlicerWorkflowSegmentationModuleWidget.h"
#include "ui_qSlicerWorkflowSegmentationModule.h"
#include "qSlicerIO.h"
#include "qSlicerIOManager.h"
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "qMRMLThreeDWidget.h"
#include <QtGui>

// MRMLWidgets includes
#include <qMRMLUtils.h>


#include "vtkMRMLWorkflowSegmentationNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "qMRMLNodeComboBox.h"
#include "vtkMRMLViewNode.h"
#include "vtkSlicerWorkflowSegmentationLogic.h"
#include "vtkMRMLWorkflowSegmentationNode.h"
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_WorkflowSegmentation
class qSlicerWorkflowSegmentationModuleWidgetPrivate: public Ui_qSlicerWorkflowSegmentationModule
{
  Q_DECLARE_PUBLIC( qSlicerWorkflowSegmentationModuleWidget ); 

protected:
  qSlicerWorkflowSegmentationModuleWidget* const q_ptr;
public:
  qSlicerWorkflowSegmentationModuleWidgetPrivate( qSlicerWorkflowSegmentationModuleWidget& object );
  ~qSlicerWorkflowSegmentationModuleWidgetPrivate();

  vtkSlicerWorkflowSegmentationLogic* logic() const;

  // Add embedded widgets here
  qSlicerTrackedSequenceBrowserWidget* BrowserWidget;
  qSlicerWorkflowSegmentationRecorderControlsWidget* RecorderControlsWidget;
  qSlicerTrackedSequenceMessagesWidget* MessagesWidget;
  qSlicerWorkflowToolWidget* ToolWidget;
  qSlicerWorkflowToolSummaryWidget* ToolSummaryWidget;
  std::vector< qSlicerWorkflowGuideDisplayWidget* > WorkflowDisplayWidgets;
};

//-----------------------------------------------------------------------------
// qSlicerWorkflowSegmentationModuleWidgetPrivate methods



qSlicerWorkflowSegmentationModuleWidgetPrivate::qSlicerWorkflowSegmentationModuleWidgetPrivate( qSlicerWorkflowSegmentationModuleWidget& object ) : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------

qSlicerWorkflowSegmentationModuleWidgetPrivate::~qSlicerWorkflowSegmentationModuleWidgetPrivate()
{
}


vtkSlicerWorkflowSegmentationLogic* qSlicerWorkflowSegmentationModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerWorkflowSegmentationModuleWidget );
  return vtkSlicerWorkflowSegmentationLogic::SafeDownCast( q->logic() );
}


//-----------------------------------------------------------------------------
// qSlicerWorkflowSegmentationModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerWorkflowSegmentationModuleWidget::qSlicerWorkflowSegmentationModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerWorkflowSegmentationModuleWidgetPrivate( *this ) )
{
}



qSlicerWorkflowSegmentationModuleWidget::~qSlicerWorkflowSegmentationModuleWidget()
{
}


void qSlicerWorkflowSegmentationModuleWidget
::setupEmbeddedWidgets()
{
  Q_D(qSlicerWorkflowSegmentationModuleWidget);

  // Adding embedded widgets
  d->BrowserWidget = new qSlicerTrackedSequenceBrowserWidget();
  d->BufferGroupBox->layout()->addWidget( d->BrowserWidget );
  d->BrowserWidget->setMRMLScene( NULL );
  d->BrowserWidget->setMRMLScene( d->logic()->GetMRMLScene() );
  d->BrowserWidget->setTrackedSequenceBrowserNode( NULL ); // Do not automatically select a node on entering the widget

  d->RecorderControlsWidget = new qSlicerWorkflowSegmentationRecorderControlsWidget();
  d->ControlsGroupBox->layout()->addWidget( d->RecorderControlsWidget );
  d->RecorderControlsWidget->setMRMLScene( NULL );
  d->RecorderControlsWidget->setMRMLScene( d->logic()->GetMRMLScene() );

  d->MessagesWidget = new qSlicerTrackedSequenceMessagesWidget();
  d->MessagesGroupBox->layout()->addWidget( d->MessagesWidget );
  d->MessagesWidget->setMRMLScene( NULL );
  d->MessagesWidget->setMRMLScene( d->logic()->GetMRMLScene() );
  
  d->ToolWidget = new qSlicerWorkflowToolWidget();
  d->ToolEditorGroupBox->layout()->addWidget( d->ToolWidget );
  d->ToolWidget->setMRMLScene( NULL );
  d->ToolWidget->setMRMLScene( d->logic()->GetMRMLScene() );
  
  d->ToolSummaryWidget = new qSlicerWorkflowToolSummaryWidget();
  d->ToolSummaryGroupBox->layout()->addWidget( d->ToolSummaryWidget );
  d->ToolSummaryWidget->setMRMLScene( NULL );
  d->ToolSummaryWidget->setMRMLScene( d->logic()->GetMRMLScene() );

  // Setting up connections for embedded widgets
  // Connect the child widget to the transform buffer node change event (they already observe the modified event)


}


void qSlicerWorkflowSegmentationModuleWidget::setup()
{
  Q_D(qSlicerWorkflowSegmentationModuleWidget);

  d->setupUi(this);

  // Add the embedded widgets
  this->setupEmbeddedWidgets();

  // Module node selection
  connect( d->WorkflowSegmentationNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( mrmlNodeChanged( vtkMRMLNode* ) ) );

  // Transform buffer
  connect( d->BrowserWidget, SIGNAL( transformBufferNodeChanged( vtkMRMLNode* ) ), this, SLOT( onTrackedSequenceBrowserNodeChanged( vtkMRMLNode* ) ) );

  // Display
  connect( d->BrowserWidget, SIGNAL( transformBufferNodeChanged( vtkMRMLNode* ) ), d->RecorderControlsWidget, SLOT( setTransformBufferNode( vtkMRMLNode* ) ) );
  connect( d->WorkflowSegmentationNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), d->RecorderControlsWidget, SLOT( setWorkflowSegmentationNode( vtkMRMLNode* ) ) );
  connect( d->BrowserWidget, SIGNAL( transformBufferNodeChanged( vtkMRMLNode* ) ), d->MessagesWidget, SLOT( setTransformBufferNode( vtkMRMLNode* ) ) );
  
  // Advanced
  connect( d->RefreshButton, SIGNAL( clicked() ), this, SLOT( createWorkflowDisplaysForExistingNodes() ) );
  connect( d->WorkflowSegmentationNodeComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), d->ToolSummaryWidget, SLOT( setWorkflowSegmentationNode( vtkMRMLNode* ) ) );
  
  // Workflows
  this->createWorkflowDisplaysForExistingNodes();
  this->qvtkConnect( d->logic()->GetMRMLScene(), vtkMRMLScene::NodeAddedEvent, this, SLOT( onNodeAdded( vtkObject*, vtkObject* ) ) );
  this->qvtkConnect( d->logic()->GetMRMLScene(), vtkMRMLScene::NodeRemovedEvent, this, SLOT( onNodeRemoved( vtkObject*, vtkObject* ) ) );

  this->updateWidgetFromMRML();
}


void qSlicerWorkflowSegmentationModuleWidget
::onTrackedSequenceBrowserNodeChanged( vtkMRMLNode* newTrackedSequenceBrowserNode )
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );

  vtkMRMLWorkflowSegmentationNode* wsNode = vtkMRMLWorkflowSegmentationNode::SafeDownCast( d->WorkflowSegmentationNodeComboBox->currentNode() );
  if ( wsNode == NULL )
  {
    return;
  }

  if ( newTrackedSequenceBrowserNode == NULL )
  {
    wsNode->SetTrackedSequenceBrowserNodeID( "" );
    return;
  }

  wsNode->SetTrackedSequenceBrowserNodeID( newTrackedSequenceBrowserNode->GetID() );
  // The Workflow Segmentation node automatically call the "updateWidgetFromMRML" function to deal with the widget
}


void qSlicerWorkflowSegmentationModuleWidget
::mrmlNodeChanged( vtkMRMLNode* wsNode )
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  this->qvtkDisconnectAll(); // Remove connections to previous node
  if ( wsNode != NULL )
  {
    this->qvtkConnect( wsNode, vtkCommand::ModifiedEvent, this, SLOT( updateWidgetFromMRML() ) );
  }
  // Reconnect to scene
  this->qvtkConnect( d->logic()->GetMRMLScene(), vtkMRMLScene::NodeAddedEvent, this, SLOT( onNodeAdded( vtkObject*, vtkObject* ) ) );
  this->qvtkConnect( d->logic()->GetMRMLScene(), vtkMRMLScene::NodeRemovedEvent, this, SLOT( onNodeRemoved( vtkObject*, vtkObject* ) ) );

  this->updateWidgetFromMRML();
}


void qSlicerWorkflowSegmentationModuleWidget
::onNodeAdded( vtkObject* caller, vtkObject* node )
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  // Check if it is a workflow tool node
  vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( node );
  if ( toolNode == NULL )
  {
    return;
  }

  qSlicerWorkflowGuideDisplayWidget* currentWidget = new qSlicerWorkflowGuideDisplayWidget( d->WorkflowsCollapsibleButton );  
  d->WorkflowsCollapsibleButton->layout()->addWidget( currentWidget );

  currentWidget->setWorkflowToolNode( toolNode );

  d->WorkflowDisplayWidgets.push_back( currentWidget );
}


void qSlicerWorkflowSegmentationModuleWidget
::onNodeRemoved( vtkObject* caller, vtkObject* node )
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  // Check if it is a workflow tool node
  vtkMRMLWorkflowToolNode* toolNode = vtkMRMLWorkflowToolNode::SafeDownCast( node );
  if ( toolNode == NULL )
  {
    return;
  }

  // Iterate through all widgets and find the right one to remove
  for ( int i = 0; i < d->WorkflowDisplayWidgets.size(); i++ )
  {
    if ( d->WorkflowDisplayWidgets.at( i )->getWorkflowToolNode() == toolNode )
    {
      d->WorkflowsCollapsibleButton->layout()->removeWidget( d->WorkflowDisplayWidgets.at( i ) );
      delete d->WorkflowDisplayWidgets.at( i );
      d->WorkflowDisplayWidgets.erase( d->WorkflowDisplayWidgets.begin() + i );
    }
  }

}


void qSlicerWorkflowSegmentationModuleWidget
::createWorkflowDisplaysForExistingNodes()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );
  
  // Grab all of the tool nodes
  vtkSmartPointer< vtkCollection > toolNodes = vtkSmartPointer< vtkCollection >::Take( d->logic()->GetMRMLScene()->GetNodesByClass( "vtkMRMLWorkflowToolNode" ) );

  for ( int i = 0; i < toolNodes->GetNumberOfItems(); i++ )
  {
    this->onNodeRemoved( d->logic()->GetMRMLScene(), toolNodes->GetItemAsObject( i ) );
    this->onNodeAdded( d->logic()->GetMRMLScene(), toolNodes->GetItemAsObject( i ) );
  }
}


void qSlicerWorkflowSegmentationModuleWidget
::updateWidgetFromMRML()
{
  Q_D( qSlicerWorkflowSegmentationModuleWidget );

  // Grab the MRML node
  vtkMRMLWorkflowSegmentationNode* wsNode = vtkMRMLWorkflowSegmentationNode::SafeDownCast( d->WorkflowSegmentationNodeComboBox->currentNode() );
  if ( wsNode == NULL )
  {
    return;
  }
  
  d->BrowserWidget->setTrackedSequenceBrowserNode( wsNode->GetTrackedSequenceBrowserNode() );
  d->RecorderControlsWidget->setTrackedSequenceBrowserNode( wsNode->GetTrackedSequenceBrowserNode() );
  d->MessagesWidget->setTrackedSequenceBrowserNode( wsNode->GetTrackedSequenceBrowserNode() );

  d->RecorderControlsWidget->setWorkflowSegmentationNode( wsNode );
  d->ToolSummaryWidget->setWorkflowSegmentationNode( wsNode ); // TODO: Is this necessary
  // Everything else taken care of by the child widgets
}
