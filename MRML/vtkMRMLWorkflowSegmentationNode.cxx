
// WorkflowSegmentation MRML includes
#include "vtkMRMLWorkflowSegmentationNode.h"


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

  this->ToolCollection = vtkWorkflowToolCollection::New();
  this->ToolCompletion = vtkWorkflowToolCollection::New();
  this->Parser = vtkXMLDataParser::New();
}



vtkMRMLWorkflowSegmentationNode
::~vtkMRMLWorkflowSegmentationNode()
{
  vtkDelete( this->ToolCompletion );
  vtkDelete( this->ToolCollection );
  vtkDelete( this->Parser );
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

  vtkWorkflowToolCollection* mergedTools = vtkWorkflowToolCollection::New();
  for ( int i = 0; i < this->ToolCollection->GetNumTools(); i++ )
  {
    mergedTools->AddTool( this->ToolCollection->GetToolAt(i)->DeepCopy() );
  }
  for ( int i = 0; i < this->ToolCompletion->GetNumTools(); i++ )
  {
    mergedTools->AddTool( this->ToolCompletion->GetToolAt(i)->DeepCopy() );
  }
  output << mergedTools->TrainingToXMLString();
  mergedTools->Delete();

  output.close();  
}


void vtkMRMLWorkflowSegmentationNode
::ImportWorkflowProcedure( std::string newWorkflowProcedureFileName )
{
  if ( newWorkflowProcedureFileName.compare( "" ) != 0 )
  {
    this->WorkflowProcedureFileName = newWorkflowProcedureFileName;
  }

  // Create a parser to parse the XML data from the procedure definition
  vtkXMLDataElement* element1 = this->ParseXMLFile( this->WorkflowProcedureFileName );
  this->ToolCollection->ProcedureFromXMLElement( element1 );
  vtkXMLDataElement* element2 = this->ParseXMLFile( this->WorkflowProcedureFileName );
  this->ToolCompletion->ProcedureFromXMLElement( element2 );

  // Change the names of the procedures and tasks associated with completion
  for ( int i = 0; i < this->ToolCompletion->GetNumTools(); i++ )
  {
    vtkWorkflowTool* currentTool = this->ToolCompletion->GetToolAt(i);
    currentTool->Name += "_Completion";
    int numTasks = currentTool->Procedure->GetNumTasks();

	for ( int j = 0; j < numTasks; j++ )
	{
	  vtkWorkflowTask* completionTask = currentTool->Procedure->GetTaskAt(j)->DeepCopy();
	  completionTask->Name = completionTask->Name + "_Completion";
	  currentTool->Procedure->AddTask( completionTask );
	}
  }

}


void
vtkMRMLWorkflowSegmentationNode
::ImportWorkflowInput( std::string newWorkflowInputFileName )
{
  if ( newWorkflowInputFileName.compare( "" ) != 0 )
  {
    this->WorkflowInputFileName = newWorkflowInputFileName;
  }
  if ( ! this->ToolCollection->GetDefined() )
  {
    return;
  }

  // Create a parser to parse the XML data from the input parameters
  vtkXMLDataElement* element = this->ParseXMLFile( this->WorkflowInputFileName );
  this->ToolCollection->InputFromXMLElement( element );

  // The tool collections are constructed so that they have the same order
  for ( int i = 0; i < this->ToolCollection->GetNumTools(); i++ )
  {
    vtkWorkflowTool* currentCompletionTool = this->GetCompletionTool( this->ToolCollection->GetToolAt(i) );
    currentCompletionTool->Input = this->ToolCollection->GetToolAt(i)->Input->DeepCopy();
	currentCompletionTool->Inputted = true;
  }

}


void
vtkMRMLWorkflowSegmentationNode
::ImportWorkflowTraining( std::string newWorkflowTrainingFileName )
{
  if ( newWorkflowTrainingFileName.compare( "" ) != 0 )
  {
    this->WorkflowTrainingFileName = newWorkflowTrainingFileName;
  }
  if ( ! this->ToolCollection->GetInputted() )
  {
    return;
  }

  // Create a parser to parse the XML data from the training parameters
  vtkXMLDataElement* element1 = this->ParseXMLFile( this->WorkflowTrainingFileName );
  this->ToolCollection->TrainingFromXMLElement( element1 );
  vtkXMLDataElement* element2 = this->ParseXMLFile( this->WorkflowTrainingFileName );
  this->ToolCompletion->TrainingFromXMLElement( element2 );
}


void vtkMRMLWorkflowSegmentationNode
::ImportAllWorkflowData()
{
  // Checks already exist to make sure input has procedure and training has input
  this->ImportWorkflowProcedure();
  this->ImportWorkflowInput();
  this->ImportWorkflowTraining();
}


vtkWorkflowTool* vtkMRMLWorkflowSegmentationNode
::GetCompletionTool( vtkWorkflowTool* tool )
{
  for ( int i = 0; i < this->ToolCompletion->GetNumTools(); i++ )
  {
    if ( this->ToolCompletion->GetToolAt(i)->Name.compare( tool->Name + "_Completion" ) == 0 )
	{
      return this->ToolCompletion->GetToolAt(i);
	}
  }

  return NULL;
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


vtkXMLDataElement* vtkMRMLWorkflowSegmentationNode
::ParseXMLFile( std::string fileName )
{
  // Parse the file here, not in the widget
  this->Parser->SetFileName( fileName.c_str() );
  this->Parser->Parse();
  return Parser->GetRootElement();
}