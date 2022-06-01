#include "stdafx.h"

#include "Str.h"

void PathGetExeFullPath(wchar_t *path, size_t length)
{
    DWORD result = GetModuleFileNameW(NULL, path, length);
    if (result >= length)
        path[0] = 0;
}

void PathGetProgramDirectory(wchar_t *path, size_t length)
{
    PathGetExeFullPath(path, length);

    wchar_t *filename = PathFindFileNameW(path);
    if (filename != path)
        filename[0] = 0;
}

bool PathCompose(wchar_t *dest, size_t length, const wchar_t *left, const wchar_t *right)
{
    assert(MAX_PATH <= length);

    size_t left_size = StrBytesW(left);
    if (length < left_size)
    {
        fprintf(stderr, "Left string too long for the destination buffer\n");
        return false;
    }

    if (dest != left)
    {
        memmove(dest, left, left_size);
    }

    if (PathAppendW(dest, right) != TRUE)
    {
        fprintf(stderr, "PathAppendW failed\n");
        return false;
    }

    return true;
}
