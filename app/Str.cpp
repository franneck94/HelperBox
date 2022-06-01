#include "stdafx.h"

#include "Str.h"

void StrCopyW(wchar_t *dest, size_t size, const wchar_t *src)
{
    size_t i;
    for (i = 0; i < (size - 1); i++)
    {
        if (src[i] == 0)
            break;
        dest[i] = src[i];
    }
    dest[i] = 0;
}

void StrAppendW(wchar_t *dest, size_t size, const wchar_t *src)
{
    size_t start = wcsnlen(dest, size);
    size_t remaining = size - start;
    StrCopyW(&dest[start], remaining, src);
}

size_t StrBytesW(const wchar_t *str)
{
    return (wcslen(str) + 2) * sizeof(wchar_t);
}
