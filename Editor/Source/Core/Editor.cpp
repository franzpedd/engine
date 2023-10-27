#include "Editor.h"

#include "Renderer/Gizmo.h"
#include "Renderer/Grid.h"

#include "UI/Dockspace.h"
#include "UI/Explorer.h"
#include "UI/Mainmenu.h"
#include "UI/Viewport.h"

namespace Cosmos
{
	Editor::Editor()
		: mCamera(mWindow)
	{
		mDockspace = new Dockspace();
		mExplorer = new Explorer(mRenderer);
		mViewport = new Viewport(mUI, mRenderer, mCamera);
		mGrid = new Grid(mRenderer, mCamera);
		mMainmenu = new Mainmenu(mCamera, mGrid);

		mUI->ElementStack().Push(mDockspace);
		mUI->ElementStack().Push(mViewport);
		mUI->ElementStack().Push(mExplorer);
		mUI->ElementStack().Push(mMainmenu);

		mScene->Entities().Push(mGrid);

		// testing Primitives
		mScene->Entities().Push(new Plane(mRenderer, mCamera));
		mScene->Entities().Push(new Plane(mRenderer, mCamera));
		mScene->Entities().Push(new Cube(mRenderer, mCamera));
		mScene->Entities().Push(new Gizmo(mRenderer, mCamera));
	}

	Editor::~Editor()
	{

	}

	void Editor::OnUpdate(float timestep)
	{
		mCamera.OnUpdate(timestep);
	}

	void Editor::OnMouseMove(float xPos, float yPos, float xOffset, float yOffset)
	{
		mCamera.OnMouseMove(xOffset, yOffset);
	}

	void Editor::OnMouseScroll(float yOffset)
	{
		mCamera.OnMouseScroll(yOffset);
	}

	void Editor::OnMousePress(Buttoncode button)
	{
		
	}

	void Editor::OnMouseRelease(Buttoncode button)
	{
		
	}

	void Editor::OnKeyboardPress(Keycode key)
	{
		mCamera.OnKeyboardPress(key);
	}

	void Editor::OnKeyboardRelease(Keycode key)
	{

	}
}