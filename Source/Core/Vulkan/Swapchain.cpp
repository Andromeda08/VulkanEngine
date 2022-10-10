#include "Swapchain.hpp"

#include <limits>
#include <stdexcept>

#include "ImageView.hpp"

Swapchain::Swapchain(const Device &device, vk::PresentModeKHR preferredPresentMode)
: mDevice(device)
, mPhysicalDevice(device.physicalDevice())
{
    auto supportDetails = querySwapChainSupport();
    if (supportDetails.formats.empty() || supportDetails.presentModes.empty())
    {
        throw std::runtime_error("Supported formats or present modes might be empty.");
    }

    const auto format = pickFormat(supportDetails.formats);
    mFormat = format.format;
    mPresentMode = pickPresentMode(supportDetails.presentModes, preferredPresentMode);
    mExtent = pickExtent(supportDetails.capabilities);
    mMinImages = supportDetails.capabilities.minImageCount + 1;

    vk::SwapchainCreateInfoKHR createInfo = {};

    createInfo.setSurface(mDevice.surface().handle());
    createInfo.setMinImageCount(mMinImages);
    createInfo.setImageFormat(format.format);
    createInfo.setImageColorSpace(format.colorSpace);
    createInfo.setImageExtent(mExtent);
    createInfo.setImageArrayLayers(1);
    createInfo.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst);
    createInfo.setPreTransform(supportDetails.capabilities.currentTransform);
    createInfo.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);
    createInfo.setPresentMode(mPresentMode);
    createInfo.setClipped(VK_TRUE);
    createInfo.setOldSwapchain(VK_NULL_HANDLE);

    if (mDevice.graphics_index() != mDevice.present_index())
    {
        uint32_t indices[] = { mDevice.graphics_index(), mDevice.present_index() };
        createInfo.setImageSharingMode(vk::SharingMode::eConcurrent);
        createInfo.setQueueFamilyIndexCount(2);
        createInfo.setPQueueFamilyIndices(indices);
    }
    else
    {
        createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
        // Optional stuff =>
        createInfo.setQueueFamilyIndexCount(0);
        createInfo.setPQueueFamilyIndices(VK_NULL_HANDLE);
    }

    auto result = mDevice.handle().createSwapchainKHR(&createInfo, nullptr, &mSwapchain);

    mImages = mDevice.handle().getSwapchainImagesKHR(mSwapchain);

    mImageViews.resize(mImages.size());
    for (const auto image : mImages)
    {
        mImageViews.push_back(std::make_shared<ImageView>(mDevice, image, mFormat, vk::ImageAspectFlagBits::eColor));
    }
}

Swapchain::SwapChainSupportDetails Swapchain::querySwapChainSupport()
{
    SwapChainSupportDetails details;

    details.capabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mDevice.surface().handle());
    details.formats = mPhysicalDevice.getSurfaceFormatsKHR(mDevice.surface().handle());
    details.presentModes = mPhysicalDevice.getSurfacePresentModesKHR(mDevice.surface().handle());

    return details;
}

vk::SurfaceFormatKHR Swapchain::pickFormat(const std::vector<vk::SurfaceFormatKHR>& formats) {
    // Pick a format that is of the format B8G8R8A8_UNORM and non-linear srgb colorspace.
    for (const auto& format : formats)
    {
        if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eExtendedSrgbNonlinearEXT) {
            return format;
        }
    }

    return formats[0];
}

vk::PresentModeKHR Swapchain::pickPresentMode(const std::vector<vk::PresentModeKHR>& presentModes,
                                              const vk::PresentModeKHR& preferred)
{
    if (std::find(std::begin(presentModes), std::end(presentModes), preferred) != std::end(presentModes))
    {
        return preferred;
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::pickExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }

    auto actualExtent = mDevice.surface().instance().window().get_framebuffer_extent();
    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actualExtent;
}

void Swapchain::destroy()
{
    mImageViews.clear();
    if (mSwapchain)
    {
        vkDestroySwapchainKHR(mDevice.handle(), mSwapchain, nullptr);
    }
}