#pragma once

#include <ostream>
#include "../../Picture/Picture.h"

namespace shapes {

    class ConsolePictureObserver {
    public:
        explicit ConsolePictureObserver(std::ostream& output, Picture& picture)
            : m_output(output)
            , m_picture(picture)
        {
            m_addSubscription = m_picture.SubscribeToShapeAdded([this](const IShape& shape) {
                m_output << "AddShape " << shape.GetId() << "\n";
                m_output << "Picture changed. Shapes: " << m_picture.GetShapesCount() << "\n";
            });
            m_removeSubscription = m_picture.SubscribeToShapeRemoved([this](const IShape& shape) {
                m_output << "DeleteShape " << shape.GetId() << "\n";
                m_output << "Picture changed. Shapes: " << m_picture.GetShapesCount() << "\n";
            });
            m_moveSubscription = m_picture.SubscribeToShapeMoved([this](const IShape& shape, double dx, double dy) {
                m_output << "MoveShape " << shape.GetId() << " " << dx << " " << dy << "\n";
                m_output << "Picture changed. Shapes: " << m_picture.GetShapesCount() << "\n";
            });
            m_colorSubscription = m_picture.SubscribeToShapeColorChanged([this](const IShape& shape, const gfx::Color& color) {
                m_output << "ChangeColor " << shape.GetId() << " " << color.GetColor() << "\n";
                m_output << "Picture changed. Shapes: " << m_picture.GetShapesCount() << "\n";
            });
        }

    private:
        std::ostream& m_output;
        Picture& m_picture;
        EventSubscription<const IShape&> m_addSubscription;
        EventSubscription<const IShape&> m_removeSubscription;
        EventSubscription<const IShape&, double, double> m_moveSubscription;
        EventSubscription<const IShape&, const gfx::Color&> m_colorSubscription;
    };
}