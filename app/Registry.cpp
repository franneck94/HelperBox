#include "stdafx.h"

#include "Registry.h"

bool OpenSettingsKey(PHKEY phkResult)
{
    LSTATUS status;
    DWORD Disposition;

    LPCWSTR lpSubKey = L"Software\\GWToolbox";
    status = RegCreateKeyExW(HKEY_CURRENT_USER,
                             lpSubKey,
                             0,
                             nullptr,
                             REG_OPTION_NON_VOLATILE,
                             KEY_SET_VALUE | KEY_READ,
                             nullptr,
                             phkResult,
                             &Disposition);

    if (status != ERROR_SUCCESS)
    {
        fprintf(stderr, "RegCreateKeyExW failed: {status:0x%lX, lpSubKey:'%ls'}\n", status, lpSubKey);
        phkResult = nullptr;
        return false;
    }

    return true;
}

bool OpenUninstallKey(PHKEY phkResult)
{
    LPCWSTR lpSubKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\GWToolbox";
    LSTATUS status = RegOpenKeyExW(HKEY_CURRENT_USER, lpSubKey, 0, KEY_READ, phkResult);

    if (status != ERROR_SUCCESS)
    {
        fprintf(stderr, "RegOpenKeyExW failed: {status:0x%lX, lpSubKey:'%ls'}\n", status, lpSubKey);
        phkResult = nullptr;
        return false;
    }

    return true;
}
