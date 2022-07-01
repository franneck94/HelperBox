#pragma once

#define HELPERBOX_CHAN GW::Chat::Channel::CHANNEL_GWCA2
#define HELPERBOX_SENDER L"HelperBox++"
#define HELPERBOX_SENDER_COL 0x00ccff
#define HELPERBOX_WARNING_COL 0xFFFF44
#define HELPERBOX_ERROR_COL 0xFF4444
#define HELPERBOX_INFO_COL 0xFFFFFF

namespace Log
{
// === Setup and cleanup ====
bool InitializeLog();
void InitializeChat();
void Terminate();

// === File/console logging ===
void Log(const char *msg, ...);
void LogW(const wchar_t *msg, ...);

// === Game chat logging ===
void Info(const char *format, ...);
void Error(const char *format, ...);
void Warning(const char *format, ...);
}; // namespace Log
