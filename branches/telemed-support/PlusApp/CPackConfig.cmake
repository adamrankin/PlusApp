include(InstallRequiredSystemLibraries)

SET(CPACK_GENERATOR "NSIS;ZIP")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Plus (Public software Library for UltraSound) for ${PLUSLIB_PLATFORM}")
SET(CPACK_PACKAGE_VENDOR "PerkLab, Queen's University")
# SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${PLUSLIB_SOURCE_DIR}/src/License.txt")
SET(CPACK_PACKAGE_VERSION_MAJOR ${PLUSAPP_VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${PLUSAPP_VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${PLUSAPP_VERSION_PATCH})
SET(CPACK_NSIS_PACKAGE_NAME "Plus Applications ${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}.${PLUSAPP_REVISION} (${PLUSLIB_PLATFORM})" )
SET(CPACK_PACKAGE_FILE_NAME "PlusApp-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}.${PLUSAPP_REVISION}-${PLUSLIB_PLATFORM}" )
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_FILE_NAME}")

SET(CPACK_INSTALL_CMAKE_PROJECTS "${PlusApp_BINARY_DIR};PlusApp;ALL;/")

# Install into c:\Users\<username>\PlusApp_...
SET(CPACK_NSIS_INSTALL_ROOT "$PROFILE")

# Do not ask for admin access rights (no UAC dialog), to allow installation without admin access
SET(CPACK_NSIS_DEFINES ${CPACK_NSIS_DEFINES} "RequestExecutionLevel user")

SET(CPACK_PACKAGE_EXECUTABLES 
  "PlusServerLauncher" "Plus server"
  "fCal" "Free-hand calibration (fCal)"
  )

# Windows users may not be familiar how to open a command prompt, so create a shortcut for that
IF(WIN32)
  SET(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Plus command prompt.lnk\\\" \\\"$INSTDIR\\\\bin\\\\StartPlusCommandPrompt.bat\\\" \\\"$INSTDIR\\\\bin\\\\StartPlusCommandPrompt.bat\\\" \\\"$INSTDIR\\\\bin\\\\StartPlusCommandPrompt.ico\\\"
    ")
  SET(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
    Delete \\\"$SMPROGRAMS\\\\$MUI_TEMP\\\\Plus command prompt.lnk\\\"
    ")
ENDIF(WIN32)

IF(EXISTS "${PLUSLIB_DIR}/CMakeCache.txt")
  SET(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${PLUSLIB_DIR};PlusLib;RuntimeExecutables;/")
  SET(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${PLUSLIB_DIR};PlusLib;RuntimeLibraries;/")
  SET(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${PLUSLIB_DIR};PlusLib;Scripts;/")
ELSE(EXISTS "${PLUSLIB_DIR}/CMakeCache.txt")
    MESSAGE(WARNING "Unable to set PLUSLIB_DIR for package generation!")
ENDIF(EXISTS "${PLUSLIB_DIR}/CMakeCache.txt")
  
IF(EXISTS "${VTK_DIR}/CMakeCache.txt")
  SET(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${VTK_DIR};VTK;RuntimeLibraries;/")
ELSE(EXISTS "${VTK_DIR}/CMakeCache.txt")
  MESSAGE(WARNING "Unable to set VTK_DIR for package generation!")
ENDIF(EXISTS "${VTK_DIR}/CMakeCache.txt")

IF(EXISTS "${ITK_DIR}/CMakeCache.txt")
	SET(ITK_USE_REVIEW OFF)
	SET(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${ITK_DIR};ITK;RuntimeLibraries;/") 
ELSE(EXISTS "${ITK_DIR}/CMakeCache.txt")
  MESSAGE(WARNING "Unable to set ITK_DIR for package generation!")
ENDIF(EXISTS "${ITK_DIR}/CMakeCache.txt")

IF ( PLUS_USE_OpenIGTLink )
	IF(EXISTS "${OpenIGTLink_DIR}/CMakeCache.txt")
		SET(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${OpenIGTLink_DIR};OpenIGTLink;RuntimeLibraries;/")
	ELSE(EXISTS "${OpenIGTLink_DIR}/CMakeCache.txt")
		MESSAGE(WARNING "Unable to set OpenIGTLink_DIR for package generation!")
	ENDIF(EXISTS "${OpenIGTLink_DIR}/CMakeCache.txt")
ENDIF (PLUS_USE_OpenIGTLink)

#-----------------------------------------------------------------------------
# Installation vars.
# PLUSAPP_INSTALL_BIN_DIR          - binary dir (executables)
# PLUSAPP_INSTALL_LIB_DIR          - library dir (libs)
# PLUSAPP_INSTALL_DATA_DIR         - share dir (say, examples, data, etc)
# PLUSAPP_INSTALL_CONFIG_DIR       - config dir (configuration files)
# PLUSAPP_INSTALL_GNUPLOT_DIR      - gnuplot dir (gnuplot binary files)
# PLUSAPP_INSTALL_SCRIPTS_DIR      - scripts dir (gnuplot script files)
# PLUSAPP_INSTALL_INCLUDE_DIR      - include dir (headers)
# PLUSAPP_INSTALL_PACKAGE_DIR      - package/export configuration files
# PLUSAPP_INSTALL_NO_DEVELOPMENT   - do not install development files
# PLUSAPP_INSTALL_NO_RUNTIME       - do not install runtime files
# PLUSAPP_INSTALL_NO_DOCUMENTATION - do not install documentation files
# Applications
# RuntimeLibraries
# Development

IF(NOT PLUSAPP_INSTALL_BIN_DIR)
  SET(PLUSAPP_INSTALL_BIN_DIR "bin")
ENDIF(NOT PLUSAPP_INSTALL_BIN_DIR)

IF(NOT PLUSAPP_INSTALL_LIB_DIR)
  SET(PLUSAPP_INSTALL_LIB_DIR "lib")
ENDIF(NOT PLUSAPP_INSTALL_LIB_DIR)

IF(NOT PLUSAPP_INSTALL_DATA_DIR)
  SET(PLUSAPP_INSTALL_DATA_DIR "data")
ENDIF(NOT PLUSAPP_INSTALL_DATA_DIR)

IF(NOT PLUSAPP_INSTALL_CONFIG_DIR)
  SET(PLUSAPP_INSTALL_CONFIG_DIR "config")
ENDIF(NOT PLUSAPP_INSTALL_CONFIG_DIR)

IF(NOT PLUSAPP_INSTALL_GNUPLOT_DIR)
  SET(PLUSAPP_INSTALL_GNUPLOT_DIR "gnuplot")
ENDIF(NOT PLUSAPP_INSTALL_GNUPLOT_DIR)

IF(NOT PLUSAPP_INSTALL_SCRIPTS_DIR)
  SET(PLUSAPP_INSTALL_SCRIPTS_DIR "scripts")
ENDIF(NOT PLUSAPP_INSTALL_SCRIPTS_DIR)

IF(NOT PLUSAPP_INSTALL_INCLUDE_DIR)
  SET(PLUSAPP_INSTALL_INCLUDE_DIR "include")
ENDIF(NOT PLUSAPP_INSTALL_INCLUDE_DIR)

IF(NOT PLUSAPP_INSTALL_NO_DOCUMENTATION)
  SET(PLUSAPP_INSTALL_NO_DOCUMENTATION 0)
ENDIF(NOT PLUSAPP_INSTALL_NO_DOCUMENTATION)

INCLUDE(CPack)