#pragma once
// Integrated equipmentDamage contract. Native item refinement is tested separately.
// Columns: class, leftUnique, rightUnique, leftDamage, rightDamage, otherDamage, learned, expected.
namespace equipment_damage_fixture {
inline constexpr int Cases[][8] = {
    {0, 41, 41, 101, 50, 7, 0, 133},
    {0, 41, 42, 101, 50, 7, 0, 123},
    {3, 41, 42, 101, 50, 7, 1024, 128},
    {3, 41, 41, 101, 50, 7, 1024, 138},
    {0, 41, 42, 101, 50, 7, 512, 123},
    {1, 47, 45, 101, 50, 7, 0, 123},
    {0, 0, 41, 0, 101, 7, 0, 108},
    {0, 41, 0, 101, 0, 7, 0, 108},
    {3, 41, 41, 1, 2, 0, 1024, 2},
    {3, 41, 42, 1, 2, 0, 1024, 2},
    {0, 0, 0, 0, 0, 7, 0, 7},
};
}
