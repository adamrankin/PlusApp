#include "PlusConfigure.h"
#include "vtkSavedDataTracker.h"
#include "vtkObjectFactory.h"
#include <sstream>
#include "vtkTracker.h"
#include "vtkTrackerTool.h"
#include "vtkTrackerBuffer.h"
#include "vtkFrameToTimeConverter.h"
#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtksys/SystemTools.hxx"
#include "vtkXMLDataElement.h"
#include "vtkTrackedFrameList.h"

//----------------------------------------------------------------------------
vtkSavedDataTracker* vtkSavedDataTracker::New()
{
	// First try to create the object from the vtkObjectFactory
	vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSavedDataTracker");
	if(ret)
	{
		return (vtkSavedDataTracker*)ret;
	}
	// If the factory was unable to create the object, then create it here.
	return new vtkSavedDataTracker;
}

//----------------------------------------------------------------------------
vtkSavedDataTracker::vtkSavedDataTracker()
{
	this->LocalTrackerBuffer = NULL;
	this->SequenceMetafile = NULL; 
	this->StartTimestamp = 0.0; 
	this->Tracking = 0;
	this->Initialized = false;
	this->ReplayEnabled = false; 
}

//----------------------------------------------------------------------------
vtkSavedDataTracker::~vtkSavedDataTracker() 
{
	if (this->Tracking)
	{
		this->StopTracking();
	}

	if ( this->LocalTrackerBuffer != NULL )
	{
		this->LocalTrackerBuffer->Delete(); 
		this->LocalTrackerBuffer = NULL; 
	}
}

//----------------------------------------------------------------------------
void vtkSavedDataTracker::PrintSelf(ostream& os, vtkIndent indent)
{
	vtkTracker::PrintSelf(os,indent);
}


/**
 * @returns 1 on success, 0 on failure.
 */
int vtkSavedDataTracker::Connect()
{
	LOG_TRACE("vtkSavedDataTracker::Connect"); 
	vtkSmartPointer<vtkTrackedFrameList> savedDataBuffer = vtkSmartPointer<vtkTrackedFrameList>::New(); 
		
	if ( !this->Probe() )
	{
		return 0; 
	}
  
  if ( this->Initialized )
    {
    return 1;
    }
  
  
	// Read metafile
	savedDataBuffer->ReadFromSequenceMetafile(this->GetSequenceMetafile()); 
	
	// TODO: Do we need other tools than the defult tool? 
	this->SetNumberOfTools(1);

	// Enable tools
	for ( int tool = 0; tool < this->GetNumberOfTools(); tool++ )
	{
		this->GetTool(tool)->EnabledOn(); 
		
	}
  
	// Set tool names
	this->SetToolName(0, savedDataBuffer->GetDefaultFrameTransformName().c_str()); 
	
	// Set default tool name
	this->SetDefaultToolName(savedDataBuffer->GetDefaultFrameTransformName().c_str()); 
	
	// TODO: Read this from the config file.
	this->GetTool( this->GetDefaultTool() )->SetSendToLink( "localhost:18944" );
	
	if ( this->LocalTrackerBuffer == NULL )
	{
		this->LocalTrackerBuffer = vtkTrackerBuffer::New(); 
	}

	this->LocalTrackerBuffer->SetBufferSize(savedDataBuffer->GetNumberOfTrackedFrames()); 

	// Fill local buffers 
	for ( unsigned int frame = 0; frame < savedDataBuffer->GetNumberOfTrackedFrames(); ++frame)
	{
		TrackedFrame* trackedFrame = savedDataBuffer->GetTrackedFrame(frame); 

		// Get default transform 
		double defaultTransform[16]; 
		if ( !trackedFrame->GetDefaultFrameTransform(defaultTransform) )
		{
			LOG_WARNING("Unable to get default frame transform for frame #" << frame ); 
			continue; 
		}
		vtkSmartPointer<vtkMatrix4x4> defaultTransformMatrix = vtkSmartPointer<vtkMatrix4x4>::New(); 
		defaultTransformMatrix->DeepCopy(defaultTransform); 
		
		// Get frame number
		const char* strFrameNumber = trackedFrame->GetCustomFrameField("FrameNumber"); 
		long frameNumber = -1;
		if ( strFrameNumber == NULL ) 
		{
			frameNumber = frame;
		}
		else
		{
		  frameNumber = atol(strFrameNumber); 
		}

		// Get Timestamp
		const char* strTimestamp = trackedFrame->GetCustomFrameField("Timestamp"); 
		double timestamp = -1;
		if ( strTimestamp == NULL ) 
		{
			timestamp = frameNumber / 10.0;  // This is not a normal behaviour.
		}
		else
		{
		  timestamp = atof(strTimestamp); 
		}

		// Get UnfilteredTimestamp
		const char* strUnfilteredTimestamp = trackedFrame->GetCustomFrameField("UnfilteredTimestamp"); 
		double unfilteredTimestamp = -1;
		if ( strUnfilteredTimestamp == NULL ) 
		{
			unfilteredTimestamp = timestamp;
		}
		else
		{
		  unfilteredTimestamp = atof(strUnfilteredTimestamp); 
		}

		// Get Status
		const char* strStatus = trackedFrame->GetCustomFrameField("Status"); 
		long status = TR_OK; 
		if ( strStatus == NULL ) 
		{
			LOG_DEBUG("Unable to get Status for frame #" << frame ); 
		}
		else
		{
		  if ( STRCASECMP(strStatus, "OK") != 0 )
		  {
			  status = TR_MISSING; 
		  }
		}
		
		LocalTrackerBuffer->AddItem(defaultTransformMatrix, status, frameNumber, unfilteredTimestamp, timestamp); 
	}
  
	this->Initialized = true;
	return 1; 
}

//-----------------------------------------------------------------------------
void vtkSavedDataTracker::Disconnect()
{
	LOG_TRACE("vtkSavedDataTracker::Disconnect"); 
	this->StopTracking(); 
}

//----------------------------------------------------------------------------
int vtkSavedDataTracker::Probe()
{
	LOG_TRACE("vtkSavedDataTracker::Probe"); 
	if ( !vtksys::SystemTools::FileExists(this->GetSequenceMetafile(), true) )
	{
		LOG_ERROR("SavedDataTracker Probe failed: Unable to read sequence metafile!"); 
		return 0; 
	}
	return 1; 
} 

//----------------------------------------------------------------------------
int vtkSavedDataTracker::InternalStartTracking()
{
	LOG_TRACE("vtkSavedDataTracker::InternalStartTracking"); 
	if (this->Tracking)
	{
		return 1;
	}

	if (!this->InitSavedDataTracker())
	{
		LOG_ERROR("Couldn't initialize SavedDataTracker");
		this->Tracking = 0;
		return 0;
	} 

	this->SetStartTimestamp(vtkAccurateTimer::GetSystemTime()); 

	// for accurate timing
	this->Timer->Initialize();
	this->Tracking = 1;

	return 1;
}

//----------------------------------------------------------------------------
int vtkSavedDataTracker::InternalStopTracking()
{
	LOG_TRACE("vtkSavedDataTracker::InternalStopTracking"); 
	return 1;
}

//----------------------------------------------------------------------------
void vtkSavedDataTracker::InternalUpdate()
{
	LOG_TRACE("vtkSavedDataTracker::InternalUpdate"); 
	if (!this->Tracking)
	{
		LOG_WARNING("Called Update() when SavedDataTracker was not tracking");
		return;
	}

	const double elapsedTime = vtkAccurateTimer::GetSystemTime() - this->GetStartTimestamp(); 
	const double localStartTime = this->LocalTrackerBuffer->GetTimeStamp(LocalTrackerBuffer->GetNumberOfItems() - 1); 

	// Get buffer index 
	const int bufferIndex = this->LocalTrackerBuffer->GetIndexFromTime(localStartTime + elapsedTime); 

	// Get frame number 
	int frameNumber = this->LocalTrackerBuffer->GetFrameNumber(bufferIndex); 

	// Get default transfom
	vtkSmartPointer<vtkMatrix4x4> defaultTransMatrix = vtkSmartPointer<vtkMatrix4x4>::New(); 
	this->LocalTrackerBuffer->GetMatrix(defaultTransMatrix, bufferIndex); 

	// Get flags
	long flags = this->LocalTrackerBuffer->GetFlags(bufferIndex); 

	// Create timestamp 
	double unfilteredtimestamp(0), filteredtimestamp(0); 
	this->Timer->GetTimeStampForFrame(frameNumber, unfilteredtimestamp, filteredtimestamp);

	// send the transformation matrix and flags to the tool
	this->ToolUpdate(0, defaultTransMatrix, flags, frameNumber, unfilteredtimestamp, filteredtimestamp);   

	// Replay the buffer after we reached the most recent element if desired
	if ( bufferIndex == 0  && this->ReplayEnabled)
	{
		this->SetStartTimestamp(vtkAccurateTimer::GetSystemTime() + 1.0/this->GetFrequency() ); 
	}
}

//----------------------------------------------------------------------------
bool vtkSavedDataTracker::InitSavedDataTracker()
{
	LOG_TRACE("vtkSavedDataTracker::InitSavedDataTracker"); 
	// Connect to device 
	 return this->Connect(); 
}

//----------------------------------------------------------------------------
void vtkSavedDataTracker::ReadConfiguration(vtkXMLDataElement* config)
{
	LOG_TRACE("vtkSavedDataTracker::ReadConfiguration"); 
	if ( config == NULL ) 
	{
		LOG_WARNING("Unable to find SavedDataset XML data element");
		return; 
	}
	
	Superclass::ReadConfiguration(config); 

	const char* sequenceMetafile = config->GetAttribute("SequenceMetafile"); 
	if ( sequenceMetafile != NULL ) 
	{
		this->SetSequenceMetafile(sequenceMetafile);
	}

	const char* replayEnabled = config->GetAttribute("ReplayEnabled"); 
	if ( replayEnabled != NULL ) 
	{
		if ( STRCASECMP("TRUE", replayEnabled ) == 0 )
		{
			this->ReplayEnabled = true; 
		}
		else if ( STRCASECMP("FALSE", replayEnabled ) == 0 )
		{
			this->ReplayEnabled = false; 
		}
		else
		{
			LOG_WARNING("Unable to recognize ReplayEnabled attribute: " << replayEnabled << " - changed to false by default!"); 
			this->ReplayEnabled = false; 
		}
	}
}

//----------------------------------------------------------------------------
void vtkSavedDataTracker::WriteConfiguration(vtkXMLDataElement* config)
{
	LOG_TRACE("vtkSavedDataTracker::WriteConfiguration"); 
	if ( config == NULL )
	{
		config = vtkXMLDataElement::New(); 
	}

	config->SetName("SavedDataset");  

	config->SetAttribute( "SequenceMetafile", this->GetSequenceMetafile() ); 
}
