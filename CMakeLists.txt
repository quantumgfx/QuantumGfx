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
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/Align.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/ArrayProxy.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/DebugOutput.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/Error.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/FixedBlockMemoryAllocator.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/FlagsEnum.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/FormatString.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/HashUtils.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/MemoryAllocator.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/PoolAllocator.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/SpinLock.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/STDAllocator.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/TypeCompatibleBytes.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/RefPtr.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/RefCountedObject.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Common/ValidatedCast.hpp
        # Platform Include Files
        ${QGFX_INCLUDE_DIR}/Qgfx/Platform/Atomics.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Platform/NativeWindow.hpp
        # Basic Platform Implementation
        ${QGFX_INCLUDE_DIR}/Qgfx/Platform/Basic/BasicAtomics.hpp
        # Graphics Include Files
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/IBase.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/IRenderer.hpp
        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/IResource.hpp

        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/StateObjectsRegistry.hpp)

set(QGFX_SOURCE_FILES
        # Common Implementation
        ${QGFX_SOURCE_DIR}/Common/DebugOutput.cpp
        ${QGFX_SOURCE_DIR}/Common/FixedBlockMemoryAllocator.cpp
        ${QGFX_SOURCE_DIR}/Common/MemoryAllocator.cpp
        ${QGFX_SOURCE_DIR}/Common/SpinLock.cpp
        # Graphics Implementation
        ${QGFX_SOURCE_DIR}/Graphics/IBase.cpp
        ${QGFX_SOURCE_DIR}/Graphics/IRenderer.cpp
        ${QGFX_SOURCE_DIR}/Graphics/IResource.cpp)

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

    find_package(Vulkan REQUIRED)
    target_include_directories(Qgfx PUBLIC ${Vulkan_INCLUDE_DIR})
    target_include_directories(Qgfx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/VulkanMemoryAllocator/src)

    set(QGFX_INCLUDE_FILES ${QGFX_INCLUDE_FILES}
                        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Vulkan/VulkanBase.hpp
                        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Vulkan/VulkanRenderer.hpp
                        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Vulkan/VulkanResource.hpp

                        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Vulkan/CommandBufferVk.hpp
                        ${QGFX_INCLUDE_DIR}/Qgfx/Graphics/Vulkan/CommandQueueVk.hpp)

    set(QGFX_SOURCE_FILES ${QGFX_SOURCE_FILES}
                        ${QGFX_SOURCE_DIR}/Graphics/Vulkan/VulkanBase.cpp
                        ${QGFX_SOURCE_DIR}/Graphics/Vulkan/VulkanRenderer.cpp
                        ${QGFX_SOURCE_DIR}/Graphics/Vulkan/VulkanResource.cpp

                        ${QGFX_SOURCE_DIR}/Graphics/Vulkan/CommandBufferVk.cpp
                        ${QGFX_SOURCE_DIR}/Graphics/Vulkan/CommandQueueVk.cpp)

    target_compile_definitions(Qgfx PUBLIC QGFX_VULKAN_SUPPORTED=1)

endif()

target_sources(Qgfx PRIVATE ${QGFX_INCLUDE_FILES} ${QGFX_SOURCE_FILES})