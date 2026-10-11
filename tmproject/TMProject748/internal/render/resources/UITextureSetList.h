#pragma once

#include <cstdlib>
#include <cerrno>
#include <climits>
#include <cstring>
#include <string>
#include <vector>

// One source rectangle of a UI texture set (UI\UITextureSetList.txt row):
// texture index, source x/y/width/height and destination offset x/y.
struct ControlTextureCoord
{
	int nTextureIndex;
	int nStartX;
	int nStartY;
	int nWidth;
	int nHeight;
	int nDestX;
	int nDestY;
};

struct ControlTextureSet
{
	int nCount;
	ControlTextureCoord* pTextureCoord;
};

// Every consumer indexes pTextureCoord with a control state or digit. The
// renderer used to accept index == nCount and never checked negative indices.
inline const ControlTextureCoord* UITextureCoordAt(const ControlTextureSet* set, int index)
{
	if (!set || !set->pTextureCoord || index < 0 || index >= set->nCount)
		return nullptr;
	return &set->pTextureCoord[index];
}

inline ControlTextureCoord* UITextureCoordAt(ControlTextureSet* set, int index)
{
	if (!set || !set->pTextureCoord || index < 0 || index >= set->nCount)
		return nullptr;
	return &set->pTextureCoord[index];
}

// Match existing digit spacing for valid glyphs; missing glyphs reserve the
// same gap as an unsupported character and are skipped by the renderer.
inline int UITextureGlyphAdvance(const ControlTextureSet* set, int index, float uiScale, float glyphScale = 1.0f)
{
	const auto* coord = UITextureCoordAt(set, index);
	return coord ? static_cast<int>((static_cast<float>(coord->nWidth) * uiScale) * glyphScale)
		: static_cast<int>(5.0f * uiScale);
}

namespace ui_texture_set
{
	// The largest shipped set has a few hundred rows; anything larger is corrupt.
	constexpr int MaxItemsPerSet = 1024;

	struct Record
	{
		std::string name;
		int setIndex = -1;
		std::vector<ControlTextureCoord> coords;
	};

	struct Report
	{
		int sets = 0;
		int skipped = 0;    // index outside the table, bad header or count
		int truncated = 0;  // fewer rows than ItemCount declares
		int duplicates = 0; // a later block replaced an earlier index
	};

	inline bool ParseIntAfter(const std::string& line, const char* key, int& value)
	{
		const std::size_t keyLength = std::strlen(key);
		if (line.compare(0, keyLength, key) != 0)
			return false;
		const char* begin = line.c_str() + keyLength;
		while (*begin == ' ' || *begin == '\t')
			++begin;
		char* end = nullptr;
		errno = 0;
		const long parsed = std::strtol(begin, &end, 10);
		if (end == begin || errno == ERANGE)
			return false;
		while (*end == ' ' || *end == '\t')
			++end;
		if (end != line.c_str() + line.size() || parsed < INT_MIN || parsed > INT_MAX)
			return false;
		value = static_cast<int>(parsed);
		return true;
	}

	inline bool ParseRow(const std::string& line, ControlTextureCoord& coord)
	{
		int values[7]{};
		const char* cursor = line.c_str();
		for (int i = 0; i < 7; ++i)
		{
			char* end = nullptr;
			errno = 0;
			const long parsed = std::strtol(cursor, &end, 10);
			if (end == cursor || errno == ERANGE || parsed < -1000000L || parsed > 1000000L)
				return false;
			values[i] = static_cast<int>(parsed);
			cursor = end;
			while (*cursor == ' ' || *cursor == '\t')
				++cursor;
			if (i < 6)
			{
				if (*cursor != ',')
					return false;
				++cursor;
			}
		}
		if (cursor != line.c_str() + line.size())
			return false;
		coord = {values[0], values[1], values[2], values[3], values[4], values[5], values[6]};
		return true;
	}

	// Line-based reader for the native text format:
	//   [Name] / SetIndex: N / ItemCount: N / N rows of seven comma integers.
	// A block with an index outside [0, capacity) or a bad header is skipped
	// with its rows; a short block keeps the rows it has instead of leaving the
	// missing entries uninitialized (the shipped set 198 declares 5, has 4).
	inline std::vector<Record> Parse(const std::string& text, int capacity, Report& report)
	{
		report = {};
		std::vector<std::string> lines;
		std::size_t start = 0;
		while (start <= text.size())
		{
			std::size_t end = text.find('\n', start);
			if (end == std::string::npos)
				end = text.size();
			std::string line = text.substr(start, end - start);
			while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
				line.pop_back();
			std::size_t first = 0;
			while (first < line.size() && (line[first] == ' ' || line[first] == '\t'))
				++first;
			lines.push_back(line.substr(first));
			start = end + 1;
		}

		std::vector<Record> records;
		std::size_t i = 0;
		const auto nextContent = [&]() -> const std::string* {
			while (i < lines.size() && lines[i].empty())
				++i;
			return i < lines.size() ? &lines[i] : nullptr;
		};
		while (const std::string* header = nextContent())
		{
			if (header->size() < 2 || header->front() != '[' || header->back() != ']')
			{
				++i; // stray line outside a block
				continue;
			}
			Record record;
			record.name = header->substr(1, header->size() - 2);
			++i;
			int count = -1;
			const std::string* indexLine = nextContent();
			const bool indexOk = indexLine && ParseIntAfter(*indexLine, "SetIndex:", record.setIndex);
			if (indexOk)
				++i;
			const std::string* countLine = indexOk ? nextContent() : nullptr;
			const bool countOk = countLine && ParseIntAfter(*countLine, "ItemCount:", count);
			if (countOk)
				++i;

			ControlTextureCoord coord{};
			while (const std::string* row = nextContent())
			{
				if (!ParseRow(*row, coord))
					break;
				++i;
				if (countOk && count >= 0 && static_cast<int>(record.coords.size()) < count &&
					static_cast<int>(record.coords.size()) < MaxItemsPerSet)
					record.coords.push_back(coord);
			}

			if (!indexOk || !countOk || count < 0 || count > MaxItemsPerSet ||
				record.setIndex < 0 || record.setIndex >= capacity)
			{
				++report.skipped;
				continue;
			}
			if (static_cast<int>(record.coords.size()) < count)
				++report.truncated;
			for (const Record& previous : records)
				if (previous.setIndex == record.setIndex)
				{
					++report.duplicates;
					break;
				}
			++report.sets;
			records.push_back(std::move(record));
		}
		return records;
	}
}
