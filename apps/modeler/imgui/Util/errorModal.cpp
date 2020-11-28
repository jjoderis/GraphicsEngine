#include "errorModal.h"

bool errorModalRendered{ false };
std::string errorMessage{};

void UIUtil::drawErrorModal(std::string& errorMessage) {
    // make sure that the error modal is only rendered once
    if (!errorModalRendered && ImGui::BeginPopupModal("Error Info", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        errorModalRendered = true;
        ImGui::Text(errorMessage.c_str());
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            errorMessage.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }
}