# Engine Factory

The engine factory is the base handle into all Qgfx functionality. It roughly
mirrors the vk::Instance in vulkan, or ID3D12Factory from directx 12. It deals with
all platform specific initialization code.

# Render Device

The render device serves as the programmatic interface for the GPU. It can be used to create buffers, fences, textures, samplers, and other resources.

# Differences in Backends

Really the main difference between different graphics APIs is how they are initialized. 
Qgfx doesn't attempt to hide this difference and require that all applications use the
most restrictive API's initialization process. Thus the three initialization related 
objects, the `IRenderDevice`, the `IRenderContext`, and the `ISwapChain`, are created via the `IEngineFactory` interface.

## Vulkan

Vulkan has the most expressive and low level initialization API. This functionality is exposed via the `EngineFactoryVk` object, which can be created in the following way:

```c++
#include <Qgfx/Qgfx.hpp>
#include <Qgfx/Graphics/Vulkan/EngineFactoryVk.hpp>

int main()
{
    RefAutoPtr<EngineFactoryVk> pEngineFactoryVk;

    EngineFactoryCreateInfoVk CreateInfo{};
    CreateInfo.bEnableValidation = true;
    CreateInfo.AppName = "My App";
    CreateInfo.EngineName = "My Engine";

    CreateEngineFactoryVk(CreateInfo, &pEngineFactoryVk);
}
```

Vulkan also has some extra options when creating render devices. First, you can 
specify which GPU (vk::PhysicalDevice in Vulkan terminology) to use. Also, Vulkan supports multiple seperate `IRenderDevice`s running in parallel. Finally, it also 
allows you to specify exactly which hardware queues to use when creating a render 
context (DirectX 12 and Metal abstract this decision), potentially resulting in 
higher performance.

```c++
#include <Qgfx/Qgfx.hpp>
#include <Qgfx/Graphics/Vulkan/EngineFactoryVk.hpp>

int main()
{
    RefAutoPtr<EngineFactoryVk> pEngineFactoryVk;

    EngineFactoryCreateInfoVk CreateInfo{};
    CreateInfo.bEnableValidation = true;
    CreateInfo.AppName = "My App";
    CreateInfo.EngineName = "My Engine";

    CreateEngineFactoryVk(CreateInfo, &pEngineFactoryVk);
}
```