#pragma once

#include <string>
#include <string_view>

#include "EditorTypes.h"

namespace editor {
    class FilePrompt {
    public:
        FilePrompt(FilePromptMode mode, std::string_view title, std::u32string path);

        [[nodiscard]] FilePromptMode GetMode() const;
        [[nodiscard]] const std::string& GetTitle() const;
        [[nodiscard]] const std::u32string& GetPath() const;

        void AppendCodepoint(char32_t codepoint);

        void EraseLastCharacter();

    private:
        FilePromptMode m_mode;
        std::string m_title;
        std::u32string m_path;
    };
}
