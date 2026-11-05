#pragma once

#include <windows.h>
#include <cstddef>
#include <vector>

// Reference ownership for the DirectShow BGM graph. Interfaces are template
// parameters, so tests can count references without a real filter graph.
namespace dirshow_graph
{
    // A BGM graph holds a few filters; the bound stops a faulty enumerator.
    constexpr std::size_t kMaxFilters = 64;
    // Events read for one graph notification message.
    constexpr int kMaxEventsPerNotification = 64;
    // EC_COMPLETE in evcode.h; DirShow.cpp checks the value at compile time.
    constexpr long kCompleteEvent = 0x01;

    template <typename Interface>
    void Release(Interface*& value)
    {
        if (value)
        {
            value->Release();
            value = nullptr;
        }
    }

    // Releases each non-null interface exactly once, in argument order.
    template <typename... Interfaces>
    void ReleaseAll(Interfaces*&... values)
    {
        (Release(values), ...);
    }

    template <typename Source, typename Interface>
    bool Query(Source& source, Interface*& out)
    {
        void* raw = nullptr;
        if (FAILED(source.QueryInterface(__uuidof(Interface), &raw)) || !raw)
            return false;
        out = static_cast<Interface*>(raw);
        return true;
    }

    // Queries the interfaces in argument order. Every output starts null; on
    // failure, the outputs acquired so far stay set so ReleaseAll frees them.
    template <typename Source, typename... Interfaces>
    bool QueryAll(Source& source, Interfaces*&... outputs)
    {
        ((outputs = nullptr), ...);
        return (Query(source, outputs) && ...);
    }

    // Collects the filters with the one reference that Next adds to each.
    // Only S_OK continues: S_FALSE (end) and errors stop the enumeration.
    template <typename Filter, typename Enumerator>
    std::vector<Filter*> CollectFilters(Enumerator& enumerator)
    {
        std::vector<Filter*> filters;
        if (FAILED(enumerator.Reset()))
            return filters;
        while (filters.size() < kMaxFilters)
        {
            Filter* filter = nullptr;
            ULONG fetched = 0;
            const HRESULT result = enumerator.Next(1, &filter, &fetched);
            if (result != S_OK || fetched != 1 || !filter)
            {
                Release(filter);
                break;
            }
            filters.push_back(filter);
        }
        return filters;
    }

    // Removes each collected filter from the graph and releases the
    // collection reference once. Other holders keep their references.
    template <typename Graph, typename Filter>
    void RemoveAndRelease(Graph& graph, std::vector<Filter*>& filters)
    {
        for (Filter*& filter : filters)
        {
            graph.RemoveFilter(filter);
            Release(filter);
        }
        filters.clear();
    }

    // Reads the queued graph events and frees the parameters of each one.
    // Returns true when playback completed at least once.
    template <typename Events>
    bool DrainEvents(Events& events)
    {
        bool completed = false;
        for (int i = 0; i < kMaxEventsPerNotification; ++i)
        {
            long code = 0;
            LONG_PTR first = 0, second = 0;
            if (events.GetEvent(&code, &first, &second, 0) != S_OK)
                break;
            if (code == kCompleteEvent)
                completed = true;
            events.FreeEventParams(code, first, second);
        }
        return completed;
    }
}
