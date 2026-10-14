#pragma once

// Rider placement on a mount. TMHuman::Render passes (x, y, z) to the rider
// skin mesh, whose base matrix is the mount's output bone matrix.
//
// Evidence:
// - Native 7.48 (WYD.exe 0x504040..0x5042bd) only distinguishes mount skin
//   types 20, 25, 28 and 29 plus the class-40 rider.
// - The KR client (WYD.exe 0x5588cc..0x558db0) adds the imported mount
//   types (31, 34, 35, 38, 41, 48..53, 59) and extra meshes for 20/29. Its
//   x87 sequence was emulated over 580,800 input combinations and matches
//   this port exactly.
// Native 7.48 mounts keep the native rules for types 20 and 29; every other
// case uses the KR rules, which equal 7.48 for types 25 and 28.
namespace mount_rider
{
    struct Offset
    {
        float x;
        float y;
        float z;
    };

    // mountMesh0: m_pMount->m_Look.Mesh0. lookMesh0/lookMesh1: the human's
    // m_stMountLook. riderClass: m_nClass. mountScale: m_fMountScale.
    // riderScale: m_fScale. nativeMount: the mount came from a native 7.48
    // mount item rather than an imported KR visual.
    inline Offset Compute(int type, int mountMesh0, int lookMesh0, int lookMesh1,
        int riderClass, float mountScale, float riderScale, bool nativeMount)
    {
        const float s = mountScale;
        const float baseY = 1.0f / s;
        Offset o{ -0.37f * s, baseY, 0.2f * s };

        if (type == 25)
        {
            o.x = -0.35f * s;
            o.z = -0.1f * s;
        }
        else if (type == 28)
        {
            o.x = -0.2f * s;
            o.z = 0.1f * s;
        }
        else if (type == 29 && nativeMount)
        {
            o.x = mountMesh0 == 1 ? -0.1f * s : -0.37f * s;
            o.z = -0.6f * s;
        }
        else if (type == 29)
        {
            if (mountMesh0 == 1 || mountMesh0 == 4 || mountMesh0 == 40 || mountMesh0 == 41)
                o.x = -0.1f * s;
            else if (mountMesh0 == 0 || mountMesh0 == 42)
                o.y = 1.2f;
            else if (mountMesh0 == 11 || mountMesh0 == 12)
            {
                o.x = -0.28f;
                o.z -= 0.8f;
            }
            if (mountMesh0 != 11 && mountMesh0 != 12)
                o.z = -0.6f * s;
        }
        else if (type == 20 && nativeMount)
        {
            o.x = mountMesh0 == 7 ? 0.5f * s : 0.25f * s;
            o.z = -0.1f * s;
        }
        else if (type == 20)
        {
            if (mountMesh0 == 7 || mountMesh0 == 47 || mountMesh0 == 48 || mountMesh0 == 50)
            {
                o.x = 0.5f * s;
                o.z = -0.1f * s;
                if (mountMesh0 == 50)
                {
                    o.z = -0.5f;
                    o.x -= 0.3f;
                }
            }
            else if (mountMesh0 == 6)
            {
                o.x = 0.0f;
                o.z = -0.8f * s;
            }
            else
            {
                o.x = 0.25f * s;
                o.z = -0.1f * s;
            }
        }

        if (riderClass == 40)
        {
            if (type == 20 && lookMesh0 != 7)
            {
                o.y = riderScale / 1.3f;
                o.x /= 1.45f;
            }
            else if (type == 20)
                o.y = riderScale * 1.7f;
            else
            {
                o.y = riderScale;
                o.x += 0.05f;
            }
        }

        if (nativeMount && (type == 20 || type == 25 || type == 28 || type == 29))
            return o;

        if (type == 29 && (lookMesh1 == 5 || lookMesh1 == 10))
        {
            o.x = -0.2f;
            o.y = baseY;
        }
        else if (type == 29 && lookMesh1 == 6)
        {
            o.x = -0.2f;
            o.y = 1.1f / s;
        }
        else if (type == 29 && (lookMesh1 == 7 || lookMesh1 == 8))
        {
            o.x = -0.4f;
            o.y = 1.1f / s;
        }
        else if ((type == 38 && ((lookMesh1 >= 1 && lookMesh1 <= 4) || lookMesh1 == 45)) ||
            (type == 31 && lookMesh1 == 17))
            o.x = -0.6f;
        else if (type == 48)
        {
            o.x -= 0.8f;
            o.z += 0.2f;
        }
        else if (type == 49 || type == 52)
        {
            o.x -= 0.2f;
            o.z -= 0.2f;
            if (lookMesh0 == 24)
                o.z += 0.2f;
        }
        else if (type == 50)
        {
            o.x -= 0.5f;
            o.z -= 0.2f;
            if (lookMesh0 == 4)
            {
                o.z += 0.2f;
                o.x -= 0.2f;
            }
            if (lookMesh0 == 5)
            {
                o.z += 0.1f;
                o.x -= 0.2f;
            }
        }
        else if (type == 53)
            o = { -0.28f, 1.0f, 0.4f };
        else if (type == 51)
        {
            o.x = -0.38f;
            o.z = 0.3f;
        }
        else if (type == 35)
        {
            o.x = -0.3f;
            o.z = -0.25f;
        }
        else if (type == 41)
        {
            o.x = -0.45f;
            o.z = -0.25f;
        }
        else if (type == 34 || type == 59)
            o = { -0.18f, 1.0f, -0.2f };
        return o;
    }
}
