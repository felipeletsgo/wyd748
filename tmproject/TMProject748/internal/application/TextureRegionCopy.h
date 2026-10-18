#pragma once

#include <cstddef>
#include <cstring>

// Bounded copy of a source surface region into a dynamic destination surface.
// Both surfaces are addressed through their locked pitch; neither is assumed
// to be tightly packed or to have the dynamic texture dimensions.
namespace texture_region_copy
{
    // D3DFORMAT codes accepted for byte-wise copying. Compressed and
    // unknown formats are rejected because their rows are not pixel rows.
    inline int BytesPerPixel(int format)
    {
        switch (format)
        {
        case 21: case 22: case 31: case 32: case 33: return 4; // A8R8G8B8, X8R8G8B8, A2B10G10R10, A8B8G8R8, X8B8G8R8
        case 23: case 24: case 25: case 26: case 29: case 30: return 2; // R5G6B5, X1R5G5B5, A1R5G5B5, A4R4G4B4, A8R3G3B2, X4R4G4B4
        default: return 0;
        }
    }

    // A destination is reusable only with the source format and exactly the
    // dimensions the dynamic copy is planned against.
    inline bool DestinationCompatible(int destFormat, int destWidth, int destHeight,
        int srcFormat, int requiredWidth, int requiredHeight)
    {
        return destFormat == srcFormat && BytesPerPixel(srcFormat) != 0 &&
            destWidth == requiredWidth && destHeight == requiredHeight;
    }

    struct Plan
    {
        bool valid = false;
        int width = 0;
        int height = 0;
        int destX = 0;
        int destY = 0;
        int srcX = 0;
        int srcY = 0;
        int scale = 1;
    };

    // Destination coordinates and size are already in destination pixels.
    // Source row = scale * (row + srcY); source column = scale * column + srcX,
    // matching the legacy half-resolution sampling.
    inline Plan Build(int destWidth, int destHeight, int srcWidth, int srcHeight,
        int destX, int destY, int srcX, int srcY, int width, int height, int scale)
    {
        Plan plan;
        if (scale != 1 && scale != 2) return plan;
        if (destWidth <= 0 || destHeight <= 0 || srcWidth <= 0 || srcHeight <= 0) return plan;
        if (destX < 0 || destY < 0 || destX >= destWidth || destY >= destHeight) return plan;
        if (srcX < 0 || srcY < 0 || srcX >= srcWidth) return plan;
        if (static_cast<long long>(srcY) * scale >= srcHeight) return plan;
        if (width <= 0 || height <= 0) return plan;

        long long maxWidth = static_cast<long long>(destWidth) - destX;
        long long maxHeight = static_cast<long long>(destHeight) - destY;
        const long long srcColumns = (static_cast<long long>(srcWidth) - 1 - srcX) / scale + 1;
        const long long srcRows = (static_cast<long long>(srcHeight) - 1) / scale - srcY + 1;
        if (srcColumns < maxWidth) maxWidth = srcColumns;
        if (srcRows < maxHeight) maxHeight = srcRows;
        if (maxWidth <= 0 || maxHeight <= 0) return plan;

        plan.width = width < maxWidth ? width : static_cast<int>(maxWidth);
        plan.height = height < maxHeight ? height : static_cast<int>(maxHeight);
        plan.destX = destX;
        plan.destY = destY;
        plan.srcX = srcX;
        plan.srcY = srcY;
        plan.scale = scale;
        plan.valid = true;
        return plan;
    }

    // Locked pitches must cover a full row of each surface.
    inline bool PitchesCover(int bytesPerPixel, int srcWidth, int srcPitch, int destWidth, int destPitch)
    {
        if (bytesPerPixel <= 0 || srcPitch <= 0 || destPitch <= 0) return false;
        return static_cast<long long>(srcPitch) >= static_cast<long long>(srcWidth) * bytesPerPixel &&
            static_cast<long long>(destPitch) >= static_cast<long long>(destWidth) * bytesPerPixel;
    }

    inline void Copy(const Plan& plan, int bytesPerPixel,
        const unsigned char* src, int srcPitch, unsigned char* dest, int destPitch)
    {
        if (!plan.valid || !src || !dest || bytesPerPixel <= 0) return;
        for (int row = 0; row < plan.height; ++row)
        {
            const unsigned char* srcRow = src + static_cast<std::size_t>(srcPitch) *
                static_cast<std::size_t>(plan.scale * (row + plan.srcY));
            unsigned char* destRow = dest + static_cast<std::size_t>(destPitch) *
                static_cast<std::size_t>(row + plan.destY) +
                static_cast<std::size_t>(plan.destX) * bytesPerPixel;
            if (plan.scale == 1)
            {
                std::memcpy(destRow, srcRow + static_cast<std::size_t>(plan.srcX) * bytesPerPixel,
                    static_cast<std::size_t>(plan.width) * bytesPerPixel);
                continue;
            }
            for (int column = 0; column < plan.width; ++column)
            {
                std::memcpy(destRow + static_cast<std::size_t>(column) * bytesPerPixel,
                    srcRow + static_cast<std::size_t>(plan.scale * column + plan.srcX) * bytesPerPixel,
                    static_cast<std::size_t>(bytesPerPixel));
            }
        }
    }

    // Every successful lock must be balanced, even if the driver returns a
    // null pixel pointer. Surface references remain owned by the caller.
    template<class LockedRect, class Surface>
    bool CopyLocked(Surface& source, Surface& destination, const Plan& plan,
        int bytesPerPixel, int srcWidth, int destWidth, unsigned long readOnlyFlag)
    {
        if (!plan.valid) return false;
        LockedRect src{}, dest{};
        if (source.LockRect(&src, nullptr, readOnlyFlag) < 0) return false;
        if (!src.pBits)
        {
            source.UnlockRect();
            return false;
        }
        if (destination.LockRect(&dest, nullptr, 0) < 0)
        {
            source.UnlockRect();
            return false;
        }
        const bool valid = dest.pBits && PitchesCover(bytesPerPixel, srcWidth, src.Pitch, destWidth, dest.Pitch);
        if (valid)
            Copy(plan, bytesPerPixel, static_cast<const unsigned char*>(src.pBits), src.Pitch,
                static_cast<unsigned char*>(dest.pBits), dest.Pitch);
        const bool destUnlocked = destination.UnlockRect() >= 0;
        const bool srcUnlocked = source.UnlockRect() >= 0;
        return valid && destUnlocked && srcUnlocked;
    }
}
