#pragma once

#include <vector>

struct IDirect3DDevice9;

namespace Framework
{
    class FileSystem;
    class IRenderPass;
    class RenderContext;

    // Non-owning ordered list of render passes.
    //
    // CharacterApp or another owner keeps the actual pass objects alive. This class
    // just provides one place to create, destroy, and render them in order.
    class RenderPassList
    {
    public:
        void Add(IRenderPass& pass);
        void Clear();

        bool CreateDeviceResources(IDirect3DDevice9* device, const FileSystem& files);
        void DestroyDeviceResources();
        void Render(RenderContext& context);

        bool IsCreated() const { return m_created; }
        bool Empty() const { return m_passes.empty(); }

    private:
        std::vector<IRenderPass*> m_passes;
        bool m_created = false;
    };
}
