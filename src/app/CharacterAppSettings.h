#pragma once

#include "../framework/PropertiesFile.h"

#include <d3d9.h>
#include <d3dx9.h>

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#include <vector>

struct CharacterAppSettings
{
    std::string windowTitle = "CharacterEngine - Standalone D3D9";
    int windowWidth = 800;
    int windowHeight = 600;
    bool windowed = true;
    D3DCOLOR clearColor = D3DCOLOR_XRGB(0, 0, 255);

    std::string assetRoot = ".";
    std::string mediaRoot = "../media";
    std::string shaderRoot = "shaders";
    std::string logFile = "app.log";

    std::string modelName = "dragon";
    std::string skinName = "default";
    std::string weaponName = "railgun";
    std::string weaponSkinName = "default";

    std::string modelPath = "../media/dragon";
    std::string weaponPath = "../media/railgun";

    int shaderProfile = 2;
    bool wireframe = false;
    bool showWeapon = false;
    bool moveLight = false;

    float zoom = -120.0f;
    float fovDegrees = 70.0f;
    float nearPlane = 1.0f;
    float farPlane = 1000.0f;

    D3DXVECTOR4 lightPosition = D3DXVECTOR4(0.0f, -10.0f, 40.0f, 1.0f);
};

inline std::wstring ToWideString(const std::string& text)
{
    return std::wstring(text.begin(), text.end());
}

inline std::string JoinPathString(const std::string& lhs, const std::string& rhs)
{
    if (lhs.empty())
        return rhs;

    if (rhs.empty())
        return lhs;

    return (std::filesystem::path(lhs) / rhs).generic_string();
}

inline D3DCOLOR ParseRgbColor(const std::string& text, D3DCOLOR defaultColor)
{
    if (text.empty())
        return defaultColor;

    // Hex forms: 0xRRGGBB or #RRGGBB.
    try
    {
        if (text.rfind("0x", 0) == 0 || text.rfind("0X", 0) == 0)
        {
            const unsigned int value = static_cast<unsigned int>(std::stoul(text, nullptr, 16));
            return D3DCOLOR_XRGB((value >> 16) & 0xff, (value >> 8) & 0xff, value & 0xff);
        }

        if (text[0] == '#')
        {
            const unsigned int value = static_cast<unsigned int>(std::stoul(text.substr(1), nullptr, 16));
            return D3DCOLOR_XRGB((value >> 16) & 0xff, (value >> 8) & 0xff, value & 0xff);
        }
    }
    catch (...)
    {
        return defaultColor;
    }

    // CSV form: r,g,b
    std::istringstream stream(text);
    int r = 0;
    int g = 0;
    int b = 0;
    char comma1 = 0;
    char comma2 = 0;

    if (stream >> r >> comma1 >> g >> comma2 >> b && comma1 == ',' && comma2 == ',')
    {
        r = std::max(0, std::min(255, r));
        g = std::max(0, std::min(255, g));
        b = std::max(0, std::min(255, b));
        return D3DCOLOR_XRGB(r, g, b);
    }

    return defaultColor;
}

inline D3DXVECTOR4 ParseVector4(const std::string& text, const D3DXVECTOR4& defaultValue)
{
    if (text.empty())
        return defaultValue;

    std::istringstream stream(text);
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
    char comma1 = 0;
    char comma2 = 0;
    char comma3 = 0;

    if (stream >> x >> comma1 >> y >> comma2 >> z)
    {
        if (stream >> comma3 >> w)
        {
            if (comma1 == ',' && comma2 == ',' && comma3 == ',')
                return D3DXVECTOR4(x, y, z, w);
        }
        else if (comma1 == ',' && comma2 == ',')
        {
            return D3DXVECTOR4(x, y, z, 1.0f);
        }
    }

    return defaultValue;
}

inline CharacterAppSettings LoadCharacterAppSettings(const char* filename = "CharacterEngine.properties")
{
    CharacterAppSettings settings;

    Framework::PropertiesFile properties;
    properties.LoadFirstFound({
        std::filesystem::current_path() / filename,
        Framework::PropertiesFile::ExecutableDirectory() / filename,
        Framework::PropertiesFile::ExecutableDirectory() / ".." / filename,
    });

    settings.windowTitle = properties.GetString("window.title", settings.windowTitle);
    settings.windowWidth = properties.GetInt("window.width", settings.windowWidth);
    settings.windowHeight = properties.GetInt("window.height", settings.windowHeight);
    settings.windowed = properties.GetBool("window.windowed", settings.windowed);
    settings.clearColor = ParseRgbColor(properties.GetString("window.clearColor"), settings.clearColor);

    settings.assetRoot = properties.GetString("paths.assetRoot", settings.assetRoot);
    settings.mediaRoot = properties.GetString("paths.mediaRoot", settings.mediaRoot);
    settings.shaderRoot = properties.GetString("paths.shaderRoot", settings.shaderRoot);
    settings.logFile = properties.GetString("paths.logFile", settings.logFile);

    settings.modelName = properties.GetString("player.model", settings.modelName);
    settings.skinName = properties.GetString("player.skin", settings.skinName);
    settings.weaponName = properties.GetString("player.weapon", settings.weaponName);
    settings.weaponSkinName = properties.GetString("player.weaponSkin", settings.weaponSkinName);

    settings.modelPath = properties.GetString("player.modelPath", JoinPathString(settings.mediaRoot, settings.modelName));
    settings.weaponPath = properties.GetString("player.weaponPath", JoinPathString(settings.mediaRoot, settings.weaponName));

    settings.shaderProfile = properties.GetInt("render.shaderProfile", settings.shaderProfile);
    settings.wireframe = properties.GetBool("render.wireframe", settings.wireframe);
    settings.showWeapon = properties.GetBool("render.showWeapon", settings.showWeapon);
    settings.moveLight = properties.GetBool("render.moveLight", settings.moveLight);

    settings.zoom = properties.GetFloat("camera.zoom", settings.zoom);
    settings.fovDegrees = properties.GetFloat("camera.fovDegrees", settings.fovDegrees);
    settings.nearPlane = properties.GetFloat("camera.nearPlane", settings.nearPlane);
    settings.farPlane = properties.GetFloat("camera.farPlane", settings.farPlane);

    settings.lightPosition = ParseVector4(properties.GetString("light.position"), settings.lightPosition);

    return settings;
}
