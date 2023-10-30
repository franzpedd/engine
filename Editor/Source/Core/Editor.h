#pragma once

#include <Engine.h>

namespace Cosmos
{
	// forward declarations
	class Dockspace;
	class Explorer;
	class Grid;
	class Gizmo;
	class Mainmenu;
	class Viewport;

	class Editor : public Application
	{
	public:

		// constructor
		Editor();

		// destructor
		virtual ~Editor() = default;

	private:

		// ui
		Dockspace* mDockspace;
		Explorer* mExplorer;
		Viewport* mViewport;
		Mainmenu* mMainmenu;

		// entity
		Grid* mGrid;
		Gizmo* mGizmo;

		Camera* mCamera;
	};
}