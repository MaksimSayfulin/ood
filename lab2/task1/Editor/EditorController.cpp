#include "EditorController.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "../Picture/model/DocumentSerializer.h"
#include "../Shape/model/ShapeFactory.h"
#include "EditorLayout.h"
#include "SelectionFrame.h"

editor::EditorController::EditorController(shapes::Picture& document) : m_document(document) {
}

void editor::EditorController::OnCommand(const UiCommand command) {
    m_lastError.clear();

    switch (command) {
    case UiCommand::Select:
        SetTool(Tool::Select);
        break;
    case UiCommand::DrawRectangle:
        SetTool(Tool::Rectangle);
        break;
    case UiCommand::DrawEllipse:
        SetTool(Tool::Ellipse);
        break;
    case UiCommand::DrawTriangle:
        SetTool(Tool::Triangle);
        break;
    case UiCommand::DeleteShape:
        DeleteSelected();
        break;
    case UiCommand::NewDocument:
        NewDocument();
        break;
    case UiCommand::OpenDocument:
        m_openFileRequested();
        break;
    case UiCommand::SaveDocument:
        if (m_filePath.empty()) {
            m_saveFileRequested();
        } else {
            SaveDocument();
        }
        break;
    }
}

void editor::EditorController::OnPointerPressed(const gfx::Point& point, const bool toggle) {
    m_lastError.clear();

    if (m_create) {
        return;
    }

    if (m_tool != Tool::Select) {
        m_create = CreateGesture{point, point};
        return;
    }

    if (const auto hit = HitTestSelectedHandle(point); hit.has_value()) {
        m_resize = MakeResizeGesture(*hit, point);
        return;
    }

    if (HitTestSelectionFrame(point)) {
        m_move = MakeMoveGesture(point);
        return;
    }

    if (const shapes::IShape* shape = m_document.FindShapeAt(point.x, point.y); shape != nullptr) {
        const auto id = shape->GetId();

        if (toggle) {
            ToggleSelection(id);
            if (!IsSelected(id)) {
                CancelGesture();
                return;
            }
        } else if (!IsSelected(id)) {
            SelectOnly(id);
        }

        m_move = MakeMoveGesture(point);
        if (!toggle) {
            m_move->pressedShapeId = id;
        }
        return;
    }

    if (!toggle) {
        ClearSelection();
    }

    CancelGesture();
}

void editor::EditorController::OnPointerMoved(const gfx::Point& point, const bool disableSnap) {
    constexpr double overhang = SelectionFrame::overhang;
    const bool nearCanvas = point.x >= -overhang && point.y >= -overhang
        && point.x <= m_canvasSize.width + overhang && point.y <= m_canvasSize.height + overhang;

    const bool interactive = !m_selectedIds.empty() && !m_resize && !m_move && nearCanvas;
    const auto hit = interactive ? HitTestSelectedHandle(point) : std::nullopt;
    m_hoveredHandle = hit.has_value() ? hit->handle : ResizeHandle::None;

    if (m_resize) {
        ApplyResize(point, disableSnap);
        return;
    }

    if (m_create) {
        m_create->current = point;
        return;
    }

    if (m_move) {
        ApplyMove(point, disableSnap);
    }
}

void editor::EditorController::OnPointerReleased(const gfx::Point&) {
    if (m_create) {
        CommitCreatedShape();
    }

    if (m_move && !m_move->moved && m_move->pressedShapeId.has_value() && m_selectedIds.size() > 1) {
        SelectOnly(*m_move->pressedShapeId);
    }

    CancelGesture();
}

void editor::EditorController::OnCancel() {
    SetTool(Tool::Select);
    CancelGesture();
}

void editor::EditorController::OnCanvasResized(const gfx::Size size) {
    m_canvasSize = size;
}

void editor::EditorController::OnListRowPressed(const std::optional<std::size_t> row, const bool toggle) {
    m_lastError.clear();

    if (!row.has_value()) {
        if (!toggle) {
            ClearSelection();
        }
        return;
    }

    const auto id = ShapeIdAtRow(*row);
    if (!id.has_value()) {
        return;
    }

    if (toggle) {
        ToggleSelection(*id);
    } else {
        SelectOnly(*id);
    }

    m_listDraggedId = id;
}

void editor::EditorController::OnListRowMoved(const std::optional<std::size_t> row) const {
    if (!m_listDraggedId.has_value() || !row.has_value()) {
        return;
    }

    const auto count = m_document.GetShapesCount();
    if (count == 0) {
        return;
    }

    const auto target = count - 1 - *row;
    m_document.MoveShapeToIndex(*m_listDraggedId, target);
}

void editor::EditorController::OnListRowReleased() {
    m_listDraggedId.reset();
}

void editor::EditorController::OnListScrolled(const double steps) {
    m_listScroll = std::clamp(m_listScroll + steps * layout::shapeListRowHeight, 0.0, GetListMaxScroll());
}

editor::Tool editor::EditorController::GetTool() const {
    return m_tool;
}

std::optional<std::string> editor::EditorController::GetSelectedId() const {
    if (m_selectedIds.empty()) {
        return std::nullopt;
    }
    return m_selectedIds.back();
}

const std::vector<std::string> &editor::EditorController::GetSelectedIds() const {
    return m_selectedIds;
}

std::optional<gfx::Rect> editor::EditorController::GetSelectionBounds() const {
    if (m_selectedIds.empty()) {
        return std::nullopt;
    }

    std::vector<gfx::Rect> bounds;
    bounds.reserve(m_selectedIds.size());
    for (const auto& id : m_selectedIds) {
        bounds.push_back(m_document.GetShapeBounds(id));
    }

    return GroupBounds(bounds);
}

std::optional<gfx::Rect> editor::EditorController::GetDraftBounds() const {
    if (!m_create) {
        return std::nullopt;
    }

    return ConstrainToCanvas(Normalize(m_create->anchor, m_create->current));
}

editor::ResizeHandle editor::EditorController::GetHoveredHandle() const {
    return m_hoveredHandle;
}

double editor::EditorController::GetListScroll() const {
    return m_listScroll;
}

std::size_t editor::EditorController::GetShapesCount() const {
    return m_document.GetShapesCount();
}

const std::filesystem::path &editor::EditorController::GetFilePath() const {
    return m_filePath;
}

const std::string &editor::EditorController::GetLastError() const {
    return m_lastError;
}

void editor::EditorController::NewDocument() {
    m_document.Clear();
    m_filePath.clear();
    m_lastError.clear();
    ClearSelection();
    CancelGesture();
    SetTool(Tool::Select);
}

bool editor::EditorController::LoadDocument(const std::filesystem::path& path) {
    if (path.empty()) {
        m_lastError = "Не выбран файл для загрузки";
        return false;
    }

    try {
        const auto shapes = shapes::DocumentSerializer::LoadFromFile(path);

        m_document.ReplaceWith(shapes);
        m_filePath = path;
        m_lastError.clear();
    } catch (const shapes::DocumentError& error) {
        m_lastError = error.what();
        return false;
    }

    ClearSelection();
    CancelGesture();
    return true;
}

bool editor::EditorController::SaveDocument(const std::filesystem::path& path) {
    if (path.empty()) {
        m_lastError = "Не выбран файл для сохранения";
        return false;
    }

    try {
        shapes::DocumentSerializer::SaveToFile(m_document.GetShapes(), path);
        m_filePath = path;
        m_lastError.clear();
    } catch (const shapes::DocumentError& error) {
        m_lastError = error.what();
        return false;
    }

    return true;
}

bool editor::EditorController::SaveDocument() {
    return SaveDocument(m_filePath);
}

bool editor::EditorController::HandleFilePrompt(const FilePromptMode mode, const std::filesystem::path& path) {
    return mode == FilePromptMode::Open ? LoadDocument(path) : SaveDocument(path);
}

boost::signals2::signal<void()> &editor::EditorController::OnOpenFileRequested() {
    return m_openFileRequested;
}

boost::signals2::signal<void()> &editor::EditorController::OnSaveFileRequested() {
    return m_saveFileRequested;
}

void editor::EditorController::SetTool(const Tool tool) {
    m_tool = tool;
    CancelGesture();
}

void editor::EditorController::SelectOnly(const std::string& id) {
    m_selectedIds.clear();
    m_selectedIds.push_back(id);
}

void editor::EditorController::ToggleSelection(const std::string& id) {
    const auto it = std::find(m_selectedIds.begin(), m_selectedIds.end(), id);
    if (it != m_selectedIds.end()) {
        m_selectedIds.erase(it);
    } else {
        m_selectedIds.push_back(id);
    }
}

void editor::EditorController::ClearSelection() {
    m_selectedIds.clear();
    CancelGesture();
}

void editor::EditorController::DeleteSelected() {
    if (m_selectedIds.empty()) {
        return;
    }

    const auto ids = m_selectedIds;
    m_selectedIds.clear();
    CancelGesture();

    for (const auto& id : ids) {
        m_document.DeleteShape(id);
    }
}

void editor::EditorController::CancelGesture() {
    m_move.reset();
    m_resize.reset();
    m_create.reset();
    m_activeGuides.clear();
}

std::vector<gfx::Rect> editor::EditorController::ForeignBounds() const {
    std::vector<gfx::Rect> bounds;

    for (const auto& shape : m_document.GetShapes()) {
        if (IsSelected(shape.GetId())) {
            continue;
        }
        bounds.push_back(shape.GetBounds());
    }

    return bounds;
}

editor::SnapEngine::Snap editor::EditorController::SnapTo(const gfx::Rect& bounds, const int edges) const {
    return SnapEngine::Find(bounds, ForeignBounds(), edges);
}

int editor::EditorController::SnapEdgesForHandle(const ResizeHandle handle) {
    using SnapEdge = SnapEngine::SnapEdge;

    switch (handle) {
    case ResizeHandle::TopLeft: return SnapEdge::Left | SnapEdge::Top;
    case ResizeHandle::Top: return SnapEdge::Top;
    case ResizeHandle::TopRight: return SnapEdge::Right | SnapEdge::Top;
    case ResizeHandle::Right: return SnapEdge::Right;
    case ResizeHandle::BottomRight: return SnapEdge::Right | SnapEdge::Bottom;
    case ResizeHandle::Bottom: return SnapEdge::Bottom;
    case ResizeHandle::BottomLeft: return SnapEdge::Left | SnapEdge::Bottom;
    case ResizeHandle::Left: return SnapEdge::Left;
    case ResizeHandle::None: break;
    }

    return 0;
}

const std::vector<editor::GuideLine> &editor::EditorController::GetActiveGuides() const {
    return m_activeGuides;
}

void editor::EditorController::AddShape(const gfx::Rect& bounds, const Tool tool) {
    const auto shape = shapes::ShapeFactory::CreateShape(
        ToShapeType(tool), ConstrainToCanvas(bounds), gfx::Color{std::string{DefaultColorFor(tool)}});

    m_document.AddShape(shape);

    SelectOnly(shape.GetId());
}

std::optional<editor::EditorController::HandleHit>
editor::EditorController::HitTestSelectedHandle(const gfx::Point& point) const {
    std::optional<HandleHit> hit;
    double bestDistance = 0;

    for (const auto& id : m_selectedIds) {
        const auto bounds = m_document.GetShapeBounds(id);
        const auto handle = SelectionFrame::HitTestHandle(bounds, point);
        if (handle == ResizeHandle::None) {
            continue;
        }

        const auto center = SelectionFrame::GetHandleCenter(bounds, handle);
        const double distance = std::hypot(point.x - center.x, point.y - center.y);
        if (!hit.has_value() || distance < bestDistance) {
            hit = HandleHit{id, handle};
            bestDistance = distance;
        }
    }

    return hit;
}

bool editor::EditorController::HitTestSelectionFrame(const gfx::Point& point) const {
    for (const auto& id : m_selectedIds) {
        if (SelectionFrame::HitTestFrame(m_document.GetShapeBounds(id), point)) {
            return true;
        }
    }
    return false;
}

bool editor::EditorController::IsSelected(const std::string& id) const {
    return std::ranges::find(m_selectedIds, id) != m_selectedIds.end();
}

editor::EditorController::ResizeGesture editor::EditorController::MakeResizeGesture(const HandleHit& hit,
                                                                                const gfx::Point& point) const {
    ResizeGesture gesture;
    gesture.handle = hit.handle;
    gesture.id = hit.id;
    gesture.startBounds = m_document.GetShapeBounds(hit.id);
    gesture.startPointer = point;

    for (const auto& id : m_selectedIds) {
        if (id == hit.id) {
            continue;
        }
        gesture.otherIds.push_back(id);
        gesture.otherBounds.push_back(m_document.GetShapeBounds(id));
    }

    return gesture;
}

editor::EditorController::MoveGesture editor::EditorController::MakeMoveGesture(const gfx::Point& point) const {    MoveGesture gesture;

    for (const auto& id : m_selectedIds) {
        gesture.ids.push_back(id);
        gesture.startBounds.push_back(m_document.GetShapeBounds(id));
    }

    const auto group = GroupBounds(gesture.startBounds);
    gesture.grabOffset = {point.x - group.x, point.y - group.y};

    return gesture;
}

void editor::EditorController::ApplyMove(const gfx::Point& point, const bool disableSnap) {
    if (!m_move || m_move->ids.empty()) {
        return;
    }

    const auto group = GroupBounds(m_move->startBounds);
    gfx::Point delta{point.x - m_move->grabOffset.x - group.x, point.y - m_move->grabOffset.y - group.y};

    m_activeGuides.clear();

    if (!disableSnap) {
        gfx::Rect current = group;
        current.x += delta.x;
        current.y += delta.y;

        const auto [dx, dy, guides] = SnapTo(current); // Как можно эффективней
        delta.x += dx;
        delta.y += dy;
        m_activeGuides = guides;
    }

    if (group.x + delta.x < 0) {
        delta.x = -group.x;
    }
    if (group.y + delta.y < 0) {
        delta.y = -group.y;
    }
    if (group.Right() + delta.x > m_canvasSize.width) {
        delta.x = m_canvasSize.width - group.Right();
    }
    if (group.Bottom() + delta.y > m_canvasSize.height) {
        delta.y = m_canvasSize.height - group.Bottom();
    }

    if (delta.x != 0 || delta.y != 0) {
        m_move->moved = true;
    }

    for (std::size_t i = 0; i < m_move->ids.size(); ++i) {
        auto bounds = m_move->startBounds[i];
        bounds.x += delta.x;
        bounds.y += delta.y;

        m_document.SetShapeBounds(m_move->ids[i], ConstrainToCanvas(bounds)); // Сделать сохраннеи в модель оптимальнее
    }
}

void editor::EditorController::ApplyResize(const gfx::Point& point, const bool disableSnap) {
    if (!m_resize || !IsSelected(m_resize->id)) {
        return;
    }

    const gfx::Point delta{point.x - m_resize->startPointer.x, point.y - m_resize->startPointer.y};

    auto bounds = SelectionFrame::ApplyHandle(m_resize->startBounds, m_resize->handle, delta,
                                              minShapeSize, m_canvasSize);

    m_activeGuides.clear();

    if (!disableSnap) {
        const auto snap = SnapTo(bounds, SnapEdgesForHandle(m_resize->handle));
        bounds.x += snap.dx;
        bounds.y += snap.dy;
        m_activeGuides = snap.guides;
        bounds = ConstrainToCanvas(bounds);
    }

    m_document.SetShapeBounds(m_resize->id, bounds);

    if (m_resize->startBounds.width <= 0 || m_resize->startBounds.height <= 0) {
        return;
    }

    const double scaleX = bounds.width / m_resize->startBounds.width;
    const double scaleY = bounds.height / m_resize->startBounds.height;

    for (std::size_t i = 0; i < m_resize->otherIds.size(); ++i) {
        const auto& other = m_resize->otherBounds[i];

        const gfx::Rect scaled{m_resize->startBounds.x + (other.x - m_resize->startBounds.x) * scaleX,
                               m_resize->startBounds.y + (other.y - m_resize->startBounds.y) * scaleY,
                               other.width * scaleX,
                               other.height * scaleY};

        m_document.SetShapeBounds(m_resize->otherIds[i], ConstrainToCanvas(scaled));
    }
}

void editor::EditorController::CommitCreatedShape() {
    const auto anchor = m_create->anchor;
    const auto drawn = Normalize(anchor, m_create->current);
    const auto tool = m_tool;

    CancelGesture();

    auto bounds = drawn;
    if (bounds.width < minShapeSize && bounds.height < minShapeSize) {
        AddShape(DefaultBoundsAt(anchor), tool);
        return;
    }

    bounds.width = std::max(bounds.width, minShapeSize);
    bounds.height = std::max(bounds.height, minShapeSize);
    AddShape(ConstrainToCanvas(bounds), tool);
}

gfx::Rect editor::EditorController::DefaultBoundsAt(const gfx::Point& anchor) const {
    return {anchor.x - defaultShapeSize.width / 2,
            anchor.y - defaultShapeSize.height / 2,
            defaultShapeSize.width,
            defaultShapeSize.height};
}

gfx::Rect editor::EditorController::GroupBounds(const std::vector<gfx::Rect>& bounds) const {
    if (bounds.empty()) {
        return {};
    }

    double left = bounds.front().x;
    double top = bounds.front().y;
    double right = bounds.front().Right();
    double bottom = bounds.front().Bottom();

    for (const auto& bound : bounds) {
        left = std::min(left, bound.x);
        top = std::min(top, bound.y);
        right = std::max(right, bound.Right());
        bottom = std::max(bottom, bound.Bottom());
    }

    return {left, top, right - left, bottom - top};
}

gfx::Rect editor::EditorController::Normalize(const gfx::Point& anchor, const gfx::Point& point) const {
    return {std::min(anchor.x, point.x), std::min(anchor.y, point.y),
            std::abs(point.x - anchor.x), std::abs(point.y - anchor.y)};
}

gfx::Rect editor::EditorController::ConstrainToCanvas(gfx::Rect bounds) const {
    bounds.width = std::clamp(bounds.width, 0.0, m_canvasSize.width);
    bounds.height = std::clamp(bounds.height, 0.0, m_canvasSize.height);
    bounds.x = std::clamp(bounds.x, 0.0, m_canvasSize.width - bounds.width);
    bounds.y = std::clamp(bounds.y, 0.0, m_canvasSize.height - bounds.height);

    return bounds;
}

std::optional<std::string> editor::EditorController::ShapeIdAtRow(const std::size_t row) const {
    const auto count = m_document.GetShapesCount();
    if (row >= count) {
        return std::nullopt;
    }

    return m_document.GetShapes()[count - 1 - row].GetId();
}

double editor::EditorController::GetListMaxScroll() const {
    const double visible = std::max(0.0, m_canvasSize.height - layout::shapeListHeaderHeight);
    const double content = static_cast<double>(m_document.GetShapesCount()) * layout::shapeListRowHeight;

    return std::max(0.0, content - visible);
}
