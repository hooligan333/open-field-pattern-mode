# pattern mode

An [open-field](https://github.com/LaP0573/open-field) pack for the OP-1 Field on OS 1.7.3: **the pattern sequencer keeps its play mode over a restart.**

On the stock firmware, set the pattern sequencer to play backward or ping-pong, switch the unit off and on, and it plays forward again. The firmware writes the play mode (`pm`) into the pattern sequencer's settings along with swing, length, start and the notes, but when it reads the settings back after a start it reads everything except `pm`. This pack reads it back.

## What it does

- It changes one place of the firmware: the entry of the function that reads the pattern sequencer's settings (`0x8fc9d7d0`), once for the synth's and once for the drum's.
- It lets that function run as before (through a trampoline), then reads the same settings entry again and, when it holds a `pm` of 0, 1 or 2, puts it where the player and the page read it.
- A settings entry with no `pm` (never saved yet) or one out of range changes nothing: the firmware's default stays.
- It needs no other pack and no pack needs it. It has no page and no settings of its own.

Its lines in the unit's log, tag `pattern_mode`, at the first use of the pattern sequencer after a start:

```
pattern mode: the synth's play mode 1 read back
pattern mode: the drum's settings hold no play mode to read back; the default stays
```

## Status

**Tried on a unit** (OS 1.7.3, base 32, with a full set of other packs, 2026-11-24): with the play mode set to ping-pong, it was still ping-pong after a restart, and the log said `the synth's play mode 1 read back`. Built with open-field's `tools/pack_build.py build3` against its main branch (base 32 era), checked with `pack_build.py check`, `stub_calls.py` and `publish_check.py` (no byte of the firmware in any file here).

If the unit does not start with it, hold the **help** key while switching on: the unit then starts with no packs, and you can delete `user/pk/pattern mode.pack` in disk mode.

## Files

| File | What |
| --- | --- |
| `pattern mode.pack` | The pack: copy it into `user/pk/` on the unit |
| `source/recipe.json` | Its name, version and sources |
| `source/profile.json` | Its one place in the firmware: address, length and checksum. No byte of the firmware |
| `source/pattern_mode_table.s` | Its table of changes, made from `profile.json` by `tools/table_build.py` |
| `source/pattern_mode.s` | The routine at the firmware's function: lets it run, then calls ours |
| `source/pattern_mode.cpp` | Reads the play mode back |

To build: copy `source/` into an open-field tree as `examples/pattern_mode/`, then

```bash
python -I tools/pack_build.py build3 --recipe examples/pattern_mode/recipe.json --profile examples/pattern_mode/profile.json --out out/pattern-mode.pack
```

Unofficial; not made, endorsed or supported by Teenage Engineering. MIT licence.
