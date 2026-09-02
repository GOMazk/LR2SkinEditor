#include "seLocalization.h"

#include <Windows.h>

#include <string>

namespace {

SEUILanguage currentLanguage = SEUILanguage::English;

std::wstring SettingsPath() {
    wchar_t localAppData[MAX_PATH] = {};
    if (!GetEnvironmentVariableW(L"LOCALAPPDATA", localAppData, MAX_PATH))
        return L"SkinEditor.settings.ini";
    std::wstring directory = std::wstring(localAppData) + L"\\SkinEditor";
    CreateDirectoryW(directory.c_str(), NULL);
    return directory + L"\\settings.ini";
}

} // namespace

SEUILanguage SEGetUILanguage() {
    return currentLanguage;
}

void SESetUILanguage(SEUILanguage language, bool persist) {
    currentLanguage = language;
    if (!persist) return;
    WritePrivateProfileStringW(L"UI", L"Language",
        language == SEUILanguage::Korean ? L"ko" : L"en",
        SettingsPath().c_str());
}

void SELoadUILanguageSetting() {
    wchar_t language[16] = {};
    GetPrivateProfileStringW(L"UI", L"Language", L"en", language,
        (DWORD)(sizeof(language) / sizeof(language[0])), SettingsPath().c_str());
    currentLanguage = _wcsicmp(language, L"ko") == 0
        ? SEUILanguage::Korean : SEUILanguage::English;
}

const char* SEText(const char* english, const char8_t* korean) {
    return currentLanguage == SEUILanguage::Korean
        ? reinterpret_cast<const char*>(korean) : english;
}
