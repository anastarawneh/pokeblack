#ifndef CONSTANTS_MOVE_FLAGS_H
#define CONSTANTS_MOVE_FLAGS_H

enum MoveFlag {
    MOVE_FLAG_NONE                     = 0,
    MOVE_FLAG_MAKES_CONTACT            = (1 << 0),
    MOVE_FLAG_REQUIRES_CHARGE_UP_TURN  = (1 << 1),
    MOVE_FLAG_REQUIRES_RECHARGE_TURN   = (1 << 2),
    MOVE_FLAG_AFFECTED_BY_PROTECT      = (1 << 3),
    MOVE_FLAG_AFFECTED_BY_MAGIC_COAT   = (1 << 4),
    MOVE_FLAG_AFFECTED_BY_SNATCH       = (1 << 5),
    MOVE_FLAG_CALLABLE_BY_MIRROR_MOVE  = (1 << 6),
    MOVE_FLAG_PUNCHING                 = (1 << 7),
    MOVE_FLAG_SOUND_BASED              = (1 << 8),
    MOVE_FLAG_BLOCKED_BY_GRAVITY       = (1 << 9),
    MOVE_FLAG_THAWS_USER               = (1 << 10),
    MOVE_FLAG_HITS_NONADJACENT_TARGETS = (1 << 11),
    MOVE_FLAG_HEALING                  = (1 << 12),
    MOVE_FLAG_BYPASSES_SUBSTITUTE      = (1 << 13)
};

#endif // CONSTANTS_MOVE_FLAGS_H
