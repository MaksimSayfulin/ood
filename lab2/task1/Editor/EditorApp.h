#pragma once

#include <memory>
#include <vector>

#include <SFML/Graphics.hpp>

#include "EditorSession.h"

namespace editor {
    class EditorApp {
    public:
        int Run();

    private:
        EditorSession* CreateSession();
        void RemoveClosedSessions();

        [[nodiscard]] static bool TryLoadFont(sf::Font& font);

        std::vector<std::unique_ptr<EditorSession>> m_sessions;
        sf::Font m_font;
        bool m_hasFont = false;
        bool m_newDocumentRequested = false;
        bool m_openDocumentRequested = false;
    };
}
