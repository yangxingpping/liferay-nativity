cmake_minimum_required (VERSION 3.20)

project (liferay)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
  include(GenerateExportHeader)
  set(CMAKE_CXX_STANDARD 17)
  set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR})
  set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
  add_definitions(-DOS_WIN -DUNICODE -D_UNICODE)
  add_subdirectory(windows/LiferayNativityShellExtensions/LiferayNativityOverlays)
  add_subdirectory(windows/LiferayNativityShellExtensions/LiferayNativityUtil)
  add_subdirectory(windows/LiferayNativityShellExtensions/LiferayNativityContextMenus)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_definitions(-DOS_LINUX)
  #add_subdirectory(linux)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "CYGWIN")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")
