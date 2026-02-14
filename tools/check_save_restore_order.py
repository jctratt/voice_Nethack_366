#!/usr/bin/env python3
"""
Simple static checker to compare the sequence of bwrite() calls in save functions
with mread() calls in corresponding restore functions.  It is heuristic-based
but useful to find obvious 1:1 mismatches.

Usage: python3 tools/check_save_restore_order.py

It reports mismatches for these pairs (by default):
 - save_notes <-> restore_notes
 - saveobj <-> restobj
 - saveobjchn <-> restobjchn
 - savegamestate <-> restgamestate
 - savemon <-> restmon

This is not a semantic proof, but a developer aid.
"""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / 'src'
PAIRS = [
    ('save_notes', 'restore_notes', 'notes.c'),
    ('saveobj', 'restobj', 'save.c'),
    ('saveobjchn', 'restobjchn', 'save.c'),
    ('savemon', 'restmon', 'save.c'),
    ('savegamestate', 'restgamestate', 'save.c'),
    ('savelev', 'restlevelfile', 'save.c'),
]

bwrite_re = re.compile(r"\bbwrite\s*\(\s*fd\s*,\s*\(genericptr_t\)\s*([^,\)]+)\s*,\s*([^\)]+)\)\s*;")
mread_re = re.compile(r"\bmread\s*\(\s*fd\s*,\s*\(genericptr_t\)\s*([^,\)]+)\s*,\s*([^\)]+)\)\s*;")
func_re = re.compile(r"\b({fname})\b\s*\((?:[^)]*)\)\s*\{{", re.M)


def extract_calls(filename, funcname, write=True):
    content = filename.read_text()
    # find function start
    m = re.search(r"\b%s\b\s*\([^{]*\)\s*\{" % re.escape(funcname), content)
    if not m:
        return []
    start = m.end()
    # find end by counting braces
    brace = 1
    i = start
    n = len(content)
    while i < n and brace > 0:
        if content[i] == '{':
            brace += 1
        elif content[i] == '}':
            brace -= 1
        i += 1
    body = content[start:i]
    calls = []
    for line in body.splitlines():
        if write:
            m2 = re.search(r"\bbwrite\s*\(.*?\);", line)
            if m2:
                calls.append(line.strip())
        else:
            m2 = re.search(r"\bmread\s*\(.*?\);", line)
            if m2:
                calls.append(line.strip())
    return calls


def summarize_call(line):
    # Reduce to a canonical short form for comparison
    # Examples: bwrite(fd, (genericptr_t) &u, sizeof u) -> WRITE:&u:sizeof u
    bw = re.search(r"\bbwrite\s*\(\s*fd\s*,\s*\(genericptr_t\)\s*([^,\)]+)\s*,\s*([^\)]+)\)", line)
    if bw:
        target = bw.group(1).strip()
        size = bw.group(2).strip()
        return f"W|{target}|{size}"
    mr = re.search(r"\bmread\s*\(\s*fd\s*,\s*\(genericptr_t\)\s*([^,\)]+)\s*,\s*([^\)]+)\)", line)
    if mr:
        target = mr.group(1).strip()
        size = mr.group(2).strip()
        return f"R|{target}|{size}"
    # fallback: include the function-like name
    if 'bwrite' in line:
        return 'W|<complex>'
    if 'mread' in line:
        return 'R|<complex>'
    return 'UNK'


def compare_pair(save_calls, rest_calls):
    ssum = [summarize_call(l) for l in save_calls]
    rsum = [summarize_call(l) for l in rest_calls]
    # naive compare: walk both lists and report first difference
    minlen = min(len(ssum), len(rsum))
    diffs = []
    for i in range(minlen):
        if ssum[i] != rsum[i]:
            diffs.append((i, ssum[i], rsum[i]))
    if len(ssum) != len(rsum):
        diffs.append(('len', len(ssum), len(rsum)))
    return diffs, ssum, rsum


def main():
    failures = 0
    for save_name, rest_name, filename in PAIRS:
        fpath = SRC / filename
        if not fpath.exists():
            print(f"SKIP: {filename} not found")
            continue
        save_calls = extract_calls(fpath, save_name, write=True)
        # Prefer to find restore() in the same file as save(); fall back to restore.c
        rest_calls = extract_calls(fpath, rest_name, write=False)
        if not rest_calls and 'rest' in rest_name:
            rest_calls = extract_calls(SRC / 'restore.c', rest_name, write=False)
        diffs, ssum, rsum = compare_pair(save_calls, rest_calls)
        print('\n==== compare %s()  <->  %s()  (in %s) ====' % (save_name, rest_name, filename))
        print(f"save:  {len(ssum)} calls, restore: {len(rsum)} calls")
        if not diffs:
            print('OK: call sequences look aligned (text heuristic)')
        else:
            failures += 1
            print('MISMATCHES:')
            for d in diffs[:5]:
                print('  at', d)
            print('\n  first 10 save summaries:')
            for x in ssum[:10]:
                print('   ', x)
            print('  first 10 restore summaries:')
            for x in rsum[:10]:
                print('   ', x)
    if failures:
        print(f"\nCHECKER: found {failures} mismatch groups — investigate those functions.")
        return 2
    print('\nCHECKER: no obvious mismatches found among checked pairs.')
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
