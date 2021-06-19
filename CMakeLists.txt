cmake_minimum_required (VERSION 2.8)

project (cfg_test C)
include(CTest)
set(CMAKE_C_STANDARD 11)
file(GLOB HEADER_FILES  *.h)


set(SOURCE
   ${CMAKE_CURRENT_SOURCE_DIR}/unit_test/main.c
   ${CMAKE_CURRENT_SOURCE_DIR}/wwcfg.c
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Add source to this project's executable.
add_executable (${PROJECT_NAME}  ${SOURCE} ${HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "UNIT_TEST")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/FAC0000.tlv      DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/CF50000.tlv      DESTINATION ${CMAKE_BINARY_DIR})

add_test(NAME CFG_VERIFY_ALL            COMMAND $<TARGET_FILE:${PROJECT_NAME}>  "cfg_test_001" )
add_test(NAME CFG_MODIFY_SIMPLE         COMMAND $<TARGET_FILE:${PROJECT_NAME}>  "cfg_test_002" )


set_property(TEST CFG_VERIFY_ALL    PROPERTY TIMEOUT  10)
set_property(TEST CFG_MODIFY_SIMPLE PROPERTY TIMEOUT  10)



