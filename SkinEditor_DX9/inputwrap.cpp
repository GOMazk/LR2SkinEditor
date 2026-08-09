#include <imgui.h>
#include "../LR2/strclass.h"

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
    return ImGui::InputText(label, my_str->body, my_str->msize(), flags | ImGuiInputTextFlags_CallbackResize,
        CstrResizeCallback, (void*)my_str);
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