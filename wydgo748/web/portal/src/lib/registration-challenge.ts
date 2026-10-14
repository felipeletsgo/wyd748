type TurnstileAPI = {
  render(container: HTMLElement, options: Record<string, unknown>): string;
  reset(id: string): void;
};
declare global { interface Window { turnstile?: TurnstileAPI } }

// Keep the token in memory, never storage or logs. Every submitted token is
// single-use, including submissions that fail after successful verification.
export class RegistrationChallenge {
  token = '';
  private widget: string | undefined;
  constructor(private container: HTMLElement, private changed: (error?: string) => void) {}

  async start(): Promise<void> {
    const sitekey = this.container.dataset.sitekey;
    if (!sitekey) throw new Error('Human verification is not configured.');
    const action = this.container.dataset.action ?? 'register';
    if (!['register', 'login', 'staff_login'].includes(action)) throw new Error('Invalid verification action.');
    await new Promise<void>((resolve, reject) => {
      const script = document.createElement('script');
      let settled = false;
      const fail = () => {
        if (settled) return;
        settled = true; window.clearTimeout(timeout); script.remove();
        reject(new Error('Human verification unavailable. Reload this page to try again.'));
      };
      const timeout = window.setTimeout(fail, 15000);
      script.src = 'https://challenges.cloudflare.com/turnstile/v0/api.js?render=explicit';
      script.async = true;
      script.onload = () => {
        if (settled) return;
        const api = window.turnstile;
        if (!api) { fail(); return; }
        try {
          // onload guarantees the asynchronous API script has executed.
          // Turnstile rejects ready() for scripts loaded with async/defer.
          this.widget = api.render(this.container, {
            sitekey, action, theme: 'dark', size: 'flexible', language: 'en',
            'response-field': false,
            callback: (token: string) => { this.token = token; this.changed(); },
            'expired-callback': () => { this.token = ''; this.changed('Human verification expired. Please verify again.'); },
            'timeout-callback': () => { this.token = ''; this.changed('Human verification timed out. Please verify again.'); },
            'error-callback': () => { this.token = ''; this.changed('Human verification unavailable. Please retry or reload this page.'); },
          });
          settled = true; window.clearTimeout(timeout); resolve();
        } catch { fail(); }
      };
      script.onerror = fail;
      document.head.append(script);
    });
  }

  reset(): void {
    this.token = '';
    this.changed();
    if (this.widget !== undefined) window.turnstile?.reset(this.widget);
  }
}
