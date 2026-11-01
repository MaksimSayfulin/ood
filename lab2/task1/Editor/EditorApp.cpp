#include "EditorApp.h"

#include <array>
#include <filesystem>
#include <utility>

namespace {
    constexpr std::array<const char*, 1> fontCandidates = {
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
    };
}

int editor::EditorApp::Run() {
    m_hasFont = TryLoadFont(m_font);
    CreateSession();

    while (!m_sessions.empty()) {
        m_newDocumentRequested = false;
        m_openDocumentRequested = false;

        for (auto& session : m_sessions) {
            session->Update();
        }

        RemoveClosedSessions();

        if (m_newDocumentRequested) {
            CreateSession();
        }

        if (m_openDocumentRequested) {
            CreateSession()->ShowOpenPrompt();
        }
    }

    return 0;
}

editor::EditorSession *editor::EditorApp::CreateSession() {
    auto session = std::make_unique<EditorSession>(m_hasFont ? &m_font : nullptr);

    session->OnNewDocumentRequested().connect([this] { m_newDocumentRequested = true; });
    session->OnOpenDocumentRequested().connect([this] { m_openDocumentRequested = true; });

    auto* result = session.get();
    m_sessions.push_back(std::move(session));

    return result;
}

void editor::EditorApp::RemoveClosedSessions() {
    for (auto it = m_sessions.begin(); it != m_sessions.end();) {
        if ((*it)->IsOpen()) {
            ++it;
            continue;
        }
        it = m_sessions.erase(it);
    }
}

bool editor::EditorApp::TryLoadFont(sf::Font& font) {
    for (const auto* path : fontCandidates) {
        if (font.openFromFile(path)) {
            return true;
        }
    }
    return false;
}
