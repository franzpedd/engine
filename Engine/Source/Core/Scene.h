#pragma once

#include "Camera.h"
#include "Entity/Entity.h"
#include "Util/Timestep.h"
#include <memory>

namespace Cosmos
{
	// forward declarations
	class Window;

	class Scene
	{
	public:

		// returns a smart pointer to a new scene
		static std::shared_ptr<Scene> Create(std::shared_ptr<Window>& window);

		// constructor
		Scene(std::shared_ptr<Window>& window);

		// destructor
		~Scene();

		// returns a reference to the entities
		inline EntityStack& Entities() { return mEntities; }

	public:

		// returns the camera
		inline Camera& GetCamera() { return mCamera; }

	public:

		// updates the scene objects
		void OnUpdate(Timestep ts);

		// draws the scene drawables
		void OnDraw();

		// cleans the entities resources
		void Destroy();

	private:

		std::shared_ptr<Window>& mWindow;
		EntityStack mEntities;
		Camera mCamera;
	};
}