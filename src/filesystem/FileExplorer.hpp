#pragma once
#include "FileExplorerModel.hpp"
#include "FileExplorerView.hpp"
#include "../UI/UIManager.hpp"

class FileExplorer {
public:
    FileExplorer(ui::UIManager* ui);

    FileExplorerView m_view;
    FileExplorerModel m_model;
};