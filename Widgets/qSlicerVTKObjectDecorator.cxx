
// WrapperQtVTK includes
#include "qSlicerVTKObjectDecorator.h"

qSlicerVTKObjectDecorator
::qSlicerVTKObjectDecorator()
{
  this->Object = NULL;
}


qSlicerVTKObjectDecorator
::~qSlicerVTKObjectDecorator()
{
}


qSlicerVTKObjectDecorator
::qSlicerVTKObjectDecorator( const qSlicerVTKObjectDecorator& source )
{
  this->Object = source.Object;
}


vtkObject* qSlicerVTKObjectDecorator
::GetObject()
{
  return this->Object;
}


void qSlicerVTKObjectDecorator
::SetObject( vtkObject* newObject )
{
  this->Object = newObject;
}