#pragma once

namespace kefra_portal
{
    // Own no scene/object pointers. Streaming can destroy and recreate the
    // mesh and both billboards; each queries the current field-scene snapshot.
    template <typename Visual, bool (*IsOpen)()>
    class VisualGate final : public Visual
    {
    public:
        using Visual::Visual;

        int IsVisible() override
        {
            if (!IsOpen())
            {
                this->m_bVisible = 0;
                return 0;
            }
            return Visual::IsVisible();
        }

        int Render() override
        {
            if (!IsOpen())
            {
                this->m_bVisible = 0;
                return 1;
            }
            return Visual::Render();
        }
    };
}
