#pragma once

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

namespace Framework
{
    // Tiny name=value properties loader.
    //
    // Supported format:
    //   name=value
    //   # comment
    //   ; comment
    //   // comment
    //
    // Values are kept as strings and converted by typed getters.
    class PropertiesFile
    {
    public:
        bool Load(const std::filesystem::path& path)
        {
            m_values.clear();
            m_loadedPath.clear();

            std::ifstream file(path);
            if (!file.is_open())
                return false;

            m_loadedPath = path;

            std::string line;
            while (std::getline(file, line))
            {
                StripComment(line);
                line = Trim(line);

                if (line.empty())
                    continue;

                const std::size_t equals = line.find('=');
                if (equals == std::string::npos)
                    continue;

                std::string key = Trim(line.substr(0, equals));
                std::string value = Trim(line.substr(equals + 1));

                if (!key.empty())
                    m_values[key] = value;
            }

            return true;
        }

        bool LoadFirstFound(const std::vector<std::filesystem::path>& candidates)
        {
            for (const auto& candidate : candidates)
            {
                if (Load(candidate))
                    return true;
            }

            return false;
        }

        bool Has(const std::string& key) const
        {
            return m_values.find(key) != m_values.end();
        }

        std::string GetString(const std::string& key, const std::string& defaultValue = {}) const
        {
            const auto it = m_values.find(key);
            return it != m_values.end() ? it->second : defaultValue;
        }

        int GetInt(const std::string& key, int defaultValue) const
        {
            const auto value = GetString(key);
            if (value.empty())
                return defaultValue;

            try
            {
                return std::stoi(value);
            }
            catch (...)
            {
                return defaultValue;
            }
        }

        float GetFloat(const std::string& key, float defaultValue) const
        {
            const auto value = GetString(key);
            if (value.empty())
                return defaultValue;

            try
            {
                return std::stof(value);
            }
            catch (...)
            {
                return defaultValue;
            }
        }

        bool GetBool(const std::string& key, bool defaultValue) const
        {
            std::string value = ToLower(GetString(key));
            if (value.empty())
                return defaultValue;

            if (value == "true" || value == "yes" || value == "on" || value == "1")
                return true;

            if (value == "false" || value == "no" || value == "off" || value == "0")
                return false;

            return defaultValue;
        }

        const std::filesystem::path& LoadedPath() const { return m_loadedPath; }

        static std::filesystem::path ExecutableDirectory()
        {
            wchar_t path[MAX_PATH] = {};
            GetModuleFileNameW(nullptr, path, MAX_PATH);
            return std::filesystem::path(path).parent_path();
        }

    private:
        static std::string Trim(const std::string& text)
        {
            const auto first = std::find_if_not(text.begin(), text.end(), [](unsigned char c) { return std::isspace(c); });
            if (first == text.end())
                return {};

            const auto last = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char c) { return std::isspace(c); }).base();
            return std::string(first, last);
        }

        static std::string ToLower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return value;
        }

        static void StripComment(std::string& line)
        {
            const std::size_t hash = line.find('#');
            const std::size_t semi = line.find(';');
            const std::size_t slash = line.find("//");

            std::size_t cut = std::string::npos;
            for (const std::size_t pos : { hash, semi, slash })
            {
                if (pos != std::string::npos)
                    cut = cut == std::string::npos ? pos : std::min(cut, pos);
            }

            if (cut != std::string::npos)
                line.erase(cut);
        }

        std::unordered_map<std::string, std::string> m_values;
        std::filesystem::path m_loadedPath;
    };
}
