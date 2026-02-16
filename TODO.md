# TODOs (merged)

Merged from stashed TODO (stash@{0}) + local additions — merged on 2026-02-15.

A short list of items to investigate and prioritize. Reporter: user:Jeff
Date: 2026-02-10 (original)

---

1) **#showlines should skip known monsters** ✅

Done

---

2) **#intrinsics tracking must NOT reveal item sources without enlightenment** ⚠️

- Type: Bug / Gameplay balance
- Description: Running `#intrinsics` currently reveals which items grant an intrinsic even when the player has not been enlightened (i.e., has no way of knowing). Example: I was wearing a ring and `#intrinsics` identified the ring as the source despite no enlightenment event occurring. This is a cheat.
- Reproduction steps:
  1. Equip a ring that grants an intrinsic.
  2. Do not trigger any enlightenment event (level-up, identify, or other mechanic that legitimately reveals the source).
  3. Run `#intrinsics` and observe that it identifies the ring.
- Expected behavior: `#intrinsics` should list known intrinsics and not reveal their sources (items) unless the player has been legitimately enlightened about that source.
- Suggested files to inspect: `src/intrinsics.c` (or wherever intrinsic tracking and `#intrinsics` command is implemented), `src/invent.c` (item state helpers), UI handlers for `#intrinsics`.
- Next step: Add tests to ensure `#intrinsics` output does not include item sources unless enlightenment flag/event is set.
- Priority: P0

---

3) **Inventory weight per line when pressing `F1`** ✨
DONE

---

4) **Sticky OPTIONS assignment: support `OPTIONS=<letter> <item>`** ✨

- Type: Feature
- Description: Enhance the sticky/OPTIONS handling so a player can specify assignments like `OPTIONS=p pickaxe` (or `OPTIONS=P Pickaxe`) and have the game automatically assign that inventory letter to a matching item if it exists in the player's inventory at game start (or immediately after load). Multiple `OPTIONS` lines should be allowed so players can prebind several items.
- Behavior considerations:
  - If multiple matching items exist, choose a deterministic preference (first found, or the best match by full name); document the behavior.
  - If the requested letter is already used, either prefer the sticky assignment (reassign) or skip with a logged message; pick one and document it.
  - The assignment should happen after inventory is loaded/restored so it works on saved games as well as new starts.
  - Case-insensitive item matching would improve usability but require clear specification.
- Suggested files to inspect: `src/options.c` (OPTIONS parsing), `src/invent.c` (letter assignment helpers), `sys/unix/sticky.diff` or `sticky.patch` (patch integration), and the startup/load sequence where inventory is initialized.
- Next step: Implement OPTIONS parser to accept `<letter> <item>` pairs, add an application point after inventory load to assign letters when matches exist, and add tests to verify behavior under these scenarios.
- Priority: P2

---

5) **Quiver ordering and in-game quiver management** ✨
DONE

---

6) **Improve pet → hostile-monster tracking (pet targeting & pursuit)**

- Type: Feature / AI / Gameplay
- feature/pet-tracking-improvements
- Description: Improve tame-pet targeting so pets reliably detect and pursue *hostile* monsters on the same level while avoiding overly distant or suicidal moves.
- Goals / Acceptance criteria:
  - Pets (tame only) prioritize hostile monsters within a bounded radius (recommend ~7 tiles).
  - Pets must not move to a square that increases the player's distance to the pet beyond a safe limit (recommend max 5 tiles from player).
  - Pets prefer hostile targets over neutral/peaceful ones and avoid obvious suicide (traps, petrifiers, lethal passive damage).
  - No cross-level chasing by default; prefer ephemeral in-memory behaviour (avoid `struct edog` changes unless necessary).
- Suggested files to inspect: `src/dogmove.c`, `src/dog.c`; only touch `include/mextra.h` / `src/save.c` / `src/restore.c` if persistent edog fields become required.
- Next step: Implement ephemeral-only changes on branch `feature/pet-tracking-improvements`; follow up with unit + integration tests.
- Priority: P2

---

7) **In-game `OPTIONS` editor + split quiver controls**

- Type: Feature / UX
- Status: Done — implemented and validated (startup + savefile + inventory display + invlet rules verified)

  - Notes: quiver pre-draw sync removes stale `[qN]` markers; ignore/invlet rules are now authoritative at assignment time; sticky-inventory pickup-letter collisions fixed; case-sensitive invlet semantics preserved.
  - Follow-up: add automated unit tests for `display_pickinv()` qc-list behavior and a pickup menu regression test (TODO).
- Request:
  - Use a shared `#name`-style text editor path for selected in-game `O` options.
  - Prefill editor inputs with the current option value (not blank).
  - Replace single quiver editor with split channels:
    - `quiverorder_otypes`
    - `quiverorder_invlet`
    - `quiverorder_ignore_type`
    - `quiverorder_ignore_invlet`
- Rules:
  - `quiverorder_invlet` precedence over `quiverorder_otypes`.
  - Ignore lists always applied.
  - Keep `/` as internal separator for object-type lists.
  - Keep legacy `quiverorder` as compatibility alias.
- Save/restore:
  - Persist all four split fields independently via savefile feature bit.
  - Keep backward compatibility; avoid `struct flag` changes and `EDITLEVEL` bump.
- Suggested files: `src/options.c`, `src/do_name.c`, `src/quiver.c`, `src/save.c`, `src/restore.c`, `include/global.h`, `src/decl.c`.
- Priority: P2

---

Notes
- Prefer ephemeral (in-memory) solutions to avoid savefile `EDITLEVEL` bumps.
- Keep pet search radius and performance impact conservative (≤7 tiles recommended).
