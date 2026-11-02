#include "EditorSession.h"

namespace {
    constexpr auto defaultFileName = U"document.shapes";
    constexpr auto windowTitle = "Графический редактор";
}

editor::EditorSession::EditorSession(const sf::Font* font) : m_view(font) {
    const auto canvas = m_window.GetCanvasBounds();
    m_controller.OnCanvasResized({canvas.width, canvas.height});

    ConnectSignals();
}

bool editor::EditorSession::IsOpen() const {
    return m_window.IsOpen();
}

void editor::EditorSession::Update() {
    m_window.Update(m_view, m_controller, m_document);
}

void editor::EditorSession::ShowOpenPrompt() {
    m_window.ShowFilePrompt(FilePromptMode::Open, "Открыть документ", CurrentFileName());
}

shapes::Picture &editor::EditorSession::GetDocument() {
    return m_document;
}

editor::EditorController &editor::EditorSession::GetController() {
    return m_controller;
}

boost::signals2::signal<void()> &editor::EditorSession::OnNewDocumentRequested() {
    return m_newDocumentRequested;
}

boost::signals2::signal<void()> &editor::EditorSession::OnOpenDocumentRequested() {
    return m_openDocumentRequested;
}

void editor::EditorSession::ConnectSignals() {
    m_window.OnCommand().connect([this](const UiCommand command) {
        if (command == UiCommand::NewDocument) {
            m_newDocumentRequested();
            return;
        }
        if (command == UiCommand::OpenDocument) {
            m_openDocumentRequested();
            return;
        }

        m_controller.OnCommand(command);
    });

    m_window.OnPointerPressed().connect(
        [this](const gfx::Point point, const bool toggle) { m_controller.OnPointerPressed(point, toggle); });
    m_window.OnPointerMoved().connect(
        [this](const gfx::Point point, const bool disableSnap) { m_controller.OnPointerMoved(point, disableSnap); });
    m_window.OnPointerReleased().connect([this](const gfx::Point point) { m_controller.OnPointerReleased(point); });
    m_window.OnCancel().connect([this] { m_controller.OnCancel(); });
    m_window.OnCanvasResized().connect([this](const gfx::Size size) { m_controller.OnCanvasResized(size); });

    m_window.OnListRowPressed().connect(
        [this](const std::optional<std::size_t> row, const bool toggle) {
            m_controller.OnListRowPressed(row, toggle);
        });
    m_window.OnListRowMoved().connect(
        [this](const std::optional<std::size_t> row) { m_controller.OnListRowMoved(row); });
    m_window.OnListRowReleased().connect([this] { m_controller.OnListRowReleased(); });
    m_window.OnListScrolled().connect([this](const double steps) { m_controller.OnListScrolled(steps); });

    m_window.OnPromptAccepted().connect([this](const FilePromptMode mode, const std::u32string path) {
        const bool success = m_controller.HandleFilePrompt(mode, std::filesystem::path{path});
        if (!success) {
            return;
        }

        UpdateTitle();
        m_window.SetStatusMessage(mode == FilePromptMode::Open ? "Документ загружен" : "Документ сохранён");
    });
    m_window.OnPromptCancelled().connect([this] { m_window.SetStatusMessage("Работа с файлом отменена"); });

    m_controller.OnSaveFileRequested().connect([this] {
        m_window.ShowFilePrompt(FilePromptMode::Save, "Сохранить документ", CurrentFileName());
    });
}

void editor::EditorSession::UpdateTitle() {
    const auto& path = m_controller.GetFilePath();
    if (path.empty()) {
        m_window.SetTitle(std::string{windowTitle});
        return;
    }

    m_window.SetTitle(std::string{windowTitle} + " - " + path.string());
}

std::u32string editor::EditorSession::CurrentFileName() const {
    const auto& path = m_controller.GetFilePath();
    return path.empty() ? std::u32string{defaultFileName} : path.u32string();
}
