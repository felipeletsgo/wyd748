type Entry = { id: number; name: string; class?: number | null };
export type Filters = { search: string; selectedClass: string; page: number };
const classes = ['TransKnight', 'Foema', 'BeastMaster', 'Hunter'];
const pageSize = 20;
export function readFilters(params: URLSearchParams): Filters {
  const selectedClass = params.get('class') ?? '';
  const page = Number(params.get('page') ?? 1);
  return { search: (params.get('q') ?? '').slice(0, 64), selectedClass: /^[0-3]$/.test(selectedClass) || selectedClass === 'shared' ? selectedClass : '', page: Number.isSafeInteger(page) && page > 0 ? page : 1 };
}
export function searchPage(entries: Entry[], filters: Filters) {
  const search = filters.search.trim().toLowerCase();
  const matches = entries.filter(entry => (!search || entry.name.toLowerCase().includes(search) || String(entry.id) === search)
    && (!filters.selectedClass || (entry.class === null ? 'shared' : String(entry.class)) === filters.selectedClass));
  const pages = Math.max(1, Math.ceil(matches.length / pageSize));
  const page = Math.min(filters.page, pages);
  return { entries: matches.slice((page - 1) * pageSize, page * pageSize), page, pages, total: matches.length };
}
export function mountCatalog(root: HTMLElement) {
  const kind = root.dataset.kind === 'skills' ? 'skills' : root.dataset.kind === 'quests' ? 'quests' : 'items';
  const search = root.querySelector<HTMLInputElement>('[data-search]')!;
  const selectedClass = root.querySelector<HTMLSelectElement>('[data-class]');
  const previous = root.querySelector<HTMLButtonElement>('[data-previous]')!;
  const next = root.querySelector<HTMLButtonElement>('[data-next]')!;
  const reset = root.querySelector<HTMLButtonElement>('[data-reset]')!;
  const body = root.querySelector<HTMLTableSectionElement>('tbody')!;
  const status = root.querySelector<HTMLElement>('[data-status]')!;
  const position = root.querySelector<HTMLElement>('[data-page]')!;
  const fieldset = root.querySelector<HTMLFieldSetElement>('fieldset')!;
  let filters = readFilters(new URLSearchParams(location.search));
  let entries: Entry[] = [];
  function inputs() { search.value = filters.search; if (selectedClass) selectedClass.value = filters.selectedClass; }
  function remember() {
    const url = new URL(location.href);
    for (const key of ['q', 'class', 'page']) url.searchParams.delete(key);
    if (filters.search) url.searchParams.set('q', filters.search);
    if (filters.selectedClass && kind === 'skills') url.searchParams.set('class', filters.selectedClass);
    if (filters.page > 1) url.searchParams.set('page', String(filters.page));
    history.replaceState(null, '', url);
  }
  function render() {
    if (kind !== 'skills') filters.selectedClass = '';
    const result = searchPage(entries, filters); filters.page = result.page;
    const fragment = document.createDocumentFragment();
    for (const entry of result.entries) {
      const row = document.createElement('tr'); const id = document.createElement('td'); id.textContent = String(entry.id);
      const name = document.createElement('td'); const link = document.createElement('a');
      link.href = `/database/${kind}/${entry.id}/`; link.textContent = entry.name; name.append(link); row.append(id, name);
      if (kind === 'skills') { const cell = document.createElement('td'); cell.textContent = entry.class === null ? 'Shared' : classes[entry.class!] ?? 'Unknown'; row.append(cell); }
      fragment.append(row);
    }
    if (!result.entries.length) { const row = document.createElement('tr'); const cell = document.createElement('td'); cell.colSpan = kind === 'skills' ? 3 : 2; cell.className = 'empty-row'; cell.textContent = 'No matches.'; row.append(cell); fragment.append(row); }
    body.replaceChildren(fragment); position.textContent = `${result.page} / ${result.pages} · ${result.total} ${result.total === 1 ? kind.slice(0, -1) : kind}`;
    previous.disabled = result.page <= 1; next.disabled = result.page >= result.pages;
  }
  function change() { filters = readFilters(new URLSearchParams({ q: search.value, class: selectedClass?.value ?? '' })); render(); remember(); }
  search.addEventListener('input', change); selectedClass?.addEventListener('change', change);
  previous.addEventListener('click', () => { filters.page--; render(); remember(); });
  next.addEventListener('click', () => { filters.page++; render(); remember(); });
  reset.addEventListener('click', () => { filters = readFilters(new URLSearchParams()); inputs(); render(); remember(); });
  window.addEventListener('popstate', () => { filters = readFilters(new URLSearchParams(location.search)); inputs(); if (!fieldset.disabled) render(); });
  inputs();
  void (async () => {
    try {
      const response = await fetch('/database/index.json', { credentials: 'omit', signal: AbortSignal.timeout(8000) });
      if (!response.ok) throw new Error('Unavailable');
      const data = await response.json(); const rows = data[kind];
      if (data.schemaVersion !== 2 || data.contentHash !== root.dataset.hash || !Array.isArray(rows) || rows.length > 6500
        || !rows.every((row: Entry) => row && Number.isSafeInteger(row.id) && row.id >= 0 && (kind === 'quests' || row.id <= 6500) && typeof row.name === 'string'
          && (kind !== 'skills' || row.class === null || Number.isInteger(row.class) && row.class! >= 0 && row.class! <= 3))) throw new Error('Invalid catalog');
      entries = rows; fieldset.disabled = false; reset.disabled = false; render(); remember(); status.textContent = '';
    } catch { status.textContent = 'Search unavailable. Reload to try again.'; status.dataset.error = 'true'; }
  })();
}
