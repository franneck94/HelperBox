#pragma once

#include <cstring>

void PathGetExeFullPath(wchar_t *path, size_t length);

void PathGetProgramDirectory(wchar_t *path, size_t length);

bool PathCompose(wchar_t *dest, size_t length, const wchar_t *left, const wchar_t *right);
