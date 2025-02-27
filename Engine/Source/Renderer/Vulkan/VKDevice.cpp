#include "epch.h"
#include "VKDevice.h"

#include "Core/Application.h"
#include "VKInstance.h"
#include "Platform/Window.h"

namespace Cosmos
{
	std::shared_ptr<VKDevice> VKDevice::Create(std::shared_ptr<VKInstance> instance)
	{
		return std::make_shared<VKDevice>(instance);
	}

	VKDevice::VKDevice(std::shared_ptr<VKInstance> instance)
		: mInstance(instance)
	{
		Logger() << "Creating VKDevice";

		if (!SDL_Vulkan_CreateSurface(Application::GetInstance()->GetWindow()->GetNativeWindow(), mInstance->GetInstance(), &mSurface))
		{
			LOG_TO_TERMINAL(Logger::Assert, "Error when creating SDL Window Surface for Vulkan. Error: %s", SDL_GetError());
		}

		SelectPhysicalDevice();

		// get physical device properties
		vkGetPhysicalDeviceMemoryProperties(mPhysicalDevice, &mMemoryProperties);
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &mProperties);
		vkGetPhysicalDeviceFeatures(mPhysicalDevice, &mFeatures);

		CreateLogicalDevice();

		LOG_TO_TERMINAL(Logger::Severity::Warn, "Put MSAA on settings.ini");
		mMSAACount = GetMaxUsableSamples();
	}

	VKDevice::~VKDevice()
	{
		vkDestroyDevice(mDevice, nullptr);
		vkDestroySurfaceKHR(mInstance->GetInstance(), mSurface, nullptr);
	}

	VkSurfaceKHR& VKDevice::GetSurface()
	{
		return mSurface;
	}

	VkDevice& VKDevice::GetDevice()
	{
		return mDevice;
	}

	VkPhysicalDevice& VKDevice::GetPhysicalDevice()
	{
		return mPhysicalDevice;
	}

	VkPhysicalDeviceFeatures& VKDevice::GetFeatures()
	{
		return mFeatures;
	}

	VkPhysicalDeviceProperties& VKDevice::GetProperties()
	{
		return mProperties;
	}

	VkQueue& VKDevice::GetGraphicsQueue()
	{
		return mGraphicsQueue;
	}

	VkQueue& VKDevice::GetPresentQueue()
	{
		return mPresentQueue;
	}

	VkQueue& VKDevice::GetComputeQueue()
	{
		return mComputeQueue;
	}

	VkSampleCountFlagBits VKDevice::GetMSAA()
	{
		return mMSAACount;
	}

	VKDevice::QueueFamilyIndices VKDevice::FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphics = i;
			}

			if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)
			{
				indices.compute = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (presentSupport)
			{
				indices.present = i;
			}

			if (indices.IsComplete())
			{
				break;
			}
		}

		if (!indices.compute.has_value())
		{
			LOG_TO_TERMINAL(Logger::Severity::Warn, "A compute queue was not found");
		}

		return indices;
	}

	uint32_t VKDevice::GetMemoryType(uint32_t bits, VkMemoryPropertyFlags properties, VkBool32* found)
	{
		for (uint32_t i = 0; i < mMemoryProperties.memoryTypeCount; i++)
		{
			if ((bits & 1) == 1)
			{
				if ((mMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
				{
					if (found)
					{
						*found = true;
					}

					return i;
				}
			}

			bits >>= 1;
		}

		if (found)
		{
			*found = false;
			return 0;
		}

		LOG_ASSERT(false, "Device: Could not find a matching memory type");
		return 0;
	}

	VkSampleCountFlagBits VKDevice::GetMaxUsableSamples()
	{
		LOG_TO_TERMINAL(Logger::Severity::Trace, "TODO: Move MSAA samples to settings.ini");

		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(mPhysicalDevice, &props);

		VkSampleCountFlags counts = props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	void VKDevice::SelectPhysicalDevice()
	{
		uint32_t gpuCount = 0;
		vkEnumeratePhysicalDevices(mInstance->GetInstance(), &gpuCount, nullptr);

		std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
		vkEnumeratePhysicalDevices(mInstance->GetInstance(), &gpuCount, physicalDevices.data());

		LOG_ASSERT(gpuCount > 0, "Could not find any GPU");

		// choose the only gpu available if there's only one available
		if (gpuCount == 1)
		{
			mPhysicalDevice = physicalDevices[0];
			return;
		}

		// selects first discrete gpu available if computer has multiple gpus
		LOG_TO_TERMINAL(Logger::Severity::Warn, "Choosing first discrete GPU available as it's a multiple GPU computer");

		for (VkPhysicalDevice& device : physicalDevices)
		{
			VkPhysicalDeviceProperties properties = {};
			vkGetPhysicalDeviceProperties(device, &properties);

			if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				mPhysicalDevice = device;
				return;
			}
		}
	}

	void VKDevice::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(mPhysicalDevice, mSurface);

		float queuePriority = 1.0f;
		std::vector<VkDeviceQueueCreateInfo> deviceQueueCIs;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphics.value(), indices.present.value(), indices.compute.value() };

		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo deviceQueueCI = {};
			deviceQueueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueCI.pNext = nullptr;
			deviceQueueCI.flags = 0;
			deviceQueueCI.queueCount = 1;
			deviceQueueCI.queueFamilyIndex = queueFamily;
			deviceQueueCI.pQueuePriorities = &queuePriority;
			deviceQueueCIs.push_back(deviceQueueCI);
		}

		std::vector<const char*> extensions = {};
		extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		std::vector<const char*> validations = mInstance->GetValidationsList();

#if defined VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
		extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

		VkDeviceCreateInfo deviceCI = {};
		deviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCI.pNext = nullptr;
		deviceCI.flags = 0;
		deviceCI.queueCreateInfoCount = (uint32_t)deviceQueueCIs.size();
		deviceCI.pQueueCreateInfos = deviceQueueCIs.data();
		deviceCI.pEnabledFeatures = &mFeatures;
		deviceCI.enabledExtensionCount = (uint32_t)extensions.size();
		deviceCI.ppEnabledExtensionNames = extensions.data();

		if (mInstance->GetValidations())
		{
			deviceCI.enabledLayerCount = (uint32_t)validations.size();
			deviceCI.ppEnabledLayerNames = validations.data();
		}

		else
		{
			deviceCI.enabledLayerCount = 0;
			deviceCI.ppEnabledLayerNames = nullptr;
		}

		VK_ASSERT(vkCreateDevice(mPhysicalDevice, &deviceCI, nullptr, &mDevice), "Failed to create Vulkan Device");

		vkGetDeviceQueue(mDevice, indices.graphics.value(), 0, &mGraphicsQueue);
		vkGetDeviceQueue(mDevice, indices.present.value(), 0, &mPresentQueue);
		vkGetDeviceQueue(mDevice, indices.compute.value(), 0, &mComputeQueue);
	}
}