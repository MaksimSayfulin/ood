#pragma once

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <boost/signals2/signal.hpp>

#include "../../Graphics/Geometry/Rect.h"
#include "../../Picture/model/Picture.h"
#include "../Commands/Commands.h"
#include "EditorTypes.h"
#include "../Geometry/SnapEngine.h"
#include "../Commands/UndoHistory.h"

namespace editor {
    class EditorController {
    public:
        explicit EditorController(shapes::Picture& document);

        void OnCommand(UiCommand command);
        void OnPointerPressed(const gfx::Point& point, bool toggle = false);
        void OnPointerMoved(const gfx::Point& point, bool disableSnap = false);
        void OnPointerReleased(const gfx::Point& point);
        void OnCancel();
        void OnCanvasResized(gfx::Size size);

        void OnListRowPressed(std::optional<std::size_t> row, bool toggle = false);
        void OnListRowMoved(std::optional<std::size_t> row);
        void OnListRowReleased();
        void OnListScrolled(double steps);

        [[nodiscard]] Tool GetTool() const;

        [[nodiscard]] std::optional<std::string> GetSelectedId() const;

        [[nodiscard]] const std::vector<std::string>& GetSelectedIds() const;

        [[nodiscard]] std::optional<gfx::Rect> GetSelectionBounds() const;

        [[nodiscard]] std::optional<gfx::Rect> GetDraftBounds() const;
        [[nodiscard]] ResizeHandle GetHoveredHandle() const;
        [[nodiscard]] double GetListScroll() const;
        [[nodiscard]] std::size_t GetShapesCount() const;

        [[nodiscard]] const std::vector<GuideLine>& GetActiveGuides() const;

        [[nodiscard]] const std::filesystem::path& GetFilePath() const;
        [[nodiscard]] const std::string& GetLastError() const;

        void Undo();
        void Redo();
        [[nodiscard]] bool CanUndo() const;
        [[nodiscard]] bool CanRedo() const;

        void ChangeColor(const gfx::Color& color);

        void NewDocument();
        bool LoadDocument(const std::filesystem::path& path);
        bool SaveDocument(const std::filesystem::path& path);
        bool SaveDocument();

        bool HandleFilePrompt(FilePromptMode mode, const std::filesystem::path& path);

        boost::signals2::signal<void()>& OnOpenFileRequested();
        boost::signals2::signal<void()>& OnSaveFileRequested();

    private:
        struct MoveGesture {
            gfx::Point grabOffset;
            std::vector<std::string> ids;
            std::vector<gfx::Rect> startBounds;
            std::optional<std::string> pressedShapeId;
            bool moved = false;
        };

        struct ResizeGesture {
            ResizeHandle handle;
            std::string id;
            gfx::Rect startBounds;
            gfx::Point startPointer;
            std::vector<std::string> otherIds;
            std::vector<gfx::Rect> otherBounds;
        };

        struct CreateGesture {
            gfx::Point anchor;
            gfx::Point current;
        };

        struct HandleHit {
            std::string id;
            ResizeHandle handle;
        };

        void SetTool(Tool tool);
        void SelectOnly(const std::string& id);
        void ToggleSelection(const std::string& id);
        void ClearSelection();
        void DeleteSelected();
        void CancelGesture();
        void PruneInvalidSelection();

        void AddShape(const gfx::Rect& bounds, Tool tool);

        [[nodiscard]] std::optional<HandleHit> HitTestSelectedHandle(const gfx::Point& point) const;
        [[nodiscard]] bool HitTestSelectionFrame(const gfx::Point& point) const;
        [[nodiscard]] bool IsSelected(const std::string& id) const;

        [[nodiscard]] MoveGesture MakeMoveGesture(const gfx::Point& point) const;
        [[nodiscard]] ResizeGesture MakeResizeGesture(const HandleHit& hit, const gfx::Point& point) const;
        void ApplyMove(const gfx::Point& point, bool disableSnap = false);
        void ApplyResize(const gfx::Point& point, bool disableSnap = false);
        void CommitCreatedShape();
        void CommitMove();
        void CommitResize();

        [[nodiscard]] static gfx::Rect GroupBounds(const std::vector<gfx::Rect>& bounds);
        [[nodiscard]] static gfx::Rect Normalize(const gfx::Point& anchor, const gfx::Point& point);
        [[nodiscard]] gfx::Rect ConstrainToCanvas(gfx::Rect bounds) const;

        [[nodiscard]] std::optional<std::string> ShapeIdAtRow(std::size_t row) const;
        [[nodiscard]] double GetListMaxScroll() const;

        [[nodiscard]] const std::vector<gfx::Rect>& ForeignBounds();
        [[nodiscard]] SnapEngine::Snap SnapTo(const gfx::Rect& bounds, int edges = SnapEngine::allEdges);
        [[nodiscard]] static int SnapEdgesForHandle(ResizeHandle handle);

        shapes::Picture& m_document;

        Tool m_tool = Tool::Select;
        std::vector<std::string> m_selectedIds;
        std::optional<MoveGesture> m_move;
        std::optional<ResizeGesture> m_resize;
        std::optional<CreateGesture> m_create;
        ResizeHandle m_hoveredHandle = ResizeHandle::None;
        gfx::Size m_canvasSize;
        std::filesystem::path m_filePath;
        std::string m_lastError;

        double m_listScroll = 0.0;
        std::optional<std::string> m_listDraggedId;
        std::vector<GuideLine> m_activeGuides;

        std::vector<gfx::Rect> m_foreignBoundsCache;
        bool m_foreignBoundsDirty = true;

        boost::signals2::signal<void()> m_openFileRequested;
        boost::signals2::signal<void()> m_saveFileRequested;

        UndoHistory m_history;
    };
}
