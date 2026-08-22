#pragma once

struct BinCheckBox
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nColor;
};

struct BinListBox
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nMaxCount;
    int nVisibleCount;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nColor;
    int nFillType;
    int nSelect;
    int nScroll;
};

struct BinGrid
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nRowCount;
    int nColumnCount;
    int nType;
};

struct BinPanel
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nColor;
    int nFillType;
    int nPickable;
};

struct Bin3DObj
{
    int nID;
    int nParentID;
    int n3DObjIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
};

struct BinButton
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nColor;
    int nSound;
    int nStringIndex;
};

// WYD 7.48 stores button captions directly in older RC records. Keeping this
// disk-only shape separate prevents the modern string-table index from being
// read out of the first four caption bytes.
struct LegacyBinButton
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nColor;
    int nSound;
    char szString[128];
};

static_assert(sizeof(LegacyBinButton) == 164, "WYD 7.48 button RC ABI changed");

struct BinText
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nFontColor;
    int nBorder;
    int nBorderColor;
    int nTextType;
    int nAlignType;
    int nStringIndex;
};

// Text controls in the original 7.48 RC format also own a 128-byte inline
// caption; newer TMProject RC files replaced it with nStringIndex.
struct LegacyBinText
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nFontColor;
    int nBorder;
    int nBorderColor;
    int nTextType;
    int nAlignType;
    char szString[128];
};

static_assert(sizeof(LegacyBinText) == 176, "WYD 7.48 text RC ABI changed");

struct BinEdit
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nFontColor;
    int nBorder;
    int nBorderColor;
    int nTextType;
    int nAlignType;
    int nMaxStringLength;
    int nPassword;
    char szString[128];
};

struct BinProgress
{
    int nID;
    int nParentID;
    int nTextureSetIndex;
    int nCurrent;
    int nMaxValue;
    int nStartX;
    int nStartY;
    int nWidth;
    int nHeight;
    int nProgressColor;
    int nColor;
    int nStyle;
};
