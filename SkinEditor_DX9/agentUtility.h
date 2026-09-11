#pragma once
#include <string>

bool SEAgentUtilityMode();
// -1 means this invocation is not handled yet (normal UI or needs graphics).
int SERunAgentUtility(bool graphicsReady);
bool SECreateAgentPreset(int type, int width, int height, const std::string& title,
    const std::string& maker, std::string& skin, std::string& atlas, std::string& error);
