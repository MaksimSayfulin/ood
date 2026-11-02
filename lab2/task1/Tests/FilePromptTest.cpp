#include <filesystem>
#include <fstream>
#include <string>

#include "../Catch/Catch.hpp"
#include "../Editor/Controller/EditorController.h"
#include "../Editor/File/FilePrompt.h"
#include "../Graphics/Canvas/CanvasSVG/CanvasSvg.h"
#include "../Shape/model/ShapeFactory.h"

namespace {
    constexpr gfx::Size canvas{800, 600};

    struct Editor {
        std::ostringstream svg;
        gfx::CanvasSvg canvasOutput;
        shapes::Picture document;
        editor::EditorController controller;

        Editor() : canvasOutput(svg), document(canvasOutput), controller(document) {
            controller.OnCanvasResized(canvas);
        }

        void Add(const std::string& id, const std::string& type, const gfx::Rect& bounds) {
            document.AddShape({id, gfx::Color("#4363D8"), shapes::ShapeFactory::CreateGeometry(type, bounds)});
        }
    };
}

TEST_CASE("Диалог файла: запоминает режим и введённый путь") {
    editor::FilePrompt prompt{editor::FilePromptMode::Save, "Сохранить документ", U"document.shapes"};

    REQUIRE(prompt.GetMode() == editor::FilePromptMode::Save);
    REQUIRE(prompt.GetTitle() == "Сохранить документ");
    REQUIRE(prompt.GetPath() == U"document.shapes");
}

TEST_CASE("Диалог файла: ввод символов и удаление работают с путём") {
    editor::FilePrompt prompt{editor::FilePromptMode::Open, "Открыть документ", U""};

    for (const char symbol : std::string{"abc.txt"}) {
        prompt.AppendCodepoint(static_cast<char32_t>(symbol));
    }

    REQUIRE(prompt.GetPath() == U"abc.txt");

    prompt.EraseLastCharacter();
    REQUIRE(prompt.GetPath() == U"abc.tx");

    prompt.EraseLastCharacter();
    prompt.EraseLastCharacter();
    prompt.EraseLastCharacter();
    REQUIRE(prompt.GetPath() == U"abc");
}

TEST_CASE("Диалог файла: путь с кириллицей не ломается при удалении символа") {
    editor::FilePrompt prompt{editor::FilePromptMode::Save, "Сохранить документ", U""};

    prompt.AppendCodepoint(0x0434);
    prompt.AppendCodepoint(0x043E);
    REQUIRE(prompt.GetPath() == U"до");

    prompt.EraseLastCharacter();
    REQUIRE(prompt.GetPath() == U"д");

    prompt.EraseLastCharacter();
    REQUIRE(prompt.GetPath().empty());
}

TEST_CASE("Диалог файла: удаление из пустого пути ничего не ломает") {
    editor::FilePrompt prompt{editor::FilePromptMode::Open, "Открыть документ", U""};

    prompt.EraseLastCharacter();
    REQUIRE(prompt.GetPath().empty());
}

TEST_CASE("Диалог файла: набранный путь с кириллицей становится файлом") {
    editor::FilePrompt prompt{editor::FilePromptMode::Save, "Сохранить документ", U""};

    for (const char32_t symbol : U"документ_проверка.shapes") {
        prompt.AppendCodepoint(symbol);
    }

    const std::filesystem::path path{std::filesystem::temp_directory_path() / prompt.GetPath()};
    std::filesystem::remove(path);

    Editor editor;
    editor.Add("первая", "rectangle", {10, 20, 100, 50});

    REQUIRE(editor.controller.HandleFilePrompt(editor::FilePromptMode::Save, path));
    REQUIRE(std::filesystem::exists(path));

    editor.controller.NewDocument();
    REQUIRE(editor.controller.HandleFilePrompt(editor::FilePromptMode::Open, path));
    REQUIRE(editor.document.GetShapesCount() == 1);
    REQUIRE(editor.document.GetShapes().front().GetId() == "первая");

    std::filesystem::remove(path);
}

TEST_CASE("Файлы: режим диалога определяет, что происходит с документом") {
    const auto path = std::filesystem::temp_directory_path() / "editor_prompt_test.shapes";
    std::filesystem::remove(path);

    Editor editor;
    editor.Add("первая", "rectangle", {10, 20, 100, 50});

    SECTION("Режим сохранения создаёт файл с фигурами") {
        REQUIRE(editor.controller.HandleFilePrompt(editor::FilePromptMode::Save, path));
        REQUIRE(std::filesystem::exists(path));
        REQUIRE(editor.controller.GetFilePath() == path);

        std::ifstream file(path);
        std::string content{std::istreambuf_iterator<char>{file}, {}};
        REQUIRE(content.find("shape rectangle первая 10 20 100 50") != std::string::npos);
    }

    SECTION("Режим открытия загружает файл, а не перезаписывает его") {
        REQUIRE(editor.controller.HandleFilePrompt(editor::FilePromptMode::Save, path));

        editor.controller.NewDocument();
        REQUIRE(editor.document.GetShapesCount() == 0);

        REQUIRE(editor.controller.HandleFilePrompt(editor::FilePromptMode::Open, path));
        REQUIRE(editor.document.GetShapesCount() == 1);
        REQUIRE(editor.document.GetShapes().front().GetId() == "первая");
        REQUIRE(editor.document.GetShapes().front().GetBounds() == (gfx::Rect{10, 20, 100, 50}));
    }

    SECTION("Режим открытия не перезаписывает уже сохранённый файл") {
        REQUIRE(editor.controller.HandleFilePrompt(editor::FilePromptMode::Save, path));

        editor.Add("вторая", "ellipse", {1, 2, 30, 40});
        REQUIRE(editor.controller.HandleFilePrompt(editor::FilePromptMode::Open, path));

        REQUIRE(editor.document.GetShapesCount() == 1);
        REQUIRE(editor.document.GetShapes().front().GetId() == "первая");
    }

    SECTION("Пустой путь не приводит к сохранению и не роняет документ") {
        REQUIRE_FALSE(editor.controller.HandleFilePrompt(editor::FilePromptMode::Save, {}));
        REQUIRE_FALSE(editor.controller.GetLastError().empty());
        REQUIRE(editor.document.GetShapesCount() == 1);
    }

    SECTION("Открытие несуществующего файла оставляет документ в покое") {
        const auto missing = std::filesystem::temp_directory_path() / "editor_missing_test.shapes";
        std::filesystem::remove(missing);

        REQUIRE_FALSE(editor.controller.HandleFilePrompt(editor::FilePromptMode::Open, missing));
        REQUIRE_FALSE(editor.controller.GetLastError().empty());
        REQUIRE(editor.document.GetShapesCount() == 1);
        REQUIRE(editor.document.GetShapes().front().GetId() == "первая");
    }

    std::filesystem::remove(path);
}
