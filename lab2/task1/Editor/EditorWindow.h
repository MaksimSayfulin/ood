#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include <boost/signals2/signal.hpp>
#include <SFML/Graphics.hpp>

#include "../Graphics/Geometry/Rect.h"
#include "EditorController.h"
#include "EditorTypes.h"
#include "FilePrompt.h"

namespace editor {
    class EditorView;

    class EditorWindow {
    public:
        EditorWindow();

        [[nodiscard]] bool IsOpen() const;
        [[nodiscard]] gfx::Size GetWindowSize() const;
        [[nodiscard]] gfx::Rect GetCanvasBounds() const;

        void Update(EditorView& view, const EditorController& controller, const shapes::Picture& document);

        void ShowFilePrompt(FilePromptMode mode, std::string_view title, std::u32string initialPath);

        void SetStatusMessage(std::string message);
        void SetTitle(const std::string& title);

        boost::signals2::signal<void(UiCommand)>& OnCommand();
        boost::signals2::signal<void(gfx::Point, bool)>& OnPointerPressed();
        boost::signals2::signal<void(gfx::Point, bool)>& OnPointerMoved();
        boost::signals2::signal<void(gfx::Point)>& OnPointerReleased();
        boost::signals2::signal<void()>& OnCancel();
        boost::signals2::signal<void(gfx::Size)>& OnCanvasResized();
        boost::signals2::signal<void(FilePromptMode, std::u32string)>& OnPromptAccepted();
        boost::signals2::signal<void()>& OnPromptCancelled();

        boost::signals2::signal<void(std::optional<std::size_t>, bool)>& OnListRowPressed();
        boost::signals2::signal<void(std::optional<std::size_t>)>& OnListRowMoved();
        boost::signals2::signal<void()>& OnListRowReleased();
        boost::signals2::signal<void(double)>& OnListScrolled();

    private:
        void HandleEvents(const EditorController& controller);
        void HandleKey(const sf::Event::KeyPressed& key);
        void HandleTextEntered(const sf::Event::TextEntered& entered);
        void UpdateCursor(const EditorController& controller);
        void ResetView();

        [[nodiscard]] gfx::Point ToCanvasPoint(sf::Vector2i windowPoint) const;
        [[nodiscard]] std::optional<FilePromptView> GetPromptForView() const;
        [[nodiscard]] bool IsInShapeList(const gfx::Point& windowPoint) const;
        [[nodiscard]] std::optional<std::size_t> HitTestShapeListRow(const gfx::Point& windowPoint,
                                                                     const EditorController& controller) const;

        sf::RenderWindow m_window;
        gfx::Rect m_canvasBounds;
        std::string m_statusMessage;
        std::optional<FilePrompt> m_prompt;
        sf::Cursor::Type m_appliedCursor = sf::Cursor::Type::Arrow;
        bool m_dragStartedOnCanvas = false;

        boost::signals2::signal<void(UiCommand)> m_commandEvent;
        boost::signals2::signal<void(gfx::Point, bool)> m_pointerPressedEvent;
        boost::signals2::signal<void(gfx::Point, bool)> m_pointerMovedEvent;
        boost::signals2::signal<void(gfx::Point)> m_pointerReleasedEvent;
        boost::signals2::signal<void()> m_cancelEvent;
        boost::signals2::signal<void(gfx::Size)> m_canvasResizedEvent;
        boost::signals2::signal<void(FilePromptMode, std::u32string)> m_promptAcceptedEvent;
        boost::signals2::signal<void()> m_promptCancelledEvent;
        boost::signals2::signal<void(std::optional<std::size_t>, bool)> m_listRowPressedEvent;
        boost::signals2::signal<void(std::optional<std::size_t>)> m_listRowMovedEvent;
        boost::signals2::signal<void()> m_listRowReleasedEvent;
        boost::signals2::signal<void(double)> m_listScrolledEvent;
    };
}