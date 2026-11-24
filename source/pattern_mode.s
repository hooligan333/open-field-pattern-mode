/* The pack `pattern mode`: the routine the firmware arrives in. It holds no instruction of the firmware.
 *
 * The loader writes a jump over the first eight bytes of the function that reads the pattern sequencer's settings,
 * 0x8fc9d7d0 (pattern_mode_table.s, made from profile.json). It is called as (data, synth): R0 the pattern data of
 * the synth or of the drum, R1 1 for the synth's and 0 for the drum's. The jump lands here.
 *
 * We let the firmware's function run whole and come back to us after it: we keep the caller's return address and
 * the data in our own words, put the address of `after` into RETS and go on through the room, where the loader
 * copied the eight bytes the jump stands on (LINK, a push, two register moves) with a jump back behind them. The
 * function's UNLINK puts our RETS back and its RTS comes to `after`. The stack is never touched by us before the
 * function runs, as at the base's own hook of the tape's load. After it, we call the C++ function with the data,
 * with the twelve free bytes every call must have, and return to the firmware's caller.
 *
 * At a function's first instruction the registers a call may change hold nothing but the arguments, so P1, P2 and
 * R2 are free here. The function gives back nothing.
 */
.text
.align 2

.global op1f_pm_load
.type op1f_pm_load, STT_FUNC
op1f_pm_load:
    P2.H = op1f_pm_state;
    P2.L = op1f_pm_state;
    R2 = RETS;
    [P2] = R2;                          /* the caller's return */
    [P2 + 0x4] = R0;                    /* the pattern data */
    P1.H = op1f_pm_load_after;
    P1.L = op1f_pm_load_after;
    R2 = P1;
    RETS = R2;                          /* the firmware's UNLINK puts this back, its RTS comes here */
    P2.H = op1f_pm_load_room;           /* the eight bytes the entry displaced, then a jump back behind them */
    P2.L = op1f_pm_load_room;
    JUMP (P2);
op1f_pm_load_after:
    P2.H = op1f_pm_state;
    P2.L = op1f_pm_state;
    R0 = [P2 + 0x4];
    SP += -12;
    CALL op1f_pm_loaded;                /* void (void* data) */
    SP += 12;
    P2.H = op1f_pm_state;
    P2.L = op1f_pm_state;
    R2 = [P2];
    RETS = R2;
    RTS;
.size op1f_pm_load, .-op1f_pm_load

/* The state (the caller's return, the data) and the room: zeroed data, which the loader clears when it loads the
 * pack and fills when it applies the table: the eight bytes copied and the eight of the jump back. */
.section .bss
.balign 4
.global op1f_pm_state
.type op1f_pm_state, STT_OBJECT
op1f_pm_state:
    .space 8
.size op1f_pm_state, .-op1f_pm_state
.global op1f_pm_load_room
.type op1f_pm_load_room, STT_OBJECT
op1f_pm_load_room:
    .space 16
.size op1f_pm_load_room, .-op1f_pm_load_room
