// The pack `pattern mode`: the pattern sequencer keeps its play mode over a restart.
//
// OS 1.7.3 keeps the pattern sequencer's settings, one set for the synth and one for the drum, in its settings
// database: the group "sequencers", the keys "drumseq_synth" and "drumseq_drum". When the settings change it writes
// swing, length, start, the play mode `pm` and the notes there. When it reads them back, the first time the pattern
// sequencer's settings are asked for after a start, it reads swing, length, start and the notes, but not `pm`. So
// the play mode the owner chose (backward, ping-pong or forward) is lost at every start, and the sequencer plays
// forward again.
//
// The routine in pattern_mode.s lets the firmware's function read the settings, then calls the function below with
// the pattern data the firmware just filled. It reads the same entry of the database again, and when it holds a
// `pm` of 0, 1 or 2 it puts that into the data, where the player and the page read it. An entry with no `pm`, or with
// one out of range, changes nothing: the firmware's own default stays.
//
// Called on the thread that first asks for the pattern sequencer's settings, twice at that time (the synth's, then
// the drum's), never on the audio thread. The functions of the firmware it calls:
//
//   0x8d4f1b64  db2_get(db, group, key, value): copies the entry `key` of the group `group` of the settings database
//               `db` (0 here, as the firmware's own function asks) into a JSON value the caller made. Not 0 when the
//               entry was there. It takes the database's lock itself.
//   0x8c085ba8, 0x8c0844e0, 0x8c085068, 0x8c084f74, 0x8c084840: json_ctor, json_dtor, json_has, json_get, json_int,
//               as docs/firmware-interfaces.md describes them.
//   0x8c053a20  the logger.
//
// The texts are the firmware's own, at their addresses: "sequencers", "drumseq_synth", "drumseq_drum" and "pm".
// The data's fields: the play mode is the word at +0x194, and the byte at +0x198 is 1 for the synth's data and 0
// for the drum's (the firmware's function writes it first, from its second argument).

typedef unsigned int u32;
typedef unsigned char u8;

typedef void (*LogFn)(int level, const char* file, int line, const char* format, ...);
typedef int (*DbGetFn)(int db, const char* group, const char* key, void* value);
typedef void (*JsonCtorFn)(void* value, int kind);
typedef void (*JsonDtorFn)(void* value);
typedef int (*JsonHasFn)(const void* value, const char* key);
typedef const void* (*JsonGetFn)(const void* value, const char* key);
typedef int (*JsonIntFn)(const void* value);

#define STOCK_LOG ((LogFn)0x8c053a20u)
#define STOCK_DB_GET ((DbGetFn)0x8d4f1b64u)
#define STOCK_JSON_CTOR ((JsonCtorFn)0x8c085ba8u)
#define STOCK_JSON_DTOR ((JsonDtorFn)0x8c0844e0u)
#define STOCK_JSON_HAS ((JsonHasFn)0x8c085068u)
#define STOCK_JSON_GET ((JsonGetFn)0x8c084f74u)
#define STOCK_JSON_INT ((JsonIntFn)0x8c084840u)

#define TEXT_SEQUENCERS ((const char*)0x8490c63cu)    // "sequencers"
#define TEXT_SYNTH_KEY ((const char*)0x8490c61cu)     // "drumseq_synth"
#define TEXT_DRUM_KEY ((const char*)0x8490c62cu)      // "drumseq_drum"
#define TEXT_PM ((const char*)0x8490c608u)            // "pm"

enum { DATA_PM = 0x194, DATA_IS_SYNTH = 0x198, PM_MOST = 2 };

static const char LOG_FILE[] = "op1f/pattern_mode.cpp";

extern "C" void op1f_pm_loaded(u8* data) asm("op1f_pm_loaded");

void op1f_pm_loaded(u8* data) {
    if (!data) return;
    const int synth = data[DATA_IS_SYNTH] != 0;
    u32 value[4];                                     // a JSON value is 12 bytes; 16 kept
    STOCK_JSON_CTOR(value, 0);
    int pm = -1;
    if (STOCK_DB_GET(0, TEXT_SEQUENCERS, synth ? TEXT_SYNTH_KEY : TEXT_DRUM_KEY, value) && STOCK_JSON_HAS(value, TEXT_PM))
        pm = STOCK_JSON_INT(STOCK_JSON_GET(value, TEXT_PM));
    STOCK_JSON_DTOR(value);
    if (pm >= 0 && pm <= PM_MOST) {
        *(int*)(data + DATA_PM) = pm;
        STOCK_LOG(3, LOG_FILE, 1, "pattern mode: the %s's play mode %d read back", synth ? "synth" : "drum", pm);
    } else {
        STOCK_LOG(3, LOG_FILE, 2, "pattern mode: the %s's settings hold no play mode to read back; the default stays", synth ? "synth" : "drum");
    }
}
