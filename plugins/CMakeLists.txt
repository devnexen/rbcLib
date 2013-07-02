include_directories(..)
add_library(diagtool SHARED diagtool.cpp)
set_target_properties(diagtool PROPERTIES PREFIX "")
target_link_libraries(diagtool ${RBC_LIBRARY} ${Boost_LIBRARIES})
