#pragma once

#include <filesystem>
#include <string>

#include <boost/signals2/signal.hpp>
#include <SFML/Graphics.hpp>

#include "../Graphics/Canvas/NullCanvas/NullCanvas.h"
#include "../Picture/model/Picture.h"
#include "EditorController.h"
#include "EditorView.h"
#include "EditorWindow.h"

namespace editor {
    class EditorSession {
    public:
        explicit EditorSession(const sf::Font* font);

        [[nodiscard]] bool IsOpen() const;
        void Update();

        void ShowOpenPrompt();

        [[nodiscard]] shapes::Picture& GetDocument();
        [[nodiscard]] EditorController& GetController();

        boost::signals2::signal<void()>& OnNewDocumentRequested();
        boost::signals2::signal<void()>& OnOpenDocumentRequested();

    private:
        void ConnectSignals();
        void UpdateTitle();
        [[nodiscard]] std::u32string CurrentFileName() const;

        gfx::NullCanvas m_nullCanvas;
        shapes::Picture m_document{m_nullCanvas};
        EditorController m_controller{m_document};
        EditorWindow m_window;
        EditorView m_view;

        boost::signals2::signal<void()> m_newDocumentRequested;
        boost::signals2::signal<void()> m_openDocumentRequested;
    };
}
