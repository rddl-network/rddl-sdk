file(GLOB TEST_LIB_SOURCES_C "*.c")

#add_subdirectory(Unity)

add_custom_target(checkTest
    COMMENT "Creating Coverage HTML and XML summary"
    COMMAND ctest -V -T Test --no-compress-output || true 
    COMMAND gcovr -k  --branches  -r  ${CMAKE_SOURCE_DIR}
              --xml-pretty -o ${CMAKE_BINARY_DIR}/coverage.xml
    COMMAND gcovr -k  --branches  -r  ${CMAKE_SOURCE_DIR}
              --html --html-details -o ${CMAKE_BINARY_DIR}/coverage.html
    )

macro(do_test_c test_name sources)
    add_executable(${test_name}
        ${sources}
    )

    target_include_directories(${test_name} PUBLIC 
        .
        ../libs/proto
        /usr/include
    )

    target_link_libraries(${test_name}
        rddlSDK
        Unity
    )

    target_compile_options(${test_name} PRIVATE "$<$<CONFIG:Debug>:-g>")
    target_link_options(${test_name} PRIVATE "$<$<CONFIG:Debug>:-g>")

    set(CMAKE_CXX_FLAGS "-fpermissive")
    target_compile_features(${test_name} PRIVATE
        c_std_99
    )
    add_test(${test_name} ${test_name})
endmacro()

foreach(arg1  ${TEST_LIB_SOURCES_C} )
    get_filename_component(name ${arg1} NAME_WE)
    get_filename_component(file ${arg1} NAME)
    do_test_c(${name} ${file})
endforeach(arg1)