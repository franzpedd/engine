#include "Explorer.h"

namespace Cosmos
{
	Explorer::Explorer(Shared<Renderer> renderer)
		: Widget("UI:Explorer"), mRenderer(renderer)
	{
		mCurrentDir = GetAssetDir();

		// create default resources
		mUndefinedResource.texture = Texture2D::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), GetAssetSubDir("Textures/editor/undefined.png").c_str());
		mUndefinedResource.descriptor = AddTexture(mUndefinedResource.texture->GetSampler(), mUndefinedResource.texture->GetView());

		mFolderResource.texture = Texture2D::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), GetAssetSubDir("Textures/editor/folder.png").c_str());
		mFolderResource.descriptor = AddTexture(mFolderResource.texture->GetSampler(), mFolderResource.texture->GetView());

		mTextResource.texture = Texture2D::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), GetAssetSubDir("Textures/editor/txt.png").c_str());
		mTextResource.descriptor = AddTexture(mTextResource.texture->GetSampler(), mTextResource.texture->GetView());

		mModelResource.texture = Texture2D::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), GetAssetSubDir("Textures/editor/mdl.png").c_str());
		mModelResource.descriptor = AddTexture(mModelResource.texture->GetSampler(), mModelResource.texture->GetView());

		mVertexResource.texture = Texture2D::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), GetAssetSubDir("Textures/editor/vert.png").c_str());
		mVertexResource.descriptor = AddTexture(mVertexResource.texture->GetSampler(), mVertexResource.texture->GetView());

		mFragmentResource.texture = Texture2D::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), GetAssetSubDir("Textures/editor/frag.png").c_str());
		mFragmentResource.descriptor = AddTexture(mFragmentResource.texture->GetSampler(), mFragmentResource.texture->GetView());

		mSpirvResource.texture = Texture2D::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), GetAssetSubDir("Textures/editor/spv.png").c_str());
		mSpirvResource.descriptor = AddTexture(mSpirvResource.texture->GetSampler(), mSpirvResource.texture->GetView());
	}

	Explorer::~Explorer()
	{
		mCurrentDirAssets.clear();
	}

	void Explorer::OnUpdate()
	{
		// layout variable
		ImVec2 currentPos = {};
		const ImVec2 buttonSize = { 50, 50 };
		const float offSet = 5.0f;

		// refresh directory only when needed
		if (mCurrentDirRefresh)
		{
			ReadFolder(mCurrentDir);
			mCurrentDirRefresh = false;
		}
		
		// begin the window
		ImGui::Begin("Explorer");

		// draw te parent folder
		{
			ImGui::BeginGroup();
			
			if (ImGui::ImageButton("...", mFolderResource.descriptor, buttonSize))
			{
				std::filesystem::path parentPath(mCurrentDir);
				parentPath = parentPath.parent_path();

				auto dir = std::filesystem::directory_entry(parentPath);

				if (dir.is_directory())
				{
					mCurrentDirRefresh = true;
					mCurrentDir = dir.path().string();
				}
			}
			
			currentPos = { ImGui::GetCursorPos().x, ImGui::GetCursorPos().y };
			ImGui::SetCursorPos(currentPos);
			
			ImGui::Text("...");
			
			ImGui::EndGroup();
		}

		ImGui::SameLine();

		// draw asset vector
		{
			for (size_t i = 0; i < mCurrentDirAssets.size(); i++)
			{
				// checks if must draw in the same line
				if ((currentPos.x + (buttonSize.x * 2)) <= ImGui::GetContentRegionAvail().x) ImGui::SameLine();
		
				// draws the asset
				ImGui::BeginGroup();

				// resoruce representation of the asset
				if (ImGui::ImageButton(mCurrentDirAssets[i].path.c_str(), mCurrentDirAssets[i].resource.descriptor, buttonSize))
				{
					std::filesystem::path path(mCurrentDirAssets[i].path);
					auto dir = std::filesystem::directory_entry(path);
					
					// folder asset, must flag to refresh folder on next frame
					if (dir.is_directory())
					{
						mCurrentDirRefresh = true;
						mCurrentDir = dir.path().string();
					}
				}
				
				// display name
				currentPos = ImGui::GetCursorPos();
				ImGui::SetCursorPosX({ currentPos.x /*+ offSet*/});

				// name is too large, displaying a shortter name
				if (mCurrentDirAssets[i].displayName.size() > EXPLORER_ASSET_NAME_MAX_DISPLAY_SIZE)
				{
					std::string shortName = mCurrentDirAssets[i].displayName.substr(0, EXPLORER_ASSET_NAME_MAX_DISPLAY_SIZE);
					ImGui::Text(shortName.c_str());
				}

				else
				{
					ImGui::Text(mCurrentDirAssets[i].displayName.c_str());
				}
		
				ImGui::EndGroup();
		
				// drag and drop behaviour
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					ImGui::SetDragDropPayload
					(
						"EXPLORER",
						mCurrentDirAssets[i].path.c_str(),
						mCurrentDirAssets[i].path.size() + 1,
						ImGuiCond_Once
					);
					ImGui::EndDragDropSource();
				}
			}
		}

		ImGui::End();
	}

	void Explorer::ReadFolder(std::string path)
	{
		// clear current path assets
		mCurrentDirAssets.clear();
			
		for (const std::filesystem::directory_entry& dirEntry : std::filesystem::directory_iterator(path))
		{
			// default asset configs
			std::string path = dirEntry.path().string();
			Cosmos::replace(path.begin(), path.end(), char('\\'), char('/'));
			
			Asset asset = {};
			asset.type = AssetType::Undefined;
			asset.resource = mUndefinedResource;
			asset.path = path;
			asset.displayName = dirEntry.path().filename().replace_extension().string();
			
			// folder asset
			if (std::filesystem::is_directory(dirEntry))
			{
				asset.type = AssetType::Folder;
				asset.resource = mFolderResource;
				
				mCurrentDirAssets.push_back(asset);
				continue;
			}

			// assign asset custom resource
			for (size_t extId = 0; extId < mValidExtensions.size(); extId++)
			{
				if (strcmp(mValidExtensions[extId], dirEntry.path().extension().string().c_str()) == 0)
				{
					switch (extId)
					{
						case 0: // txt
						{
							asset.type = AssetType::Text;
							asset.resource = mTextResource;
							break;
						}

						case 1: // gltf
						case 2: // obj
						{
							asset.type = AssetType::Model;
							asset.resource = mModelResource;
							break;
						}

						case 3: // vert
						{
							asset.type = AssetType::Shader;
							asset.resource = mVertexResource;
							break;
						}

						case 4: // frag
						{
							asset.type = AssetType::Shader;
							asset.resource = mFragmentResource;
							break;
						}

						case 5: // spir-v
						{
							asset.type = AssetType::Shader;
							asset.resource = mSpirvResource;
							break;
						}

						case 6: // png
						case 7: // jpg
						{
							asset.type = AssetType::Image;
							asset.resource.texture = Texture2D::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), asset.path.c_str());
							asset.resource.descriptor = AddTexture(asset.resource.texture->GetSampler(), asset.resource.texture->GetView());
							break;
						}
					}
				}
			}

			mCurrentDirAssets.push_back(asset);
		}
	}
}