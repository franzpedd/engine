#pragma once

#include "Defines.h"
#include "Commander.h"
#include "Entity/Entity.h"

#include "Vulkan/VKBuffer.h"
#include "Vulkan/VKInstance.h"
#include "Vulkan/VKDevice.h"
#include "Vulkan/VKPipeline.h"
#include "Vulkan/VKSwapchain.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace Cosmos
{
	// forward declaration
	class GUI;
	class Window;
	class Scene;

	class Renderer
	{
	public:

		// returns a smart poitner to a new renderer
		static std::shared_ptr<Renderer> Create(std::shared_ptr<Window>& window, std::shared_ptr<Scene>& scene);

		// constructor
		Renderer(std::shared_ptr<Window>& window, std::shared_ptr<Scene>& scene);

		// destructor
		~Renderer();

	public:

		// returns the backend instance class object
		inline std::shared_ptr<VKInstance>& BackendInstance() { return mInstance; }

		// returns the backend device class object
		inline std::shared_ptr<VKDevice>& BackendDevice() { return mDevice; }

		// returns the backend swapchain class object
		inline std::shared_ptr<VKSwapchain>& BackendSwapchain() { return mSwapchain; }

	public:

		// returns the vulkan pipeline cache
		inline VkPipelineCache& PipelineCache() { return mPipelineCache; }

		// returns the current in-process frame
		inline uint32_t CurrentFrame() { return mCurrentFrame; }

		// returns the current image index
		inline uint32_t ImageIndex() { return mImageIndex; }

		// returns a reference to the commander
		inline Commander& GetCommander() { return mCommander; }

		// returns a reference to the pipeline library
		inline VKPipelineLibrary& GetPipelineLibrary() { return mPipelineLibrary; }

	public:

		// updates the renderer
		void OnUpdate(EntityStack& entities);

		// links the user interface to the renderer
		inline void ConnectUI(std::shared_ptr<GUI>& ui) { mUI = ui; }

	private:

		// submit all render passes
		void ManageRenderPasses(uint32_t& imageIndex);

		// creates global structures shared across the renderer
		void CreateGlobalStates();

	private:

		std::shared_ptr<Window>& mWindow;
		std::shared_ptr<Scene>& mScene;
		std::shared_ptr<VKInstance> mInstance;
		std::shared_ptr<VKDevice> mDevice;
		std::shared_ptr<VKSwapchain> mSwapchain;

		Commander mCommander;
		VKPipelineLibrary mPipelineLibrary;
		VkPipelineCache mPipelineCache;

		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mInFlightFences;
		uint32_t mCurrentFrame = 0;
		uint32_t mImageIndex = 0;

		std::shared_ptr<GUI> mUI;
	};
}