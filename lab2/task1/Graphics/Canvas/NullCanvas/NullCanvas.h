#pragma once

#include "../ICanvas/ICanvas.h"

namespace gfx {
    class NullCanvas : public ICanvas {
    public:
        void LineTo(double, double) override {}
        void Fill() override {}
        void FillEllipse(double, double, double, double) override {}
        void DrawEllipse(double, double, double, double) override {}
        void DrawText(double, double, double, std::string) override {}
    };
}