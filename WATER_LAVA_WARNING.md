# Water/Lava Stepping Warning Implementation

## Overview
Implements a feature from NetHack 3.7 that warns the player before stepping on dangerous water or lava terrain if they lack protection.

## Files Modified
1. **src/hack.c** - Added warning check in domove_core()

## Implementation Details

### Warning Conditions

#### Water Danger
Player is warned before stepping on water if:
- NOT in water form
- NOT swimming
- NOT water walking  
- NOT levitating
- NOT flying
- NOT magical breathing
- NOT on Plane of Water (Is_waterlevel)

#### Lava Danger  
Player is warned before stepping on lava if:
- NOT fire resistant
- NOT levitating
- NOT flying
- NOT in lava form

### Warning Message
When danger is detected, player sees: "The <water/lava> would hurt you. Really move? [yn]"

Only confirmed moves proceed; 'n' cancels the move.

## Code Locations

The warning check is placed in domove_core() at:
- **File**: src/hack.c
- **Location**: Just before `u.ux += u.dx; u.uy += u.dy;` (the actual movement update)
- **Trigger**: After test_move() returns TRUE but before hero position changes

## Related Functions
- `is_pool()` - Check if location is water
- `is_lava()` - Check if location is lava  
- `Levitation` - Levitation protection
- `Flying` - Flying protection
- `Water_walking` - Water walking
- `Swimming` - Swimming ability
- `Underwater` - In water form
- `Fire_resistance` - Fire resistance
- `Magical_breathing` - Magical breathing
- `Is_waterlevel()` - Plane of Water check

## Testing Notes
The warning only fires when moving to dangerous terrain without protection.
Can be bypassed by pressing 'y' to confirm the move.
