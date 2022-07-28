PROJECT(LiferayNativityOverlays)


include_directories(. ../LiferayNativityUtil)


AUX_SOURCE_DIRECTORY(. CC)



FILE(GLOB_RECURSE MY_HEADERS2 "./*.h")
source_group("Headers" FILES ${MY_HEADERS2})

ADD_LIBRARY(LiferayNativityOverlays SHARED  ${CC} LiferayNativityOverlays.def)

target_link_libraries(LiferayNativityOverlays PUBLIC LiferayNativityUtil)

target_sources(LiferayNativityOverlays PRIVATE NativityOverlay.rc)

