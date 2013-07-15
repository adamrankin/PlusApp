/*=Plus=header=begin======================================================
Program: Plus
Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
See License.txt for details.
=========================================================Plus=header=end*/ 

#include "PlusConfigure.h"

#include "vtkPlusCommandProcessor.h"
#include "vtkPlusStartStopRecordingCommand.h"
#include "vtkVirtualDiscCapture.h"
#include "vtkDataCollector.h"

vtkStandardNewMacro( vtkPlusStartStopRecordingCommand );

static const char START_CMD[]="StartRecording";
static const char SUSPEND_CMD[]="SuspendRecording";
static const char RESUME_CMD[]="ResumeRecording";
static const char STOP_CMD[]="StopRecording";

//----------------------------------------------------------------------------
vtkPlusStartStopRecordingCommand::vtkPlusStartStopRecordingCommand()
: OutputFilename(NULL)
, CaptureDeviceId(NULL)
{
}

//----------------------------------------------------------------------------
vtkPlusStartStopRecordingCommand::~vtkPlusStartStopRecordingCommand()
{
  SetOutputFilename(NULL);
  SetCaptureDeviceId(NULL);
}

//----------------------------------------------------------------------------
void vtkPlusStartStopRecordingCommand::SetNameToStart() { SetName(START_CMD); }
void vtkPlusStartStopRecordingCommand::SetNameToSuspend() { SetName(SUSPEND_CMD); }
void vtkPlusStartStopRecordingCommand::SetNameToResume() { SetName(RESUME_CMD); }
void vtkPlusStartStopRecordingCommand::SetNameToStop() { SetName(STOP_CMD); }

//----------------------------------------------------------------------------
void vtkPlusStartStopRecordingCommand::GetCommandNames(std::list<std::string> &cmdNames)
{ 
  cmdNames.clear(); 
  cmdNames.push_back(START_CMD);
  cmdNames.push_back(SUSPEND_CMD);
  cmdNames.push_back(RESUME_CMD);
  cmdNames.push_back(STOP_CMD);  
}

//----------------------------------------------------------------------------
std::string vtkPlusStartStopRecordingCommand::GetDescription(const char* commandName)
{ 
  std::string desc;
  if (commandName==NULL || STRCASECMP(commandName, START_CMD))
  {
    desc+=START_CMD;
    desc+=": Start collecting data into file with a VirtualStreamCapture device. CaptureDeviceId: ID of the capture device, if not specified then the first VirtualStreamCapture device will be started (optional)";
  }
  if (commandName==NULL || STRCASECMP(commandName, SUSPEND_CMD))
  {
    desc+=SUSPEND_CMD;
    desc+=": Suspend data collection. Attributes: CaptureDeviceId: (optional)";
  }
  if (commandName==NULL || STRCASECMP(commandName, RESUME_CMD))
  {
    desc+=RESUME_CMD;
    desc+=": Resume suspended data collection. Attributes: CaptureDeviceId (optional)";
  }
  if (commandName==NULL || STRCASECMP(commandName, STOP_CMD))
  {
    desc+=STOP_CMD;
    desc+=": Stop collecting data into file with a VirtualStreamCapture device. Attributes: OutputFilename: name of the output file (optional if base file name is specified in config file). CaptureDeviceId (optional)";
  }
  return desc;
}

//----------------------------------------------------------------------------
void vtkPlusStartStopRecordingCommand::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusStartStopRecordingCommand::ReadConfiguration(vtkXMLDataElement* aConfig)
{  
  if (vtkPlusCommand::ReadConfiguration(aConfig) != PLUS_SUCCESS)
  {
    return PLUS_FAIL;
  }
  // Common parameters
  SetCaptureDeviceId(aConfig->GetAttribute("CaptureDeviceId"));

  // Stop parameters
  SetOutputFilename(aConfig->GetAttribute("OutputFilename"));

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusStartStopRecordingCommand::WriteConfiguration(vtkXMLDataElement* aConfig)
{  
  if (vtkPlusCommand::WriteConfiguration(aConfig)!=PLUS_SUCCESS)
  {
    return PLUS_FAIL;
  }  
  
  // Common parameters
  aConfig->SetAttribute("Name",this->Name);
  if (this->CaptureDeviceId!=NULL)
  {
    aConfig->SetAttribute("CaptureDeviceId",this->CaptureDeviceId);
  }
  else
  {
    aConfig->RemoveAttribute("CaptureDeviceId");
  }

  // Start parameters
  if (this->OutputFilename != NULL)
  {
    aConfig->SetAttribute("OutputFilename",this->OutputFilename);
  }
  else
  {
    aConfig->RemoveAttribute("OutputFilename");
  }

  return PLUS_SUCCESS;
}

//----------------------------------------------------------------------------
vtkVirtualDiscCapture* vtkPlusStartStopRecordingCommand::GetCaptureDevice(const char* captureDeviceId)
{
  vtkDataCollector* dataCollector=GetDataCollector();
  if (dataCollector==NULL)
  {
    LOG_ERROR("Data collector is invalid");    
    return NULL;
  }
  vtkVirtualDiscCapture *captureDevice=NULL;
  if (captureDeviceId!=NULL)
  {
    // Capture device ID is specified
    vtkPlusDevice* device=NULL;
    if (dataCollector->GetDevice(device, captureDeviceId)!=PLUS_SUCCESS)
    {
      LOG_ERROR("No VirtualStreamCapture has been found by the name "<<captureDeviceId);
      return NULL;
    }
    // device found
    captureDevice = dynamic_cast<vtkVirtualDiscCapture*>(device);
    if (captureDevice==NULL)
    {
      // wrong type
      LOG_ERROR("The specified device "<<captureDeviceId<<" is not VirtualStreamCapture");
      return NULL;
    }
  }
  else
  {
    // No capture device id is specified, auto-detect the first one and use that
    for( DeviceCollectionConstIterator it = dataCollector->GetDeviceConstIteratorBegin(); it != dataCollector->GetDeviceConstIteratorEnd(); ++it )
    {
      captureDevice = dynamic_cast<vtkVirtualDiscCapture*>(*it);
      if (captureDevice!=NULL)
      {      
        // found a recording device
        break;
      }
    }
    if (captureDevice==NULL)
    {
      LOG_ERROR("No VirtualStreamCapture has been found");
      return NULL;
    }
  }  
  return captureDevice;
}

//----------------------------------------------------------------------------
PlusStatus vtkPlusStartStopRecordingCommand::Execute()
{
  if (this->Name==NULL)
  {
    LOG_ERROR("Command failed, no command name specified");
    SetCommandCompleted(PLUS_FAIL,"Command failed, no command name specified");
    return PLUS_FAIL;
  }

  vtkVirtualDiscCapture *captureDevice=GetCaptureDevice(this->CaptureDeviceId);
  if (captureDevice==NULL)
  {            
    std::string reply="VirtualStreamCapture has not been found (";
    reply+= this->CaptureDeviceId ? this->CaptureDeviceId : "auto-detect";
    reply+= "), ";
    reply+= this->Name;
    reply+= "failed";
    SetCommandCompleted(PLUS_FAIL,reply);
    return PLUS_FAIL;
  }    

  PlusStatus status=PLUS_SUCCESS;
  std::string reply=std::string("VirtualStreamCapture (")+captureDevice->GetDeviceId()+") "+this->Name+" ";  
  LOG_INFO("vtkPlusStartStopRecordingCommand::Execute: "<<this->Name);
  if (STRCASECMP(this->Name, START_CMD)==0)
  {
    if (captureDevice->OpenFile(this->OutputFilename)!=PLUS_SUCCESS)
    {
      status=PLUS_FAIL;
    }
    captureDevice->SetEnableCapturing(true);
  }
  else if (STRCASECMP(this->Name, SUSPEND_CMD)==0)
  {    
    captureDevice->SetEnableCapturing(false);
  }
  else if (STRCASECMP(this->Name, RESUME_CMD)==0)
  {    
    captureDevice->SetEnableCapturing(true);
  }
  else if (STRCASECMP(this->Name, STOP_CMD)==0)
  {    
    captureDevice->SetEnableCapturing(false);
    if (captureDevice->CloseFile(this->OutputFilename) != PLUS_SUCCESS)
    {
      status=PLUS_FAIL;
    }   
  }
  else
  {
    reply+="unknown command, ";
    SetCommandCompleted(PLUS_FAIL,reply);
  }
  if (status==PLUS_SUCCESS)
  {
    reply+="completed successfully";
  }
  else
  {
    reply+="failed";
  }
  SetCommandCompleted(status,reply);
  return status;
}
