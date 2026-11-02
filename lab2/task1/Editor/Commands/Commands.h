#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../../Graphics/Color/Color.h"
#include "../../Graphics/Geometry/Rect.h"
#include "../../Picture/model/Picture.h"
#include "ICommand.h"

namespace editor {
    class AddShapeCommand : public ICommand {
    public:
        AddShapeCommand(shapes::Picture& document, std::string_view type, gfx::Rect bounds, gfx::Color color);

        void Execute() override;
        void Unexecute() override;

        [[nodiscard]] const std::string& GetShapeId() const;

    private:
        shapes::Picture& m_document;
        std::string m_type;
        gfx::Rect m_bounds;
        gfx::Color m_color;
        std::string m_id;
        bool m_added = false;
    };

    class DeleteShapeCommand : public ICommand {
    public:
        explicit DeleteShapeCommand(shapes::Picture& document, std::vector<std::string> ids);

        void Execute() override;
        void Unexecute() override;

    private:
        struct Snapshot {
            std::string id;
            gfx::Color color;
            std::string type;
            gfx::Rect bounds;
            std::size_t index = 0;
        };

        shapes::Picture& m_document;
        std::vector<std::string> m_ids;
        std::vector<Snapshot> m_snapshots;
    };

    class MoveShapesCommand : public ICommand {
    public:
        MoveShapesCommand(shapes::Picture& document, std::vector<std::string> ids,
                          std::vector<gfx::Rect> startBounds, std::vector<gfx::Rect> bounds);

        void Execute() override;
        void Unexecute() override;

    private:
        shapes::Picture& m_document;
        std::vector<std::string> m_ids;
        std::vector<gfx::Rect> m_startBounds;
        std::vector<gfx::Rect> m_bounds;
    };

    class ResizeShapesCommand : public ICommand {
    public:
        ResizeShapesCommand(shapes::Picture& document, std::vector<std::string> ids,
                            std::vector<gfx::Rect> startBounds, std::vector<gfx::Rect> bounds);

        void Execute() override;
        void Unexecute() override;

    private:
        shapes::Picture& m_document;
        std::vector<std::string> m_ids;
        std::vector<gfx::Rect> m_startBounds;
        std::vector<gfx::Rect> m_bounds;
    };

    class ChangeColorCommand : public ICommand {
    public:
        ChangeColorCommand(shapes::Picture& document, std::vector<std::string> ids, gfx::Color newColor);

        void Execute() override;
        void Unexecute() override;

    private:
        shapes::Picture& m_document;
        std::vector<std::string> m_ids;
        std::vector<gfx::Color> m_oldColors;
        gfx::Color m_newColor;
        bool m_executed = false;
    };

    class ReorderShapesCommand : public ICommand {
    public:
        ReorderShapesCommand(shapes::Picture& document, std::string id, std::size_t fromIndex, std::size_t toIndex);

        void Execute() override;
        void Unexecute() override;

    private:
        shapes::Picture& m_document;
        std::string m_id;
        std::size_t m_fromIndex;
        std::size_t m_toIndex;
    };
}
