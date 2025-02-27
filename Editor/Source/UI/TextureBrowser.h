#pragma once

#include <Engine.h>

#include <filesystem>

namespace Cosmos
{
	class TextureBrowser : public Widget
	{
	public:

		enum Extension
		{
			UNDEFINED = -1,
			JPG,
			PNG,
			TGA,
			BMP,

			EXTENSION_MAX
		};

		struct TextureProperties
		{
			Shared<Texture2D> texture;
			std::filesystem::path path;
			Extension type = UNDEFINED;
			VkDescriptorSet descriptor = VK_NULL_HANDLE;
		};

	public:

		// constructure
		TextureBrowser(Shared<Renderer> renderer);

		// destructor
		virtual ~TextureBrowser();

		// for user interface drawing
		virtual void OnUpdate() override;

	public:

		// opens/closes the widget
		void ToogleOnOff(bool value);

		// returns a copy of the last selected texture in the texture browser
		inline TextureProperties GetSelectedTexture() { return mLastSelectedTexture; }

	private:

		// free current textures, searches for textures in the path
		void RefreshFilesExplorer(std::string path);

	private:

		Shared<Renderer> mRenderer;
		std::string mRoot = {};
		bool mShowWindow = false;
		std::string mSearchStr;
		bool mUpdatedStr = false;
		const std::array<const char*, EXTENSION_MAX> mValidExtensions = { ".jpg", ".png", ".tga", ".bmp" };
		ImVec2 mTextureSize = { 128.0f, 128.0f };

		TextureProperties mLastSelectedTexture;

		std::vector<TextureProperties> mTextures;
	};
}