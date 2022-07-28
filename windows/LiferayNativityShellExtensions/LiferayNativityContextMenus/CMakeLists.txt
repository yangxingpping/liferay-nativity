PROJECT(LiferayNativityContextMenus)


include_directories(. ../LiferayNativityUtil)


AUX_SOURCE_DIRECTORY(. CC)

FILE(GLOB_RECURSE MY_HEADERS2 "./*.h")
source_group("Headers" FILES ${MY_HEADERS2})

ADD_LIBRARY(LiferayNativityContextMenus SHARED  ${CC} ContextMenuDef.def)

target_link_libraries(LiferayNativityContextMenus PUBLIC LiferayNativityUtil)

