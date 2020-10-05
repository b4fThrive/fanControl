add_library(utils utils.cpp)
add_library(menu menu.cpp)

target_include_directories(utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(menu INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS utils menu DESTINATION lib)
install(FILES utils.h menu.h DESTINATION include)
