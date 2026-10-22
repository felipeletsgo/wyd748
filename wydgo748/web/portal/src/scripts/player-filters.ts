// Mirrors control.ParseQuery: bounded integers, x/y/radius together, and
// minLevel not above maxLevel. Empty fields mean "any".
const fields: [string, number, number][] = [
  ['minLevel', 1, 1000], ['maxLevel', 1, 1000], ['class', 0, 255], ['guild', 1, 65535],
  ['x', 0, 4095], ['y', 0, 4095], ['radius', 1, 256],
];

export function playerFilters(raw: Record<string, string>): { filters?: Record<string, string>; error?: string } {
  const filters: Record<string, string> = {};
  for (const [name, low, high] of fields) {
    const value = (raw[name] ?? '').trim();
    if (!value) continue;
    const number = Number(value);
    if (!Number.isInteger(number) || number < low || number > high) return { error: `${name} must be a whole number from ${low} to ${high}.` };
    filters[name] = String(number);
  }
  const area = ['x', 'y', 'radius'].filter(name => name in filters).length;
  if (area !== 0 && area !== 3) return { error: 'An area filter needs X, Y and radius together.' };
  if (filters.minLevel && filters.maxLevel && Number(filters.minLevel) > Number(filters.maxLevel)) return { error: 'Min level must not be above max level.' };
  return { filters };
}
