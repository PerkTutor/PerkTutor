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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// FooBar Widgets includes
#include "qSlicerTransformBufferWidgetHelper.h"
#include "qSlicerApplication.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"

#include <QtGui>


// Static methods -------------------------------------------------------------

vtkMRMLAbstractLogic* qSlicerTransformBufferWidgetHelper
::GetSlicerModuleLogic( std::string moduleName )
{
  qSlicerAbstractCoreModule* Module = qSlicerApplication::application()->moduleManager()->module( moduleName.c_str() );
  if ( Module != NULL )
  {
    return Module->logic();
  }
  return NULL;
}

//-----------------------------------------------------------------------------
// qSlicerTransformBufferWidgetHelper methods

//-----------------------------------------------------------------------------
qSlicerTransformBufferWidgetHelper
::qSlicerTransformBufferWidgetHelper() : qSlicerWidget()
{
  this->TransformBufferNode = NULL;
}


qSlicerTransformBufferWidgetHelper
::~qSlicerTransformBufferWidgetHelper()
{
}


vtkMRMLTransformBufferNode* qSlicerTransformBufferWidgetHelper
::GetTransformBufferNode()
{
  return this->TransformBufferNode;
}


// SLOTS ----------------------------------------------------------------------

void qSlicerTransformBufferWidgetHelper
::SetTransformBufferNode( vtkMRMLTransformBufferNode* newTransformBufferNode )
{
  // Reconnect listeners
  this->qvtkDisconnect( this->TransformBufferNode, vtkCommand::ModifiedEvent, this, SLOT( onTransformBufferNodeModified() ) );
  this->TransformBufferNode = newTransformBufferNode;
  this->qvtkConnect( this->TransformBufferNode, vtkCommand::ModifiedEvent, this, SLOT( onTransformBufferNodeModified() ) );

  emit transformBufferNodeChanged( this->TransformBufferNode );
}


void qSlicerTransformBufferWidgetHelper
::onTransformBufferNodeModified()
{
  emit transformBufferNodeModified();
}