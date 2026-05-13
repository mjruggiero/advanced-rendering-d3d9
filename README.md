# Advanced Rendering D3D9

A legacy Direct3D 9 advanced rendering sample modernized as a personal graphics programming project.

This project explores real-time rendering techniques, shader pipelines, and incremental C++ cleanup in an older D3D9 codebase. The goal is not to rewrite the renderer from scratch, but to preserve the original visual behavior while improving structure, buildability, and maintainability.

## Features

- MD3 character rendering
- Direct3D 9 rendering pipeline
- Shadow mapping
- HDR and bloom post-processing
- Moveable light/debug marker
- Shader profile system
- Legacy HLSL shader compilation workflow
- Incremental application-level refactoring

## Recent Modernization Work

- Extracted HDR post-processing into a dedicated renderer class
- Extracted shadow-map rendering into a dedicated renderer class
- Fixed light marker rendering so it appears correctly in HDR mode
- Restored shader compilation using the DirectX SDK `fxc` tool
- Added a shader rebuild script for legacy `.fx` / `.fxp` files
- Fixed Cook-Torrance shader shadow-map support
- Fixed diffuse-specular shadow intensity by applying shadow visibility to the full lighting result
- Normalized inconsistent shader profile mappings

## Technical Focus

This project demonstrates work in:

- real-time rendering
- shader debugging
- Direct3D 9 resource management
- legacy code modernization
- C++ application structure cleanup
- render pass organization
- graphics pipeline debugging

## Current Status

This is an active personal graphics programming project.

The project began as an older advanced-rendering demo and still retains parts of the original media, shader, and profile layout. Cleanup is being done incrementally to avoid changing rendering behavior while improving the architecture.

## Build Requirements

- Windows
- Visual Studio
- Direct3D 9-compatible development environment
- June 2010 DirectX SDK for rebuilding legacy shader assembly files

## Shader Rebuild Notes

Legacy shaders are compiled from:

- `.fx` files into vertex shader assembly `.vsh`
- `.fxp` files into pixel shader assembly `.psh`

The rebuild script is located in:

```text
shaders/rebuild_shaders.bat