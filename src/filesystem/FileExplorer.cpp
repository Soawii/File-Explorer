#include "FileExplorer.hpp"

FileExplorer::FileExplorer(ui::UIManager* ui)
: m_model(), m_view(ui, m_model) {}