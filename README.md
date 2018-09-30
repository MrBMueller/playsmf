# playsmf

playsmf is a small, lean and generic Windows32 commandline standard midi file (SMF) player with specific features such as:

- intrinsic realtime player flow control using jump/branch commands in combination with target address labels (primarily used for loops)
- realtime midi controlled jumps (interrupts) in combination with target address labels
- chord recognition for midi realtime interrupt/jump control
- multiport output controlled by SMF Port metaevents
- SysEx support
- multi-layer/multi-zone midi thru functionality based on "track follow mode" (thru port/channel follows selected tracks)
- realtime midi controlled mutes/unmutes based on "mute-sets" (mute/unmute multiple tracks simultaneously)
- record/save all incoming midi and sysex events into standard midi files
- the player is designed to keep CPU usage as low as possible to leave CPU power for other tasks such as softsynths, etc.
