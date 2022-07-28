PROJECT(LiferayNativityUtil)


include_directories(.)


AUX_SOURCE_DIRECTORY(. CC)

link_libraries(Ws2_32)

FILE(GLOB_RECURSE MY_HEADERS2 "./*.h")
source_group("Headers" FILES ${MY_HEADERS2})

ADD_LIBRARY(LiferayNativityUtil SHARED  ${CC})



