#pragma once

enum class SEUILanguage {
    English,
    Korean,
};

SEUILanguage SEGetUILanguage();
void SESetUILanguage(SEUILanguage language, bool persist = true);
void SELoadUILanguageSetting();
const char* SEText(const char* english, const char8_t* korean);
