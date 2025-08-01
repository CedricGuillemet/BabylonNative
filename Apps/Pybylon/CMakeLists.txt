FetchContent_MakeAvailable_With_Message(pybind11)

find_package(Python3 3.11 REQUIRED COMPONENTS Interpreter Development)

set(MAIN "Shared/main.cpp")
set(BABYLON_SCRIPTS
    "../node_modules/babylonjs-loaders/babylonjs.loaders.js"
    "../node_modules/babylonjs/babylon.max.js"
    "../node_modules/babylonjs-materials/babylonjs.materials.js"
    "../node_modules/babylonjs-gui/babylon.gui.js")

set(SCRIPTS "Scripts/main.js")

if(APPLE)
    find_library(JAVASCRIPTCORE_LIBRARY JavaScriptCore)
    
    set(PLIST_FILE "${CMAKE_CURRENT_LIST_DIR}/macOS/Info.plist")
    set(STORYBOARD "${CMAKE_CURRENT_LIST_DIR}/macOS/Base.lproj/Main.storyboard")
    set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} PRIVATE NativeCamera)
    set(RESOURCE_FILES ${STORYBOARD})
    set(SOURCES
        ${SOURCES}
        "macOS/main.m"
        "macOS/AppDelegate.m"
        "macOS/AppDelegate.h"
        "macOS/ViewController.mm"
        "macOS/ViewController.h")
    set_source_files_properties(${SCRIPTS} ${BABYLON_SCRIPTS} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/Scripts")
    set(ADDITIONAL_LIBRARIES ${ADDITIONAL_LIBRARIES} PRIVATE ${JAVASCRIPTCORE_LIBRARY})
    set(RESOURCE_FILES ${STORYBOARD})

    # Remove -Wall and -Werror from the debug flags to make simulators work.
    string(REPLACE "-Wall" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "-Werror" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
elseif(UNIX)
    set(SOURCES
        ${SOURCES}
        "X11/App.cpp")
elseif(WIN32)
    set(SOURCES
        ${SOURCES}
        "Win32/window.cpp"
        )
endif()

pybind11_add_module(Pybylon ${BABYLON_SCRIPTS} ${SCRIPTS} ${SOURCES} ${RESOURCE_FILES} ${MAIN})

warnings_as_errors(Pybylon)
target_compile_definitions(Pybylon PRIVATE UNICODE)
target_compile_definitions(Pybylon PRIVATE _UNICODE)

target_include_directories(Pybylon PRIVATE "Source" ".")

target_link_libraries(Pybylon
    PRIVATE AppRuntime
    PRIVATE Canvas
    PRIVATE Console
    PRIVATE ExternalTexture
    PRIVATE GraphicsDevice
    PRIVATE NativeCapture
    PRIVATE NativeEngine
    PRIVATE NativeInput
    PRIVATE NativeOptimizations
    PRIVATE ScriptLoader
    PRIVATE Window
    PRIVATE XMLHttpRequest
    PRIVATE TestUtils
    ${ADDITIONAL_LIBRARIES}
    ${BABYLON_NATIVE_PLAYGROUND_EXTENSION_LIBRARIES})

# See https://gitlab.kitware.com/cmake/cmake/-/issues/23543
# If we can set minimum required to 3.26+, then we can use the `copy -t` syntax instead.
add_custom_command(TARGET Pybylon POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E $<IF:$<BOOL:$<TARGET_RUNTIME_DLLS:Pybylon>>,copy,true> $<TARGET_RUNTIME_DLLS:Pybylon> $<TARGET_FILE_DIR:Pybylon> COMMAND_EXPAND_LISTS)

if(APPLE)
    target_link_libraries(Pybylon PUBLIC "-framework MetalKit")

    set_target_properties(Pybylon PROPERTIES
        MACOSX_BUNDLE true
        MACOSX_BUNDLE_INFO_PLIST "${PLIST_FILE}"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
        RESOURCE "${RESOURCE_FILES}"
        FOLDER "Pybylon")
    set_property(TARGET Pybylon PROPERTY UNITY_BUILD false)
    set_property(TARGET Pybylon PROPERTY XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED YES)
endif()

foreach(SCRIPT ${SCRIPTS} ${BABYLON_SCRIPTS})
    get_filename_component(SCRIPT_NAME "${SCRIPT}" NAME)
    add_custom_command(
        OUTPUT "${CMAKE_CFG_INTDIR}/Scripts/${SCRIPT_NAME}"
        COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT}" "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/Scripts/${SCRIPT_NAME}"
        COMMENT "Copying ${SCRIPT_NAME}"
        MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT}")
endforeach()

set_property(TARGET Pybylon PROPERTY FOLDER Apps)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../node_modules PREFIX Scripts FILES ${BABYLON_SCRIPTS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SCRIPTS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCES})
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Pybylon)

set_property(TARGET Pybylon PROPERTY XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED YES)
