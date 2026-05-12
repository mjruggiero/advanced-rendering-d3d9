#pragma once

#include <d3d9.h>

#include "ComPtr.h"

namespace Framework
{
	// RAII helper that captures the current D3D9 device state and restores it
	// when the object goes out of scope.
	//
	// This is useful while legacy code, HUD text, and new render passes still
	// share the same device and may stomp each other's render states.
	class D3D9ScopedStateBlock
	{
	public:
		explicit D3D9ScopedStateBlock(IDirect3DDevice9* device);
		~D3D9ScopedStateBlock();

		D3D9ScopedStateBlock(const D3D9ScopedStateBlock&) = delete;
		D3D9ScopedStateBlock& operator=(const D3D9ScopedStateBlock&) = delete;

		bool IsValid() const { return m_stateBlock.Get() != nullptr; }

		// Manually restore early. The destructor will not restore twice.
		void Restore();

	private:
		IDirect3DDevice9* m_device = nullptr;
		ComPtr<IDirect3DStateBlock9> m_stateBlock;
		bool m_restored = false;
	};

	// Small helper for restoring one render state.
	//
	// Use this when a full state block is overkill and you only need to protect
	// one state, such as D3DRS_FILLMODE or D3DRS_CULLMODE.
	class D3D9ScopedRenderState
	{
	public:
		D3D9ScopedRenderState(
			IDirect3DDevice9* device,
			D3DRENDERSTATETYPE state,
			DWORD newValue);

		~D3D9ScopedRenderState();

		D3D9ScopedRenderState(const D3D9ScopedRenderState&) = delete;
		D3D9ScopedRenderState& operator=(const D3D9ScopedRenderState&) = delete;

		void Restore();

	private:
		IDirect3DDevice9* m_device = nullptr;
		D3DRENDERSTATETYPE m_state = D3DRS_FORCE_DWORD;
		DWORD m_oldValue = 0;
		bool m_valid = false;
		bool m_restored = false;
	};
}
