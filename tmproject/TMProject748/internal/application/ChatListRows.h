#pragma once

#include <cstddef>
#include <cstring>

// Chat list rows for one message. A message that does not fit on one row is
// shown as a head row and a tail row. Items are created only for rows that
// have text, and each created item goes to the list immediately. The list
// (SListBox::AddItem) takes ownership of a new item: it keeps the item or
// deletes it. Thus no item exists without an owner, and no row is added twice.
namespace chat_rows
{
// Head length in bytes: `limit`, or one byte less when the byte at limit - 1
// starts a double-byte pair. `clearAtLimit` is IsClearString(text, limit - 1).
constexpr std::size_t CutAt(std::size_t limit, bool clearAtLimit)
{
    return clearAtLimit || limit == 0 ? limit : limit - 1;
}

// Copies at most `cut` bytes of `text` into `head` (bounded and terminated)
// and returns the tail that starts at `cut`. The tail is empty when the text
// has `cut` bytes or fewer.
template <std::size_t HeadWidth>
const char* Split(const char* text, std::size_t cut, char (&head)[HeadWidth])
{
    static_assert(HeadWidth > 0, "the head needs a terminator");
    const std::size_t length = std::strlen(text);
    if (cut > length)
        cut = length;
    const std::size_t copied = cut < HeadWidth ? cut : HeadWidth - 1;
    std::memcpy(head, text, copied);
    head[copied] = '\0';
    return text + cut;
}

// Gives the list one new item for each row that has text, in order. A null
// list creates no item. Returns the number of items given to the list.
template <typename List, typename MakeItem>
int Add(List* list, const char* first, const char* second, MakeItem make)
{
    if (!list)
        return 0;
    int given = 0;
    const char* const rows[2] = {first, second};
    for (const char* row : rows)
    {
        if (!row || !row[0])
            continue;
        list->AddItem(make(row));
        ++given;
    }
    return given;
}
}
