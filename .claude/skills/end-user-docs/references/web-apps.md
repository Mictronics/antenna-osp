# Discovery notes: web apps (Vue and similar)

Read this when the product has a browser UI — including a UI served by a device.

## Finding the app

- Locate the `package.json` that depends on `vue` (or `react`, `svelte`, …). In a monorepo it is often nested (e.g. `packages/frontend`). Firmware projects often keep the web UI in `data/`, `www/`, `web/` or `ui/` and embed or serve it from flash/filesystem.
- Find the router (`router/index.ts`, `routes.ts`, file-based `pages/`) and enumerate every screen.

## What to learn per screen

1. **Who can see it.** Route guards, `meta` fields (`requiresAdmin`, `roles: [...]`), conditionally rendered nav items → everyone vs. admin/advanced.
2. **What it is for.** A settings page matters because it lets someone change their password or notification preferences, not because it has a form. Infer purpose from labels, placeholders, button text and validation messages in the templates, not from variable names.
3. **Exact visible labels** — button text, tab names, menu items — so steps can say "Click **Save**".
4. **Validation rules and limits** — messages and constraints shown to users become "Things to know".
5. **Multi-step flows** — wizards and sequences of routes become one article.

## Translating into docs

- "Filter the list by status", not "the filter prop".
- A pop-up confirming changes, not "Dialog".
- Skip loading states, error boundaries, redirects-only routes and technical settings users wouldn't recognize.
- Where an API call has a real-world consequence ("sends an email to the team", "may take a minute"), say that in user terms.
