/* Simple test to call add_note, save, and restore */
#include "hack.h"
#include <fcntl.h>

int
main(void)
{
    int fd;
    
    /* Initialize minimal game state */
    u.note_count = 0;
    u.note_list = NULL;
    
    /* Add a test note */
    add_note("Test note 1");
    
    printf("After add_note: note_count=%d\n", u.note_count);
    if (u.note_count > 0)
        printf("Note[0]='%s'\n", u.note_list[0]);
    
    /* Open file for writing */
    fd = open("/tmp/test_notes.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);
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
    fd = open("/tmp/test_notes.dat", O_RDONLY);
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
