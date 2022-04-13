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
// in release redirects stdout and stderr to log file
// in debug creates console
bool InitializeLog();
void InitializeChat();
void Terminate();

// === File/console logging ===
// printf-style log
void Log(const char *msg, ...);

// printf-style wide-string log
void LogW(const wchar_t *msg, ...);

// flushes log file.
//static void FlushFile() { fflush(logfile); }

// === Game chat logging ===
// Shows to the user in the form of a white chat message
void Info(const char *format, ...);
void InfoW(const wchar_t *format, ...);

// Shows to the user in the form of a red chat message
void Error(const char *format, ...);
void ErrorW(const wchar_t *format, ...);

// Shows to the user in the form of a yellow chat message
void Warning(const char *format, ...);
void WarningW(const wchar_t *format, ...);
}; // namespace Log
