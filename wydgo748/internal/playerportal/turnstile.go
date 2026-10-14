package playerportal

import (
	"context"
	"encoding/json"
	"errors"
	"io"
	"net/http"
	"net/netip"
	"net/url"
	"strings"
	"time"
)

const turnstileEndpoint = "https://challenges.cloudflare.com/turnstile/v0/siteverify"

var (
	errVerificationRejected    = errors.New("portal: human verification rejected")
	errVerificationUnavailable = errors.New("portal: human verification unavailable")
)

// RegistrationVerifier must reject invalid, expired and previously used tokens.
// A successful verification is required before password work or database access.
type RegistrationVerifier interface {
	Verify(context.Context, string, string) error
}

type Turnstile struct {
	secret, hostname, action string
	client                   *http.Client
}

// NewTurnstile keeps the secret server-side and binds responses to this site.
func NewTurnstile(secret, origin string) (*Turnstile, error) {
	return NewTurnstileAction(secret, origin, "register")
}

// Action binding prevents registration tokens from authorizing a login.
func NewTurnstileAction(secret, origin, action string) (*Turnstile, error) {
	if action != "register" && action != "login" && action != "staff_login" {
		return nil, errors.New("portal: invalid Turnstile action")
	}
	u, err := url.Parse(origin)
	if err != nil {
		return nil, errors.New("portal: invalid Turnstile origin")
	}
	localIP, localErr := netip.ParseAddr(u.Hostname())
	localHTTP := u.Scheme == "http" && localErr == nil && localIP.IsLoopback()
	if (!localHTTP && u.Scheme != "https") || u.Hostname() == "" || u.User != nil || u.Path != "" || u.RawQuery != "" || u.ForceQuery || u.Fragment != "" {
		return nil, errors.New("portal: Turnstile requires an exact HTTPS or literal-loopback HTTP origin")
	}
	if secret == "" || len(secret) > 256 || strings.TrimSpace(secret) != secret || strings.ContainsAny(secret, "\r\n") {
		return nil, errors.New("WYD_TURNSTILE_SECRET is required and must be a single nonempty value")
	}
	// Cloudflare's published dummy secrets must never enable production registration.
	for _, dummy := range []string{"1x0000000000000000000000000000000AA", "2x0000000000000000000000000000000AA", "3x0000000000000000000000000000000AA"} {
		if secret == dummy {
			return nil, errors.New("portal: production Turnstile cannot use a test secret")
		}
	}
	return &Turnstile{secret: secret, hostname: u.Hostname(), action: action, client: &http.Client{
		Timeout:       3 * time.Second,
		CheckRedirect: func(*http.Request, []*http.Request) error { return http.ErrUseLastResponse },
	}}, nil
}

func validTurnstileToken(token string) bool {
	return len(token) > 0 && len(token) <= 2048 && strings.TrimSpace(token) == token
}

func (v *Turnstile) Verify(ctx context.Context, token, peer string) error {
	if !validTurnstileToken(token) {
		return errVerificationRejected
	}
	ip, err := netip.ParseAddr(peer)
	if err != nil {
		return errVerificationRejected
	}
	ctx, cancel := context.WithTimeout(ctx, 3*time.Second)
	defer cancel()
	form := url.Values{"secret": {v.secret}, "response": {token}, "remoteip": {ip.Unmap().String()}}
	r, err := http.NewRequestWithContext(ctx, http.MethodPost, turnstileEndpoint, strings.NewReader(form.Encode()))
	if err != nil {
		return errVerificationUnavailable
	}
	r.Header.Set("Content-Type", "application/x-www-form-urlencoded")
	response, err := v.client.Do(r)
	if err != nil {
		// Never return request errors: they can contain credentials or token values.
		return errVerificationUnavailable
	}
	defer response.Body.Close()
	if response.StatusCode != http.StatusOK {
		return errVerificationUnavailable
	}
	body, err := io.ReadAll(io.LimitReader(response.Body, 8193))
	if err != nil || len(body) > 8192 {
		return errVerificationUnavailable
	}
	var result struct {
		Success  bool     `json:"success"`
		Hostname string   `json:"hostname"`
		Action   string   `json:"action"`
		Errors   []string `json:"error-codes"`
	}
	if json.Unmarshal(body, &result) != nil {
		return errVerificationUnavailable
	}
	for _, code := range result.Errors {
		if code == "internal-error" || code == "invalid-input-secret" || code == "missing-input-secret" || code == "bad-request" {
			return errVerificationUnavailable
		}
	}
	if !result.Success || len(result.Errors) != 0 || result.Hostname != v.hostname || result.Action != v.action {
		return errVerificationRejected
	}
	return nil
}
