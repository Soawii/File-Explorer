#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <SFML/System.hpp>
#include "../engine/ThreadPool.hpp"
#include "../util/BoundPriorityQueue.hpp"
#include "../util/ThreadSafeBoundPriorityQueue.hpp"
#include <atomic>
#include <optional>

struct File {
    std::filesystem::path path;
    std::filesystem::path name;
    size_t bytes;
    std::string last_write_time;
    bool is_directory;
    size_t id;

    template <typename PathT, typename NameT, typename TimeT>
    File(PathT&& path, NameT&& name, size_t bytes, TimeT&& last_write_time, bool is_directory, size_t id)
        : path(std::forward<PathT>(path)),
          name(std::forward<NameT>(name)),
          bytes(bytes),
          last_write_time(std::forward<TimeT>(last_write_time)),
          is_directory(is_directory),
          id(id)
    {}
};

enum class OperationResult {
    Success = 0,
    TargetAlreadyExists,
    Failure
};

struct SearchFile {
    std::filesystem::path path;
    int score;

    template <typename PathT>
    SearchFile(PathT&& path, int score)
        : path(std::forward<PathT>(path)),
            score(score)
    {}

    bool operator<(const SearchFile& other) const { return score < other.score; }
    bool operator>(const SearchFile& other) const { return score > other.score; }
};

class FileSystem {
public:
    static std::filesystem::path::string_type searchFileName;
    static ThreadPool searchThreadPool;
    static ThreadSafeBoundPriorityQueue<SearchFile> searchPQueue;
    static int searchActiveTasks;

    static std::vector<File> rootContents;
    static const std::filesystem::path VIRTUAL_ROOT;

    static void init();

    static bool hasParent(const std::filesystem::path& path);
    static std::filesystem::path getParentPath(const std::filesystem::path& path);

    static std::optional<File> existingPathToFile(const std::filesystem::path& path);
    static std::optional<File> directoryEntryToFile(const std::filesystem::directory_entry& entry);

    static std::vector<File> getDirectoryContents(const std::filesystem::path& path);

    static OperationResult rename(const std::filesystem::path& from, const std::filesystem::path& to);
    static OperationResult remove(const std::filesystem::path& path);
    static OperationResult create(const std::filesystem::path& path, bool is_directory);

    static std::string formatLastWriteTime(const std::filesystem::file_time_type& time);

    static void searchForFileInDirectory(const std::filesystem::path& file_name, const std::filesystem::path& directory_path);
private:
    static void searchForFileInDirectoryHelper(const std::filesystem::path& directory_path);

    static int fileMatchScore(const std::filesystem::path& file_to_consider);

    #ifdef _WIN32
    static std::wstring toLower(const std::wstring& str);
    #else
    static std::string toLower(const std::string& str);
    #endif
};