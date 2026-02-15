## Plan: Pet locator in #overview

TL;DR — Add persistent pet-name summaries to the `mapseen` structure and show them in `#overview`. Pets are shown only on the level where they actually are (no stale traces). Persisted in the savefile (bump `EDITLEVEL`), show named pets by custom name and unnamed pets by species, and ensure mapseen updates when a pet leaves a level.

**Steps**
1. Create branch `feature/pet_locator`.
2. Data-model: add pet-name storage to `struct mapseen` (mirror `custom/custom_lth` pattern). *Depends on step 3.*
3. Collection: in `recalc_mapseen()` scan current-level monsters (`fmon`) and build a truncated comma-separated pet summary for `mapseen->petnames` (use `MNAME()` when present, otherwise species via `x_monnam()` / helper). Truncate to a safe byte cap (recommend 256 bytes). *Blocks UI + persistence updates.*
4. Source-level cleanup: when a pet leaves a level (pet removed from map via `relmon()` for migration/mydogs), remove it from that level's `mapseen->petnames` so no stale entries remain. *Ensures pets that follow the player are removed from the previous level.*
5. Persistence: write/read the new pet-name field in `save_mapseen()` / `load_mapseen()` (follow `custom` semantics and ordering). *Depends on step 2.*
6. Display: in `print_mapseen()` append an annotation line `pets: <list>` when `mapseen->petnames` exists. *Depends on step 3.*
7. Housekeeping: initialize/free `mapseen->petnames` in `init_mapseen()` / `rm_mapseen()` and include petnames size in `overview_stats()` reporting.
8. Savefile compatibility: increment `EDITLEVEL` in `include/patchlevel.h` and document the change in `nethack_save_file_guide.md`.
9. Tests: add unit + save/restore + integration tests (see Test section). Run CI and update changelog.

**Relevant files & insertion points**
- `include/dungeon.h` — extend `struct mapseen` with `char *petnames; unsigned petnames_lth;` placed near `custom/custom_lth`.
- `src/dungeon.c` — `recalc_mapseen()` (collect pet names); `save_mapseen()` / `load_mapseen()` (persist); `init_mapseen()` / `rm_mapseen()` (init/free); `print_mapseen()` (display); `overview_stats()` (account for text size).
- `src/mon.c` — `relmon()` (remove pet entry from source `mapseen` when monster is moved off-map to `mydogs` / `migrating_mons`).
- `include/patchlevel.h` — bump `EDITLEVEL`.
- Tests: add `test_overview_pets.c` (unit + save/restore) and update `test_save_restore.c`/integration tests accordingly.

**Behavior decisions (applied)**
- Show pets only on the level where they actually are (no stale traces). ✅
- Display presence only (no state tags like "sleeping/eating"). ✅
- Show every pet on a level (no per-pet count limit), but cap stored bytes (256 bytes recommended) to prevent excessive save growth. ✅
- Unnamed pets shown by species (e.g., "dog"). ✅
- Persist pet-summary in `mapseen` (requires `EDITLEVEL++`). ✅

**Estimate**
- Implementation + tests: 2–4 hours. Review/fixes: ~1 hour.
