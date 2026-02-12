/* quiver.c -- initial scaffolding for quiver-ordering feature
 *
 * Minimal safe stubs so feature branch can be built and tested incrementally.
 */

#include "hack.h"

/* Extended command: #quiverorder (interactive reorder UI - stub for now) */
int
doquiverorder(VOID_ARGS)
{
    pline("#quiverorder: not implemented yet.");
    return 0;
}

/* Selection helper used by dofire()/autoquiver() later.
 * Current stub: return the currently readied quiver item (uquiver).
 * Will be expanded to consult user quiver-order, sticky mappings, and skill.
 */
struct obj *
select_quiver_candidate()
{
    return uquiver;
}
