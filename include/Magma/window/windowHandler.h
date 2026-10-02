#pragma once

#include "../Uitls/Debugger.h"
#include "../Uitls/ErrorCodes.h"
#include <string>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

namespace Magma {
            /* # =============================================================================
               #  Essential class values
               # ============================================================================= */
    enum WINDOW_MODE : unsigned int {
        BORDERLESS = 0,
        FULL_SCREEN = 1,
        BORDERLESS_FULL_SCREEN = 2,
        OVERLAY = 3,
    };

   enum USER_OS : unsigned int {
      UNSUPPORTED = 0,
      LINUX = 1,
      WIN_32_BIT = 2,
      WIN_64_BIT = 3,
      MAC = 4,
  };
            /* # =============================================================================
               #  Main class
               # ============================================================================= */
    class MWindowHandler { //This class will be parent to any new OS handler implementation, it is just here to be overridden
    public:
        static std::unique_ptr<MWindowHandler> create(const std::string& appName, WINDOW_MODE windowMode, const unsigned int windowSize[2], MDebugger& debugger);

        /* # =============================================================================
           #  Class init and destruction
           # ============================================================================= */
        MWindowHandler(const std::string& appName, WINDOW_MODE windowMode, const unsigned int windowSize[2], MDebugger& debugger);
        virtual ~MWindowHandler();

        /* # =============================================================================
           #  Window logic (inheritance)
           # ============================================================================= */
        virtual void initWindow() = 0;
        virtual void createWindow() = 0;
         static USER_OS getUserOS() {return userOS;};
        static std::vector<const char*>  getOSSpecificExtensions() {return OSSpecificExtensions;};

        /* # =============================================================================
           #  Vulkan logic
           # ============================================================================= */
        virtual void createVulkanSurface(VkInstance instance) = 0;
        [[nodiscard]] VkSurfaceKHR getVkSurface() const {return surface;};
        /* # =============================================================================
           #  Event logic
           # ============================================================================= */
        virtual void pollEvents() = 0;
        virtual void extractEvents() = 0;

    protected:

        /* # =============================================================================
           #  Window info
           # ============================================================================= */
        static USER_OS userOS;
       static std::vector<const char*> OSSpecificExtensions;
        bool created { false };
        std::string appName;
        WINDOW_MODE mode;
        unsigned int size[2]{}; // [0] width & [1] height

       /* # =============================================================================
          #  Vulkan data
          # ============================================================================= */
       VkSurfaceKHR surface = VK_NULL_HANDLE;

       /* # =============================================================================
           #  Error handling
           # ============================================================================= */
        static ErrorCode errorCode;
        MDebugger* pDebugger{nullptr};
    };
};