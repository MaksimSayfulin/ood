#include "FilePrompt.h"

#include <utility>

editor::FilePrompt::FilePrompt(const FilePromptMode mode, std::string_view title, std::u32string path)
    : m_mode(mode), m_title(title), m_path(std::move(path)) {
}

editor::FilePromptMode editor::FilePrompt::GetMode() const {
    return m_mode;
}

const std::string &editor::FilePrompt::GetTitle() const {
    return m_title;
}

const std::u32string &editor::FilePrompt::GetPath() const {
    return m_path;
}

void editor::FilePrompt::AppendCodepoint(const char32_t codepoint) {
    m_path += codepoint;
}

void editor::FilePrompt::EraseLastCharacter() {
    if (m_path.empty()) {
        return;
    }

    m_path.pop_back();
}
