#pragma once

#include <cstddef>

// State of the legacy WYD.log descriptor. Io supplies Open, Write, Commit and
// Close with the CRT _open/_write/_commit/_close contracts, so tests can count
// descriptor use without files.
namespace legacy_log
{
    constexpr int kClosedDescriptor = -1;

    // Bytes that vsnprintf placed in a buffer of the given capacity. The CRT
    // returns the untruncated length, or a negative value on failure.
    inline int FormattedLength(int result, std::size_t capacity)
    {
        if (result <= 0 || capacity == 0)
            return 0;
        const std::size_t limit = capacity - 1;
        return static_cast<std::size_t>(result) < limit ? result : static_cast<int>(limit);
    }

    template <typename Io>
    class File
    {
    public:
        // Closes a previous descriptor, then opens path. False keeps the log closed.
        bool Open(const char* path)
        {
            Close();
            const int descriptor = Io::Open(path);
            if (descriptor < 0)
                return false;
            descriptor_ = descriptor;
            return true;
        }

        // Writes the formatted part of buffer, then commits it. Ignored while closed.
        void Write(const char* buffer, int formatted, std::size_t capacity)
        {
            const int length = FormattedLength(formatted, capacity);
            if (!IsOpen() || !buffer || length == 0)
                return;
            if (Io::Write(descriptor_, buffer, static_cast<unsigned int>(length)) == length)
                Io::Commit(descriptor_);
        }

        // Commits and closes an open descriptor once. Ignored while closed.
        void Close()
        {
            if (!IsOpen())
                return;
            Io::Commit(descriptor_);
            Io::Close(descriptor_);
            descriptor_ = kClosedDescriptor;
        }

        bool IsOpen() const { return descriptor_ >= 0; }
        int Descriptor() const { return descriptor_; }

    private:
        int descriptor_ = kClosedDescriptor;
    };
}
