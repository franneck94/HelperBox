#include "stdafx.h"

#include <Base/HelperBoxWindow.h>

ImGuiWindowFlags HelperBoxWindow::GetWinFlags(ImGuiWindowFlags flags) const
{
    if (lock_move)
        flags |= ImGuiWindowFlags_NoMove;
    if (lock_size)
        flags |= ImGuiWindowFlags_NoResize;

    return flags;
}
