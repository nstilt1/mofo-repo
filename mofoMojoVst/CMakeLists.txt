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
set(PROJECT_NAME  "ViatorJuceCMake")
set(PRODUCT_NAME  "The Mofo's Mojo v1")
set(COMPANY_NAME  "Altered Brain Chemistry")
set(BUNDLE_ID     "com.alteredbrainchemistry.MofoMojoFilter")

project(ViatorJuceCMake VERSION ${CURRENT_VERSION})

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
# Must be MultiThreadedDLL (/MD) to match the Rust static library's CRT linkage.
# The Rust toolchain links against MSVCRT (the dynamic CRT) by default.
# Using static /MT here causes LNK4098 (MSVCRT vs libcmt conflict) and dozens
# of LNK2001 unresolved __imp__wassert / __imp_bsearch etc. errors because the
# static CRT stubs don't export decorated import-library symbols that the Rust
# .a objects expect to find via __imp_ indirection.
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL$<$<CONFIG:Debug>:Debug>" CACHE INTERNAL "")
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
        # juce_recommended_config_flags intentionally NOT used here:
        # it propagates -O2 as a LINK flag which causes the macOS universal-binary
        # and Windows MSVC linkers to hang for 1+ hours on JUCE + Rust .a.
        # We apply optimisation flags at compile time only (see below).
        juce::juce_recommended_warning_flags
)

# ── Compile-only optimisation flags (no link-time codegen) ────────────────────
# Replaces juce_recommended_config_flags.  We set -O2/-O0 only on compile
# units; the linker sees no LTO / codegen flags so it stays fast.
if(MSVC)
    target_compile_options("${PROJECT_NAME}" PRIVATE
        $<$<CONFIG:Release>:/O2 /Oi /Gy>
        $<$<CONFIG:Debug>:/Od /Zi>
    )
    # Disable incremental linking in Release — it conflicts with Rust .a
    target_link_options("${PROJECT_NAME}" PRIVATE
        $<$<CONFIG:Release>:/INCREMENTAL:NO>
    )
else()
    target_compile_options("${PROJECT_NAME}" PRIVATE
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Debug>:-O0 -g>
    )
    if(APPLE)
        # Visibility flags help dead-stripping but must be compile-only
        target_compile_options("${PROJECT_NAME}" PRIVATE
            -fvisibility=hidden
            -fvisibility-inlines-hidden
        )
        # Pass -dead_strip to the linker (fast; does NOT require codegen passes)
        target_link_options("${PROJECT_NAME}" PRIVATE
            $<$<CONFIG:Release>:-Wl,-dead_strip>
        )
    endif()
endif()

target_include_directories("${PROJECT_NAME}" PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
)