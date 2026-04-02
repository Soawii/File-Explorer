#include "FileExplorerModel.hpp"
#include "../engine/EventBus.hpp"
#include "../util/util.hpp"
#include <algorithm>
#include <iostream>
#include "../engine/sync.hpp"

namespace fs = std::filesystem;

FileExplorerModel::FileExplorerModel() {
    state = State::NORMAL;
    currentPath = FileSystem::VIRTUAL_ROOT;
    refresh();
}

void FileExplorerModel::update() {
    if (state == State::SEARCHING) {
        if (FileSystem::searchPQueue.canConsume()) {
            auto pq = FileSystem::searchPQueue.consume();
            entries.clear();

            while (!pq.empty()) {
                const auto SF = pq.top(); 
                pq.pop();
                std::optional<File> opt = FileSystem::existingPathToFile(SF.path);
                if (opt)
                    entries.emplace_back(std::move(*opt));
            }
            std::reverse(entries.begin(), entries.end());

            EventBus::publish("refresh_search");
        }
        if (FileSystem::searchActiveTasks == 0) {
            state = State::NORMAL;
        }
    }
}

void FileExplorerModel::search(const std::filesystem::path& file) {
    if (file.wstring().size() < 2)
        return;
    state = State::SEARCHING;
    FileSystem::searchForFileInDirectory(file, currentPath);
    EventBus::publish("search_start");
}

void FileExplorerModel::setPath(const std::filesystem::path& path) {
    if (state == State::NORMAL && path == currentPath)
        return;

    const std::filesystem::path oldPath = currentPath;
    try {
        if (path != FileSystem::VIRTUAL_ROOT) {
            std::error_code ec;

            bool exists = fs::exists(path, ec);
            if (ec || !exists) {
                std::wcout << "Path does not exist or is inaccessible: " << path.wstring() << '\n';
                return;
            }

            bool isDir = fs::is_directory(path, ec);
            if (ec || !isDir) {
                std::wcout << "Path is not a directory or is inaccessible: " << path.wstring() << '\n';
                return;
            }
        }
        currentPath = path;
        refresh();
    }
    catch (std::exception& e) {
        std::cout << e.what() << '\n';
        if (currentPath != oldPath) {
            currentPath = oldPath;
            refresh();
        }
    }
}

void FileExplorerModel::goUp() {
    setPath(FileSystem::getParentPath(currentPath));
}

void FileExplorerModel::sortEntriesNoRefresh(FileField field, bool ascending) {
    std::sort(entries.begin(), entries.end(), [field, ascending](const File& a, const File& b) {
        if (field == FileField::NAME) {
            return ascending ? a.name < b.name : a.name > b.name;
        }
        else if (field == FileField::SIZE) {
            return ascending ? a.bytes < b.bytes : a.bytes > b.bytes;
        }
        else if (field == FileField::LAST_WRITE_TIME) {
            return ascending ? a.last_write_time < b.last_write_time : a.last_write_time > b.last_write_time;
        }
        if (a.is_directory != b.is_directory)
            return a.is_directory > b.is_directory;
        return ascending ? a.name < b.name : a.name > b.name;
    });
}

void FileExplorerModel::sortEntries(FileField field, bool ascending) {
    sortEntriesNoRefresh(field, ascending);
    EventBus::publish("sort");
}

void FileExplorerModel::refresh() {
    if (state == State::SEARCHING) {
        state = State::NORMAL;
        FileSystem::searchThreadPool.stopNow();
    }

    entries = FileSystem::getDirectoryContents(currentPath);

    fullPath.clear();
    std::filesystem::path temp = currentPath;
    while (true) {
        bool has_parent = FileSystem::hasParent(temp);
        std::filesystem::path p = FileSystem::getParentPath(temp);
        fullPath.emplace_back(std::move(temp));
        if (!has_parent)
            break;
        temp = std::move(p);
    }
    std::reverse(fullPath.begin(), fullPath.end());

    sortEntriesNoRefresh(FileField::FIELD_AMOUNT, true);

    EventBus::publish("refresh");
}

OperationResult FileExplorerModel::rename(size_t file_id, const std::filesystem::path& new_name) {
    if (new_name.has_parent_path())
        return OperationResult::Failure;

    const std::filesystem::path new_path = currentPath / new_name;
    for (int i = 0; i < entries.size(); i++) {
        if (entries[i].id == file_id) {
            OperationResult res = FileSystem::rename(entries[i].path, new_path);
            if (res != OperationResult::Success)
                return res;
            std::optional<File> opt = FileSystem::existingPathToFile(new_path);
            if (opt) {
                entries[i] = std::move(*opt);
                EventBus::publish("rename", std::to_string(file_id) + "," + std::to_string(entries[i].id));
                return OperationResult::Success;
            }
            std::cerr << "Something went wrong with renaming the file." << std::endl;
            return OperationResult::Failure;
        }
    }
    return OperationResult::Failure;
}

OperationResult FileExplorerModel::remove(size_t file_id) {
    for (int i = 0; i < entries.size(); i++) {
        if (entries[i].id == file_id) {
            OperationResult res = FileSystem::remove(entries[i].path);
            if (res != OperationResult::Success)
                return res;
            entries.erase(entries.begin() + i);
            EventBus::publish("refresh", "no_scroll_reset");
            return OperationResult::Success;
        }
    }
    return OperationResult::Failure;
}

OperationResult FileExplorerModel::create(const std::filesystem::path& name, bool is_directory) {
    if (name.has_parent_path())
        return OperationResult::Failure;

    const std::filesystem::path new_path = currentPath / name;

    OperationResult res = FileSystem::create(new_path, is_directory);
    if (res != OperationResult::Success)
        return res;
    
    std::optional<File> opt = FileSystem::existingPathToFile(new_path);
    if (opt) {
        entries.emplace_back(std::move(*opt));
        sortEntriesNoRefresh(FileField::FIELD_AMOUNT, true);
        EventBus::publish("refresh", "no_scroll_reset");
        return OperationResult::Success;
    }

    std::cerr << "Something went wrong with creating the file." << std::endl;
    return OperationResult::Failure;
}

const std::vector<std::filesystem::path>& FileExplorerModel::getFullPath() const {
    return fullPath;
}

const std::vector<File>& FileExplorerModel::getEntries() const {
    return entries;
}

const std::filesystem::path& FileExplorerModel::getCurrentPath() const {
    return currentPath;
}