cmake_minimum_required(VERSION 3.22.1)

# ── Extract version from the .jucer file ──────────────────────────────────────
# The .jucer XML has an attribute like: version="1.0.8"
# We parse it here so the version is the single source of truth.
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/Mofo Filter.jucer" JUCER_CONTENT)
string(REGEX MATCH "version=\"([0-9]+\\.[0-9]+\\.[0-9]+)\"" _version_match "${JUCER_CONTENT}")
if(NOT CMAKE_MATCH_1)
    message(FATAL_ERROR "Could not extract version from 'Mofo Filter.jucer'. "
            "Make sure the file contains a version=\"X.Y.Z\" attribute.")
endif()
set(CURRENT_VERSION "${CMAKE_MATCH_1}")
message(STATUS "Plugin version: ${CURRENT_VERSION}")

project(ViatorJuceCMake VERSION ${CURRENT_VERSION})

# ── Platform conditionals ──────────────────────────────────────────────────────
if(NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    include_directories(${GTK3_INCLUDE_DIRS})
    link_directories(${GTK3_LIBRARY_DIRS})
    add_definitions(${GTK3_CFLAGS_OTHER})
endif()

set(CMAKE_CXX_STANDARD 17)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment target")
    # Default to universal build; override on the command line if needed.
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

if(UNIX AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    pkg_check_modules(WEBKIT2 REQUIRED webkit2gtk-4.0)
    include_directories(${WEBKIT2_INCLUDE_DIRS})
    link_directories(${WEBKIT2_LIBRARY_DIRS})
    add_definitions(${WEBKIT2_CFLAGS_OTHER})
endif()

# ── General setup ──────────────────────────────────────────────────────────────
set(PROJECT_NAME      "ViatorJuceCMake")
set(PRODUCT_NAME      "The Mofo's Mojo v1")
set(COMPANY_NAME      "Altered Brain Chemistry")
set(BUNDLE_ID         "com.alteredbrainchemistry.MofoMojoFilter")

if(APPLE)
    set(FORMATS Standalone AU VST3)
else()
    set(FORMATS Standalone VST3)
endif()

add_definitions(-w)

# ── JUCE 7.0.7 (pinned) ───────────────────────────────────────────────────────
add_subdirectory(modules/JUCE-7.0.7)

# ── Rust static library ───────────────────────────────────────────────────────
# RUST_LIBRARY_PATH may be passed in by the CI via -DRUST_LIBRARY_PATH=...
# If not provided, fall back to sensible per-platform defaults (useful for
# local developer builds where the library has already been compiled).
if(DEFINED RUST_LIBRARY_PATH AND EXISTS "${RUST_LIBRARY_PATH}")
    set(RUST_LIBRARY "${RUST_LIBRARY_PATH}")
    message(STATUS "Using Rust library from: ${RUST_LIBRARY}")
else()
    # Fallback search paths for local builds
    if(WIN32)
        set(_rust_search_paths
            "${CMAKE_SOURCE_DIR}/../../software-licensor-client-side-library/software_licensor_static_rust_lib/target/x86_64-pc-windows-msvc/release"
            "${CMAKE_SOURCE_DIR}/../software-licensor-client-side-library/software_licensor_static_rust_lib/target/x86_64-pc-windows-msvc/release"
        )
        set(_rust_names softwarelicensor)
    elseif(APPLE)
        set(_rust_search_paths
            "${CMAKE_SOURCE_DIR}/../../software-licensor-client-side-library/software_licensor_static_rust_lib/target/universal"
            "${CMAKE_SOURCE_DIR}/../software-licensor-client-side-library/software_licensor_static_rust_lib/target/universal"
        )
        set(_rust_names softwarelicensor)
    else()
        set(_rust_search_paths
            "${CMAKE_SOURCE_DIR}/../../software-licensor-client-side-library/software_licensor_static_rust_lib/target/release"
            "${CMAKE_SOURCE_DIR}/../software-licensor-client-side-library/software_licensor_static_rust_lib/target/release"
        )
        set(_rust_names softwarelicensor)
    endif()

    find_library(RUST_LIBRARY
        NAMES ${_rust_names}
        PATHS ${_rust_search_paths}
        NO_DEFAULT_PATH
    )

    if(NOT RUST_LIBRARY)
        message(FATAL_ERROR
            "Rust static library not found.\n"
            "Pass -DRUST_LIBRARY_PATH=<absolute path to .a/.lib> to cmake,\n"
            "or build the library first with 'cargo build --release --features logging'.")
    endif()
    message(STATUS "Found Rust library (fallback): ${RUST_LIBRARY}")
endif()

# ── Plugin target ─────────────────────────────────────────────────────────────
juce_add_plugin("${PROJECT_NAME}"
        COMPANY_NAME          "${COMPANY_NAME}"
        BUNDLE_ID             "${BUNDLE_ID}"
        COPY_PLUGIN_AFTER_BUILD TRUE
        PLUGIN_MANUFACTURER_CODE Abch
        PLUGIN_CODE           Mojo
        FORMATS               ${FORMATS}
        PRODUCT_NAME          "${PRODUCT_NAME}"
        NEEDS_MIDI_INPUT      FALSE
        NEEDS_MIDI_OUTPUT     FALSE
        IS_MIDI_EFFECT        FALSE
        IS_SYNTH              FALSE
)

juce_add_binary_data(Assets SOURCES
        unlockedUI.png
)

juce_generate_juce_header("${PROJECT_NAME}")

add_compile_definitions(JUCE_PROJECT_NAME="${PROJECT_NAME}")

# ── SharedCode interface library ──────────────────────────────────────────────
add_library(SharedCode INTERFACE)
target_compile_features(SharedCode INTERFACE cxx_std_20)
target_include_directories(SharedCode INTERFACE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/JUCE-7.0.7/modules"
)
target_compile_definitions(SharedCode INTERFACE
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1
        JUCE_WEB_BROWSER=1
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        # VERSION is now derived from the .jucer file above
        VERSION="${CURRENT_VERSION}"
        JUCE_DISPLAY_SPLASH_SCREEN=0
        PRODUCT_NAME_WITHOUT_VERSION="TheMofosMojo"
)

# ── Sources ───────────────────────────────────────────────────────────────────
file(GLOB_RECURSE SourceFiles CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
)
target_sources("${PROJECT_NAME}" PRIVATE ${SourceFiles})

# ── Link libraries ────────────────────────────────────────────────────────────
# Platform-specific system libraries needed by the Rust static lib
set(PLATFORM_LIBS "")
if(WIN32)
    list(APPEND PLATFORM_LIBS Userenv Ntdll Bcrypt ws2_32 userenv)
elseif(APPLE)
    find_library(SECURITY_FW  Security)
    find_library(COREFOUND_FW CoreFoundation)
    list(APPEND PLATFORM_LIBS "${SECURITY_FW}" "${COREFOUND_FW}")
endif()

target_link_libraries("${PROJECT_NAME}"
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_plugin_client
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_product_unlocking
        SharedCode
        Assets
        "${RUST_LIBRARY}"
        ${PLATFORM_LIBS}
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

target_include_directories("${PROJECT_NAME}" PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)