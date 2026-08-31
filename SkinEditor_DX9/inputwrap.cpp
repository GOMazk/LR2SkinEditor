#include <imgui.h>
#include "../LR2/strclass.h"
#include <Windows.h>
#include <algorithm>
#include <string>
#include <vector>

static std::string ConvertCodePage(const char* text, UINT sourceCodePage, UINT targetCodePage)
{
    if (!text || !*text) return std::string();
    const int wideLength = MultiByteToWideChar(sourceCodePage, 0, text, -1, NULL, 0);
    if (wideLength <= 0) return std::string(text);
    std::vector<wchar_t> wide((size_t)wideLength);
    if (!MultiByteToWideChar(sourceCodePage, 0, text, -1, wide.data(), wideLength))
        return std::string(text);
    const int outputLength = WideCharToMultiByte(targetCodePage, 0, wide.data(), -1,
        NULL, 0, NULL, NULL);
    if (outputLength <= 0) return std::string(text);
    std::vector<char> output((size_t)outputLength);
    if (!WideCharToMultiByte(targetCodePage, 0, wide.data(), -1,
        output.data(), outputLength, NULL, NULL)) return std::string(text);
    return std::string(output.data());
}

std::string Cp932ToUtf8(const char* text)
{
    return ConvertCodePage(text, 932, CP_UTF8);
}

std::string Utf8ToCp932(const char* text)
{
    return ConvertCodePage(text, CP_UTF8, 932);
}

int CstrResizeCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        CSTR* mystr = (CSTR*)data;
        if(mystr->length() >= mystr->msize())
            mystr->resize(data->BufSize);
    }
    return 0;
}

// Note: Because ImGui:: is a namespace you would typically add your own function into the namespace.
// For example, you code may declare a function 'ImGui::InputText(const char* label, MyString* my_str)'
bool CstrInputText(const char* label, CSTR* my_str, ImGuiInputTextFlags flags = 0)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    if (!my_str) return false;
    const std::string utf8 = Cp932ToUtf8(my_str->body ? my_str->outstr() : "");
    const size_t capacity = (std::max)((size_t)256, utf8.size() + 256);
    std::vector<char> buffer(capacity, '\0');
    memcpy(buffer.data(), utf8.c_str(), utf8.size());
    if (!ImGui::InputText(label, buffer.data(), buffer.size(), flags)) return false;
    const std::string cp932 = Utf8ToCp932(buffer.data());
    my_str->assign(cp932.c_str());
    return true;
}

bool CstrInputTextMultiline(const char* label, ImVector<char>* my_str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    return ImGui::InputTextMultiline(label, my_str->begin(), (size_t)my_str->size(), size, flags | ImGuiInputTextFlags_CallbackResize, 
        CstrResizeCallback, (void*)my_str);
}

int InputTextExample() {
    // For this demo we are using ImVector as a string container.
    // Note that because we need to store a terminating zero character, our size/capacity are 1 more
    // than usually reported by a typical string class.
    static ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
    CSTR example;
    CstrInputText("##MyStr", &example, flags);
    ImGui::Text("Data: %p\nSize: %d\nCapacity: %d", (void*)example.body, example.length(), example.msize());

    return 0;
}
