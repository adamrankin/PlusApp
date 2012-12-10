/*=Plus=header=begin======================================================
Program: Plus
Copyright (c) Laboratory for Percutaneous Surgery. All rights reserved.
See License.txt for details.
=========================================================Plus=header=end*/

#ifndef __vtkTracker_h
#define __vtkTracker_h

#include "PlusConfigure.h"
#include "vtkPlusDevice.h"
#include "vtkRecursiveCriticalSection.h"
#include "vtkXMLDataElement.h"
#include "TrackedFrame.h"
#include "vtkMultiThreader.h"

#include <string>
#include <map>

class vtkMatrix4x4;
class vtkTrackerTool;
class vtkXMLDataElement;
class vtkHTMLGenerator; 
class vtkGnuplotExecuter;
class vtkPlusDataBuffer; 

typedef std::map<std::string, vtkTrackerTool*> ToolContainerType;
typedef ToolContainerType::const_iterator ToolIteratorType; 

/*! Flags for tool LEDs (specifically for the POLARIS) */
enum {
  TR_LED_OFF   = 0,
  TR_LED_ON    = 1,
  TR_LED_FLASH = 2
};

/*!
\class vtkTracker 
\brief Generic interfaces with real-time 3D tracking systems

The vtkTracker is a generic VTK interface to real-time tracking
systems.  Derived classes should override the Connect(), Disconnect(), 
Probe(), InternalUpdate(), InternalStartTracking(), and InternalStopTracking() methods.
The InternalUpdate() method is called from within a separate
thread, therefore its contents must be thread safe.  Use the
vtkBrachyTracker or vtkNDICertusTracker as a framework for developing subclasses
for new tracking systems.

\ingroup PlusLibTracking
*/
class VTK_EXPORT vtkTracker : public vtkPlusDevice
{
public:
  static vtkTracker *New();
  vtkTypeMacro(vtkTracker,vtkPlusDevice);
  void PrintSelf(ostream& os, vtkIndent indent);

  /*! 
  Probe to see to see if the tracking system is connected to the 
  computer.  This method should be overridden in subclasses. 
  */
  virtual PlusStatus Probe();

  /*! 
  Start the tracking system. The tracking system is brought from
  its ground state (i.e. on but not necessarily initialized) into
  full tracking mode.  This method calls InternalStartTracking()
  after doing a bit of housekeeping.
  */
  virtual PlusStatus StartTracking();

  /*! Stop the tracking system and bring it back to its ground state. This method calls InternalStopTracking(). */
  virtual PlusStatus StopTracking();

  /*! Test whether or not the system is tracking. */
  virtual int IsTracking() { return this->Recording; };

  /*! Set recording start time for each tool */
  virtual void SetStartTime( double startTime ); 

  /*! Get recording start time */
  virtual double GetStartTime();
  
  /*! Read main configuration from xml data */
  virtual PlusStatus ReadConfiguration(vtkXMLDataElement* config); 

  /*! Write main configuration to xml data */
  virtual PlusStatus WriteConfiguration(vtkXMLDataElement* config); 

  /*! Convert tool status to string */
  static std::string ConvertToolStatusToString(ToolStatus status); 

  /*! Convert tool status to TrackedFrameFieldStatus */
  static TrackedFrameFieldStatus ConvertToolStatusToTrackedFrameFieldStatus(ToolStatus status); 

  /*! Convert TrackedFrameFieldStatus to tool status */
  static ToolStatus ConvertTrackedFrameFieldStatusToToolStatus(TrackedFrameFieldStatus fieldStatus); 

  /*! Get tracked frame containing all transforms from buffer element values of each tool by timestamp */
  virtual PlusStatus GetTrackedFrame(double timestamp, TrackedFrame *trackedFrame);

  /*! Add generated html report from tracking data acquisition to the existing html report. htmlReport and plotter arguments has to be defined by the caller function */
  virtual PlusStatus GenerateTrackingDataAcquisitionReport( vtkHTMLGenerator* htmlReport, vtkGnuplotExecuter* plotter ); 

  /*! Get the internal update rate for this tracking system.  This is the number of transformations sent by the tracking system per second per tool. */
  double GetInternalUpdateRate() { return this->InternalUpdateRate; };

  /*! Get the tool object for the specified tool name */
  PlusStatus GetTool(const char* aToolName, vtkTrackerTool* &aTool);
  
  /*! Get the first active tool object */
  PlusStatus GetFirstActiveTool(vtkTrackerTool* &aTool); 

  /*! Get the tool object for the specified tool port name */
  PlusStatus GetToolByPortName( const char* aPortName, vtkTrackerTool* &aTool); 

  /*! Get the begining of the tool iterator */
  ToolIteratorType GetToolIteratorBegin(); 
  
  /*! Get the end of the tool iterator */
  ToolIteratorType GetToolIteratorEnd(); 

  /*! Add tool to the tracker */
  PlusStatus AddTool( vtkTrackerTool* tool ); 

  /*! Get number of tools */
  int GetNumberOfTools();

  /*! Set Reference name of the tools */
	vtkSetStringMacro(ToolReferenceFrameName);

  /*! Get Reference name of the tools */
	vtkGetStringMacro(ToolReferenceFrameName);

  /*! Set buffer size of all available tools */
  void SetToolsBufferSize( int aBufferSize ); 

  /*! Set local time offset of all available tools */
  void SetToolsLocalTimeOffsetSec( double aLocalTimeOffsetSec ); 

  /*! Make the unit emit a string of audible beeps.  This is supported by the POLARIS. */
  void Beep(int n);

  /*! Turn one of the LEDs on the specified tool on or off.  This is supported by the POLARIS. */
  void SetToolLED(const char* portName, int led, int state);

  /*! 
  The subclass will do all the hardware-specific update stuff
  in this function. It should call ToolUpdate() for each tool.
  Note that vtkTracker.cxx starts up a separate thread after
  InternalStartTracking() is called, and that InternalUpdate() is
  called repeatedly from within that thread.  Therefore, any code
  within InternalUpdate() must be thread safe.  You can temporarily
  pause the thread by locking this->UpdateMutex->Lock() e.g. if you
  need to communicate with the device from outside of InternalUpdate().
  A call to this->UpdateMutex->Unlock() will resume the thread.
  */
  virtual PlusStatus InternalUpdate() { return PLUS_SUCCESS; };

  //BTX
  // These are used by static functions in vtkTracker.cxx, and since
  // VTK doesn't generally use 'friend' functions they are public
  // instead of protected.  Do not use them anywhere except inside
  // vtkTracker.cxx.
  vtkRecursiveCriticalSection *UpdateMutex;
  vtkTimeStamp UpdateTime;
  double InternalUpdateRate;  
  //ETX

  /*! Connects to device. Derived classes should override this */
  virtual PlusStatus Connect();

  /*! Disconnects from device. Derived classes should override this */
  virtual PlusStatus Disconnect();

  /*! Make this tracker into a copy of another tracker. */
  void DeepCopy(vtkTracker *tracker);

  /*! Clear all tool buffers */
  void ClearAllBuffers();

  /*! Copy the current state of the tracker buffer  */
  virtual PlusStatus CopyBuffer( vtkPlusDataBuffer* trackerBuffer, const char* aToolName);

  /*! Dump the current state of the tracker to metafile (with each tools and buffers) */
  virtual PlusStatus WriteToMetafile(const char* outputFolder, const char* metaFileName, bool useCompression = false );

public:
  /*! Set the acquisition rate */
  vtkSetMacro(AcquisitionRate, double);

protected:
  vtkTracker();
  virtual ~vtkTracker();

  /*! Tracking thread */
  static void* vtkTrackerThread(vtkMultiThreader::ThreadInfo *data); 

  /*! 
  This function is called by InternalUpdate() so that the subclasses
  can communicate information back to the vtkTracker base class, which
  will in turn relay the information to the appropriate vtkTrackerTool.
  */
  PlusStatus ToolTimeStampedUpdate(const char* aToolName, vtkMatrix4x4 *matrix, ToolStatus status, unsigned long frameNumber, double unfilteredtimestamp);

  /*! 
  This function is called by InternalUpdate() so that the subclasses
  can communicate information back to the vtkTracker base class, which
  will in turn relay the information to the appropriate vtkTrackerTool.
  This function is for devices has no frame numbering, just auto increment tool frame number if new frame received
  */
  PlusStatus ToolTimeStampedUpdateWithoutFiltering(const char* aToolName, vtkMatrix4x4 *matrix, ToolStatus status, double unfilteredtimestamp, double filteredtimestamp);

 /*! InternalStartTracking() initialize the tracking device, this methods should be overridden in derived classes */
  virtual PlusStatus InternalStartTracking() { return PLUS_SUCCESS; };

  /*! InternalStopTracking() free all resources associated with the device, this methods should be overridden in derived classes */
  virtual PlusStatus InternalStopTracking() { return PLUS_SUCCESS; };

  /*! This method should be overridden in derived classes that can make an audible beep. */
  virtual PlusStatus InternalBeep(int n) { return PLUS_SUCCESS; };

  /*! This method should be overridden for devices that have one or more LEDs on the tracked tools. */
  virtual PlusStatus InternalSetToolLED(const char* portName, int led, int state) { return PLUS_SUCCESS; };

protected:
  /*! Tracker tools */
  ToolContainerType ToolContainer; 

  /*! Flag to strore tracking thread state */
  bool TrackingThreadAlive; 

  /*! Reference name of the tools */
  char* ToolReferenceFrameName; 

private:
  vtkTracker(const vtkTracker&);  // Not implemented.
  void operator=(const vtkTracker&);  // Not implemented. 
};

#endif
