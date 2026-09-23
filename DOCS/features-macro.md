# Water macro

This module owns event entry points and controller lifetimes for the water
macro. It does not change inherited item IDs, coordinates, opcodes, or packet
formats.

- `MacroMsg` receives the intent from `TMFieldScene` and selects the level.
- `MacroFactory` creates and retains one strategy for each valid level.
- `MacroLevel` still lives in `internal/core/` because it sends packets and
  depends on several monolithic facades. Moving it requires a separate batch
  that verifies the relevant client/server contracts.

Pointers returned by `MacroFactory` are borrowed. The factory alone owns the
strategies and releases them when the singleton shuts down.
