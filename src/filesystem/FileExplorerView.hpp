#pragma once
#include "../UI/UIManager.hpp"
#include "../UI/UIElement.hpp"
#include "../UI/Flex.hpp"
#include "../UI/TextInput.hpp"
#include "FileExplorerModel.hpp"
#include "Pool.hpp"

enum class ButtonType {
    FILE = 0,
    FILE_DETAILED,
    TYPE_COUNT
};

class FileExplorerView {
public:
    FileExplorerModel& m_model;

    ui::UIManager* m_ui;
    ui::WindowElement* m_win;

    ui::Flex* m_layout;
    ui::Flex* m_top_bar;
    ui::Flex* m_file_path;
    ui::Flex* m_search_bar;
    ui::TextInput* m_search_input;
    ui::UIElement* m_search_button;
    ui::Flex* m_file_window;
    ui::Flex* m_file_options;
    ui::UIElement *m_file_option_rename, *m_file_option_delete; 
    ui::TextInput* m_file_rename_input, *m_new_file_input;

    FileExplorerView(ui::UIManager* ui, FileExplorerModel& model);

    ui::UIElement* createFileButton(const File& f);
    void renameFileButton(ui::UIElement* e, const File& f);

    ui::UIElement* createDetailedFileButton(const File& f);
    void renameDetailedFileButton(ui::UIElement* e, const File& f);

    ui::UIElement* createPathButton(const std::filesystem::path& path, const std::filesystem::path& name);
    void renamePathButton(ui::UIElement* e, const std::filesystem::path& path, const std::filesystem::path& name);

    ui::UIElement* createPathDelimeter();
private:
    Pool<ui::UIElement> m_fileElementPool;
    Pool<ui::UIElement> m_detailedFileElementPool;
    Pool<ui::UIElement> m_pathElementPool;
    Pool<ui::UIElement> m_pathDelmiterPool;

    sf::String m_stringDirectory, m_stringFile;

    std::vector<std::string> fileFields;
    std::vector<std::pair<float, float>> fileFieldsFlexSize;
};