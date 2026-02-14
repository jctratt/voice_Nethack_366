# NetHack 3.6.6 Save File Modification Guide

## Overview
When adding any feature to NetHack that requires persistence across save/load cycles, you must modify the save/restore system. This guide covers the core, OS-independent changes needed.

---

## Step-by-Step Process

### 1. **Define Your Data Structure**
**Location:** Appropriate header file (e.g., `include/you.h`, `include/flag.h`, `include/hack.h`)

```c
/* Example: Adding a new player attribute */
struct you {
    /* existing fields... */
    int your_new_field;      /* Your new data */
    boolean another_field;   /* Another example */
};
```

**Important:** Consider alignment and padding for portability.

---

### 2. **Initialize Your Data**
**Location:** Appropriate initialization function

- For player data: `src/u_init.c` in `initializeyou()` or similar
- For game flags: `src/flag.c` 
- For level data: Level creation functions

```c
void initializeyou() {
    /* existing initialization... */
    u.your_new_field = 0;
    u.another_field = FALSE;
}
```

---

### 3. **Add Save Functionality**
**Location:** `src/save.c`

Find the appropriate save function:
- `savelev()` - for level-specific data
- `savestateinlock()` - for game state
- `savegamestate()` - for general game state

Add your save calls in the correct location:

```c
/* In the appropriate save function */
bwrite(fd, (genericptr_t) &u.your_new_field, sizeof(u.your_new_field));
bwrite(fd, (genericptr_t) &u.another_field, sizeof(u.another_field));
```

**Critical:** The order of writes in save MUST match the order of reads in restore!

---

### 4. **Add Restore Functionality**
**Location:** `src/restore.c`

Find the corresponding restore function:
- `getlev()` - restores level data
- `restgamestate()` - restores game state
- `restore_mvitals()` - restores monster vitals, etc.

Add matching restore calls **in the exact same order** as saves:

```c
/* In the corresponding restore function */
mread(fd, (genericptr_t) &u.your_new_field, sizeof(u.your_new_field));
mread(fd, (genericptr_t) &u.another_field, sizeof(u.another_field));
```

---

### 5. **Update Save Compatibility**
**Location:** `include/patchlevel.h`

When you change the save format, you typically need to:

```c
/* Increment the VERSION_COMPATIBILITY or add compatibility code */
#define VERSION_COMPATIBILITY 0  /* or increment if needed */
```

**Note:** For development, you may choose to simply delete old save files. For release, you'd need migration code.

---

## Critical Rules to Follow

### ✓ **DO:**
1. **Match order exactly** - Save and restore order must be identical
2. **Use fixed-size types** when possible for portability
3. **Save size information** for dynamic arrays before the array data
4. **Test thoroughly** - Save, quit, restore, verify all data
5. **Handle pointers carefully** - Save what they point to, not addresses
6. **Check return values** from mread/bwrite
7. **Document your changes** in comments
8. **Validate on restore** - check lengths/counts (buflen, list counts, record sizes) before allocating or reading; log and sanitize or abort gracefully on suspicious values to avoid crashes and data corruption
9. **Add regression tests** that create saves, intentionally mutate edge cases (corrupt lengths/fields) and verify restore either corrects, rejects, or logs the condition (prevents silent UB)

### ✗ **DON'T:**
1. **Don't save pointers directly** - they're meaningless across sessions
2. **Don't assume alignment** - different platforms vary
3. **Don't forget to initialize** - uninitialized data causes crashes
4. **Don't change order** without updating both save and restore
5. **Don't save FILE* or other OS handles** - they won't restore
6. **Don't forget nested structures** - save all levels of data

---

## Common Patterns

### Pattern 1: Simple Scalar Values
```c
/* Save */
bwrite(fd, (genericptr_t) &value, sizeof(value));

/* Restore */
mread(fd, (genericptr_t) &value, sizeof(value));
```

### Pattern 2: Fixed-Size Arrays
```c
/* Save */
bwrite(fd, (genericptr_t) array, sizeof(array));

/* Restore */
mread(fd, (genericptr_t) array, sizeof(array));
```

### Pattern 3: Dynamic/Variable-Length Data
```c
/* Save */
bwrite(fd, (genericptr_t) &count, sizeof(count));
for (i = 0; i < count; i++) {
    bwrite(fd, (genericptr_t) &data[i], sizeof(data[i]));
}

/* Restore */
mread(fd, (genericptr_t) &count, sizeof(count));
for (i = 0; i < count; i++) {
    mread(fd, (genericptr_t) &data[i], sizeof(data[i]));
}
```

### Pattern 4: Strings
```c
/* Save */
bwrite(fd, (genericptr_t) string, strlen(string) + 1);

/* Restore */
mread(fd, (genericptr_t) string, strlen(string) + 1);

/* Or for safety with known max length: */
/* Save */
int len = strlen(string);
bwrite(fd, (genericptr_t) &len, sizeof(len));
bwrite(fd, (genericptr_t) string, len + 1);

/* Restore */
mread(fd, (genericptr_t) &len, sizeof(len));
mread(fd, (genericptr_t) string, len + 1);
```

### Pattern 5: Structures with Pointers
```c
/* DON'T save the pointer itself */
/* DO save what it points to */

/* Save */
if (obj->oextra) {
    bwrite(fd, (genericptr_t) &otrue, sizeof(otrue));
    save_oextra(fd, obj);  /* Function to save pointed-to data */
} else {
    bwrite(fd, (genericptr_t) &ofalse, sizeof(ofalse));
}

/* Restore */
mread(fd, (genericptr_t) &flag, sizeof(flag));
if (flag) {
    obj->oextra = alloc_oextra();
    restore_oextra(fd, obj);
}
```

---

## Testing Checklist

- [ ] Initialize your new fields in the appropriate init function
- [ ] Add save code in `src/save.c` 
- [ ] Add matching restore code in `src/restore.c` (same order!)
- [ ] Compile without warnings
- [ ] Start new game - verify field initializes correctly
- [ ] Modify your field during gameplay
- [ ] Save the game
- [ ] Restore the game
- [ ] Verify your field retained its value
- [ ] Test edge cases (empty values, maximum values, etc.)
- [ ] Test on different architectures if possible (32-bit vs 64-bit)

---

## Quick Reference: Key Files

| File | Purpose |
|------|---------|
| `include/you.h` | Player structure |
| `include/flag.h` | Game flags and options |
| `include/hack.h` | General game structures |
| `src/u_init.c` | Player initialization |
| `src/save.c` | Save game state |
| `src/restore.c` | Restore game state |
| `include/patchlevel.h` | Version compatibility |

---

## Example: Complete Feature Addition

Let's add a "lucky charm count" feature:

**1. Define (include/you.h):**
```c
struct you {
    /* ... existing fields ... */
    int lucky_charms;  /* Number of lucky charms collected */
};
```

**2. Initialize (src/u_init.c):**
```c
void initializeyou() {
    /* ... existing initialization ... */
    u.lucky_charms = 0;
}
```

**3. Save (src/save.c in savestateinlock or appropriate function):**
```c
/* After other u.field saves */
bwrite(fd, (genericptr_t) &u.lucky_charms, sizeof(u.lucky_charms));
```

**4. Restore (src/restore.c in restgamestate or matching function):**
```c
/* In exact same position as save */
mread(fd, (genericptr_t) &u.lucky_charms, sizeof(u.lucky_charms));
```

**5. Use it:**
```c
/* Somewhere in your game logic */
u.lucky_charms++;
pline("You now have %d lucky charm%s!", 
      u.lucky_charms, plur(u.lucky_charms));
```

---

## Troubleshooting

**Problem:** Game crashes on restore  
**Solution:** Check that save/restore order matches exactly, verify all pointers are handled correctly

**Problem:** Data is corrupted after restore  
**Solution:** Check structure alignment, ensure sizeof() matches on save/restore, verify endianness handling

**Problem:** Old save files don't work  
**Solution:** Either implement compatibility code or increment version and invalidate old saves

**Problem:** Works on one platform, fails on another  
**Solution:** Check pointer sizes, int sizes, alignment assumptions

---

## Notes

- NetHack uses `bwrite()` and `mread()` as wrappers around write/read for portability
- The 'b' in bwrite stands for "binary" conceptually
- `genericptr_t` is NetHack's portable pointer type (usually `void *`)
- Save files are binary and platform-dependent by default
- For complex changes, study existing similar features in the codebase

---

## Additional Resources

- Examine `src/save.c` and `src/restore.c` for existing patterns
- Look at how objects are saved/restored (`save_objects`, `restore_objects`)
- Study how levels are saved/restored (`savelev`, `getlev`)
- Check the NetHack wiki and dev documentation for more examples

---

**Remember:** The golden rule is **SAVE AND RESTORE IN THE SAME ORDER**. Everything else follows from careful attention to data types and pointer handling.

Good luck with your NetHack development!
