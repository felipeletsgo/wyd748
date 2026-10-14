export type RankedCharacter = { name: string; class: number; level: number; evolution: string };
export type RankingFilters = { search: string; class: string; evolution: string; page: number };
export const classNames = ['TransKnight', 'Foema', 'BeastMaster', 'Hunter'];
export const evolutionNames: Record<string, string> = { mortal: 'Mortal', arch: 'Arch', celestial: 'Celestial', subcelestial: 'Subcelestial' };
const pageSize = 20;

export function readFilters(params: URLSearchParams): RankingFilters {
  const selectedClass = params.get('class') ?? '';
  const evolution = params.get('evolution') ?? '';
  const page = Number(params.get('page') ?? 1);
  return { search: (params.get('q') ?? '').slice(0, 24),
    class: /^[0-3]$/.test(selectedClass) ? selectedClass : '',
    evolution: Object.hasOwn(evolutionNames, evolution) ? evolution : '',
    page: Number.isInteger(page) && page > 0 && page <= 5 ? page : 1 };
}

export function rankingPage(rows: RankedCharacter[], filters: RankingFilters) {
  const search = filters.search.trim().toLowerCase();
  // Filter the published top 100, never renumber global positions or claim
  // that this is a class-wide ranking of characters outside that snapshot.
  const matches = rows.slice(0, 100).map((character, index) => ({ character, rank: index + 1 }))
    .filter(({ character }) => (!search || character.name.toLowerCase().includes(search))
      && (!filters.class || String(character.class) === filters.class)
      && (!filters.evolution || character.evolution === filters.evolution));
  const pages = Math.max(1, Math.ceil(matches.length / pageSize));
  const page = Math.min(Math.max(1, filters.page), pages);
  return { rows: matches.slice((page - 1) * pageSize, page * pageSize), page, pages, total: matches.length };
}

export function mountRanking(root: HTMLElement) {
  const body = root.querySelector<HTMLTableSectionElement>('#ranking-characters')!;
  const search = root.querySelector<HTMLInputElement>('#ranking-search')!;
  const selectedClass = root.querySelector<HTMLSelectElement>('#ranking-class')!;
  const evolution = root.querySelector<HTMLSelectElement>('#ranking-evolution')!;
  const status = root.querySelector<HTMLElement>('#ranking-status')!;
  const position = root.querySelector<HTMLElement>('#ranking-page')!;
  const previous = root.querySelector<HTMLButtonElement>('#ranking-previous')!;
  const next = root.querySelector<HTMLButtonElement>('#ranking-next')!;
  const refresh = root.querySelector<HTMLButtonElement>('#ranking-refresh')!;
  const reset = root.querySelector<HTMLButtonElement>('#ranking-reset')!;
  const filtersGroup = root.querySelector<HTMLFieldSetElement>('fieldset')!;
  let filters = readFilters(new URLSearchParams(location.search));
  let rows: RankedCharacter[] = [];
  let loaded = false;
  let loading = false;
  function syncInputs() { search.value = filters.search; selectedClass.value = filters.class; evolution.value = filters.evolution; }
  function remember() {
    const url = new URL(location.href);
    for (const key of ['q', 'class', 'evolution', 'page']) url.searchParams.delete(key);
    if (filters.search) url.searchParams.set('q', filters.search);
    if (filters.class) url.searchParams.set('class', filters.class);
    if (filters.evolution) url.searchParams.set('evolution', filters.evolution);
    if (filters.page > 1) url.searchParams.set('page', String(filters.page));
    history.replaceState(null, '', url);
  }
  function render() {
    const data = rankingPage(rows, filters);
    filters.page = data.page;
    const fragment = document.createDocumentFragment();
    for (const { character, rank } of data.rows) {
      const row = document.createElement('tr');
      for (const value of [rank, character.name, classNames[character.class] ?? 'Unknown', character.level + 1, Object.hasOwn(evolutionNames, character.evolution) ? evolutionNames[character.evolution] : 'Unknown']) {
        const cell = document.createElement('td'); cell.textContent = String(value); row.append(cell);
      }
      fragment.append(row);
    }
    if (!data.rows.length) {
      const row = document.createElement('tr'); const cell = document.createElement('td');
      cell.colSpan = 5; cell.className = 'empty-row';
      cell.textContent = loaded ? rows.length ? 'No matches in the top 100.' : 'No ranked characters yet.' : 'Ranking unavailable.';
      row.append(cell); fragment.append(row);
    }
    body.replaceChildren(fragment);
    position.textContent = loaded ? `${data.page} / ${data.pages} · ${data.total} characters` : '';
    previous.disabled = !loaded || data.page <= 1;
    next.disabled = !loaded || data.page >= data.pages;
  }
  function change() {
    filters = readFilters(new URLSearchParams({ q: search.value, class: selectedClass.value, evolution: evolution.value }));
    render(); remember();
  }
  search.addEventListener('input', change);
  selectedClass.addEventListener('change', change); evolution.addEventListener('change', change);
  previous.addEventListener('click', () => { filters.page--; render(); remember(); });
  next.addEventListener('click', () => { filters.page++; render(); remember(); });
  reset.addEventListener('click', () => { filters = readFilters(new URLSearchParams()); syncInputs(); render(); remember(); });
  window.addEventListener('popstate', () => { filters = readFilters(new URLSearchParams(location.search)); syncInputs(); if (loaded) render(); });
  async function load() {
    if (loading) return;
    loading = true; refresh.disabled = true; filtersGroup.disabled = true;
    previous.disabled = true; next.disabled = true; root.setAttribute('aria-busy', 'true');
    status.textContent = 'Loading…'; status.dataset.error = 'false';
    try {
      const response = await fetch('/api/player/v1/ranking', { credentials: 'omit', cache: 'no-store', signal: AbortSignal.timeout(8000) });
      if (!response.ok) throw new Error('Unavailable');
      const data = await response.json();
      if (data.version !== 1 || !Array.isArray(data.characters) || data.characters.length > 100
        || typeof data.asOf !== 'string' || !Number.isFinite(Date.parse(data.asOf))
        || !data.characters.every((row: RankedCharacter) => row && typeof row.name === 'string'
          && Number.isInteger(row.class) && row.class >= 0 && row.class <= 255
          && Number.isInteger(row.level) && row.level >= 0 && row.level <= 4294967295
          && typeof row.evolution === 'string')) throw new Error('Invalid ranking');
      rows = data.characters; loaded = true;
      status.textContent = `Updated ${new Date(data.asOf).toLocaleString('en-US')}`;
    } catch {
      rows = []; loaded = false;
      status.textContent = 'Ranking unavailable. Try Refresh.'; status.dataset.error = 'true';
    } finally {
      loading = false; refresh.disabled = false; filtersGroup.disabled = !loaded;
      root.setAttribute('aria-busy', 'false'); render(); if (loaded) remember();
    }
  }
  refresh.addEventListener('click', () => { void load(); });
  syncInputs(); void load();
}
