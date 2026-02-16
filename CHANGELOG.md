# Changelog

Commits since `README.md` was last updated (commit c012979 — 2025-03-05):

- UNCOMMITTED 2026-02-16 Jeff Tratt — FIX: quiver & inventory
  - Prevent excluded quiver `invlet`/`otype` entries from being readied or shown; add pre-display quiver sync so inventory never shows stale `[qN]` markers at startup or after option changes.
  - Restore strict case-sensitive invlet matching (don't treat `b` as `B`).
  - FIX: reserve sticky-inventory letters for pickup menus so Alt+Y (sticky) items are never collided by floor pick-up accelerators.
  - Files: `src/quiver.c`, `src/invent.c`, `src/pickup.c`, `src/wield.c`, `include/extern.h`, docs (`TODO.md`, `CHANGELOG.md`).
  - Tested: new-game startup, save/restore, pickup menu and sticky-letter collisions.


- UNCOMMITTED 2026-02-15 Jeff Tratt — UI: take-off menu — add "All metal" and "All non‑metal" material filters for `A` / `takeoffall`; selecting either filters worn/wielded items by material (metal vs. non‑metal) and reuses existing multi‑takeoff flow.
- FIX: pets no longer attempt to move into the player's square when interposing; visible interpose actions announce a type-appropriate sound (dog → "growls", cat → "hisses", bird → "screeches", rodent/spider → "chitters", etc.) to make hallway interposes obvious.
- ADD: 2026-02-15 Jeff Tratt — Feature: `#overview` pet locator (feature/pet_locator)
  - Persist per-level short pet-name summary in `mapseen->petnames` and show `pets: ...` in `#overview` for levels that contain any of the player's tamed pets. Unnamed pets display by species. Ensures pets that follow the player are removed from the previous level's overview. Savefile `EDITLEVEL` incremented due to `mapseen` layout change; old saves/bones will be incompatible.

- bbe76a5 / b550dea0 2026-02-14 Jeff Tratt — Feature: `OPTIONS=invweight` — add persistent per-item inventory weight display (format: `[TOTALxITEM]`, e.g. `[60x20]` for 3 items of 20 each). Implemented persistent `flags.invweight`, added inventory display, documented option in `dat/opthelp`, and updated save/restore handling. NOTE: `EDITLEVEL` was incremented because `struct flag` layout changed (old savefiles/bones invalidated). Replaced an expect-based test with in-source changes; see tests for follow-up unit-style coverage.



- UNCOMMITTED 2026-02-07 Jeff Tratt — Add `show_boomerang_trajectory` option; make `f` open #showboomerang when boomerang is quivered; pressing `f` inside #showboomerang fires the quivered boomerang or hands off to `t` when no boomerang is quivered; remove auto-opening from `t`; fix display cleanup, terminal state, and direction handoff behavior.

- d4f0d43 2026-02-09 Jeff Tratt — Fix: pressing `f` in #showboomerang now fires a quivered boomerang even when the preview was started directly; add `showboom_called_from_dofire` guard and extern; tested via manual smoke tests.

- 8db68ef 2026-02-07 Jeff Tratt — Add #showboomerang interactive preview for boomerang trajectories; integrate #showlines into 't' (throw) and 'f' (fire/quiver) flows; map keypad-center to '5' to clear preview; pressing '5' now clears preview and restarts the command.

- 6c2e5be 2026-02-06 Jeff Tratt — Add #showlines: transient sight-lines aid (curses highlights + tmp_at fallback), persistent until move, auto-clear on cancel/move, integrate with 'z' and 'Z'
- 6376f7f 2026-02-05 Jeff Tratt — color: show hero color for confuse-monster charges and force redraw
- 84e4ff0 2026-02-05 Jeff Tratt — name: use price-identify logic for #name floor price ranges; compute surcharge variant to avoid L0 ranges
- 8a2cde9 2026-02-04 Jeff Tratt — priceid: show alternate candidate prices; move category selection earlier; add Ctrl-B/Ctrl-S toggle and Ctrl-G category jump; credit dizzy (Aubrey Raech/dizzyprice)
- 935e871 2026-02-04 Jeff Tratt — docall: add appearance-based short prefill and labeled shop L/H hints; use shop_get_cost/shop_set_cost
- ff3ecbb 2026-02-03 Jeff Tratt — Reapply WIP tile options and resolve BL_TILE merge conflicts
- a42aa3d 2026-02-03 Jeff Tratt — Add BL_TILE status field; populate with do_screen_description; fix do_oname not to set type name
- fc448c5 2026-02-03 Jeff Tratt — Implement engraving status display and voice messages
- ee2b7e2 2026-02-03 Jeff Tratt — Add Ctrl+Arrow key support for word jumping in naming dialog
- 5ecc497 2025-03-09 Jeff Tratt — Update pline.c with TTS_CMD changes from Claude
- d95853e 2025-03-09 Jeff Tratt — A bit of cleanup
- 79beda6 2025-03-09 Jeff Tratt — TTS to espeak from gTTS
- 7a929ec 2025-03-09 Jeff Tratt — Ignore make_all files in repo root
- 9353c85 2025-03-09 Jeff Tratt — Remove outdated make_all output files
- 372ebbe 2025-03-09 Jeff Tratt — Ignore build artifacts in parent directory

Notes:
- This list contains every commit on `main` since the README was last updated. For more context or to build release notes, I can expand each entry with file-level diffs or PR links.
