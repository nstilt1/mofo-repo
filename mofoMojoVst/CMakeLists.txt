cmake_minimum_required(VERSION 3.23.1)

# ── Extract version from the .jucer file ──────────────────────────────────────
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/Mofo Filter.jucer" JUCER_CONTENT)
string(REGEX MATCH "version=\"([0-9]+\\.[0-9]+(\\.[0-9]+)?)\"" _version_match "${JUCER_CONTENT}")
if(NOT CMAKE_MATCH_1)
    message(FATAL_ERROR "Could not extract version from 'Mofo Filter.jucer'.")
endif()
set(CURRENT_VERSION "${CMAKE_MATCH_1}")
message(STATUS "Plugin version: ${CURRENT_VERSION}")

# ── General setup ──────────────────────────────────────────────────────────────
set(PROJECT_NAME  "HyperformanceSolutionsCMake")
set(PRODUCT_NAME  "The Mofo's Mojo v1")
set(COMPANY_NAME  "Altered Brain Chemistry")
set(BUNDLE_ID     "com.alteredbrainchemistry.MofoMojoFilter")

project(HyperformanceSolutionsCMake VERSION ${CURRENT_VERSION})

# ── JUCE 7.0.7 (pinned) ───────────────────────────────────────────────────────
add_subdirectory(modules/JUCE-7.0.7)

# ── Project configuration ─────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)

# ── macOS settings ────────────────────────────────────────────────────────────
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "Architectures" FORCE)
    set(CMAKE_XCODE_ATTRIBUTE_MACOSX_DEPLOYMENT_TARGET[arch=arm64] "11.0" CACHE STRING "arm64 minimum deployment target" FORCE)
endif()

# ── Linux only: GTK / WebKit ──────────────────────────────────────────────────
if(UNIX AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3    REQUIRED gtk+-3.0)
    pkg_check_modules(WEBKIT2 REQUIRED webkit2gtk-4.0)
    include_directories(${GTK3_INCLUDE_DIRS}    ${WEBKIT2_INCLUDE_DIRS})
    link_directories(${GTK3_LIBRARY_DIRS}       ${WEBKIT2_LIBRARY_DIRS})
    add_definitions(${GTK3_CFLAGS_OTHER}        ${WEBKIT2_CFLAGS_OTHER})
endif()

# ── Disable warnings (matches reference project) ──────────────────────────────
add_definitions(-w)

# ── Windows MSVC runtime ──────────────────────────────────────────────────────
# DO NOT set CMAKE_MSVC_RUNTIME_LIBRARY here.  JUCE 7's JUCEUtils.cmake calls
# find_package(Threads) which runs a try_compile sub-project.  CMake 4.x
# validates CMAKE_MSVC_RUNTIME_LIBRARY strictly against its known enum and
# rejects 'MultiThreadedDLLDebug' (the $<CONFIG:Debug> expansion that CMake
# generates for try_compile sub-projects), aborting configure.
#
# The Rust static library is built with the dynamic CRT (/MD, MSVCRT.dll).
# JUCE's default is also /MD in Release.  We just need to make sure the static
# CRT (/MT, libcmt) is never pulled in, which we do by explicitly excluding it.
if(MSVC)
    # Exclude the static CRT — Rust .a objects reference __imp_* symbols that
    # only exist in the dynamic CRT import lib (MSVCRT.lib), not in libcmt.
    add_link_options(/NODEFAULTLIB:libcmt /NODEFAULTLIB:libcmtd)
endif()

# ── Formats ───────────────────────────────────────────────────────────────────
if(APPLE)
    set(FORMATS AU VST3)
else()
    set(FORMATS VST3)
endif()

# ── Rust static library ───────────────────────────────────────────────────────
if(DEFINED RUST_LIBRARY_PATH AND EXISTS "${RUST_LIBRARY_PATH}")
    set(RUST_LIBRARY "${RUST_LIBRARY_PATH}")
    message(STATUS "Using Rust library from: ${RUST_LIBRARY}")
else()
    if(WIN32)
        set(_rust_search_paths
            "${CMAKE_SOURCE_DIR}/../../software-licensor-client-side-library/target/x86_64-pc-windows-msvc/release"
            "${CMAKE_SOURCE_DIR}/../software-licensor-client-side-library/target/x86_64-pc-windows-msvc/release"
        )
    elseif(APPLE)
        set(_rust_search_paths
            "${CMAKE_SOURCE_DIR}/../../software-licensor-client-side-library/target/universal"
            "${CMAKE_SOURCE_DIR}/../software-licensor-client-side-library/target/universal"
        )
    else()
        set(_rust_search_paths
            "${CMAKE_SOURCE_DIR}/../../software-licensor-client-side-library/target/release"
            "${CMAKE_SOURCE_DIR}/../software-licensor-client-side-library/target/release"
        )
    endif()
    find_library(RUST_LIBRARY
        NAMES softwarelicensor
        PATHS ${_rust_search_paths}
        NO_DEFAULT_PATH
    )
    if(NOT RUST_LIBRARY)
        message(FATAL_ERROR
            "Rust static library not found. "
            "Pass -DRUST_LIBRARY_PATH=<path> to cmake.")
    endif()
    message(STATUS "Found Rust library (fallback): ${RUST_LIBRARY}")
endif()

# ── Plugin target ─────────────────────────────────────────────────────────────
juce_add_plugin("${PROJECT_NAME}"
        COMPANY_NAME          "${COMPANY_NAME}"
        BUNDLE_ID             "${BUNDLE_ID}"
        COPY_PLUGIN_AFTER_BUILD FALSE   # disabled — CI runners lack write access to system plugin dirs
        PLUGIN_MANUFACTURER_CODE Abch
        PLUGIN_CODE           Mojo
        FORMATS               ${FORMATS}
        PRODUCT_NAME          "${PRODUCT_NAME}"
        NEEDS_MIDI_INPUT      FALSE
        NEEDS_MIDI_OUTPUT     FALSE
        IS_MIDI_EFFECT        FALSE
        IS_SYNTH              FALSE
)

add_compile_definitions(JUCE_PROJECT_NAME="${PROJECT_NAME}")

# ── Binary assets ─────────────────────────────────────────────────────────────
juce_add_binary_data(Assets SOURCES unlockedUI.png)

juce_generate_juce_header("${PROJECT_NAME}")

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

# ── Platform system libs for Rust ─────────────────────────────────────────────
set(PLATFORM_LIBS "")
if(WIN32)
    list(APPEND PLATFORM_LIBS ws2_32.lib userenv.lib bcrypt.lib ntdll.lib)
elseif(APPLE)
    find_library(SECURITY_FW  Security      REQUIRED)
    find_library(COREFOUND_FW CoreFoundation REQUIRED)
    list(APPEND PLATFORM_LIBS "${SECURITY_FW}" "${COREFOUND_FW}")
endif()

# ── Link libraries ────────────────────────────────────────────────────────────
# NOTE: juce_recommended_lto_flags intentionally omitted — LTO causes
# multi-hour linker hangs on CI runners with JUCE's codebase size.
target_link_libraries("${PROJECT_NAME}"
    PRIVATE
        SharedCode
        Assets
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
        "${RUST_LIBRARY}"
        ${PLATFORM_LIBS}
    PUBLIC
        # juce_recommended_config_flags intentionally NOT used here — it
        # propagates -O2 as a LINK flag causing multi-hour linker hangs.
        # juce_recommended_warning_flags intentionally NOT used here — it
        # injects /W4 on MSVC which conflicts with add_definitions(-w) and
        # generates hundreds of D9025 'overriding /w with /W4' warnings.
)

# ── Compile-only optimisation flags (no link-time codegen) ────────────────────
# We intentionally avoid /O2 on MSVC for JUCE plugin-client TUs.
# juce_audio_plugin_client_VST3.cpp pulls in the entire VST3 SDK + JUCE
# wrapper; MSVC's /O2 optimizer spends 60–90 min on that single TU on CI.
# /O1 (optimize for size) compiles in under 5 min with negligible size cost
# for SDK glue code.  User source files in Source/ still get /O2.
if(MSVC)
    # All plugin targets: /O1 safe default (avoids optimizer hang on SDK TUs)
    target_compile_options("${PROJECT_NAME}" PRIVATE
        $<$<CONFIG:Release>:/O1 /Oi /Gy>
        $<$<CONFIG:Debug>:/Od /Zi>
    )
    # Disable incremental linking — it conflicts with the Rust .a
    target_link_options("${PROJECT_NAME}" PRIVATE
        $<$<CONFIG:Release>:/INCREMENTAL:NO>
    )
    # SharedCode (user sources only): upgrade to /O2 — these are small and fast
    target_compile_options(SharedCode INTERFACE
        $<$<CONFIG:Release>:/O2>
    )
else()
    # Self-hosted runners have ample compute and clang/gcc have no MSVC-style
    # single-TU optimizer pathology, so use -O3 for maximum Release perf.
    target_compile_options("${PROJECT_NAME}" PRIVATE
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Debug>:-O0 -g>
    )
    if(APPLE)
        target_compile_options("${PROJECT_NAME}" PRIVATE
            -fvisibility=hidden
            -fvisibility-inlines-hidden
        )
        target_link_options("${PROJECT_NAME}" PRIVATE
            $<$<CONFIG:Release>:-Wl,-dead_strip>
        )
    endif()
endif()

target_include_directories("${PROJECT_NAME}" PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)