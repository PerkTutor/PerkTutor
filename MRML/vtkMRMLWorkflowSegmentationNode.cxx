
// VTK includes
#include <vtkCleanPolyData.h>
#include <vtkCommand.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkExtractSelectedPolyDataIds.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkPlanes.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>
#include "vtkIntArray.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkXMLDataParser.h"

// WorkflowSegmentation MRML includes
#include "vtkMRMLWorkflowSegmentationNode.h"

// MRML includes
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationROINode.h>
#include "vtkMRMLLinearTransformNode.h"
// STD includes
#include <math.h>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>


// MACROS ---------------------------------------------------------------------

#define DELETE_IF_NOT_NULL(x) \
  if ( x != NULL ) \
    { \
    x->Delete(); \
    x = NULL; \
    }

#define WRITE_STRING_XML(x) \
  if ( this->x != NULL ) \
  { \
    of << indent << " "#x"=\"" << this->x << "\"\n"; \
  }

#define READ_AND_SET_STRING_XML(x) \
    if ( !strcmp( attName, #x ) ) \
      { \
      this->SetAndObserve##x( NULL ); \
      this->Set##x( attValue ); \
      }


// For testing.
void fileOutput( std::string str, double num )
{
  std::ofstream output( "_vtkMRMLWorkflowSegmentationNode.txt", std::ios_base::app );
  time_t seconds;
  seconds = time( NULL );
  
  output << seconds << " : " << str << " - " << num << std::endl;
  output.close();
}



// Helper functions.
// ----------------------------------------------------------------------------

bool
StringToBool( std::string str )
{
  bool var;
  std::stringstream ss( str );
  ss >> var;
  return var;
}

// Constructors and Destructors
// ----------------------------------------------------------------------------

vtkMRMLWorkflowSegmentationNode* vtkMRMLWorkflowSegmentationNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSegmentationNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSegmentationNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSegmentationNode;
}



vtkMRMLWorkflowSegmentationNode
::vtkMRMLWorkflowSegmentationNode()
{
  this->HideFromEditorsOff();
  this->SetSaveWithScene( true );
  // this->SetModifiedSinceRead( true );

  this->WorkflowProcedureFileName = "";
  this->WorkflowInputFileName = "";
  this->WorkflowTrainingFileName = "";
}



vtkMRMLWorkflowSegmentationNode
::~vtkMRMLWorkflowSegmentationNode()
{
}



vtkMRMLNode* vtkMRMLWorkflowSegmentationNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSegmentationNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSegmentationNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSegmentationNode;
}



// Scene: Save and load
// ----------------------------------------------------------------------------


void vtkMRMLWorkflowSegmentationNode::WriteXML( ostream& of, int nIndent )
{

  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  
  of << indent << " WorkflowProcedureFileName=\"" << this->WorkflowProcedureFileName << "\"";
  of << indent << " WorkflowInputFileName=\"" << this->WorkflowInputFileName << "\"";
  of << indent << " WorkflowTrainingFileName=\"" << this->WorkflowTrainingFileName << "\"";

}



void vtkMRMLWorkflowSegmentationNode::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
    
	if ( ! strcmp( attName, "WorkflowProcedureFileName" ) )
    {
      this->WorkflowProcedureFileName = std::string( attValue );
    }
	if ( ! strcmp( attName, "WorkflowInputFileName" ) )
    {
      this->WorkflowInputFileName = std::string( attValue );
    }
	if ( ! strcmp( attName, "WorkflowTrainingFileName" ) )
    {
      this->WorkflowTrainingFileName = std::string( attValue );
    }

  }

  // Now, read from file if the files are specified
  this->ImportAllWorkflowData();

}



// Slicer Scene
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode::Copy( vtkMRMLNode *anode )
{  
  Superclass::Copy( anode );
  vtkMRMLWorkflowSegmentationNode *node = ( vtkMRMLWorkflowSegmentationNode* ) anode;
  
  // Note: It seems that the WriteXML function copies the node then writes the copied node to file
  // So, anything we want in the MRML file we must copy here (I don't think we need to copy other things)
  this->SetWorkflowProcedureFileName( node->GetWorkflowProcedureFileName() );
  this->SetWorkflowInputFileName( node->GetWorkflowInputFileName() );
  this->SetWorkflowTrainingFileName( node->GetWorkflowTrainingFileName() );
  this->ImportAllWorkflowData();

}



void vtkMRMLWorkflowSegmentationNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
  os << indent << "WorkflowProcedureFileName: " << this->WorkflowProcedureFileName << "\n";
  os << indent << "WorkflowInputFileName: " << this->WorkflowInputFileName << "\n";
  os << indent << "WorkflowTrainingFileName: " << this->WorkflowTrainingFileName << "\n";
}




// File I/O: Saving and loading
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode
::SaveWorkflowTraining( std::string newWorkflowTrainingFileName )
{
  if ( newWorkflowTrainingFileName.compare( "" ) != 0 )
  {
    this->WorkflowTrainingFileName = newWorkflowTrainingFileName;
  }

  std::ofstream output( this->WorkflowTrainingFileName.c_str() );  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Record file could not be opened!" );
    return;
  }  
  output << this->ToolCollection->TrainingToXMLString();

  output.close();  
}



void vtkMRMLWorkflowSegmentationNode
::ImportWorkflowProcedure( std::string newWorkflowProcedureFileName )
{
  if ( newWorkflowProcedureFileName.compare( "" ) != 0 )
  {
    this->WorkflowProcedureFileName = newWorkflowProcedureFileName;
  }

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( this->WorkflowProcedureFileName.c_str() );
  parser->Parse();
  
  // Get the root element (and check it exists)
  vtkXMLDataElement* rootElement = parser->GetRootElement();

  this->ToolCollection->ProcedureFromXMLElement( rootElement );

}




void
vtkMRMLWorkflowSegmentationNode
::ImportWorkflowInput( std::string newWorkflowInputFileName )
{
  if ( newWorkflowInputFileName.compare( "" ) != 0 )
  {
    this->WorkflowInputFileName = newWorkflowInputFileName;
  }

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( this->WorkflowInputFileName.c_str() );
  parser->Parse();
  
  // Get the root element (and check it exists)
  vtkXMLDataElement* rootElement = parser->GetRootElement();

  this->ToolCollection->InputFromXMLElement( rootElement );

}


void
vtkMRMLWorkflowSegmentationNode
::ImportWorkflowTraining( std::string newWorkflowTrainingFileName )
{
  if ( newWorkflowTrainingFileName.compare( "" ) != 0 )
  {
    this->WorkflowTrainingFileName = newWorkflowTrainingFileName;
  }

  // Create a parser to parse the XML data from TransformRecorderLog
  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( this->WorkflowTrainingFileName.c_str() );
  parser->Parse();
  
  // Get the root element (and check it exists)
  vtkXMLDataElement* rootElement = parser->GetRootElement();

  this->ToolCollection->TrainingFromXMLElement( rootElement );

}


void vtkMRMLWorkflowSegmentationNode
::ImportAllWorkflowData()
{
  this->ImportWorkflowProcedure();
  this->ImportWorkflowInput();
  this->ImportWorkflowTraining();
}




// File I/O: Getters and setters
// ----------------------------------------------------------------------------

std::string vtkMRMLWorkflowSegmentationNode
::GetWorkflowProcedureFileName()
{
  return this->WorkflowProcedureFileName;
}

void vtkMRMLWorkflowSegmentationNode
::SetWorkflowProcedureFileName( std::string newWorkflowTrainingFileName )
{
  this->WorkflowProcedureFileName = newWorkflowTrainingFileName;
}

std::string vtkMRMLWorkflowSegmentationNode
::GetWorkflowInputFileName()
{
  return this->WorkflowInputFileName;
}

void vtkMRMLWorkflowSegmentationNode
::SetWorkflowInputFileName( std::string newWorkflowProcedureFileName )
{
  this->WorkflowInputFileName = newWorkflowProcedureFileName;
}

std::string vtkMRMLWorkflowSegmentationNode
::GetWorkflowTrainingFileName()
{
  return this->WorkflowTrainingFileName;
}

void vtkMRMLWorkflowSegmentationNode
::SetWorkflowTrainingFileName( std::string newWorkflowTrainingFileName )
{
  this->WorkflowTrainingFileName = newWorkflowTrainingFileName;
}