#pragma once

struct Settings
{
    bool localdll = false;
    uint32_t pid;
};

extern Settings settings;

void PrintUsage(bool terminate);

void ParseRegSettings();
void ParseCommandLine();

bool CreateProcessInt(const wchar_t *path, const wchar_t *args, const wchar_t *workdir, bool as_admin = false);
