#pragma once

#include <memory>
#include <vector>

#include "ICommand.h"

namespace editor {
    class UndoHistory {
    public:
        void Execute(std::unique_ptr<ICommand> command);
        void Undo();
        void Redo();
        void Clear();

        [[nodiscard]] bool CanUndo() const;
        [[nodiscard]] bool CanRedo() const;

    private:
        std::vector<std::unique_ptr<ICommand>> m_undo;
        std::vector<std::unique_ptr<ICommand>> m_redo;
    };
}
