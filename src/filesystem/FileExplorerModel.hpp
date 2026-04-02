#pragma once
#include "FileSystem.hpp"

enum class FileField {
    NAME,
    SIZE,
    LAST_WRITE_TIME,
    FIELD_AMOUNT
};

enum class State {
    NORMAL,
    SEARCHING
};

class FileExplorerModel {
private:
    State                               state;
    std::filesystem::path               currentPath;
    std::vector<File>                   entries;
    std::vector<std::filesystem::path>  fullPath;
public:
    FileExplorerModel();

    void update();

    void search(const std::filesystem::path& file);
    void setPath(const std::filesystem::path& path);
    void goUp();
    void refresh();
    void sortEntries(FileField field, bool ascending);
    OperationResult rename(size_t file_id, const std::filesystem::path& new_name);
    OperationResult remove(size_t file_id);
    OperationResult create(const std::filesystem::path& name, bool is_directory);

    const std::vector<std::filesystem::path>& getFullPath() const;
    const std::vector<File>& getEntries() const;
    const std::filesystem::path& getCurrentPath() const;
private:
    void sortEntriesNoRefresh(FileField field, bool ascending);
};