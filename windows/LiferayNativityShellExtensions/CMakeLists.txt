cmake_minimum_required (VERSION 3.20)

project (liferay_win)

IF(CMAKE_SYSTEM_NAME MATCHES "Windows")
 
  include_directories(LiferayNativityUtil)
  add_subdirectory(windows)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "Linux")
  add_definitions(-DOS_LINUX)
  add_subdirectory(linux)
ELSEIF(CMAKE_SYSTEM_NAME MATCHES "CYGWIN")
ENDIF(CMAKE_SYSTEM_NAME MATCHES "Windows")
