#pragma once

#include <ostream>
#include <boost/signals2/connection.hpp>
#include "../../Picture/Picture.h"

namespace shapes {

    class ConsolePictureObserver {
    public:
        explicit ConsolePictureObserver(std::ostream& output, Picture& picture)
            : m_output(output)
            , m_picture(picture)
        {
            m_addSubscription = m_picture.OnShapeAdded().connect([this](const IShape& shape) {
                m_output << "AddShape " << shape.GetId() << "\n";
                m_output << "Picture changed. Shapes: " << m_picture.GetShapesCount() << "\n";
            });
            m_removeSubscription = m_picture.OnShapeRemoved().connect([this](const IShape& shape) {
                m_output << "DeleteShape " << shape.GetId() << "\n";
                m_output << "Picture changed. Shapes: " << m_picture.GetShapesCount() << "\n";
            });
            m_moveSubscription = m_picture.OnShapeMoved().connect([this](const IShape& shape, double dx, double dy) {
                m_output << "MoveShape " << shape.GetId() << " " << dx << " " << dy << "\n";
                m_output << "Picture changed. Shapes: " << m_picture.GetShapesCount() << "\n";
            });
            m_colorSubscription = m_picture.OnShapeColorChanged().connect([this](const IShape& shape, const gfx::Color& color) {
                m_output << "ChangeColor " << shape.GetId() << " " << color.GetColor() << "\n";
                m_output << "Picture changed. Shapes: " << m_picture.GetShapesCount() << "\n";
            });
        }

    private:
        std::ostream& m_output;
        Picture& m_picture;

        boost::signals2::scoped_connection m_addSubscription;
        boost::signals2::scoped_connection m_removeSubscription;
        boost::signals2::scoped_connection m_moveSubscription;
        boost::signals2::scoped_connection m_colorSubscription;
    };
}
