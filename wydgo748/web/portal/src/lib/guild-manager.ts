import type { components } from './api.generated';
type API = <T>(path: string, body?: Record<string, string>) => Promise<T>;
type Management = components['schemas']['PlayerGuildManagement'];
type MarkResult = components['schemas']['PlayerGuildMarkResult'];

const ranks: Record<number, string> = { 9: 'Leader', 8: 'Officer', 7: 'Officer', 6: 'Officer', 0: 'Member' };
// A ready-to-use 16x12, 24-bit BMP is stored as is; other images are converted
// by the server. 64 KB matches the server limit.
const maxBytes = 64 * 1024;

function base64(bytes: Uint8Array): string {
  let binary = '';
  for (let i = 0; i < bytes.length; i += 0x8000) binary += String.fromCharCode(...bytes.subarray(i, i + 0x8000));
  return btoa(binary);
}

export class GuildManager {
  private revision = 0;
  private busy = false;
  private leader = false;
  constructor(private panel: HTMLElement, private title: HTMLElement, private details: HTMLElement,
    private preview: HTMLImageElement, private form: HTMLFormElement, private file: HTMLInputElement,
    private remove: HTMLButtonElement, private status: HTMLElement, private api: API) {
    form.addEventListener('submit', event => { event.preventDefault(); void this.upload(); });
    remove.addEventListener('click', () => { void this.clearMark(); });
  }
  clear() {
    this.revision++;
    this.panel.hidden = true;
    this.title.textContent = 'Guild';
    this.details.replaceChildren();
    this.preview.hidden = true;
    this.preview.removeAttribute('src');
    this.form.hidden = true;
    this.form.reset();
    this.report('');
  }
  private report(text: string, error = false) {
    this.status.textContent = text;
    this.status.dataset.error = String(error);
  }
  private setBusy(value: boolean) {
    this.busy = value;
    this.form.querySelectorAll<HTMLButtonElement | HTMLInputElement>('button, input').forEach(control => { control.disabled = value; });
    this.remove.disabled = value;
  }
  private showMark(mark: MarkResult['mark']) {
    if (mark) {
      this.preview.src = `${mark.url}?v=${encodeURIComponent(mark.updatedAt)}`;
      this.preview.hidden = false;
    } else {
      this.preview.hidden = true;
      this.preview.removeAttribute('src');
    }
    this.remove.hidden = !this.leader || !mark;
  }
  async load() {
    const revision = ++this.revision;
    try {
      const data = await this.api<Management>('/guild');
      if (revision !== this.revision) return;
      this.panel.hidden = false;
      this.leader = data.leader;
      this.title.textContent = `Guild: ${data.guild.name}`;
      const line = document.createElement('p');
      line.className = 'muted';
      line.textContent = `${data.character} · ${ranks[data.rank] ?? 'Member'} · ${data.guild.members.length} members · Fame ${data.guild.fame.toLocaleString('en-US')}`;
      this.details.replaceChildren(line);
      this.form.hidden = !data.leader || !data.marksEnabled;
      this.showMark(data.mark);
      this.report(data.leader ? '' : 'Only the guild leader can change the guild mark.');
    } catch {
      // Accounts without a guild simply do not see the section.
      if (revision === this.revision) this.panel.hidden = true;
    }
  }
  private async upload() {
    const chosen = this.file.files?.[0];
    if (this.busy || !this.leader || !chosen) { this.report('Choose an image first.', true); return; }
    if (chosen.size > maxBytes) { this.report('The image must be 64 KB or smaller.', true); return; }
    this.setBusy(true);
    this.report('Sending the guild mark…');
    try {
      const bytes = new Uint8Array(await chosen.arrayBuffer());
      const result = await this.api<MarkResult>('/guild/mark', { image: base64(bytes) });
      this.showMark(result.mark);
      this.form.reset();
      this.report('Guild mark saved. Players see it when their game loads your guild again.');
    } catch (error) {
      this.report(error instanceof Error ? error.message : 'The guild mark could not be saved.', true);
    } finally { this.setBusy(false); }
  }
  private async clearMark() {
    if (this.busy || !this.leader) return;
    this.setBusy(true);
    try {
      await this.api<MarkResult>('/guild/mark/delete', {});
      this.showMark(null);
      this.report('Guild mark removed.');
    } catch (error) {
      this.report(error instanceof Error ? error.message : 'The guild mark could not be removed.', true);
    } finally { this.setBusy(false); }
  }
}
