# playsmf

playsmf is a small, lean and generic Windows32 commandline standard midi file (SMF) player with specific features such as:

- intrinsic realtime player flow control using jump/branch commands in combination with target address labels (primarily used for loops)
- realtime midi controlled jumps (interrupts) in combination with target address labels
- chord recognition for midi realtime interrupt/jump control
- multiport capability controlled by SMF Port metaevents per track
- SysEx input and output support
- multi-layer/multi-zone midi thru functionality based on "track follow mode" (thru port/channel follows selected tracks)
- realtime midi controlled mutes/unmutes based on "mute-sets" (mute/unmute multiple tracks simultaneously)
- record/save all incoming midi and sysex events into standard midi files
- the player is designed to keep CPU usage as low as possible to leave CPU power for other tasks such as softsynths, etc.

usage:

`playsmf.exe <MidiFile> <TimerPeriod> <DefPortOut> <DefPortIn> <TimeOut> <Channel> <Ofs> <REC> <INT>  <KeyStart> <KeyStop>  <KeyChordStart> <KeyChordStop>`

* `<MidiFile>` - inputfile *.mid
* `<TimerPeriod>` - windows timer resolution in ms or -1 if not used
* `<DefPortOut>` - default output port
* `<DefPortIn>` - default input port
* `<TimeOut>` - midi in timeout in ms or -1 if not used (player will close if no midi in message received within this period)
* `<Channel>` - channel used for chord recognition (-1 means all channes are used)
* `<Ofs>` - midi in key offset (default 0)
* `<REC>` - turns on midi recording (0xff means off)
* `<INT>` - interrupt message control for styles (default 0x00008000 - all messages)
* `<KeyStart>` - start key (EntryLabel)
* `<KeyStop>` - stop key (ExitLabel)
* `<KeyChordStart>` - lowest key used for chord recognition
* `<KeyChordStop>` - highest key used for chord recognition`
