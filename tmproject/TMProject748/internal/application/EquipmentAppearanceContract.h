#pragma once
namespace equipment_appearance {
inline unsigned char VisualRefinement(int refinement) {
    return static_cast<unsigned char>(refinement < 0 ? 0 : refinement > 15 ? 15 : refinement);
}
inline unsigned char Legend(unsigned char grade, unsigned char refinement, unsigned char ancient) {
    const unsigned char gem = ancient >> 4;
    return refinement > 9 && grade <= 4 && gem ? static_cast<unsigned char>(gem + 8) : grade;
}
inline unsigned char Color(unsigned char refinement, unsigned char ancient) {
    const unsigned char dye = ancient & 15;
    return refinement > 9 ? (dye ? static_cast<unsigned char>(dye + 115) : 0) : ancient;
}
}
