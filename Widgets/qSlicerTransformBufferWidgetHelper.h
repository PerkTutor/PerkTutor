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

#ifndef __qSlicerTransformBufferWidgetHelper_h
#define __qSlicerTransformBufferWidgetHelper_h

// Qt includes
#include "qSlicerWidget.h"

// FooBar Widgets includes
#include "qSlicerTransformRecorderModuleWidgetsExport.h"

#include "vtkSlicerModuleLogic.h"
#include "vtkMRMLTransformBufferNode.h"

class qSlicerTransformBufferWidgetHelperPrivate;

/// \ingroup Slicer_QtModules_CreateModels
class Q_SLICER_MODULE_TRANSFORMRECORDER_WIDGETS_EXPORT 
qSlicerTransformBufferWidgetHelper : public qSlicerWidget
{
  Q_OBJECT
public:

  qSlicerTransformBufferWidgetHelper();
  virtual ~qSlicerTransformBufferWidgetHelper();
  
  vtkMRMLTransformBufferNode* GetTransformBufferNode();

  // Static methods to help get logic
  static vtkMRMLAbstractLogic* GetSlicerModuleLogic( std::string moduleName );

public slots:

  virtual void SetTransformBufferNode( vtkMRMLTransformBufferNode* newTransformBufferNode );

protected slots:

  void onTransformBufferNodeModified();
  void onTransformBufferTransformAdded( vtkObject* caller, void* callData, unsigned long event );
  void onTransformBufferTransformRemoved( vtkObject* caller, void* callData, unsigned long event );
  void onTransformBufferMessageAdded( vtkObject* caller, void* callData, unsigned long event );
  void onTransformBufferMessageRemoved( vtkObject* caller, void* callData, unsigned long event );
  void onTransformBufferActiveTransformAdded();
  void onTransformBufferActiveTransformRemoved();

signals:

   // Classes using this helper should listen to these signals to know when to update
  void transformBufferNodeChanged( vtkMRMLTransformBufferNode* );

  // Should offer an interface for all the possible events the buffer node could invoke
  void transformBufferNodeModified();
  void transformBufferTransformAdded( int );
  void transformBufferTransformRemoved( int );
  void transformBufferMessageAdded( int );
  void transformBufferMessageRemoved( int );
  void transformBufferActiveTransformAdded();
  void transformBufferActiveTransformRemoved();

protected:

  vtkMRMLTransformBufferNode* TransformBufferNode;

private:
  Q_DECLARE_PRIVATE(qSlicerTransformBufferWidgetHelper);
  Q_DISABLE_COPY(qSlicerTransformBufferWidgetHelper);

};

#endif
