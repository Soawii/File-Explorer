#include "FileSystem.hpp"
#include "../util/util.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <sstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include "../engine/ThreadPool.hpp"
#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

int                                         FileSystem::searchActiveTasks(0);
std::filesystem::path::string_type          FileSystem::searchFileName(fs::path(""));
ThreadPool                                  FileSystem::searchThreadPool(1);
ThreadSafeBoundPriorityQueue<SearchFile>    FileSystem::searchPQueue(30);
const fs::path                              FileSystem::VIRTUAL_ROOT = "/";
std::vector<File>                           FileSystem::rootContents = {};

void FileSystem::init() {
    #ifdef _WIN32
        DWORD mask = GetLogicalDrives();
        for (char letter = 'A'; letter <= 'Z'; ++letter) {
            if (mask & (1 << (letter - 'A'))) {
                fs::path drive(std::string(1, letter) + ":\\");
                std::string formatted_lwt = "";
                try {
                    formatted_lwt = formatLastWriteTime(fs::last_write_time(drive));
                }
                catch (...) {
                    formatted_lwt = "";
                }
                rootContents.emplace_back(drive, drive, 0, formatted_lwt, true, fs::hash_value(drive));
            }
        }
    #else
        for (const fs::directory_entry& entry : fs::directory_iterator("/")) {
            rootContents.emplace_back(entry.path(), entry.path().filename(), 0, formatLastWriteTime(fs::last_write_time(fs::path("/"))), fs::is_directory(entry), fs::hash_value(entry.path()));
        }
    #endif
}

std::optional<File> FileSystem::existingPathToFile(const std::filesystem::path& path) {
    std::optional<File> ret = std::nullopt;

    try {
        const std::filesystem::directory_entry entry(path);
        fs::path name = path.filename();
        size_t size = 0;
        if (entry.is_regular_file()) 
            size = entry.file_size();
        size_t path_hash = fs::hash_value(path);

        ret.emplace(std::move(path), 
                    name, 
                    size, 
                    formatLastWriteTime(entry.last_write_time()), 
                    entry.is_directory(),
                    path_hash);
    }
    catch(const std::filesystem::filesystem_error& e) {}

    return ret;
}

std::optional<File> FileSystem::directoryEntryToFile(const std::filesystem::directory_entry& entry) {
    std::optional<File> ret = std::nullopt;

    try {
        fs::path p = entry.path();
        fs::path name = p.filename();
        size_t size = 0;
        if (entry.is_regular_file())
            size = entry.file_size();
        size_t path_hash = fs::hash_value(p);

        ret.emplace(std::move(p), 
                    std::move(name),
                    size,
                    formatLastWriteTime(entry.last_write_time()),
                    entry.is_directory(),
                    path_hash);
    }
    catch(const std::filesystem::filesystem_error& e) {}

    return ret;
}

bool FileSystem::hasParent(const fs::path& path) {
    return path != VIRTUAL_ROOT;
}

fs::path FileSystem::getParentPath(const fs::path& path) {
    if (path == VIRTUAL_ROOT)
        return VIRTUAL_ROOT;
    const fs::path parent = path.parent_path();
    if (path == parent)
        return VIRTUAL_ROOT;
    return parent;
}

std::vector<File> FileSystem::getDirectoryContents(const fs::path& directory_path) {
    std::vector<File> files;
    files.reserve(32);

    if (directory_path == VIRTUAL_ROOT)
        return rootContents;

    if (!fs::is_directory(directory_path))
        return files;

    for (const fs::directory_entry& entry : fs::directory_iterator(directory_path, fs::directory_options::skip_permission_denied)) {
        std::optional<File> opt = directoryEntryToFile(entry);
        if (opt)
            files.emplace_back(std::move(*opt));
    }

    return files;
}

OperationResult FileSystem::rename(const std::filesystem::path& from, const std::filesystem::path& to) {
    if (fs::exists(to)) {
        return OperationResult::TargetAlreadyExists;
    }
    std::error_code ec;    
    std::filesystem::rename(from, to, ec);

    return ec ? OperationResult::Failure : OperationResult::Success;
}

OperationResult FileSystem::remove(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
    return ec ? OperationResult::Failure : OperationResult::Success;
}

OperationResult FileSystem::create(const std::filesystem::path& path, bool is_directory) {
    if (!path.is_absolute())
        return OperationResult::Failure;
    if (fs::exists(path)) 
        return OperationResult::TargetAlreadyExists;

    if (is_directory) {
        std::error_code ec;
        std::filesystem::create_directory(path, ec);
        return ec ? OperationResult::Failure : OperationResult::Success;
    }

    try {
        std::ofstream ofs(path);
        bool success = bool(ofs);
        ofs.close();
        if (success) 
            return OperationResult::Success;
        return OperationResult::Failure;
    }
    catch (std::exception& e) {
        return OperationResult::Failure;
    }

    return OperationResult::Failure;
}

std::string FileSystem::formatLastWriteTime(const std::filesystem::file_time_type& time) {
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        time - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
    char buf[20];
    std::strftime(buf, sizeof(buf), "%F %T", std::localtime(&cftime));
    return std::string(buf);
}

void FileSystem::searchForFileInDirectory(const fs::path& file_name, const fs::path& directory_path) {
    searchThreadPool.stopNow();
    searchThreadPool.start(1);

    searchFileName = toLower(file_name.native());

    while (!searchPQueue.empty())
        searchPQueue.pop();

    searchActiveTasks = 1;
    searchThreadPool.enqueue([file_name, directory_path]() {
        std::vector<fs::path> root_paths;
        if (directory_path == VIRTUAL_ROOT) {
            for (int i = 0; i < rootContents.size(); i++) {
                root_paths.push_back(rootContents[i].path);
            }
        }
        else {
            root_paths.push_back(directory_path);
        }

        for (const fs::path& path : root_paths) {
            int score = fileMatchScore(path);
            if (score > 0)
                searchPQueue.push(SearchFile(path, score));
            if (fs::is_directory(path)) {
                searchActiveTasks++;
                searchThreadPool.enqueue([path]() {
                    searchForFileInDirectoryHelper(path);
                    searchActiveTasks--;
                });
            }
        }
        searchActiveTasks--;
    });
}

void FileSystem::searchForFileInDirectoryHelper(const std::filesystem::path& directory_path) {
    for (const fs::directory_entry& entry : fs::directory_iterator(directory_path, fs::directory_options::skip_permission_denied)) {
        const fs::path path(entry.path());
        int score = fileMatchScore(path);
        if (entry.is_directory()) {
            searchActiveTasks++;
            searchThreadPool.enqueue([path]() {
                searchForFileInDirectoryHelper(path);
                searchActiveTasks--;
            });
        }
        if (score > 0)
            searchPQueue.push(SearchFile(std::move(path), score));
    }
}

int FileSystem::fileMatchScore(const fs::path& file_to_consider) {
    const auto candidate_lower = toLower(file_to_consider.filename().native());
    
    int score = 0;
    
    if (searchFileName == candidate_lower) {
        return 10000;
    }
    
    size_t pos = candidate_lower.find(searchFileName);
    if (pos == 0) {
        score += 5000;
    }
    else if (pos != std::wstring::npos) {
        score += 2000;
        score += (1000 - pos * 10);
    }
    
    size_t search_idx = 0;
    size_t last_match = 0;
    for (size_t i = 0; i < candidate_lower.size() && search_idx < searchFileName.size(); i++) {
        if (candidate_lower[i] == searchFileName[search_idx]) {
            score += 100;
            if (i == last_match + 1) {
                score += 50;
            }
            last_match = i;
            search_idx++;
        }
    }
    
    if (search_idx != searchFileName.size()) {
        return 0;
    }

    return score;
}

#ifdef _WIN32
std::wstring FileSystem::toLower(const std::wstring& str) {
    std::wstring result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](wchar_t c) { return towlower(c); });
    return result;
}
#else
std::string FileSystem::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return tolower(c); });
    return result;
}
#endif
