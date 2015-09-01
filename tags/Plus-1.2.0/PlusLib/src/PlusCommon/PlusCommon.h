#ifndef __PlusCommon_h
#define __PlusCommon_h

#include "vtkOutputWindow.h"

enum PlusStatus
{   
  PLUS_FAIL=0,
  PLUS_SUCCESS=1
};

enum TrackerStatus {
  TR_OK,			      // Tool OK
  TR_MISSING,       // tool or tool port is not available
  TR_OUT_OF_VIEW,   // cannot obtain transform for tool
  TR_OUT_OF_VOLUME, // tool is not within the sweet spot of system
  TR_SWITCH1_IS_ON, // various buttons/switches on tool
  TR_SWITCH2_IS_ON,
  TR_SWITCH3_IS_ON, 
  TR_REQ_TIMEOUT   // Request timeout
};

/* Define case insensitive string compare for Windows. */
#if defined( _WIN32 ) && !defined(__CYGWIN__)
#  if defined(__BORLANDC__)
#    define STRCASECMP stricmp
#  else
#    define STRCASECMP _stricmp
#  endif
#else
#  define STRCASECMP STRCASECMP
#endif

#define ROUND(x) (static_cast<int>(floor( x + 0.5 )))

///////////////////////////////////////////////////////////////////
// Logging

#define LOG_ERROR(msg) \
	{ \
	std::ostrstream msgStream; \
  msgStream << " " << msg << std::ends; \
	PlusLogger::Instance()->LogMessage(PlusLogger::LOG_LEVEL_ERROR, msgStream.str(), __FILE__, __LINE__); \
	msgStream.rdbuf()->freeze(); \
	}	

#define LOG_WARNING(msg) \
	{ \
	std::ostrstream msgStream; \
	msgStream << " " << msg << std::ends; \
  PlusLogger::Instance()->LogMessage(PlusLogger::LOG_LEVEL_WARNING, msgStream.str(), __FILE__, __LINE__); \
  msgStream.rdbuf()->freeze(); \
	}
		
#define LOG_INFO(msg) \
	{ \
	std::ostrstream msgStream; \
	msgStream << " " << msg << std::ends; \
	PlusLogger::Instance()->LogMessage(PlusLogger::LOG_LEVEL_INFO, msgStream.str(), __FILE__, __LINE__); \
	msgStream.rdbuf()->freeze(); \
	}
	
#define LOG_DEBUG(msg) \
	{ \
	std::ostrstream msgStream; \
	msgStream << " " << msg << std::ends; \
	PlusLogger::Instance()->LogMessage(PlusLogger::LOG_LEVEL_DEBUG, msgStream.str(), __FILE__, __LINE__); \
	msgStream.rdbuf()->freeze(); \
	}	
	
#define LOG_TRACE(msg) \
	{ \
	std::ostrstream msgStream; \
	msgStream << " " << msg << std::ends; \
	PlusLogger::Instance()->LogMessage(PlusLogger::LOG_LEVEL_TRACE, msgStream.str(), __FILE__, __LINE__); \
	msgStream.rdbuf()->freeze(); \
	}	
	
class vtkConsoleOutputWindow : public vtkOutputWindow 
{ 
public: 
	static vtkConsoleOutputWindow* New() 
	{ return new vtkConsoleOutputWindow; } 
};

#define VTK_LOG_TO_CONSOLE_ON \
	{  \
	vtkSmartPointer<vtkConsoleOutputWindow> console = vtkSmartPointer<vtkConsoleOutputWindow>::New();  \
	vtkOutputWindow::SetInstance(console); \
	}

#define VTK_LOG_TO_CONSOLE_OFF \
	{  \
	vtkOutputWindow::SetInstance(NULL); \
	}	

/////////////////////////////////////////////////////////////////// 

// This class is used for locking a objects (buffers, mutexes, etc.)
// and releasing the lock automatically when the object is deleted
template <typename T> class PlusLockGuard
{
public:
  PlusLockGuard(T* lockableObject)
  {
    m_LockableObject=lockableObject;
    m_LockableObject->Lock();
  }
  ~PlusLockGuard()
  {    
    m_LockableObject->Unlock();
    m_LockableObject=NULL;
  }
private:
  PlusLockGuard(PlusLockGuard&);
  void operator=(PlusLockGuard&);

  T* m_LockableObject;
};



#endif //__PlusCommon_h