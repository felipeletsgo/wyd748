import type { components } from '../lib/api.generated';
type History = components['schemas']['MetricsHistory'];
type Point = components['schemas']['MetricsPoint'];
type API = <T>(path: string, method?: string, body?: unknown) => Promise<T>;

const overflowMicros = 1000001;
const milliseconds = (micros: number) => micros >= overflowMicros ? '>1000 ms' : `${(micros / 1000).toLocaleString('en-US', { maximumFractionDigits: 1 })} ms`;
export const charts: { title: string; value: (p: Point) => number; format: (v: number) => string }[] = [
  { title: 'Online players', value: p => p.players, format: v => v.toLocaleString('en-US') },
  { title: 'Loop lag p99', value: p => p.loopLagP99Micros, format: milliseconds },
  { title: 'Tick duration p99', value: p => p.tickP99Micros, format: milliseconds },
  { title: 'Command queue age p99', value: p => p.queueAgeP99Micros, format: milliseconds },
  { title: 'PostgreSQL queue', value: p => p.postgresQueueDepth, format: v => v.toLocaleString('en-US') },
  { title: 'Heap memory', value: p => p.heapBytes, format: v => `${(v / 1048576).toFixed(0)} MiB` },
];
const breakerReasons: Record<string, string> = {
  loop_lag: 'the game loop is running late',
  command_queue_age: 'commands are waiting too long in the game queue',
  persistence_backlog: 'the PostgreSQL write queue is backed up',
};

// Builds polyline coordinates scaled into a width x height box.
export function sparkline(values: number[], width: number, height: number) {
  if (!values.length) return '';
  const top = Math.max(...values, 1);
  const step = values.length > 1 ? width / (values.length - 1) : 0;
  return values.map((v, i) => `${(i * step).toFixed(1)},${(height - (v / top) * height).toFixed(1)}`).join(' ');
}

export function setupRuntimeCharts(api: API, explain: (error: unknown) => string) {
  const el = <T extends HTMLElement = HTMLElement>(id: string) => document.getElementById(id) as T;
  const svgNS = 'http://www.w3.org/2000/svg';
  let busy = false;
  let generation = 0;

  function render(value: History) {
    if (value.version !== 1 || !Array.isArray(value.points) || value.points.length > 360) throw new Error('Invalid metrics history');
    const breaker = el('runtime-breaker');
    breaker.hidden = !value.breaker.open;
    breaker.textContent = value.breaker.open
      ? `Load protection is active because ${breakerReasons[value.breaker.reason] ?? 'the server is under load'}. Searches, filters and later pages are paused; the first page of players stays available.`
      : '';
    const cards = charts.map(chart => {
      const values = value.points.map(chart.value);
      const card = document.createElement('figure'); card.className = 'runtime-card';
      const caption = document.createElement('figcaption');
      const title = document.createElement('span'); title.textContent = chart.title;
      const latest = document.createElement('strong'); latest.textContent = values.length ? chart.format(values[values.length - 1]) : '—';
      const peak = document.createElement('small'); peak.textContent = values.length ? `peak ${chart.format(Math.max(...values))}` : 'no samples yet';
      caption.append(title, latest, peak);
      const svg = document.createElementNS(svgNS, 'svg');
      svg.setAttribute('viewBox', '0 0 240 48'); svg.setAttribute('preserveAspectRatio', 'none'); svg.setAttribute('aria-hidden', 'true');
      const line = document.createElementNS(svgNS, 'polyline');
      line.setAttribute('points', sparkline(values, 240, 46)); line.setAttribute('fill', 'none');
      svg.append(line);
      card.append(caption, svg);
      return card;
    });
    el('runtime-grid').replaceChildren(...cards);
    el('runtime-status').textContent = value.points.length
      ? `${value.points.length} point${value.points.length === 1 ? '' : 's'} · ${new Date(value.points[0].at).toLocaleString('en-US')} to ${new Date(value.points[value.points.length - 1].at).toLocaleString('en-US')}. No automatic refresh.`
      : 'The sampler has no interval yet; try again in 10 seconds.';
  }

  async function refresh() {
    if (busy) return;
    const current = generation;
    busy = true; el<HTMLButtonElement>('runtime-refresh').disabled = true;
    try {
      const value = await api<History>(`/staff/metrics?${new URLSearchParams({ window: el<HTMLSelectElement>('runtime-window').value })}`);
      if (current !== generation) return;
      render(value);
    } catch (error) {
      if (current !== generation) return;
      el('runtime-status').textContent = explain(error);
    } finally { if (current === generation) { busy = false; el<HTMLButtonElement>('runtime-refresh').disabled = false; } }
  }

  el('runtime-refresh').addEventListener('click', () => void refresh());
  el('runtime-window').addEventListener('change', () => void refresh());
  return { refresh, reset() {
    generation++; busy = false;
    el<HTMLButtonElement>('runtime-refresh').disabled = false;
    el('runtime-grid').replaceChildren(); el('runtime-breaker').hidden = true;
    el('runtime-status').textContent = 'Not queried.';
  } };
}
