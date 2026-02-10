# NetHack Notes Feature - Complete Implementation Guide

## Design Overview

We'll create a simple but robust notes system:
- **Dynamic array** of note strings
- **Minimal footprint** when empty (just stores count = 0)
- **Expandable** as needed
- **Easy to save/restore**

---

## 1. Data Structure Definition

**File: `include/flag.h`** (or create `include/notes.h` if you prefer separation)

```c
/* Note system */
struct player_notes {
    int note_count;          /* Number of notes */
    char **notes;            /* Array of note strings (dynamically allocated) */
};

/* Add to flags structure or make global */
extern struct player_notes player_notes;
```

**Alternative: If you want it in the player structure:**

**File: `include/you.h`**
```c
struct you {
    /* ... existing fields ... */
    
    /* Player notes */
    int note_count;
    char **note_list;
};
```

I'll use the second approach (in `struct you`) for this example.

---

## 2. Initialization

**File: `src/u_init.c`** in `initializeyou()` or similar:

```c
void initializeyou(void)
{
    /* ... existing initialization ... */
    
    /* Initialize notes system */
    u.note_count = 0;
    u.note_list = (char **) 0;  /* NULL pointer */
}
```

---

## 3. Save Implementation

**File: `src/save.c`** in `savestateinlock()` or wherever you save player state:

```c
/* Save player notes */
void save_notes(int fd)
{
    int i;
    int len;
    
    /* Save the count */
    bwrite(fd, (genericptr_t) &u.note_count, sizeof(u.note_count));
    
    /* Save each note */
    for (i = 0; i < u.note_count; i++) {
        if (u.note_list[i]) {
            len = strlen(u.note_list[i]);
            bwrite(fd, (genericptr_t) &len, sizeof(len));
            bwrite(fd, (genericptr_t) u.note_list[i], len + 1);
        } else {
            /* Handle null note (shouldn't happen, but be safe) */
            len = 0;
            bwrite(fd, (genericptr_t) &len, sizeof(len));
            bwrite(fd, (genericptr_t) "", 1);  /* Empty string */
        }
    }
}

/* In savestateinlock() or appropriate save function */
SAVEBREAK
/* Add this with other save calls */
save_notes(fd);
```

---

## 4. Restore Implementation

**File: `src/restore.c`** in `restgamestate()` or matching location:

```c
/* Restore player notes */
void restore_notes(int fd)
{
    int i;
    int len;
    
    /* Restore the count */
    mread(fd, (genericptr_t) &u.note_count, sizeof(u.note_count));
    
    /* Allocate the array if needed */
    if (u.note_count > 0) {
        u.note_list = (char **) alloc(u.note_count * sizeof(char *));
        
        /* Restore each note */
        for (i = 0; i < u.note_count; i++) {
            mread(fd, (genericptr_t) &len, sizeof(len));
            if (len > 0) {
                u.note_list[i] = (char *) alloc(len + 1);
                mread(fd, (genericptr_t) u.note_list[i], len + 1);
            } else {
                u.note_list[i] = (char *) alloc(1);
                mread(fd, (genericptr_t) u.note_list[i], 1);
            }
        }
    } else {
        u.note_list = (char **) 0;  /* NULL */
    }
}

/* In restgamestate() - MUST be in same position as save_notes() call */
restore_notes(fd);
```

---

## 5. Helper Functions for Managing Notes

**File: `src/notes.c`** (new file)

```c
/* notes.c - Player note management */

#include "hack.h"

/* Add a new note */
void
add_note(const char *text)
{
    char **new_list;
    int i;
    
    /* Allocate new array with one more slot */
    new_list = (char **) alloc((u.note_count + 1) * sizeof(char *));
    
    /* Copy existing notes */
    for (i = 0; i < u.note_count; i++) {
        new_list[i] = u.note_list[i];
    }
    
    /* Add new note */
    new_list[u.note_count] = dupstr(text);
    
    /* Free old array (but not the strings themselves!) */
    if (u.note_list) {
        free((genericptr_t) u.note_list);
    }
    
    /* Update */
    u.note_list = new_list;
    u.note_count++;
}

/* Delete a note by index */
void
delete_note(int index)
{
    char **new_list;
    int i, j;
    
    if (index < 0 || index >= u.note_count) {
        return;  /* Invalid index */
    }
    
    /* Free the note being deleted */
    free((genericptr_t) u.note_list[index]);
    
    if (u.note_count == 1) {
        /* Deleting the last note */
        free((genericptr_t) u.note_list);
        u.note_list = (char **) 0;
        u.note_count = 0;
        return;
    }
    
    /* Allocate new smaller array */
    new_list = (char **) alloc((u.note_count - 1) * sizeof(char *));
    
    /* Copy notes, skipping the deleted one */
    for (i = 0, j = 0; i < u.note_count; i++) {
        if (i != index) {
            new_list[j++] = u.note_list[i];
        }
    }
    
    /* Free old array */
    free((genericptr_t) u.note_list);
    
    /* Update */
    u.note_list = new_list;
    u.note_count--;
}

/* Get a note by index (for display) */
const char *
get_note(int index)
{
    if (index < 0 || index >= u.note_count) {
        return "";
    }
    return u.note_list[index];
}

/* Free all notes (for game cleanup) */
void
free_notes(void)
{
    int i;
    
    if (u.note_list) {
        for (i = 0; i < u.note_count; i++) {
            if (u.note_list[i]) {
                free((genericptr_t) u.note_list[i]);
            }
        }
        free((genericptr_t) u.note_list);
        u.note_list = (char **) 0;
    }
    u.note_count = 0;
}
```

---

## 6. Header Declarations

**File: `include/extern.h`** (add to appropriate section):

```c
/* ### notes.c ### */
extern void add_note(const char *);
extern void delete_note(int);
extern const char *get_note(int);
extern void free_notes(void);
```

---

## 7. Command Implementation

**File: `src/cmd.c`** or wherever extended commands are:

```c
/* In the extended command table */
{ "notes", "manage your notes", donotes, IFBURIED | AUTOCOMPLETE },
```

**File: `src/do.c`** or create `src/notes.c`:

```c
int
donotes(void)
{
    /* Your curses UI code goes here */
    /* Show menu with: */
    /* a) New Note */
    /* b) Delete Note (if notes exist) */
    /* Then show numbered list of existing notes */
    
    /* Placeholder example: */
    pline("Notes feature - UI to be implemented");
    
    /* When user creates a note with your UI: */
    /* add_note(the_text_they_entered); */
    
    /* When user deletes note N: */
    /* delete_note(N); */
    
    return 0;
}
```

---

## 8. Makefile Updates

**File: `src/Makefile`** (or appropriate makefile):

Add `notes.c` to your source files and `notes.o` to your object files.

---

## Key Points for Your Implementation

### Save/Restore Flow:
1. **Save:** count → for each note: length → string (with null terminator)
2. **Restore:** count → allocate array → for each note: length → allocate string → read string

### Memory Management:
- Use `alloc()` for allocation (NetHack's wrapper)
- Use `free()` for deallocation
- Use `dupstr()` for string duplication
- Always null-terminate strings
- Clean up in `free_notes()` when game ends

### Testing Checklist:
- [ ] No notes: save/restore (should take minimal space)
- [ ] One note: save/restore
- [ ] Multiple notes: save/restore
- [ ] Add note after restore
- [ ] Delete note, then save/restore
- [ ] Very long note (test buffer sizes)
- [ ] Empty note ("")
- [ ] Note with special chars/newlines

---

## Common Pitfalls to Avoid

### ❌ DON'T DO THIS:
```c
/* Saving pointer value instead of data */
bwrite(fd, &u.note_list, sizeof(u.note_list));

/* Not saving length first */
bwrite(fd, u.note_list[i], strlen(u.note_list[i]));

/* Hardcoding buffer size */
char note[256];
mread(fd, note, 256);  // What if note is longer?
```

### ✅ DO THIS:
```c
/* Save the data, not the pointer */
int len = strlen(u.note_list[i]);
bwrite(fd, &len, sizeof(len));
bwrite(fd, u.note_list[i], len + 1);

/* Restore with dynamic allocation */
mread(fd, &len, sizeof(len));
u.note_list[i] = (char *) alloc(len + 1);
mread(fd, u.note_list[i], len + 1);
```

---

## Example Usage in Your UI Code

```c
/* After user enters a note in your curses dialog */
char notebuf[BUFSZ];  /* Or however you're capturing input */

/* ... user types note into notebuf ... */

if (strlen(notebuf) > 0) {
    add_note(notebuf);
    pline("Note added.");
}

/* To show all notes */
int i;
for (i = 0; i < u.note_count; i++) {
    pline("%d) %s", i + 1, get_note(i));
}

/* To delete note 2 (index 1) */
delete_note(1);
pline("Note deleted.");
```

---

## Debugging Tips

If restore fails:
1. Add debug output to see what's being read:
   ```c
   mread(fd, &u.note_count, sizeof(u.note_count));
   fprintf(stderr, "DEBUG: Restored note_count = %d\n", u.note_count);
   ```

2. Make sure save and restore are in the EXACT same position in their respective functions

3. Check that you're not reading/writing past buffer boundaries

4. Verify the order matches:
   - Save: count, len1, str1, len2, str2...
   - Restore: count, len1, str1, len2, str2...

---

Good luck! Let me know if you hit any snags with this implementation.
