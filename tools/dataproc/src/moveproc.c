#include <getopt.h>
#include <stdarg.h>
#include <stdlib.h>

#include "common.h"
#include "dataproc.h"
#include "libenum.h"
#include "nitroarc.h"

#include "struct_defs/move.h"

static enum_template_t enums[] = {
    include_enum("constants/moves.h",           "enum Move"),
    include_enum("constants/move_categories.h", "enum MoveCategory"),
    include_enum("constants/move_classes.h",    "enum MoveClass"),
    include_enum("constants/move_durations.h",  "enum MoveDuration"),
    include_enum("constants/move_flags.h",      "enum MoveFlag"),
    include_enum("constants/move_ranges.h",     "enum MoveRange"),
    include_enum("constants/move_status.h",     "enum MoveStatus"),
    include_enum("constants/pokemon.h",         "enum Type"),
    { .from_file = NULL },
};

static archive_template_t archives[] = {
    { .out_filename = "moves.narc" },
    { .out_filename = NULL },
};

static void parse_args(int *pargc, char ***pargv);
static void proc_move(datafile_t *df);

static char *filename     = NULL;
static char *base_dir     = NULL;
static char *depfile_path = "move_data.d";
static char *output_dir   = ".";

int main(int argc, char **argv) {
    int        errc = EXIT_SUCCESS;
    datafile_t df   = { 0 };

    parse_args(&argc, &argv);
    enum_seq_t moves = common_initenum(
        filename, "Move",
        .sourcefile  = __FILE__,
        .depfile     = depfile_path,
        .outdir      = output_dir,
        .format      = DATAPROC_F_JSON,
        .enums       = enums,
        .archives    = archives,
    );

    for (size_t i = 0; i < moves.size; i++) {
        const char *stem = moves.members[i].name + lengthof("MOVE_");

        char *basename = strlower(stem);
        char *filepath = pathjoin(base_dir, basename, "data.json");
        declare_dep(filepath);

        if (dp_load(&df, filepath) == 0) {
            proc_move(&df);
        }

        if (dp_report(&df) == DIAG_ERROR) errc = EXIT_FAILURE;

        free(basename);
        free(filepath);
        dp_free(&df);
    }

    return common_done(errc, NULL);
}

static void proc_move(datafile_t *df) {
    MoveData data = {
        .type                 = enum_u8(".type", enum Type),
        .category             = enum_u8(".category", enum MoveCategory),
        .class                = enum_u8(".class", enum MoveClass),
        .power                = u8(".power"),
        .accuracy             = u8(".accuracy"),
        .pp                   = u8(".pp"),
        .priority             = s8(".priority"),
        .minHits              = u8(".minHits"),
        .maxHits              = u8(".maxHits"),
        .status               = enum_u16(".status", enum MoveStatus),
        .statusChance         = u8(".statusChance"),
        .effectDuration       = enum_u8(".effectDuration", enum MoveDuration),
        .minTurns             = u8(".minTurns"),
        .maxTurns             = u8(".maxTurns"),
        .critModifier         = u8(".critModifier"),
        .flinchChance         = u8(".flinchChance"),
        .aiScript             = u16(".aiScript"),
        .recoveryByDamage     = u8(".recoveryByDamage"),
        .recoveryByMaxHP      = u8(".recoveryByMaxHP"),
        .range                = enum_u8(".range", enum MoveRange),
        .unk_1E               = 0x5353
    };

    size_t n_statChanges = dp_arrlen(dp_get(df, ".statChanges"));
    char objectPath[32];
    for (int i = 0; i < 3; i++) {
        if (i < n_statChanges) {
            sprintf(objectPath, ".statChanges[%d][0]", i);
            data.stats[i] = dp_u8(dp_get(df, objectPath));
            sprintf(objectPath, ".statChanges[%d][1]", i);
            data.statChanges[i] = dp_s8(dp_get(df, objectPath));
            sprintf(objectPath, ".statChanges[%d][2]", i);
            data.statChangeChances[i] = dp_u8(dp_get(df, objectPath));
        }
    }

    datanode_t flags   = dp_get(df, ".flags");
    size_t     n_flags = dp_arrlen(flags);
    for (size_t i = 0; i < n_flags; i++) {
        u16 flag    = dp_u16(dp_lookup(dp_arrelem(flags, i), "enum MoveFlag"));
        data.flags |= flag;
    }

    nitroarc_ppack(&archives[0].packer, &data, sizeof(data), NULL);
}

static char *program_name  = NULL;

static void usage(const char *fmt, ...) {
    FILE *f = stdout;
    if (fmt != NULL) {
        f = stderr;

        va_list args;
        va_start(args, fmt);
        fputs(program_name, f);
        fputs(": ", f);
        vfprintf(f, fmt, args);
        fputc('\n', f);
        va_end(args);
    }

#define fputf(fmt, ...) fprintf(f, fmt, __VA_ARGS__)
    fputf("usage: %s [-M DEPFILE] [-o OUTDIR] ENUMFILE BASEDIR\n", program_name);
    fputs("\n", f);
    fputs("options:\n", f);
    fputs("  -o OUTDIR            Write output files to OUTDIR. Does not affect DEPFILE.\n", f);
    fputs("                       Defaults to the current working directory.\n", f);
    fputs("  -M DEPFILE           Specify the full path to an output dependency file.\n", f);
    fputs("                       Defaults to 'move_data.d'.\n", f);
#undef fputf

    exit(f == stdout ? EXIT_SUCCESS : EXIT_FAILURE);
}

static void parse_args(int *pargc, char ***pargv) {
    program_name = (*pargv)[0];
    opterr       = 0;

    int c = 0;
    while ((c = getopt(*pargc, *pargv, ":o:M:h")) != -1) {
        switch (c) {
        case 'o': output_dir    = optarg; break;
        case 'M': depfile_path  = optarg; break;

        case 'h': usage(NULL);                                 break;
        case ':': usage("missing argument for '-%c'", optopt); break;
        default:  usage("unknown option '-%c'", optopt);     break;
        }
    }

    *pargc -= optind;
    *pargv += optind;
    if (*pargc < 1) usage("missing required argument ENUMFILE");
    if (*pargc < 2) usage("missing required argument BASEDIR");

    filename = (*pargv)[0];
    base_dir = (*pargv)[1];
}
