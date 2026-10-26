#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>

#include "common.h"
#include "game_types.h"
#include "dataproc.h"
#include "libenum.h"
#include "nitroarc.h"

#include "constants/moves.h"
#include "struct_defs/move.h"

static const char *enumFile = NULL;
static const char *baseDir  = NULL;

enum { A_MOVE_DATA };
enum { T_MOVE_NAMES };

#define L_MOVE_CATEGORY "enum MoveCategory"
#define L_MOVE_CLASS    "enum MoveClass"
#define L_MOVE_DURATION "enum MoveDuration"
#define L_MOVE_FLAG     "enum MoveFlag"
#define L_MOVE_RANGE    "enum MoveRange"
#define L_MOVE_STATUS   "enum MoveStatus"
#define L_STAT          "enum Stat"
#define L_TYPE          "enum Type"

static Driver driver = {
    .depfile = "move_data.d",

    .options = (Option[]) {
        POSITIONAL("ENUMFILE", &enumFile, "Header defining enum Move."),
        POSITIONAL("BASEDIR",  &baseDir,  "Directory of pre-move data."),
        { 0 }
    },

    .lookups = (Lookup[]) {
        LOOKUP_ENUM("constants/moves.h",           L_MOVE_CATEGORY),
        LOOKUP_ENUM("constants/moves.h",           L_MOVE_CLASS),
        LOOKUP_ENUM("constants/moves.h",           L_MOVE_DURATION),
        LOOKUP_ENUM("constants/moves.h",           L_MOVE_FLAG),
        LOOKUP_ENUM("constants/moves.h",           L_MOVE_RANGE),
        LOOKUP_ENUM("constants/moves.h",           L_MOVE_STATUS),
        LOOKUP_ENUM("constants/pokemon.h",         L_STAT),
        LOOKUP_ENUM("constants/pokemon.h",         L_TYPE),
        { 0 }
    },

    .outputs = {
        .archives = (Archive[]) {
            [A_MOVE_DATA] = { .filename = "moves.narc" },
            { 0 }
        },

        .textBanks = (TextBank[]) {
            [T_MOVE_NAMES] = { .filename = "move_names.json" },
            { 0 }
        }
    }
};

static void ProcMove(dp_node root) {
    MoveData data = {
        .type                 = dp_u8(dp_lookup_at(root, ".type", L_TYPE)),
        .category             = dp_u8(dp_lookup_at(root, ".category", L_MOVE_CATEGORY)),
        .class                = dp_u8(dp_lookup_at(root, ".class", L_MOVE_CLASS)),
        .power                = dp_u8_at(root, ".power"),
        .accuracy             = dp_u8_at(root, ".accuracy"),
        .pp                   = dp_u8_at(root, ".pp"),
        .priority             = dp_s8_at(root, ".priority"),
        .minHits              = dp_u8_at(root, ".minHits"),
        .maxHits              = dp_u8_at(root, ".maxHits"),
        .status               = dp_u16(dp_lookup_at(root, ".status", L_MOVE_STATUS)),
        .statusChance         = dp_u8_at(root, ".statusChance"),
        .effectDuration       = dp_u8(dp_lookup_at(root, ".effectDuration", L_MOVE_DURATION)),
        .minTurns             = dp_u8_at(root, ".minTurns"),
        .maxTurns             = dp_u8_at(root, ".maxTurns"),
        .critModifier         = dp_u8_at(root, ".critModifier"),
        .flinchChance         = dp_u8_at(root, ".flinchChance"),
        .aiScript             = dp_u16_at(root, ".aiScript"),
        .recoveryByDamage     = dp_u8_at(root, ".recoveryByDamage"),
        .recoveryByMaxHP      = dp_u8_at(root, ".recoveryByMaxHP"),
        .range                = dp_u8(dp_lookup_at(root, ".range", L_MOVE_RANGE)),
        .unk_1E               = 0x5353
    };

    for (dp_each(statChange, i, dp_at(root, ".statChanges"))) {
        data.stats[i] = dp_u8(dp_lookup(dp_arr_at(statChange, 0), L_STAT));
        data.statChanges[i] = dp_s8(dp_arr_at(statChange, 1));
        data.statChangeChances[i] = dp_u8(dp_arr_at(statChange, 2));
    }

    for (dp_each(flag, i, dp_at(root, ".flags"))) {
        data.flags |= dp_u16(dp_lookup(flag, L_MOVE_FLAG));
    }

    if (data.accuracy > 101) {
        dp_warn(dp_at(root, ".accuracy"), "value is greater than 101 and will be clamped to 100");
        data.accuracy = 100;
    }

    if (data.statusChance > 100) {
        dp_warn(dp_at(root, ".statusChance"), "value is greater than 100 and will be clamped");
        data.statusChance = 100;
    }

    if (data.flinchChance > 100) {
        dp_warn(dp_at(root, ".flinchChance"), "value is greater than 100 and will be clamped");
        data.flinchChance = 100;
    }

    if (data.minHits > data.maxHits) {
        dp_warn(dp_at(root, ".minHits"), "value is greater than maxHits and will be clamped");
        data.minHits = data.maxHits;
    }

    if (data.minTurns > data.maxTurns) {
        dp_warn(dp_at(root, ".minTurns"), "value is greater than maxTurns and will be clamped");
        data.minTurns = data.maxTurns;
    }

    Archive_Pack(A_MOVE_DATA, &data, sizeof(data));
}

static void ProcText(dp_node root, size_t i, const char *stem) {
    const char *name = dp_string_at(root, ".name");
    if (name == NULL) return; // error is already staged
    TextBank_PushString(T_MOVE_NAMES, name, "MoveNames_Text_%s", stem);
}

int main(int argc, char **argv) {
    Driver_Init(&driver, argc, argv);

    for (EachEntry(entry, Entries_FromEnum(enumFile, "Move", "MOVE_", "%s/%s/data.json", baseDir))) {
        ProcMove(entry.root);
        ProcText(entry.root, entry.index, entry.stem);
    }

    return Driver_Finish(&driver);
}
