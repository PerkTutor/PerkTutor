
#ifndef __vtkLabelVector_h
#define __vtkLabelVector_h

// Standard Includes
#include <string>
#include <sstream>
#include <vector>
#include <cmath>

// VTK includes
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataElement.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkLabelVector : public vtkObject
{
public:
  vtkTypeMacro( vtkLabelVector, vtkObject );

  // Standard MRML methods
  static vtkLabelVector* New();

  vtkLabelVector* DeepCopy();

protected:

  // Constructo/destructor
  vtkLabelVector();
  virtual ~vtkLabelVector();

private:

  std::vector<double> Values;
  std::string Label;

public:

  void Initialize( int size, double value );

  void Add( double newValue );
  void Set( int index, double newValue );
  void Crement( int index, double step = 1 );
  double Get( int index );
  int Size();

  void SetValues( std::vector<double> newValues );
  std::vector<double> GetValues();

  std::string ToString();
  void FromString( std::string instring, int size );

  std::string GetLabel();
  void SetLabel( std::string newLabel );
  void SetLabel( int newLabel );

  std::string ToXMLString( std::string name );
  void FromXMLElement( vtkXMLDataElement* element, std::string name );

};


template <class T> 
void vtkDeleteVector( std::vector<T*> vector )
{
  for ( int i = 0; i < vector.size(); i++ )
  {
    vtkDelete( vector.at(i) );
  }
  vector.clear();
}


template <class T>
void vtkDelete( T* object )
{
  if ( object != NULL )
  {
    object->Delete();
  }
}


template <class T>
std::vector<T*> vtkDeepCopyVector( std::vector<T*> vector )
{
  std::vector<T*> copyVector;
  for ( int i = 0; i < vector.size(); i++ )
  {
    if ( vector.at(i) != NULL )
	{
      copyVector.push_back( vector.at(i)->DeepCopy() );
	}
  }
  return copyVector;
}


template <class T>
T* vtkDeleteAssign( T* assignTo, T* assignFrom )
{
  vtkDelete( assignTo );
  return assignFrom;
}



#endif