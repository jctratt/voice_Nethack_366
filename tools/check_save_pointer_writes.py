#!/usr/bin/env python3
"""
Heuristic checker: find bwrite/mread calls that directly save/restore pointer variables.
- Finds pointer declarations (simple heuristics) and then searches for bwrite/mread lines that take the address of those identifiers.
- This is a heuristic tool (false positives/negatives possible) but catches obvious pointer-save bugs.

Usage: python3 tools/check_save_pointer_writes.py
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'src'
INCLUDE = ROOT / 'include'

ptr_decl_re = re.compile(r"\b(?:[A-Za-z_][A-Za-z0-9_\s\*]+)\*\s*([A-Za-z_][A-Za-z0-9_]*)")
struct_member_ptr_re = re.compile(r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\*\s*([A-Za-z_][A-Za-z0-9_]*)\s*;")

# collect identifier names that are declared as pointers (heuristic)
ptr_names = set()
for p in list((SRC).rglob('*.c')) + list((INCLUDE).rglob('*.h')):
    try:
        txt = p.read_text()
    except Exception:
        continue
    for m in ptr_decl_re.finditer(txt):
        name = m.group(1)
        ptr_names.add(name)

# also collect known struct-member pointer names for patterns like obj->oextra
member_ptrs = set()
for h in INCLUDE.rglob('*.h'):
    try:
        txt = h.read_text()
    except Exception:
        continue
    # crude: find lines inside struct blocks
    for m in struct_member_ptr_re.finditer(txt):
        member = m.group(2)
        member_ptrs.add(member)

suspicious = []
pattern = re.compile(r"\b(bwrite|mread)\s*\(\s*fd\s*,\s*\(genericptr_t\)\s*&([A-Za-z0-9_\.>\[\]']+)")
# match things like &ptr, &obj->member, &u.note_list
for p in SRC.rglob('*.c'):
    try:
        lines = p.read_text().splitlines()
    except Exception:
        continue
    for i, line in enumerate(lines, start=1):
        for m in pattern.finditer(line):
            call = m.group(1)
            target = m.group(2).strip()
            # simplify target to identifier or member name
            # handle 'u.note_list' -> 'note_list', 'obj->oextra' -> 'oextra'
            simple = re.split(r"->|\.|\[|\\s", target)[-1]
            simple = simple.strip(' ]\'"')
            if simple in ptr_names or simple in member_ptrs:
                suspicious.append((p.relative_to(ROOT), i, call, target, simple))

if not suspicious:
    print("No obvious direct pointer save/restore operations found (heuristic).")
    raise SystemExit(0)

print("Potential pointer save/restore usages found (heuristic):")
for f, lineno, call, target, simple in suspicious:
    print(f"{f}:{lineno}: {call} ... &{target}  (member/id='{simple}')")

print('\nNOTE: This is a heuristic check. If you want, I can show each location and suggest fixes.')
