# TODO 7 Implementation Plan (Shared Option Editor + Split Quiver)

## Scope
- Use a shared `#name`-style text editor path for selected in-game `O` options.
- Ensure edited fields are **prefilled with current value** (fixes prior empty `voice_engine` style prompt).
- Split quiver controls into four independent option channels:
  - `quiverorder_otypes`
  - `quiverorder_invlet`
  - `quiverorder_ignore_type`
  - `quiverorder_ignore_invlet`
- Keep legacy `quiverorder` as compatibility alias.

## Behavior rules
- `quiverorder_invlet` takes precedence over `quiverorder_otypes`.
- Ignore lists are always applied.
- Object-type token matching is exact canonical name (case-insensitive), `/` separated.
- Changes are in-memory and persist on next normal save.

## Persistence/compatibility
- Persist all four split channels independently in savefiles.
- Gate new payload using a savefile feature bit (`SFI1_QUIVER_SPLIT`) for backward compatibility.
- Do not change `struct flag`; no `EDITLEVEL` bump required.

## Files
- `src/options.c`: editor path, option table, parse/display logic.
- `src/do_name.c`, `include/extern.h`: shared input helper exposure.
- `src/quiver.c`: split channel state globals.
- `include/global.h`, `src/decl.c`: savefile feature flag wiring.
- `src/save.c`, `src/restore.c`: split channel save/restore payload.

## Validation
- Build with `make all`.
- Manual in-game check with Jeff:
  - verify prefills for `voice_engine`, `voice_command`, `fruit`, `packorder`, `disclose`.
  - verify split quiver precedence/ignore behavior survives save/restore.
- Optional: run save/restore valgrind scripts.
