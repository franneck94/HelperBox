#include "GuiUtils.h"

void DrawButton(ActionState &action_state, const ImVec4 color, std::string_view text)
{
    bool pushed_style = false;

    if (action_state != ActionState::INACTIVE)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        pushed_style = true;
    }

    if (ImGui::Button(text.data(), BUTTON_SIZE))
    {
        if (IsExplorable())
            action_state = StateNegation(action_state);
    }
    if (pushed_style)
        ImGui::PopStyleColor();
}
