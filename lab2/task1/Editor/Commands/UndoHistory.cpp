#include "UndoHistory.h"

void editor::UndoHistory::Execute(std::unique_ptr<ICommand> command) {
    m_redo.clear();
    m_undo.push_back(std::move(command));
}

void editor::UndoHistory::Undo() {
    if (m_undo.empty()) {
        return;
    }

    auto command = std::move(m_undo.back());
    m_undo.pop_back();
    command->Unexecute();
    m_redo.push_back(std::move(command));
}

void editor::UndoHistory::Redo() {
    if (m_redo.empty()) {
        return;
    }

    auto command = std::move(m_redo.back());
    m_redo.pop_back();
    command->Execute();
    m_undo.push_back(std::move(command));
}

void editor::UndoHistory::Clear() {
    m_undo.clear();
    m_redo.clear();
}

bool editor::UndoHistory::CanUndo() const {
    return !m_undo.empty();
}

bool editor::UndoHistory::CanRedo() const {
    return !m_redo.empty();
}
