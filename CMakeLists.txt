cmake_minimum_required (VERSION 3.15)

project(Qgfx LANGUAGES C CXX)

# GLOBAL PROJECT PROPERTIES

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>)

# PLATFORM DETECTION

set(QGFX_PLATFORM_WIN32 FALSE CACHE INTERNAL "")
set(QGFX_VULKAN_SUPPORTED FALSE CACHE INTERNAL "Vulkan is not supported")

if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(QGFX_ARCH 64 CACHE INTERNAL "64-bit architecture")
else()
    set(QGFX_ARCH 32 CACHE INTERNAL "32-bit architecture")
endif()

if(WIN32)
    set(QGFX_PLATFORM_WIN32 TRUE CACHE INTERNAL "Target platform: Win32") #WIN32 is a variable, so we cannot use string "WIN32"
    message("Qgfx Target platform: Win32. SDK Version: " ${CMAKE_SYSTEM_VERSION})

    set(QGFX_VULKAN_SUPPORTED TRUE CACHE INTERNAL "Vulkan is supported on Win32 platform")
else()
    message(FATAL_ERROR "Unsupported platform")
endif(WIN32)

# QGFX TARGET CONFIGURATION

add_library(Qgfx STATIC)

# OPTIONS

option(QGFX_DEBUG "Enable Qgfx verification, logging, and debugging" OFF)

if(QGFX_DEBUG)
    target_compile_definitions(Qgfx PUBLIC QGFX_DEBUG=1)
endif()

# RENDERING BACKEND OPTIONS

option(QGFX_NO_VULKAN "Disable Vulkan backend" OFF)

# QGFX TARGET RENDERING BACKEND CONFIGURATION

if(${QGFX_NO_VULKAN})
    set(QGFX_VULKAN_SUPPORTED FALSE CACHE INTERNAL "Vulkan backend is forcibly disabled")
endif()

if(NOT ${QGFX_VULKAN_SUPPORTED})
    message(FATAL_ERROR "No rendering backends are select to build")
endif()

message("VULKAN_SUPPORTED: " ${QGFX_VULKAN_SUPPORTED})

# FILES

set(QGFX_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Include)
set(QGFX_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Source)

target_include_directories(Qgfx PUBLIC ${QGFX_INCLUDE_DIR})

set(QGFX_INCLUDE_FILES 
        # Main Include File
        ${QGFX_INCLUDE_DIR}/Qgfx/Qgfx.hpp
        # Common Include Files
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/ArrayProxy.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/DebugOutput.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/Error.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/FlagsEnum.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/FormatString.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/MemoryAllocator.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/SpinLock.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/RefAutoPtr.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/RefCountedObject.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/ValidatedCast.hpp
        # Platform Include Files
        ${QGFX_INCLUDE_DIR}/Qgfx/Platform/Atomics.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Platform/NativeWindow.hpp
        # Basic Platform Implementation
        ${QGFX_INCLUDE_DIR}/Qgfx/Platform/Basic/BasicAtomics.hpp
        # Graphics Include Files
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Forward.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/GraphicsTypes.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/IBuffer.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/IEngineFactory.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/IFence.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/IObject.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/IRenderContext.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/IRenderDevice.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/ISampler.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/ISwapChain.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/ITexture.hpp)

set(QGFX_SOURCE_FILES
        # Common Implementation
        ${QGFX_SOURCE_DIR}/Common/DebugOutput.cpp
        ${QGFX_SOURCE_DIR}/Common/SpinLock.cpp)

if(${QGFX_PLATFORM_WIN32})
    # PLATFORM_WIN32 specific
    
    set(QGFX_INCLUDE_FILES ${QGFX_INCLUDE_FILES}
                        ${QGFX_INCLUDE_DIR}/Qgfx/Platform/Win32/Win32Atomics.hpp
                        ${QGFX_INCLUDE_DIR}/Qgfx/Platform/Win32/Win32NativeWindow.hpp)

    set(QGFX_SOURCE_FILES ${QGFX_SOURCE_FILES} 
                        ${QGFX_SOURCE_DIR}/Platform/Win32/Win32Atomics.cpp)

    target_compile_definitions(Qgfx PUBLIC QGFX_PLATFORM_WIN32=1)

    if(${QGFX_VULKAN_SUPPORTED})
        target_compile_definitions(Qgfx PUBLIC VK_USE_PLATFORM_WIN32_KHR NOMINMAX)
    endif()
endif()

if(${QGFX_VULKAN_SUPPORTED})

    add_subdirectory(Extern/quantumvk)

    target_link_libraries(Qgfx PUBLIC quantumvk)

    set(QGFX_INCLUDE_FILES ${QGFX_INCLUDE_FILES}
                        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Vulkan/BaseVk.hpp
                        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Vulkan/EngineFactoryVk.hpp
                        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Vulkan/RenderContextVk.hpp
                        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Vulkan/RenderDeviceVk.hpp
                        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Vulkan/SwapChainVk.hpp
                        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Vulkan/TypeConversionsVk.hpp)

    set(QGFX_SOURCE_FILES ${QGFX_SOURCE_FILES} 
                        ${QGFX_SOURCE_DIR}/Graphics/Vulkan/EngineFactoryVk.cpp
                        ${QGFX_SOURCE_DIR}/Graphics/Vulkan/RenderContextVk.cpp
                        ${QGFX_SOURCE_DIR}/Graphics/Vulkan/RenderDeviceVk.cpp
                        ${QGFX_SOURCE_DIR}/Graphics/Vulkan/SwapChainVk.cpp
                        ${QGFX_SOURCE_DIR}/Graphics/Vulkan/TypeConversionsVk.cpp)

    target_compile_definitions(Qgfx PUBLIC QGFX_VULKAN_SUPPORTED=1)

endif()

target_sources(Qgfx PRIVATE ${QGFX_INCLUDE_FILES} ${QGFX_SOURCE_FILES})