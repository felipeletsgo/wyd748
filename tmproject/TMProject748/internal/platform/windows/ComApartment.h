#pragma once

#include <objbase.h>

namespace com_apartment
{
    // Owns one CoInitializeEx call on the current thread. S_OK and S_FALSE
    // each add an apartment reference and are balanced once. A failure, such
    // as RPC_E_CHANGED_MODE, adds no reference and is never balanced.
    template <typename Api>
    class BasicReference
    {
    public:
        explicit BasicReference(DWORD model) : owns_(SUCCEEDED(Api::Initialize(model))) {}
        ~BasicReference()
        {
            if (owns_)
                Api::Uninitialize();
        }
        BasicReference(const BasicReference&) = delete;
        BasicReference& operator=(const BasicReference&) = delete;

        bool Owns() const { return owns_; }

    private:
        bool owns_;
    };

    struct SystemApi
    {
        static HRESULT Initialize(DWORD model) { return CoInitializeEx(nullptr, model); }
        static void Uninitialize() { CoUninitialize(); }
    };

    using Reference = BasicReference<SystemApi>;
}
