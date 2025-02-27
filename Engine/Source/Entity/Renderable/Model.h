#pragma once

#include "Entity/Renderable/Mesh.h"
#include "Renderer/Texture.h"
#include "Util/Memory.h"
#include <vector>

// forward declarations
struct aiNode;
struct aiMesh;
struct aiScene;

namespace Cosmos
{
	// forward declarations
	class Camera;
	class Material;
	class Renderer;

	class Model
	{
	public:

		// constructor
		Model(Shared<Renderer> renderer, Shared<Camera> camera);

		// destructor
		~Model() = default;

	public:

		// returns the model's path
		inline std::string GetPath() const { return mPath; }

		// returns the model's albedo
		inline std::string GetAlbedoPath() const { return mAlbedoPath; }

		// returns a reference to the model meshes
		inline std::vector<Mesh>& GetMeshesRef() { return mMeshes; }

		// returns if model's albedo is not the default dev texture
		inline bool IsCustomAlbedoLoaded() const { return mLoadedAlbedo;  }

		// returns if the model is loaded
		inline bool IsLoaded() const { return mLoaded; }

		// sets the loaded flag to false
		inline void SetLoaded(bool value) { mLoaded = value; }

	public:

		// updates model's logic
		void OnUpdate(float deltaTime, glm::mat4 transform);
		
		// draws the model
		void OnRender(VkCommandBuffer commandBuffer);

		// free used resources
		void Destroy();

	public:

		// loads the model from file
		void LoadFromFile(std::string path);

		// loads the model albedo texture
		void LoadAlbedoTexture(std::string path);

	private:

		// recursively handle assimp nodes to create a tree of meshes
		void ProcessNode(aiNode* node, const aiScene* scene);

		// returns a cosmo mesh out of assimp mesh
		Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

		// create renderer resources
		void CreateResources();

		// updates the descriptor set (used when properties has changed)
		void UpdateDescriptorSets(); 

	private:

		Shared<Renderer> mRenderer;
		Shared<Camera> mCamera;
		std::string mPath = {};
		bool mLoaded = false;
		
		std::vector<Mesh> mMeshes;
		
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets = {};
		
		// camera's ubo
		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;

		// light's ubo
		std::vector<VkBuffer> mLightBuffers;
		std::vector<VkDeviceMemory> mLightBuffersMemory;
		std::vector<void*> mLightBuffersMapped;

		Shared<Material> mMaterial;
		std::string mAlbedoPath;
		Shared<Texture2D> mAlbedoTexture;
		bool mLoadedAlbedo = false;
	};
}