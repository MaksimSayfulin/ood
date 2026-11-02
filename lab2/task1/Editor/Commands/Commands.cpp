#include "Commands.h"

#include <algorithm>

#include "../../Shape/model/ShapeFactory.h"

namespace {
    void ApplyBounds(shapes::Picture& document, const std::vector<std::string>& ids, const std::vector<gfx::Rect>& bounds) {
        for (std::size_t i = 0; i < ids.size(); ++i) {
            document.SetShapeBounds(ids[i], bounds[i]);
        }
    }
}

editor::AddShapeCommand::AddShapeCommand(shapes::Picture& document, std::string_view type, gfx::Rect bounds, gfx::Color color)
    : m_document(document)
      , m_type(std::string{type})
      , m_bounds(bounds)
      , m_color(std::move(color)) {}

void editor::AddShapeCommand::Execute() {
    if (m_added) {
        return;
    }

    auto geometry = shapes::ShapeFactory::CreateGeometry(m_type, m_bounds);

    if (m_id.empty()) {
        m_id = shapes::ShapeFactory::GenerateShapeId();
    }

    m_document.AddShape(m_id, m_color, std::move(geometry));
    m_added = true;
}

void editor::AddShapeCommand::Unexecute() {
    if (!m_added) {
        return;
    }

    m_document.DeleteShape(m_id);
    m_added = false;
}

const std::string &editor::AddShapeCommand::GetShapeId() const {
    return m_id;
}

editor::DeleteShapeCommand::DeleteShapeCommand(shapes::Picture& document, std::vector<std::string> ids)
    : m_document(document) {
    m_ids = std::move(ids);
}

void editor::DeleteShapeCommand::Execute() {
    m_snapshots.clear();

    for (const auto& id : m_ids) {
        const auto& shape = m_document.GetShape(id);
        m_snapshots.push_back(
            {shape.GetId(), shape.GetColor(), shape.GetGeometry()->GetShapeName(), shape.GetBounds(),
             m_document.GetShapeIndex(id)});
    }

    std::ranges::sort(m_snapshots,
                      [](const Snapshot& left, const Snapshot& right) {
                          return left.index < right.index;
                      });

    for (const auto& snapshot : m_snapshots) {
        m_document.DeleteShape(snapshot.id);
    }
}

void editor::DeleteShapeCommand::Unexecute() {
    for (const auto& snapshot : m_snapshots) {
        m_document.AddShape(snapshot.id, snapshot.color,
                              shapes::ShapeFactory::CreateGeometry(snapshot.type, snapshot.bounds));
    }

    for (const auto& snapshot : m_snapshots) {
        m_document.MoveShapeToIndex(snapshot.id, snapshot.index);
    }
}

editor::MoveShapesCommand::MoveShapesCommand(shapes::Picture& document, std::vector<std::string> ids,
                                             std::vector<gfx::Rect> startBounds, std::vector<gfx::Rect> bounds)
    : m_document(document) {
    m_ids = std::move(ids);
    m_startBounds = std::move(startBounds);
    m_bounds = std::move(bounds);
}

void editor::MoveShapesCommand::Execute() {
    ApplyBounds(m_document, m_ids, m_bounds);
}

void editor::MoveShapesCommand::Unexecute() {
    ApplyBounds(m_document, m_ids, m_startBounds);
}

editor::ResizeShapesCommand::ResizeShapesCommand(shapes::Picture& document, std::vector<std::string> ids,
                                                   std::vector<gfx::Rect> startBounds, std::vector<gfx::Rect> bounds)
    : m_document(document) {
    m_ids = std::move(ids);
    m_startBounds = std::move(startBounds);
    m_bounds = std::move(bounds);
}

void editor::ResizeShapesCommand::Execute() {
    ApplyBounds(m_document, m_ids, m_bounds);
}

void editor::ResizeShapesCommand::Unexecute() {
    ApplyBounds(m_document, m_ids, m_startBounds);
}

editor::ChangeColorCommand::ChangeColorCommand(shapes::Picture& document, std::vector<std::string> ids,
                                                gfx::Color newColor)
    : m_document(document)
      , m_ids(std::move(ids))
      , m_newColor(std::move(newColor)) {}

void editor::ChangeColorCommand::Execute() {
    if (m_executed) {
        return;
    }

    m_oldColors.clear();
    m_oldColors.reserve(m_ids.size());

    for (const auto& id : m_ids) {
        m_oldColors.push_back(m_document.GetShape(id).GetColor());
        m_document.ChangeColor(id, m_newColor);
    }

    m_executed = true;
}

void editor::ChangeColorCommand::Unexecute() {
    if (!m_executed) {
        return;
    }

    for (std::size_t i = 0; i < m_ids.size(); ++i) {
        m_document.ChangeColor(m_ids[i], m_oldColors[i]);
    }

    m_executed = false;
}

editor::ReorderShapesCommand::ReorderShapesCommand(shapes::Picture& document, std::string id, std::size_t fromIndex,
                                                   std::size_t toIndex)
    : m_document(document)
      , m_id(std::move(id))
      , m_fromIndex(fromIndex)
      , m_toIndex(toIndex) {}

void editor::ReorderShapesCommand::Execute() {
    m_document.MoveShapeToIndex(m_id, m_toIndex);
}

void editor::ReorderShapesCommand::Unexecute() {
    m_document.MoveShapeToIndex(m_id, m_fromIndex);
}
