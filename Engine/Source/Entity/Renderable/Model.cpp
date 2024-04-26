#include "epch.h"
#include "Model.h"

#include "Core/Camera.h"
#include "Renderer/Renderer.h"
#include "Renderer/Texture.h"
#include "Renderer/Vulkan/VKInitializers.h"
#include "Renderer/Vulkan/VKBuffer.h"
#include "Renderer/Vulkan/VKShader.h"
#include "Renderer/Vulkan/VKRenderer.h"
#include "Util/FileSystem.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

namespace Cosmos
{
	Model::Model(Shared<Renderer> renderer, Shared<Camera> camera)
		: mRenderer(renderer), mCamera(camera)
	{
		mAlbedoPath = GetAssetSubDir("Textures/dev/colors/orange.png");
	}

	void Model::Draw(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mRenderer->GetPipeline("Model"));

		for (auto& mesh : mMeshes)
		{
			mesh.Draw(commandBuffer, mRenderer->GetPipelineLayout("Model"), mDescriptorSets[mRenderer->CurrentFrame()]);
		}
	}

	void Model::Update(float deltaTime, glm::mat4 transform)
	{
		if (!mLoaded) return;

		UniformBufferObject ubo = {};
		ubo.model = transform;
		ubo.view = mCamera->GetViewRef();
		ubo.proj = mCamera->GetProjectionRef();

		uint32_t currentFrame = mRenderer->CurrentFrame();

		memcpy(mUniformBuffersMapped[mRenderer->CurrentFrame()], &ubo, sizeof(ubo));
	}

	void Model::Destroy()
	{
		vkDeviceWaitIdle(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice());
		
		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mUniformBuffers[i], nullptr);
			vkFreeMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mUniformBuffersMemory[i], nullptr);
		}
		
		vkDestroyDescriptorPool(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mDescriptorPool, nullptr);

		for (auto& mesh : mMeshes)
		{
			mesh.DestroyResources();
		}

		if (mAlbedoTexture)
			mAlbedoTexture.reset();

		mMeshes.clear();
	}

	void Model::LoadFromFile(std::string path)
	{
		if (mLoaded) Destroy();

		Assimp::Importer importer;
		uint32_t flags = aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_FlipUVs |
			aiProcess_JoinIdenticalVertices;
		
		const aiScene* scene = importer.ReadFile(path.c_str(), flags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG_TO_TERMINAL(Logger::Error, "Could not load model %s. Error: %s", path.c_str(), importer.GetErrorString());
			return;
		}

		ProcessNode(scene->mRootNode, scene);

		mLoaded = true;
		mPath = path;

		CreateResources();
	}

	void Model::LoadAlbedoTexture(std::string path)
	{
		if (path.empty())
		{
			LOG_TO_TERMINAL(Logger::Error, "Filepath for loading albedo texture is empty");
			return;
		}

		vkDeviceWaitIdle(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice());

		if (mAlbedoTexture) mAlbedoTexture.reset();

		mAlbedoTexture = Texture2D::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), path.c_str());

		UpdateDescriptorSets();

		mAlbedoPath = path;
		mLoadedAlbedo = true;
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		if (node->mNumMeshes > 1)
		{
			LOG_TO_TERMINAL(Logger::Error, "Model with more than one mesh is not yet fully implemented");
		}
			
		for (uint32_t i = 0; i < node->mNumMeshes; i++)
			mMeshes.push_back(ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene));

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene);
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<VKVertex> vertices;
		std::vector<uint32_t> indices;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			VKVertex vertex;

			// position
			vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
			glm::mat4 initialRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

			glm::vec4 vectorRotated = initialRotation * glm::vec4(vertex.position, 1.0f);
			vertex.position = glm::vec3(vectorRotated);

			// color
			if(mesh->mColors[0])
				vertex.color = glm::vec3(mesh->mColors[0]->r, mesh->mColors[0]->g, mesh->mColors[0]->b);
			else
				vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);

			// uv0
			if (mesh->mTextureCoords[0])
				vertex.uv0 = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			else
				vertex.uv0 = glm::vec2(1.0f, -1.0f);

			vertices.push_back(vertex);
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			for (uint32_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		return Mesh(mRenderer, vertices, indices);
	}

	void Model::CreateResources()
	{
		// matrix ubo
		{
			mUniformBuffers.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
			mUniformBuffersMemory.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
			mUniformBuffersMapped.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);

			for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
			{
				BufferCreate
				(
					std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(),
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					sizeof(UniformBufferObject),
					&mUniformBuffers[i],
					&mUniformBuffersMemory[i]
				);

				vkMapMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mUniformBuffersMemory[i], 0, sizeof(UniformBufferObject), 0, &mUniformBuffersMapped[i]);
			}
		}

		// textures
		{
			mAlbedoTexture = Texture2D::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), mAlbedoPath.c_str());
		}

		// create descriptor pool and descriptor sets
		{
			std::array<VkDescriptorPoolSize, 2> poolSizes = {};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount = 2;
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = 2;

			VkDescriptorPoolCreateInfo descPoolCI = {};
			descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descPoolCI.poolSizeCount = (uint32_t)poolSizes.size();
			descPoolCI.pPoolSizes = poolSizes.data();
			descPoolCI.maxSets = 2;
			VK_ASSERT(vkCreateDescriptorPool(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), &descPoolCI, nullptr, &mDescriptorPool), "Failed to create descriptor pool");

			std::vector<VkDescriptorSetLayout> layouts(RENDERER_MAX_FRAMES_IN_FLIGHT, mRenderer->GetDescriptorSetLayout("Model"));

			VkDescriptorSetAllocateInfo descSetAllocInfo = {};
			descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descSetAllocInfo.descriptorPool = mDescriptorPool;
			descSetAllocInfo.descriptorSetCount = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;
			descSetAllocInfo.pSetLayouts = layouts.data();

			mDescriptorSets.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
			VK_ASSERT(vkAllocateDescriptorSets(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), &descSetAllocInfo, mDescriptorSets.data()), "Failed to allocate descriptor sets");

			UpdateDescriptorSets();
		}
	}

	void Model::UpdateDescriptorSets()
	{
		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			std::vector<VkWriteDescriptorSet> descriptorWrites = {};

			VkDescriptorBufferInfo uboInfo = {};
			uboInfo.buffer = mUniformBuffers[i];
			uboInfo.offset = 0;
			uboInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet uboDesc = {};
			uboDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			uboDesc.dstSet = mDescriptorSets[i];
			uboDesc.dstBinding = 0;
			uboDesc.dstArrayElement = 0;
			uboDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboDesc.descriptorCount = 1;
			uboDesc.pBufferInfo = &uboInfo;

			descriptorWrites.push_back(uboDesc);

			VkDescriptorImageInfo albedoInfo = {};
			albedoInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			albedoInfo.imageView = mAlbedoTexture->GetView();
			albedoInfo.sampler = mAlbedoTexture->GetSampler();

			VkWriteDescriptorSet albedoDesc = {};
			albedoDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			albedoDesc.dstSet = mDescriptorSets[i];
			albedoDesc.dstBinding = 1;
			albedoDesc.dstArrayElement = 0;
			albedoDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			albedoDesc.descriptorCount = 1;
			albedoDesc.pImageInfo = &albedoInfo;

			descriptorWrites.push_back(albedoDesc);

			vkUpdateDescriptorSets(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
		}
	}
}