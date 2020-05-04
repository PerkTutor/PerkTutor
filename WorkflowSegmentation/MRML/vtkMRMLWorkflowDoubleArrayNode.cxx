
#include "vtkMRMLWorkflowDoubleArrayNode.h"

// Standard MRML Node Methods ------------------------------------------------------------

//------------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkMRMLWorkflowDoubleArrayNode, Array, vtkDoubleArray)

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLWorkflowDoubleArrayNode);



void vtkMRMLWorkflowDoubleArrayNode
::CopyContent( vtkMRMLNode *anode, bool deepCopy/*=true*/ )
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLWorkflowDoubleArrayNode* sourceDoubleArrayNode = vtkMRMLWorkflowDoubleArrayNode::SafeDownCast(anode);
  if (sourceDoubleArrayNode == NULL)
  {
    return;
  }
  if (deepCopy)
  {
    if (sourceDoubleArrayNode->GetArray() != NULL)
    {
      if (this->GetArray() != NULL)
      {
        this->GetArray()->DeepCopy(sourceDoubleArrayNode->GetArray());
      }
      else
      {
        vtkSmartPointer<vtkDoubleArray> newArray = vtkSmartPointer<vtkDoubleArray>::Take(sourceDoubleArrayNode->GetArray()->NewInstance());
        newArray->DeepCopy(sourceDoubleArrayNode->GetArray());
        this->SetArray(newArray);
      }
    }
    else
    {
      // input was nullptr
      this->SetArray(nullptr);
    }
  }
  else
  {
    // shallow-copy
    this->SetArray(sourceDoubleArrayNode->GetArray());
  }
}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLWorkflowDoubleArrayNode
::vtkMRMLWorkflowDoubleArrayNode()
{
  this->Array = vtkDoubleArray::New();
}


vtkMRMLWorkflowDoubleArrayNode
::~vtkMRMLWorkflowDoubleArrayNode()
{
  if (this->Array)
  {
	this->Array->Delete();
	this->Array = nullptr;
  }
}
