#pragma once

#define ERROR_CODE uint32_t

namespace Magma {
/*The error codes don't have to be use strictly designated operations.
But it is good to abide with tha labeling. */

enum class ErrorCode : ERROR_CODE {
  NO_ERRORS = 0,
  UNDEFINED_ERROR = 1,

  /*=============================================================================
    System & File Errors
    =============================================================================*/

  OUT_OF_MEMORY = 1000,
  FILE_NOT_FOUND = 1001,
  FILE_READ_ERROR = 1002,
  INVALID_ARGUMENT = 1003,
  UNSUPPORTED_OPERATION = 1004,

  /*=============================================================================
    Vulkan Core Errors
    =============================================================================*/
  INSTANCE_CREATION_FAILED = 2000,
  PHYSICAL_DEVICE_NOT_FOUND = 2001,
  LOGICAL_DEVICE_CREATION_FAILED = 2002,
  VULKAN_EXTENSION_MISSING = 2003,
  VULKAN_LAYER_MISSING = 2004,
  COMMAND_POOL_CREATION_FAILED = 2005,
  INCOMPATIBLE_DRIVER = 2006,
  NO_SUITABLE_PHYSICAL_DEVICE = 2007,
  NO_PHYSICAL_DEVICES_FOUND = 2008,
  NO_GRAPHICAL_QUEUE_INDEX_FOUND = 2009,

  /*=============================================================================
    Windows & Surface Errors
    =============================================================================*/
  WINDOW_CREATION_FAILED = 3000,
  SURFACE_CREATION_FAILED = 3001,
  WINDOWING_SYSTEM_INIT_FAILED = 3002,
  OS_NOT_SUPPORTED = 3003,

  /*=============================================================================
    Swapchain Errors
    =============================================================================*/
  SWAPCHAIN_CREATION_FAILED = 4000,
  SWAPCHAIN_OUT_OF_DATE = 4001,
  IMAGE_ACQUIRE_FAILED = 4002,
  PRESENT_QUEUE_SUBMIT_FAILED = 4003,
  SYNC_OBJECT_CREATION_FAILED = 4004,

  /*=============================================================================
    Shader & Pipeline Errors
    =============================================================================*/
  SHADER_MODULE_CREATION_FAILED = 5000,
  PIPELINE_LAYOUT_CREATION_FAILED = 5001,
  GRAPHICS_PIPELINE_CREATION_FAILED = 5002,
  DESCRIPTOR_SET_ALLOCATION_FAILED = 5003,

  /*=============================================================================
    Memory and Buffer Errors
    =============================================================================*/
  BUFFER_CREATION_FAILED = 6000,
  IMAGE_CREATION_FAILED = 6001,
  MEMORY_ALLOCATION_FAILED = 6002,
  MEMORY_MAP_FAILED = 6003,

  /*=============================================================================
    Renderer Errors
    =============================================================================*/
  RENDERER_INITIALIZATION_FAILED = 7000,
  IMAGE_VIEWS_CREATION_FAILED = 7001,
  FRAMEBUFFER_CREATION_FAILED = 7002
};

/*=============================================================================
  Helper Functions
  =============================================================================*/
[[nodiscard]] constexpr bool isSuccess(const ErrorCode code) noexcept {
  return code == ErrorCode::NO_ERRORS;
};

[[nodiscard]] constexpr bool isError(const ErrorCode code) noexcept {
  return code != ErrorCode::NO_ERRORS;
};

[[nodiscard]] constexpr std::string
errorCodeToString(const ErrorCode code) noexcept {
  switch (code) {
  case ErrorCode::NO_ERRORS:
    return "NO_ERRORS";
  case ErrorCode::UNDEFINED_ERROR:
    return "UNDEFINED_ERROR";

  // System
  case ErrorCode::OUT_OF_MEMORY:
    return "OUT_OF_MEMORY";
  case ErrorCode::FILE_NOT_FOUND:
    return "FILE_NOT_FOUND";
  case ErrorCode::FILE_READ_ERROR:
    return "FILE_READ_ERROR";
  case ErrorCode::INVALID_ARGUMENT:
    return "INVALID_ARGUMENT";
  case ErrorCode::UNSUPPORTED_OPERATION:
    return "UNSUPPORTED_OPERATION";

  // Vulkan Core
  case ErrorCode::INSTANCE_CREATION_FAILED:
    return "INSTANCE_CREATION_FAILED";
  case ErrorCode::PHYSICAL_DEVICE_NOT_FOUND:
    return "PHYSICAL_DEVICE_NOT_FOUND";
  case ErrorCode::LOGICAL_DEVICE_CREATION_FAILED:
    return "LOGICAL_DEVICE_CREATION_FAILED";
  case ErrorCode::VULKAN_EXTENSION_MISSING:
    return "VULKAN_EXTENSION_MISSING";
  case ErrorCode::VULKAN_LAYER_MISSING:
    return "VULKAN_LAYER_MISSING";
  case ErrorCode::COMMAND_POOL_CREATION_FAILED:
    return "COMMAND_POOL_CREATION_FAILED";
  case ErrorCode::INCOMPATIBLE_DRIVER:
    return "INCOMPATIBLE_DRIVER";
  case ErrorCode::NO_SUITABLE_PHYSICAL_DEVICE:
    return "NO_SUITABLE_PHYSICAL_DEVICE";
  case ErrorCode::NO_PHYSICAL_DEVICES_FOUND:
    return "NO_PHYSICAL_DEVICES_FOUND";
  case ErrorCode::NO_GRAPHICAL_QUEUE_INDEX_FOUND:
    return "NO_GRAPHICAL_QUEUE_INDEX_FOUND";

  // Window & Surface
  case ErrorCode::WINDOW_CREATION_FAILED:
    return "WINDOW_CREATION_FAILED";
  case ErrorCode::SURFACE_CREATION_FAILED:
    return "SURFACE_CREATION_FAILED";
  case ErrorCode::WINDOWING_SYSTEM_INIT_FAILED:
    return "WINDOWING_SYSTEM_INIT_FAILED";
  case ErrorCode::OS_NOT_SUPPORTED:
    return "OS_NOT_SUPPORTED";

  // Swapchain & Presentation
  case ErrorCode::SWAPCHAIN_CREATION_FAILED:
    return "SWAPCHAIN_CREATION_FAILED";
  case ErrorCode::SWAPCHAIN_OUT_OF_DATE:
    return "SWAPCHAIN_OUT_OF_DATE";
  case ErrorCode::IMAGE_ACQUIRE_FAILED:
    return "IMAGE_ACQUIRE_FAILED";
  case ErrorCode::PRESENT_QUEUE_SUBMIT_FAILED:
    return "PRESENT_QUEUE_SUBMIT_FAILED";
  case ErrorCode::SYNC_OBJECT_CREATION_FAILED:
    return "SYNC_OBJECT_CREATION_FAILED";

  // Shaders & Pipeline
  case ErrorCode::SHADER_MODULE_CREATION_FAILED:
    return "SHADER_MODULE_CREATION_FAILED";
  case ErrorCode::PIPELINE_LAYOUT_CREATION_FAILED:
    return "PIPELINE_LAYOUT_CREATION_FAILED";
  case ErrorCode::GRAPHICS_PIPELINE_CREATION_FAILED:
    return "GRAPHICS_PIPELINE_CREATION_FAILED";
  case ErrorCode::DESCRIPTOR_SET_ALLOCATION_FAILED:
    return "DESCRIPTOR_SET_ALLOCATION_FAILED";

  // Memory & Buffers
  case ErrorCode::BUFFER_CREATION_FAILED:
    return "BUFFER_CREATION_FAILED";
  case ErrorCode::IMAGE_CREATION_FAILED:
    return "IMAGE_CREATION_FAILED";
  case ErrorCode::MEMORY_ALLOCATION_FAILED:
    return "MEMORY_ALLOCATION_FAILED";
  case ErrorCode::MEMORY_MAP_FAILED:
    return "MEMORY_MAP_FAILED";

  // Renderer
  case ErrorCode::RENDERER_INITIALIZATION_FAILED:
    return "RENDERER_INITIALIZATION_FAILED";

  case ErrorCode::IMAGE_VIEWS_CREATION_FAILED:
    return "IMAGE_VIEWS_CREATION_FAILED";

  case ErrorCode::FRAMEBUFFER_CREATION_FAILED:
    return "FRAMEBUFFER_CREATION_FAILED";
  default:
    return "ERROR_CODE: " + std::to_string(static_cast<int>(code));
  };
};
}; // namespace Magma