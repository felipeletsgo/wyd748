import type { components } from './api.generated';
export type WebSession = components['schemas']['PlayerSessions']['sessions'][number];
type API = <T>(path: string, body?: Record<string, string>) => Promise<T>;

export function displayTime(value: string): string {
  const date = new Date(value);
  return Number.isFinite(date.getTime()) ? date.toLocaleString('en-US') : 'Unavailable';
}

// No polling: a list is a process-local snapshot, not an inventory of devices.
export class AccountSessions {
  private revision = 0;
  private busy = false;
  constructor(private list: HTMLElement, private status: HTMLElement, private refresh: HTMLButtonElement,
    private revokeAll: HTMLButtonElement, private api: API, private identity: () => string | undefined,
    private signedOut: () => void) {
    refresh.addEventListener('click', () => { void this.load(); });
    revokeAll.addEventListener('click', () => { void this.revoke(); });
  }
  clear() {
    this.revision++;
    this.list.replaceChildren();
    this.status.textContent = '';
    this.busy = false;
    this.setBusy(false);
  }
  private setBusy(value: boolean) {
    this.busy = value;
    this.refresh.disabled = value || !this.identity();
    this.revokeAll.disabled = value || !this.identity();
    this.list.querySelectorAll<HTMLButtonElement>('button').forEach(button => { button.disabled = value; });
  }
  private report(text: string, error = false) {
    this.status.textContent = text;
    this.status.dataset.error = String(error);
  }
  async load() {
    const identity = this.identity();
    if (this.busy || !identity) return;
    const revision = ++this.revision;
    this.setBusy(true); this.report('Loading website sessions…');
    try {
      const data = await this.api<components['schemas']['PlayerSessions']>('/sessions');
      if (revision !== this.revision || identity !== this.identity()) return;
      const fragment = document.createDocumentFragment();
      for (const [index, session] of data.sessions.entries()) {
        const row = document.createElement('li');
        const title = document.createElement('strong');
        title.textContent = session.current ? 'This website session' : `Other website session ${index + 1}`;
        const details = document.createElement('p');
        details.textContent = `Signed in: ${displayTime(session.createdAt)} · Last activity: ${displayTime(session.lastSeenAt)} · Expires without activity: ${displayTime(session.expiresAt)}`;
        const button = document.createElement('button');
        button.type = 'button'; button.className = 'button';
        button.textContent = session.current ? 'Logout here' : 'Revoke session';
        button.addEventListener('click', () => { void this.revoke(session.id); });
        row.append(title, details, button); fragment.append(row);
      }
      this.list.replaceChildren(fragment);
      this.report(`Session snapshot: ${displayTime(data.asOf)}`);
    } catch {
      if (revision === this.revision && identity === this.identity()) {
        this.list.replaceChildren();
        this.report('Unable to load website sessions. Sign in again if your session expired, or try Refresh.', true);
      }
    } finally {
      if (revision === this.revision) this.setBusy(false);
    }
  }
  async revoke(id?: string) {
    const identity = this.identity();
    if (this.busy || !identity) return;
    const revision = ++this.revision;
    this.setBusy(true); this.report('Revoking website sessions…');
    try {
      const data = await this.api<components['schemas']['PlayerSessionRevocation']>(id ? '/sessions/revoke' : '/sessions/revoke-all', id ? { id } : {});
      if (revision !== this.revision || identity !== this.identity()) return;
      if (data.signedOut) { this.clear(); this.signedOut(); return; }
      this.setBusy(false);
      await this.load();
    } catch {
      if (revision === this.revision && identity === this.identity()) {
        // The response can be lost after a successful revocation. Never claim rollback.
        this.report('Revocation was not confirmed. Refresh to check, or sign in again if you were logged out.', true);
      }
    } finally {
      if (revision === this.revision) this.setBusy(false);
    }
  }
}
