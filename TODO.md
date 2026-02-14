# TODOs

A short list of items to investigate and prioritize. Reporter: user:Jeff
Date: 2026-02-10

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

- Type: Feature feature/weight-display
- This is already a feature for wizard mode as `wizweight` option; we just need to make it available for all players.
- Description: Add an optional weight display per inventory line when the user presses `F1`. Example desired output: `i) 3 swirly potions [60]` where `[60]` is the weight for that inventory line/item stack.
- Requirements/Design:
  - Only show weights when the user explicitly requests it (presses `F1` or similar toggle). Do not change default inventory listing.
  - For stacks, show combined weight for that line (e.g., 3 potions -> weight shown is total weight); for single items, show its weight.
  - Preserve existing keyboard mappings and non-intrusive UI behavior.
- Suggested files to inspect: `src/invent.c`, `src/cmd.c`, UI modules `src/win_*` / curses windows that render inventory.
- Next step: Prototype implemented — added per-line/stack weight display. The `invweight` option is available in `options`, via `NETHACKOPTIONS`, or in `.nethackrc` and can be persisted.
- Tests: added `test_invweight.sh` (smoke test). Manual verification: open inventory, press `W` to show/hide weights; stacks show total+per-item weight (e.g., `3 potions [60x20]`).
- Priority: P2 (prototype done — ready for review)

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
- Type: Feature feature/quiver-order
- Description: Provide a way for players to organize the quiver selection order in-game so they can control which ammunition (darts, arrows, daggers, etc.) is chosen first when firing or when autoquiver picks ammo. Ideally, support an interactive ordering UI (move up/down) and allow persistent preference using sticky `OPTIONS` mappings (e.g., allow `OPTIONS=q a` to prefer 'a' item for quiver slot 1 or mapping letters to quiver order).
- Behavior considerations:
  - Persist ordering across sessions (store in savefile or OPTIONS/sticky entries).
  - Provide a deterministic tie-breaker when multiple stacks of the same ammo type exist (e.g., prefer lowest inventory slot, or the one explicitly assigned by sticky `OPTIONS`).
  - Integrate with existing `Q` (quiver) and `f` (fire) commands and `autoquiver` behavior. When firing, follow the quiver priority order and skip empty/broken stacks.
  - Support both type-based ordering (prefer 'arrows' over 'darts') and specific-item assignments (assign specific inventory letters to priority positions via sticky `OPTIONS`).
  - Consider accessibility for keyboard-only workflows: add commands like `#quiver-order` to open an ordering dialog and keybindings to move items up/down.
- Suggested files to inspect: `src/wield.c` (`dowieldquiver`), `src/cmd.c` (`dofire`, `Q` binding), `src/invent.c` (inventory iteration and letter handling), and the sticky `sticky.diff` integration code.
- Next step: Prototype a simple quiver-order dialog, wire it into a new extended command (`#setquiver`), and add tests that verify firing follows specified ordering, including sticky OPTIONS mappings.
- Priority: P2




---

