#pragma once

#include <utility>

namespace Framework
{
    template <typename T>
    class ComPtr
    {
    public:
        ComPtr() = default;
        explicit ComPtr(T* ptr) noexcept : m_ptr(ptr) {}
        ~ComPtr() { Reset(); }

        ComPtr(const ComPtr&) = delete;
        ComPtr& operator=(const ComPtr&) = delete;

        ComPtr(ComPtr&& other) noexcept : m_ptr(other.m_ptr)
        {
            other.m_ptr = nullptr;
        }

        ComPtr& operator=(ComPtr&& other) noexcept
        {
            if (this != &other)
            {
                Reset();
                m_ptr = other.m_ptr;
                other.m_ptr = nullptr;
            }
            return *this;
        }

        T* Get() const noexcept { return m_ptr; }
        T** Put() noexcept
        {
            Reset();
            return &m_ptr;
        }

        T* const* GetAddressOf() const noexcept { return &m_ptr; }
        T** GetAddressOf() noexcept { return &m_ptr; }

        T* Detach() noexcept
        {
            T* out = m_ptr;
            m_ptr = nullptr;
            return out;
        }

        void Attach(T* ptr) noexcept
        {
            if (m_ptr != ptr)
            {
                Reset();
                m_ptr = ptr;
            }
        }

        void Reset() noexcept
        {
            if (m_ptr)
            {
                m_ptr->Release();
                m_ptr = nullptr;
            }
        }

        T* operator->() const noexcept { return m_ptr; }
        explicit operator bool() const noexcept { return m_ptr != nullptr; }

    private:
        T* m_ptr = nullptr;
    };
}
