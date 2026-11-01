#pragma once

#include <optional>
#include <string>
#include <string_view>

#include <SFML/Graphics.hpp>

#include "../Picture/model/Picture.h"
#include "EditorController.h"
#include "EditorTypes.h"

namespace editor {
    class EditorView {
    public:
        explicit EditorView(const sf::Font* font);

        void Render(sf::RenderTarget& target, const gfx::Size& windowSize,
                    const EditorController& controller, const shapes::Picture& document,
                    const std::string& statusMessage,
                    const std::optional<FilePromptView>& prompt);

    private:
        void DrawToolbar(sf::RenderTarget& target, const gfx::Size& windowSize, Tool activeTool);
        void DrawCanvas(sf::RenderTarget& target, const gfx::Rect& canvasBounds,
                        const shapes::Picture& document);
        void DrawSelectionFrame(sf::RenderTarget& target, const gfx::Rect& canvasBounds,
                                const EditorController& controller, const shapes::Picture& document);
        void DrawShapeList(sf::RenderTarget& target, const gfx::Size& windowSize,
                           const EditorController& controller, const shapes::Picture& document);
        void DrawGuides(sf::RenderTarget& target, const gfx::Rect& canvasBounds,
                        const EditorController& controller);

        static void DrawShapeIcon(sf::RenderTarget& target, const gfx::Rect& bounds,
                                  std::string_view type, const gfx::Color& color);
        void DrawDraft(sf::RenderTarget& target, const gfx::Rect& canvasBounds,
                       const EditorController& controller) const;
        void DrawStatusBar(sf::RenderTarget& target, const gfx::Size& windowSize,
                           const EditorController& controller, const std::string& statusMessage) const;
        void DrawFilePrompt(sf::RenderTarget& target, const gfx::Size& windowSize,
                            const FilePromptView& prompt);

        void DrawLabel(sf::RenderTarget& target, std::string_view text, unsigned size,
                       const gfx::Point& position, sf::Color color) const;
        void DrawLabel(sf::RenderTarget& target, const std::u32string& text, unsigned size,
                       const gfx::Point& position, sf::Color color) const;
        void FillRect(sf::RenderTarget& target, const gfx::Rect& bounds, sf::Color color,
                      float outlineThickness = 0.f, sf::Color outlineColor = sf::Color::Black) const;

        const sf::Font* m_font;
    };
}
