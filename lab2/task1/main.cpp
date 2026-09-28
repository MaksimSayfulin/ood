#include <fstream>

#include "Graphics/Canvas/CanvasSVG/CanvasSvg.h"
#include "InterfaceIO/CommandParser/CommandParser.h"
#include "InterfaceIO/ConsolePictureObserver/ConsolePictureObserver.h"
#include "InterfaceIO/UserInput/UserInput.h"
#include "Picture/Picture.h"

int main() {
    std::ofstream svg("../output/output.svg");
    gfx::CanvasSvg canvas(svg);
    shapes::Picture picture(canvas);
    shapes::ConsolePictureObserver observer(std::cout, picture);
    io::UserInput userInput;
    io::CommandParser parser(picture);

    while (parser.Parse(userInput.ReadInput())) {
    }
}

// Как сделать так, чтобы можно было менять фигуры с одними параметрами
// как запускать тесты не комментируя код
