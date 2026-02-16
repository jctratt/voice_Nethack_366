/* Simple test to call add_note, save, and restore */
#include "hack.h"
#include <fcntl.h>
#include <sys/stat.h>

int
main(void)
{
    int fd;
    const char *notes_file = "./tmp/test_notes.dat";
    
    /* Initialize minimal game state */
    u.note_count = 0;
    u.note_list = NULL;
    
    /* Add a test note */
    add_note("Test note 1");
    
    printf("After add_note: note_count=%d\n", u.note_count);
    if (u.note_count > 0)
        printf("Note[0]='%s'\n", u.note_list[0]);

    (void) mkdir("./tmp", 0755);
    
    /* Open file for writing */
    fd = open(notes_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open for write");
        return 1;
    }
    
    /* Save notes */
    printf("\n=== SAVING ===\n");
    save_notes(fd);
    close(fd);
    
    /* Clear notes */
    if (u.note_list) {
        int i;
        for (i = 0; i < u.note_count; i++)
            if (u.note_list[i])
                free(u.note_list[i]);
        free(u.note_list);
    }
    u.note_count = 0;
    u.note_list = NULL;
    printf("Cleared notes: note_count=%d\n", u.note_count);
    
    /* Open file for reading */
    fd = open(notes_file, O_RDONLY);
    if (fd < 0) {
        perror("open for read");
        return 1;
    }
    
    /* Restore notes */
    printf("\n=== RESTORING ===\n");
    restore_notes(fd);
    close(fd);
    
    printf("\nAfter restore: note_count=%d\n", u.note_count);
    if (u.note_count > 0)
        printf("Note[0]='%s'\n", u.note_list[0]);
    
    return 0;
}
