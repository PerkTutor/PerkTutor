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
#include "qSlicerTransformRecorderModuleWidget.h"
#include "ui_qSlicerTransformRecorderModule.h"
#include "qSlicerIO.h"
#include "qSlicerIOManager.h"
#include "qSlicerApplication.h"
#include <QtGui>

// MRMLWidgets includes
#include <qMRMLUtils.h>


#include "vtkMRMLTransformRecorderNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "qMRMLNodeComboBox.h"
#include "vtkMRMLViewNode.h"
#include "vtkSlicerTransformRecorderLogic.h"
#include "vtkMRMLTransformRecorderNode.h"
//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_TransformRecorder
class qSlicerTransformRecorderModuleWidgetPrivate: public Ui_qSlicerTransformRecorderModule
{
  Q_DECLARE_PUBLIC( qSlicerTransformRecorderModuleWidget ); 

protected:
  qSlicerTransformRecorderModuleWidget* const q_ptr;
public:
  qSlicerTransformRecorderModuleWidgetPrivate( qSlicerTransformRecorderModuleWidget& object );
  ~qSlicerTransformRecorderModuleWidgetPrivate();

  vtkSlicerTransformRecorderLogic* logic() const;
  vtkMRMLLinearTransformNode*   MRMLTransformNode;
};

//-----------------------------------------------------------------------------
// qSlicerTransformRecorderModuleWidgetPrivate methods



qSlicerTransformRecorderModuleWidgetPrivate::qSlicerTransformRecorderModuleWidgetPrivate( qSlicerTransformRecorderModuleWidget& object ) : q_ptr(&object)
{
  this->MRMLTransformNode = 0;
}

//-----------------------------------------------------------------------------

qSlicerTransformRecorderModuleWidgetPrivate::~qSlicerTransformRecorderModuleWidgetPrivate()
{
}


vtkSlicerTransformRecorderLogic* qSlicerTransformRecorderModuleWidgetPrivate::logic() const
{
  Q_Q( const qSlicerTransformRecorderModuleWidget );
  return vtkSlicerTransformRecorderLogic::SafeDownCast( q->logic() );
}


//-----------------------------------------------------------------------------
// qSlicerTransformRecorderModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerTransformRecorderModuleWidget::qSlicerTransformRecorderModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerTransformRecorderModuleWidgetPrivate( *this ) )
{
}

//-----------------------------------------------------------------------------
qSlicerTransformRecorderModuleWidget::~qSlicerTransformRecorderModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerTransformRecorderModuleWidget::setup()
{
  Q_D(qSlicerTransformRecorderModuleWidget);
  d->StatusResultLabel= NULL;
  d->setupUi(this);
  this->Superclass::setup();
  d->IGTComboBox->setNoneEnabled( true );
  d->ModuleComboBox->setNoneEnabled( true );
  connect( d->IGTComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onConnectorSelected() ) );
  connect( d->ModuleComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ), this, SLOT( onModuleNodeSelected() ) );
  connect( d->LoadLogButton, SIGNAL(clicked()),this, SLOT (loadLogFile() ) );
  connect( d->StartButton, SIGNAL(pressed()),this,SLOT(onStartButtonPressed() ) );
  connect( d->StopButton, SIGNAL(pressed()),this,SLOT(onStopButtonPressed() ) );
  
  // Connect node selector with module itself
  connect( d->TransformCheckableComboBox, SIGNAL( currentNodeChanged( vtkMRMLNode* ) ),this, SLOT(onTransformsNodeSelected(vtkMRMLNode*)));
  onTransformsNodeSelected(0);

}

//-----------------------------------------------------------------------------
void qSlicerTransformRecorderModuleWidget::onTransformsNodeSelected(vtkMRMLNode* node)
{
  Q_D(qSlicerTransformRecorderModuleWidget);
  
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);


  // Listen for Transform node changes
  this->qvtkReconnect(d->MRMLTransformNode, transformNode,
    vtkMRMLTransformableNode::TransformModifiedEvent,
    this, SLOT(onMRMLTransformNodeModified(vtkObject*)));
 
  d->MRMLTransformNode = transformNode;

}

//-----------------------------------------------------------------------------
void qSlicerTransformRecorderModuleWidget::onMRMLTransformNodeModified(vtkObject* caller)
{
  Q_D(qSlicerTransformRecorderModuleWidget);
  
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(caller);
  if (!transformNode) { return; }




  vtkSmartPointer<vtkTransform> transform = vtkSmartPointer<vtkTransform>::New();
 
  
}

//-----------------------------------------------------------------------------
void qSlicerTransformRecorderModuleWidget::loadLogFile()
{
  Q_D( qSlicerTransformRecorderModuleWidget );
  QString path;
    
  path = QFileDialog::getOpenFileName(this,"Choose a file to open",QString::null,QString::null);

  d->logic()->GetModuleNode()->SaveIntoFile(  path.toStdString() );
 
  
}

// Communicate to the MRML node, which transforms should be saved.
void qSlicerTransformRecorderModuleWidget::onTransformsListUpdate()
{
  Q_D( qSlicerTransformRecorderModuleWidget );

  /* 
  
  std::vector< int > transformSelections;

  for ( int row = 0; row < d->RecordedTransformTable->rowCount(); ++ row ){
    
	  transformSelections.push_back( d->RecordedTransformTable->item( row, 0 )->text().toInt() );
	  
  }

  d->logic()->GetModuleNode()->SetTransformSelections( transformSelections );
  */
  
}



void qSlicerTransformRecorderModuleWidget::enter()
{
  this->updateWidget();
}

void qSlicerTransformRecorderModuleWidget::onConnectorSelected()
{
  Q_D( qSlicerTransformRecorderModuleWidget );
 
  if( d->logic()->GetModuleNode() == NULL )
  {
    return;
  }
  
  vtkMRMLNode* node = d->IGTComboBox->currentNode();
  
  if( node != NULL )
  {
    d->logic()->GetModuleNode()->SetAndObserveConnectorNodeID( node->GetID() );
  }

}



void qSlicerTransformRecorderModuleWidget::onModuleNodeSelected()
{
  Q_D( qSlicerTransformRecorderModuleWidget );
 
  vtkMRMLNode* currentNode = d->ModuleComboBox->currentNode();
  std::cout << "Current node pointer: " << currentNode << std::endl;
  
  vtkMRMLTransformRecorderNode* TRNode = vtkMRMLTransformRecorderNode::SafeDownCast( currentNode );
  if ( TRNode != NULL )
  {
    
  }
  
  d->logic()->SetModuleNode( TRNode );
  this->updateWidget();
}

void qSlicerTransformRecorderModuleWidget::onStartButtonPressed()
{
  Q_D( qSlicerTransformRecorderModuleWidget );

  if( d->logic()->GetModuleNode() != NULL && d->IGTComboBox->currentNode() != NULL  )
  {
    d->logic()->GetModuleNode()->SetRecording( true );
  }

  this->updateWidget();
  
}

void qSlicerTransformRecorderModuleWidget::onStopButtonPressed()
{
  Q_D( qSlicerTransformRecorderModuleWidget );

  if( d->logic()->GetModuleNode() == NULL )
  {
    return;
  }

  d->logic()->GetModuleNode()->SetRecording( false );
   
  this->updateWidget();
}

void qSlicerTransformRecorderModuleWidget::updateWidget()
{
  Q_D( qSlicerTransformRecorderModuleWidget );
      // Disableing node selector widgets if there is no module node to reference input nodes.
    
  if ( d->logic()->GetModuleNode() == NULL )
  {
    d->IGTComboBox->setEnabled( false );
    
  }
  else
  {
    d->IGTComboBox->setEnabled( true );
   
  }
  
  // Check if node selection has changed.
  
  if( d->logic()->GetModuleNode() != NULL && d->IGTComboBox->currentNode() != NULL )
  {
    char* selectedID = d->IGTComboBox->currentNode()->GetID();
    char* nodeID = d->logic()->GetModuleNode()->GetConnectorNodeID();
    if ( strcmp( selectedID, nodeID ) != NULL )
    {
      this->onConnectorSelected();
    }


	//QTableWidgetItem* newItem = new QTableWidgetItem("blaa");
	//d->RecordedTransformTable->insertRow(0);
    //d->RecordedTransformTable->setItem(0, 0, newItem);
	onTransformsListUpdate();

  }
  else if (    d->logic()->GetModuleNode() != NULL
            && d->logic()->GetModuleNode()->GetConnectorNode() != NULL
            && d->IGTComboBox->currentNode() == NULL )
  {
    this->onConnectorSelected();

  }
  
  
    // Update selector widgets if selected nodes have changed.
  
  if ( d->logic()->GetModuleNode() == NULL )
  {
    return;
  }
  
  if ( d->logic()->GetModuleNode()->GetRecording() )
  {
    d->StatusResultLabel->setText( "Recording" );
  }
  else
  {
    d->StatusResultLabel->setText( "Waiting" );
  }


  
  
}
