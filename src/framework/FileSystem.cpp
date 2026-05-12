#include "FileSystem.h"

namespace Framework
{
    void FileSystem::Initialize(const std::filesystem::path& executablePath,
                                const std::filesystem::path& assetRoot)
    {
        m_executableDirectory = executablePath.parent_path();
        m_searchPaths.clear();

        AddSearchPath(m_executableDirectory / assetRoot);
        AddSearchPath(std::filesystem::current_path() / assetRoot);
        AddSearchPath(m_executableDirectory);
        AddSearchPath(std::filesystem::current_path());
    }

    void FileSystem::AddSearchPath(const std::filesystem::path& path)
    {
        if (path.empty())
            return;

        std::error_code ec;
        const auto canonical = std::filesystem::weakly_canonical(path, ec);
        const auto finalPath = ec ? path : canonical;

        for (const auto& existing : m_searchPaths)
        {
            if (existing == finalPath)
                return;
        }

        m_searchPaths.push_back(finalPath);
    }

    std::filesystem::path FileSystem::Resolve(const std::filesystem::path& relativePath) const
    {
        if (relativePath.is_absolute() && std::filesystem::exists(relativePath))
            return relativePath;

        for (const auto& root : m_searchPaths)
        {
            const auto candidate = root / relativePath;
            if (std::filesystem::exists(candidate))
                return candidate;
        }

        // Return a deterministic fallback even when missing, so callers can log the attempted path.
        if (!m_searchPaths.empty())
            return m_searchPaths.front() / relativePath;

        return relativePath;
    }

    bool FileSystem::Exists(const std::filesystem::path& relativePath) const
    {
        return std::filesystem::exists(Resolve(relativePath));
    }
}
