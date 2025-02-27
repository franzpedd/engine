#include "SceneHierarchy.h"

#include <filesystem>

namespace Cosmos
{
	SceneHierarchy::SceneHierarchy(Shared<Renderer> renderer, Shared<Camera> camera)
		: Widget("UI:Scene Hierarchy"), mRenderer(renderer), mCamera(camera)
	{
		Logger() << "Creating Scene Hierarchy";
	}

	void SceneHierarchy::OnUpdate()
	{
		DisplaySceneHierarchy();
		DisplaySelectedEntityComponents();
	}

	Entity* SceneHierarchy::GetSelectedEntity()
	{
		return mSelectedEntity;
	}

	void SceneHierarchy::UnselectEntity()
	{
		mSelectedEntity = nullptr;
	}

	void SceneHierarchy::DisplaySceneHierarchy()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar | ImGuiTreeNodeFlags_OpenOnArrow;

		ImGui::Begin("Objects", 0, flags);

		// displays edit menu
		if (ImGui::BeginMenuBar())
		{
			ImGui::BeginGroup();
			{
				ImGui::Text(ICON_FA_PAINT_BRUSH " Edit Entity");

				float itemSize = 25.0f;
				float itemCount = 1.0f;

				ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (itemSize * itemCount));

				if (ImGui::MenuItem(ICON_FA_PLUS_SQUARE))
				{
					Entity* ent = Application::GetInstance()->GetActiveScene()->CreateEntity();
				}
			}

			ImGui::EndGroup();
		}

		ImGui::EndMenuBar();

		// draws all existing entity nodes
		for (auto& ent : Application::GetInstance()->GetActiveScene()->GetEntityMapRef())
		{
			bool redraw = false;
			DrawEntityNode(&ent.second, &redraw);

			if (redraw)
				break;
		}

		ImGui::End();
	}

	void SceneHierarchy::DisplaySelectedEntityComponents()
	{
		ImGuiWindowFlags flags = {};
		flags |= ImGuiWindowFlags_HorizontalScrollbar;
		flags |= ImGuiWindowFlags_MenuBar;

		ImGui::Begin("Components", 0, flags);

		if (!mSelectedEntity)
		{
			ImGui::End();
			return;
		}

		// dispaly edit menu
		if (ImGui::BeginMenuBar())
		{
			ImGui::Text(ICON_FA_PAINT_BRUSH " Edit Components");

			float itemSize = 35.0f;
			float itemCount = 1.0f;

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (itemSize * itemCount));
			itemCount--;

			if (ImGui::BeginMenu(ICON_FA_PLUS_SQUARE))
			{
				DisplayAddComponentEntry<TransformComponent>("Transform");
				DisplayAddComponentEntry<ModelComponent>("Model");
				DisplayAddComponentEntry<SoundSourceComponent>("Sound Source");

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		DrawComponents(mSelectedEntity);

		ImGui::End();
	}

	void SceneHierarchy::DrawEntityNode(Entity* entity, bool* redraw)
	{
		if (entity == nullptr)
			return;

		// creating unique context
		ImGui::PushID((void*)(uint64_t)entity);

		{
			bool selected = (mSelectedEntity == entity) ? true : false;
			std::string name = entity->GetComponent<NameComponent>().name;

			// selects the entity
			if (ImGui::Selectable(name.c_str(), selected, ImGuiSelectableFlags_DontClosePopups))
			{
				mSelectedEntity = entity;
			}

			// right click menu
			if (selected)
			{
				if (ImGui::BeginPopupContextItem("##RightClickPopup", ImGuiPopupFlags_MouseButtonRight))
				{
					if (ImGui::MenuItem("Dupplicate"))
					{
						Application::GetInstance()->GetActiveScene()->DuplicateEntity(mSelectedEntity);
					}
				
					ImGui::Separator();
				
					if (ImGui::MenuItem("Remove Entity"))
					{
						Application::GetInstance()->GetActiveScene()->DestroyEntity(mSelectedEntity);
						mSelectedEntity = nullptr;
						*redraw = true;
					}
				
					ImGui::EndPopup();
				}
			}
		}
			
		ImGui::PopID();
	}

	void SceneHierarchy::DrawComponents(Entity* entity)
	{
		if (entity == nullptr)
			return;

		// id and name (general info)
		{
			ImGui::Separator();

			{
				uint64_t id = entity->GetUUID();
				std::string idStr = "ID: ";
				idStr.append(std::to_string(id));

				ImGui::Text("%s", idStr.c_str());
			}

			ImGui::Text("Name: ");
			ImGui::SameLine();

			{
				auto& name = entity->GetComponent<NameComponent>().name;
				char buffer[ENTITY_NAME_MAX_CHARS];
				memset(buffer, 0, sizeof(buffer));
				std::strncpy(buffer, name.c_str(), sizeof(buffer));

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 2.0f));

				if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				{
					name = std::string(buffer);
				}

				ImGui::PopStyleVar();
			}

			ImGui::Separator();
		}

		// 3d world-position
		DrawComponent<TransformComponent>("Transform", mSelectedEntity, [&](TransformComponent& component)
			{
				ImGui::Text("T: ");
				ImGui::SameLine();
				Vector3Control("Translation", component.translation);

				ImGui::Text("R: ");
				ImGui::SameLine();
				glm::vec3 rotation = glm::degrees(component.rotation);
				Vector3Control("Rotation", rotation);
				component.rotation = glm::radians(rotation);

				ImGui::Text("S: ");
				ImGui::SameLine();
				Vector3Control("Scale", component.scale);
			});

		// model component
		DrawComponent<ModelComponent>("Model", mSelectedEntity, [&](ModelComponent& component)
			{
				if(!component.model)
					component.model = std::make_shared<Model>(mRenderer, mCamera);

				// model path
				{
					ImGui::BeginGroup();

					ImGui::Text(ICON_FA_CUBE " ");
					ImGui::SameLine();

					auto modelPath = component.model->GetPath();
					char buffer[ENTITY_NAME_MAX_CHARS];
					memset(buffer, 0, sizeof(buffer));
					std::strncpy(buffer, modelPath.c_str(), sizeof(buffer));

					ImGui::InputTextWithHint("", "Drag and drop from Explorer", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

					ImGui::EndGroup();

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EXPLORER"))
						{
							std::filesystem::path path = (const char*)payload->Data;
							component.model->LoadFromFile(path.string());
						}

						ImGui::EndDragDropTarget();
					}
				}

				ImGui::Separator();
				
				// model albedo texture
				{
					ImGui::BeginGroup();
					
					ImGui::Text(ICON_FA_PAINT_BRUSH " ");
					ImGui::SameLine();
					
					auto texturePath = component.model->GetAlbedoPath();
					char buffer[ENTITY_NAME_MAX_CHARS];
					memset(buffer, 0, sizeof(buffer));
					std::strncpy(buffer, texturePath.c_str(), sizeof(buffer));
					
					ImGui::InputTextWithHint("", "Drag and drop from Explorer", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);
					
					ImGui::EndGroup();
					
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EXPLORER"))
						{
							std::filesystem::path path = (const char*)payload->Data;
							component.model->LoadAlbedoTexture(path.string());
						}
					
						ImGui::EndDragDropTarget();
					}
				}
			});

		// 3d sound source
		DrawComponent<SoundSourceComponent>("Sound Source", mSelectedEntity, [&](SoundSourceComponent& component)
			{
				if (!component.source)
					component.source = std::make_shared<sound::Source>();

				// sound path
				{
					ImGui::BeginGroup();

					ImGui::Text(ICON_LC_MUSIC " ");
					ImGui::SameLine();

					auto& soundPath = component.source->GetPath();
					char buffer[ENTITY_NAME_MAX_CHARS];
					memset(buffer, 0, sizeof(buffer));
					std::strncpy(buffer, soundPath.c_str(), sizeof(buffer));

					ImGui::InputTextWithHint("", "Drag and drop from Explorer", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

					ImGui::EndGroup();

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EXPLORER"))
						{
							std::filesystem::path path = (const char*)payload->Data;
							component.source->Create(path.string());
						}

						ImGui::EndDragDropTarget();
					}
				}
			});
	}

	template<typename T>
	void SceneHierarchy::DisplayAddComponentEntry(const char* name)
	{
		if (ImGui::MenuItem(name))
		{
			if (!mSelectedEntity->HasComponent<T>())
			{
				mSelectedEntity->AddComponent<T>();
				return;
			}
			
			LOG_TO_TERMINAL(Logger::Severity::Warn, "Entity %s already have the component %s", mSelectedEntity->GetComponent<NameComponent>().name.c_str(), name);
		}
	}

	template<typename T, typename F>
	void SceneHierarchy::DrawComponent(const char* name, Entity* entity, F func)
	{
		if (entity == nullptr)
			return;

		if (entity->HasComponent<T>())
		{
			auto& component = entity->GetComponent<T>();
			if (ImGui::TreeNodeEx((void*)typeid(T).hash_code(), 0, name))
			{
				func(component);

				if (ImGui::BeginPopupContextItem("##RightClickComponent"))
				{
					if (ImGui::MenuItem("Remove Component"))
					{
						entity->RemoveComponent<T>();
					}

					ImGui::EndPopup();
				}

				ImGui::TreePop();
			}
		}
	}
}