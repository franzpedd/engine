#include "Hierarchy.h"

#include <Engine.h>

namespace Cosmos
{
    Hierarchy::Hierarchy()
    {

    }

    Hierarchy::~Hierarchy()
    {

    }

    void Hierarchy::OnUpdate()
    {
        ImGui::Begin("Entities", nullptr);

        // right-click menu
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Add Group"))
            {
                mGroups.insert({ "Group", CreateShared<HierarchyGroup>() });
            }

            ImGui::EndPopup();
        }

        DrawGroups();

        ImGui::End();
    }

    void Hierarchy::DrawGroups()
    {
        for (auto& group : mGroups)
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
            bool popupRename = false;

            ImGui::PushID(group.second.get());

            if(ImGui::CollapsingHeader(group.second->name.c_str(), flags))
            {
                // group right-menu
                if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
                {
                    if (ImGui::MenuItem("Add Entity"))
                    {
                        Shared<HierarchyBase> base = CreateShared<HierarchyBase>();
                        base->entity = Application::GetInstance()->GetActiveScene()->CreateEntity();

                        group.second->entities.insert({ base->entity->GetComponent<NameComponent>().name, base });
                    }

                    ImGui::Separator();

                    if(ImGui::MenuItem("Rename Group"))
                    {
                        popupRename = true;
                    }

                    if(ImGui::MenuItem("Delete Group"))
                    {
                        for(auto& entNode : group.second->entities)
                        {
                            Application::GetInstance()->GetActiveScene()->DestroyEntity(entNode.second->entity);
                        }

                        group.second->entities.clear();
                        mGroups.erase(group.first);

                        ImGui::EndPopup();
                        break;
                    }

                    ImGui::EndPopup();
                }

                // rename popup
                auto& groupName = group.second->name;
                char groupBuffer[ENTITY_NAME_MAX_CHARS];
                memset(groupBuffer, 0, sizeof(groupBuffer));
                std::strncpy(groupBuffer, groupName.c_str(), sizeof(groupBuffer));

                if(popupRename) ImGui::OpenPopup("RenamePopup");
                if (ImGui::BeginPopup("RenamePopup"))
                {
                    if(ImGui::InputText("##NewName", groupBuffer, sizeof(groupBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                    {
                        groupName = std::string(groupBuffer);
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }

                // draw children nodes
                for (auto& node : group.second->entities)
                {
                    bool redraw = false;
                    DrawEntityNode(group.second, node.second, &redraw);
                
                    if(redraw) break;
                }
            }
            ImGui::PopID();
        }
    }

    void Hierarchy::DrawEntityNode(Shared<HierarchyGroup> group, Shared<HierarchyBase> base, bool* redraw)
    {
        ImGui::PushID(base.get());

        auto& nodeName = base->entity->GetComponent<NameComponent>().name;
        char nodeBuffer[ENTITY_NAME_MAX_CHARS];
        memset(nodeBuffer, 0, sizeof(nodeBuffer));
        std::strncpy(nodeBuffer, nodeName.c_str(), sizeof(nodeBuffer));

        if (Cosmos::SelectableInputText(&base->selected, nodeBuffer, sizeof(nodeBuffer)))
        {
            nodeName = std::string(nodeBuffer);
        }

        // entity right-click menu
        if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
        {
            if(ImGui::MenuItem("Dupplicate"))
            {
                Shared<HierarchyBase> dupplicate = CreateShared<HierarchyBase>();
                dupplicate->entity = Application::GetInstance()->GetActiveScene()->DuplicateEntity(base->entity);
                group->entities.insert({ dupplicate->entity->GetComponent<NameComponent>().name , dupplicate });

                *redraw = true;
            }

            if(ImGui::MenuItem("Delete"))
            {
                Application::GetInstance()->GetActiveScene()->DestroyEntity(base->entity);
                group->entities.erase(group->entities.find(base->entity->GetComponent<NameComponent>().name));
                
                *redraw = true;
            }

            ImGui::EndPopup();
        }

        ImGui::PopID();
    }
}