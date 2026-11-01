#include "EditorWindow.h"

#include <algorithm>
#include <iostream>
#include <optional>

#include "EditorLayout.h"
#include "EditorView.h"
#include "SelectionFrame.h"

namespace {
    bool HasSnapDisabled() {
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LAlt)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RAlt);
    }

    bool HasSelectionModifier() {
        return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
            || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
    }

    sf::Cursor::Type CursorTypeFor(editor::ResizeHandle handle) {
        using editor::ResizeHandle;
        using CursorType = sf::Cursor::Type;
        switch (handle) {
            case ResizeHandle::TopLeft:
            case ResizeHandle::BottomRight: return CursorType::SizeTopLeftBottomRight;
            case ResizeHandle::TopRight:
            case ResizeHandle::BottomLeft: return CursorType::SizeBottomLeftTopRight;
            case ResizeHandle::Left:
            case ResizeHandle::Right: return CursorType::SizeHorizontal;
            case ResizeHandle::Top:
            case ResizeHandle::Bottom: return CursorType::SizeVertical;
            case ResizeHandle::None: break;
        }
        return CursorType::Arrow;
    }

    sf::Cursor::Type FallbackCursorType(const sf::Cursor::Type type) {
        switch (type) {
        case sf::Cursor::Type::SizeTopLeftBottomRight:
        case sf::Cursor::Type::SizeBottomLeftTopRight:
        case sf::Cursor::Type::SizeHorizontal:
        case sf::Cursor::Type::SizeVertical:
            return sf::Cursor::Type::Hand;
        default:
            return sf::Cursor::Type::Arrow;
        }
    }

    std::optional<sf::Cursor> CreateCursor(const sf::Cursor::Type type) {
        if (auto cursor = sf::Cursor::createFromSystem(type)) {
            return cursor;
        }

        return sf::Cursor::createFromSystem(FallbackCursorType(type));
    }
}

editor::EditorWindow::EditorWindow() {
    const auto desktop = sf::VideoMode::getDesktopMode();
    const gfx::Size screen{static_cast<double>(desktop.size.x), static_cast<double>(desktop.size.y)};
    const auto size = layout::InitialWindowSize(screen);

    m_window.create(sf::VideoMode({static_cast<unsigned int>(size.width),
                                   static_cast<unsigned int>(size.height)}),
                    "Графический редактор");

    const auto origin = layout::WindowOrigin(size, screen);
    m_window.setPosition({static_cast<int>(origin.x), static_cast<int>(origin.y)});

    m_window.setFramerateLimit(60);
    ResetView();
    m_canvasBounds = layout::CanvasBounds(size);
    m_statusMessage = "";
}

bool editor::EditorWindow::IsOpen() const {
    return m_window.isOpen();
}

gfx::Size editor::EditorWindow::GetWindowSize() const {
    const auto size = m_window.getSize();
    return {static_cast<double>(size.x), static_cast<double>(size.y)};
}

gfx::Rect editor::EditorWindow::GetCanvasBounds() const {
    return m_canvasBounds;
}

void editor::EditorWindow::Update(EditorView& view, const EditorController& controller, const shapes::Picture& document) {
    HandleEvents(controller);
    UpdateCursor(controller);

    view.Render(m_window, GetWindowSize(), controller, document, m_statusMessage, GetPromptForView());
    m_window.display();
}

void editor::EditorWindow::ShowFilePrompt(const FilePromptMode mode, const std::string_view title,
                                          std::u32string initialPath) {
    m_prompt = FilePrompt{mode, title, std::move(initialPath)};
    m_dragStartedOnCanvas = false;
}

void editor::EditorWindow::SetStatusMessage(std::string message) {
    m_statusMessage = std::move(message);
}

void editor::EditorWindow::SetTitle(const std::string& title) {
    m_window.setTitle(title);
}

boost::signals2::signal<void(editor::UiCommand)> &editor::EditorWindow::OnCommand() {
    return m_commandEvent;
}

boost::signals2::signal<void(gfx::Point, bool)> &editor::EditorWindow::OnPointerPressed() {
    return m_pointerPressedEvent;
}

boost::signals2::signal<void(gfx::Point, bool)> &editor::EditorWindow::OnPointerMoved() {
    return m_pointerMovedEvent;
}

boost::signals2::signal<void(gfx::Point)> &editor::EditorWindow::OnPointerReleased() {
    return m_pointerReleasedEvent;
}

boost::signals2::signal<void()> &editor::EditorWindow::OnCancel() {
    return m_cancelEvent;
}

boost::signals2::signal<void(gfx::Size)> &editor::EditorWindow::OnCanvasResized() {
    return m_canvasResizedEvent;
}

boost::signals2::signal<void(editor::FilePromptMode, std::u32string)> &editor::EditorWindow::OnPromptAccepted() {
    return m_promptAcceptedEvent;
}

boost::signals2::signal<void()> &editor::EditorWindow::OnPromptCancelled() {
    return m_promptCancelledEvent;
}

boost::signals2::signal<void(std::optional<std::size_t>, bool)> &editor::EditorWindow::OnListRowPressed() {
    return m_listRowPressedEvent;
}

boost::signals2::signal<void(std::optional<std::size_t>)> &editor::EditorWindow::OnListRowMoved() {
    return m_listRowMovedEvent;
}

boost::signals2::signal<void()> &editor::EditorWindow::OnListRowReleased() {
    return m_listRowReleasedEvent;
}

boost::signals2::signal<void(double)> &editor::EditorWindow::OnListScrolled() {
    return m_listScrolledEvent;
}

void editor::EditorWindow::HandleEvents(const EditorController& controller) {
    while (const std::optional event = m_window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            m_window.close();
            continue;
        }

        if (event->is<sf::Event::Resized>()) {
            const auto size = m_window.getSize();
            ResetView();
            m_canvasBounds = layout::CanvasBounds({static_cast<double>(size.x), static_cast<double>(size.y)});
            m_canvasResizedEvent({m_canvasBounds.width, m_canvasBounds.height});
            continue;
        }

        if (m_prompt) {
            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                HandleKey(*key);
            } else if (const auto* entered = event->getIf<sf::Event::TextEntered>()) {
                HandleTextEntered(*entered);
            }
            continue;
        }

        if (const auto* button = event->getIf<sf::Event::MouseButtonPressed>();
            button != nullptr && button->button == sf::Mouse::Button::Left) {
            const gfx::Point windowPoint{static_cast<double>(button->position.x),
                                         static_cast<double>(button->position.y)};

            if (const auto command = layout::HitTestToolbar(windowPoint); command.has_value()) {
                m_dragStartedOnCanvas = false;
                m_commandEvent(*command);
                continue;
            }

            if (IsInShapeList(windowPoint)) {
                m_dragStartedOnCanvas = false;
                m_listRowPressedEvent(HitTestShapeListRow(windowPoint, controller), HasSelectionModifier());
                continue;
            }

            const gfx::Point canvasPoint = ToCanvasPoint(button->position);
            const auto selection = controller.GetSelectionBounds();
            const bool onSelectionUi = selection
                && (SelectionFrame::HitTestHandle(*selection, canvasPoint) != ResizeHandle::None
                    || SelectionFrame::HitTestFrame(*selection, canvasPoint));

            m_dragStartedOnCanvas = m_canvasBounds.Contains(windowPoint) || onSelectionUi;
            if (m_dragStartedOnCanvas) {
                m_pointerPressedEvent(canvasPoint, HasSelectionModifier());
            }
            continue;
        }

        if (const auto* button = event->getIf<sf::Event::MouseButtonReleased>();
            button != nullptr && button->button == sf::Mouse::Button::Left) {
            if (m_dragStartedOnCanvas) {
                m_pointerReleasedEvent(ToCanvasPoint(button->position));
            } else {
                m_listRowReleasedEvent();
            }
            m_dragStartedOnCanvas = false;
            continue;
        }

        if (const auto* moved = event->getIf<sf::Event::MouseMoved>()) {
            const gfx::Point windowPoint{static_cast<double>(moved->position.x),
                                         static_cast<double>(moved->position.y)};

            if (IsInShapeList(windowPoint)) {
                m_listRowMovedEvent(HitTestShapeListRow(windowPoint, controller));
            } else {
                m_pointerMovedEvent(ToCanvasPoint(moved->position), HasSnapDisabled());
            }
            continue;
        }

        if (const auto* wheel = event->getIf<sf::Event::MouseWheelScrolled>()) {
            const gfx::Point windowPoint{static_cast<double>(wheel->position.x),
                                         static_cast<double>(wheel->position.y)};

            if (IsInShapeList(windowPoint)) {
                m_listScrolledEvent(wheel->delta);
            }
            continue;
        }

        if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
            HandleKey(*key);
        }
    }
}

void editor::EditorWindow::HandleKey(const sf::Event::KeyPressed& key) {
    if (m_prompt) {
        if (key.code == sf::Keyboard::Key::Enter) {
            const auto mode = m_prompt->GetMode();
            const auto path = m_prompt->GetPath();
            m_prompt.reset();
            m_promptAcceptedEvent(mode, path);
        } else if (key.code == sf::Keyboard::Key::Escape) {
            m_prompt.reset();
            m_promptCancelledEvent();
        } else if (key.code == sf::Keyboard::Key::Backspace) {
            m_prompt->EraseLastCharacter();
        }
        return;
    }

    const bool control = key.control || key.system;
    if (control && key.code == sf::Keyboard::Key::S) {
        m_commandEvent(UiCommand::SaveDocument);
    } else if (control && key.code == sf::Keyboard::Key::O) {
        m_commandEvent(UiCommand::OpenDocument);
    } else if (control && key.code == sf::Keyboard::Key::N) {
        m_commandEvent(UiCommand::NewDocument);
    } else if (key.code == sf::Keyboard::Key::Delete || key.code == sf::Keyboard::Key::Backspace) {
        m_commandEvent(UiCommand::DeleteShape);
    } else if (key.code == sf::Keyboard::Key::Escape) {
        m_cancelEvent();
    }
}

void editor::EditorWindow::HandleTextEntered(const sf::Event::TextEntered& entered) {
    if (m_prompt && entered.unicode >= 32) {
        m_prompt->AppendCodepoint(entered.unicode);
    }
}

void editor::EditorWindow::UpdateCursor(const EditorController& controller) {
    const bool drawing = controller.GetTool() != Tool::Select;
    const auto type = drawing ? sf::Cursor::Type::Cross : CursorTypeFor(controller.GetHoveredHandle());

    if (type == m_appliedCursor) {
        return;
    }

    if (const auto cursor = CreateCursor(type)) {
        m_window.setMouseCursor(*cursor);
    }

    m_appliedCursor = type;
}

void editor::EditorWindow::ResetView() {
    const auto size = m_window.getSize();
    m_window.setView(sf::View(sf::FloatRect({0.f, 0.f}, {static_cast<float>(size.x), static_cast<float>(size.y)})));
}

gfx::Point editor::EditorWindow::ToCanvasPoint(const sf::Vector2i windowPoint) const {
    return {static_cast<double>(windowPoint.x) - m_canvasBounds.x,
            static_cast<double>(windowPoint.y) - m_canvasBounds.y};
}

bool editor::EditorWindow::IsInShapeList(const gfx::Point& windowPoint) const {
    return layout::ShapeListBounds(GetWindowSize()).Contains(windowPoint);
}

std::optional<std::size_t> editor::EditorWindow::HitTestShapeListRow(
    const gfx::Point& windowPoint, const EditorController& controller) const {
    const auto size = GetWindowSize();
    const auto rows = layout::ShapeListRowsBounds(size);
    if (!rows.Contains(windowPoint)) {
        return std::nullopt;
    }

    const double offset = windowPoint.y - rows.y + controller.GetListScroll();
    if (offset < 0) {
        return std::nullopt;
    }

    const auto row = static_cast<std::size_t>(offset / layout::shapeListRowHeight);
    return row < controller.GetShapesCount() ? std::optional<std::size_t>{row} : std::nullopt;
}

std::optional<editor::FilePromptView> editor::EditorWindow::GetPromptForView() const {
    if (!m_prompt) {
        return std::nullopt;
    }
    return FilePromptView{m_prompt->GetTitle(), m_prompt->GetPath()};
}