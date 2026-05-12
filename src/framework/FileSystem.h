#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Framework
{
    class FileSystem
    {
    public:
        void Initialize(const std::filesystem::path& executablePath,
                        const std::filesystem::path& assetRoot = L".");

        void AddSearchPath(const std::filesystem::path& path);
        std::filesystem::path Resolve(const std::filesystem::path& relativePath) const;
        bool Exists(const std::filesystem::path& relativePath) const;

        const std::filesystem::path& ExecutableDirectory() const { return m_executableDirectory; }
        const std::vector<std::filesystem::path>& SearchPaths() const { return m_searchPaths; }

    private:
        std::filesystem::path m_executableDirectory;
        std::vector<std::filesystem::path> m_searchPaths;
    };
}
