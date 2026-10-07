#pragma once

namespace shade_grid
{
// Every vertex index must fit the effect's unsigned-short index buffer.
inline int BoundedSize(int grid) { return grid < 2 ? 2 : (grid > 255 ? 255 : grid); }
inline int VertexCount(int grid) { const int side = BoundedSize(grid) + 1; return side * side; }
inline int IndexCount(int grid) { const int size = BoundedSize(grid); return 6 * size * size; }
}
