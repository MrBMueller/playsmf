//============================================================================//

#include <windows.h>
#include <midifile.h>

#define KW0 "Label"
#define KW1 "Jump"
#define KW2 "Mute"
#define KW3 "Solo"

#define MyMacro1 \
 if       ((Label0->Event == MidiEvents) || (Label0->Event == LastLabel->Event))                                                      { Mute0 = Mute1 = Mute2 = Mute3 = Mute11 = MuteA = MuteB = EntryMute; i = -2;                      c = Mute0[-1]; }\
  else if ((Mute == EntryMute) && ((MidiEvent->Label->Event == EntryLabel->Event) || (MidiEvent->Label->Event == FirstLabel->Event))) { Mute0 = Mute1 = Mute2 = Mute3 = Mute11 = MuteA = MuteB = FirstMute; i =  0;                      c = Mute0[-1]; }\
  else                                                                                                                                {                                                                     i = (Mute-Mutes)/(TrkNum+1); c = Mute[ -1]; }\
 printf("%4.2f -> %4x %3d %3d %2d %02x %2d %d => %6.2f (%6.2f %d/%d) -> %6.2f (%6.2f %d/%d)\n", (float)(RecEvent->event_time-LastTime)*1000*(1<<MidiEvent->TimeSigD)/((MidiEvent->Tempo<<2)*MidiEvent->TimeSigN), V0, (V0&0xfff)<=0xff?V0&0x7f:-1, V1, (signed char)Label0->Ret, IRQ, i, c,\
  (float)MidiEvent->event_time*1000*(1<<MidiEvent->TimeSigD)/((MidiEvent->Tempo<<2)*MidiEvent->TimeSigN), (float)60000000/MidiEvent->Tempo/Speed0, MidiEvent->TimeSigN, 1<<MidiEvent->TimeSigD, (float)Label0->Event->event_time*1000*(1<<Label0->Event->TimeSigD)/((Label0->Event->Tempo<<2)*Label0->Event->TimeSigN), (float)60000000/Label0->Event->Tempo/Speed0, Label0->Event->TimeSigN, 1<<Label0->Event->TimeSigD);\
 LastTime = RecEvent->event_time;

#define MyMacro0 \
 if (V0 < LabelNum && Labels[V0].Event) {\
  if (Labels[V0].Event != Label0->Event) { if (!(Label0 = &Labels[V0])->Ret) { if ((Label0 != Label2) || (MidiEvent->Label->Ret)) { Label3 = Label2 = Label1 = Label0;                                        IRQ = 0x10; MyMacro1 }}\
                    else { Label1 = Label2; if (Label0->Ret&2) { if (!MidiEvent->Label->Ret) { Label3 = &Labels[Label1->Idx&-4096|MidiEvent->Label->Idx&0xfff]; } Label1 = Label3; } Var = Label1->Idx&-4096; IRQ = 0x08; MyMacro1 }}\
   else if (Label0->Ret) { Label1 = Label2; if (Label0->Ret&2) { if (!MidiEvent->Label->Ret) { Label3 = &Labels[Label1->Idx&-4096|MidiEvent->Label->Idx&0xfff]; } Label1 = Label3; } Var = Label1->Idx&-4096; IRQ = 0x08; MyMacro1  }\
   else                  { Label3 = Label2 = Label1 = Label0 = &Labels[V0]; }}

struct MidiEvent { unsigned long     event_time;
                   unsigned long     Track;
                   unsigned short    Out;
                   unsigned long     EventData;
                   unsigned long     data_length;
                   unsigned char    *data_buffer;
                   struct MidiEvent *NextEvent;
                   struct MidiEvent *JumpEvent;
                   struct MidiEvent **TrkInfo;
                   unsigned char     FlwCtl, MsgCtl, Rec, Ch;
                   struct PNoteO    *EventIdx;
                   HMIDIOUT          midi_out;
                   unsigned long     Tempo;
                   unsigned char     TimeSigN, TimeSigD;
                   struct Label     *Label; };

struct RecEvent  { unsigned long     event_time;
                   unsigned long     EventData;
                   struct RecEvent  *NextEvent; };

struct PNoteI    { struct PNoteI    *Prev;
                   struct PNoteI    *Next;
                   signed char       Key, Note, Vel; };

struct PNoteO    { struct PNoteO    *Prev;
                   struct PNoteO    *Next;
                   unsigned long     EventOffMsg;
                   struct MidiEvent *Event; };

struct Label     {   signed long     Idx;
                   struct MidiEvent *Event;
                   unsigned char     Ret; };

struct Chord     {   signed short    Type;
                     signed char     Root, Num; };

struct Thru      { struct MidiEvent  **Trk, *Pending;
                   unsigned long     Delay;
                   unsigned char     k, v0[128], v1[128]; };

struct Key       { unsigned char     Zone;
                   unsigned long     Val;
                   struct Thru       Thrus[8+1]; };

struct MidiIn    { unsigned long     s;
                   HMIDIIN           h; };

struct MidiOut   { unsigned long     s;
                   HMIDIOUT          h; };

static HANDLE           signalling_object;
static struct Label    *Labels, *Label0, *Label1, *Label2, *Label3, *FirstLabel, *LastLabel, *EntryLabel, *ExitLabel;
static struct Key       Keys[16][128], *Key0, *Key1;
static struct Thru     *Thru;
static struct PNoteI    PendingEventsI[128], *PendingI, *LatestPendingI;
static struct PNoteO   *PendingEventsO     , *PendingO, *LatestPendingO;
static struct Chord     Chords[0xcccc+1];
static struct RecEvent  RecEvents[1024*1024], *RecEvent;
static unsigned char    sysex_message_pending, PressedNotes[12], RootKey, IRQ, *Mutes, *Mute, *Mute0, *Mute1, *Mute2, *Mute3, *Mute11, *EntryMute, *FirstMute, *MuteA, *MuteB, Dead, ExitVal, PedalLeft, PedalMid, InPortOrder[256];
static unsigned long    V0, V1, c, i, i1, v, LastTime, LabelNum, TrkNum, Var, Var0, Var1;
static struct MidiEvent *MidiEvents, *MidiEvent, **TrkInfo, ***Thrus, *ThruE, *ThruE1;
static float            Speed, Speed0;
static signed char      InOfs;

//============================================================================//

static void CALLBACK MidiInProc(HMIDIIN hMidiIn, unsigned long wMsg, unsigned long dwInstance, unsigned long dwParam1, unsigned long dwParam2) { Dead = 0; switch (wMsg) { case MIM_DATA: switch (dwParam1 & 0xf0) {

case 0x90: RecEvent->event_time = timeGetTime(); V1 = dwParam1>>16; if (!V1) { V1 = 0x40; goto L0x80; } if ((V0 = (dwParam1>>8 & 0x7f)+InOfs) & -128) { return; } Key1 = &Keys[dwParam1 & 0xf][V0]; i = -1;
 while ((Thru = &Key1->Thrus[++i])->Trk) { if ((*Thru->Trk) && ((*Thru->Trk)->Out & 0x1)) { ThruE = Thru->Pending = (*Thru->Trk); if (Thru->Delay) { Sleep(Thru->Delay); }
  midiOutShortMsg(ThruE->midi_out, Thru->v1[V1]<<16 | Thru->k<<8 | 0x90 | ThruE->Ch); }}
 switch (Key1->Zone) { case 1: if (!(PendingI = &PendingEventsI[V0])->Vel) { PendingI->Vel = V1;
  if (LatestPendingI) { LatestPendingI->Next = PendingI; } PendingI->Prev = LatestPendingI; (LatestPendingI = PendingI)->Next = NULL;
  PressedNotes[PendingI->Note] = V0; c = i = v = 0;
  while (PendingI) { c = (c<<4) | PendingI->Note + 1; v += PendingI->Vel; PendingI = PendingI->Prev; } if ((c <= 0xcccc) && (Chords[c].Type >= 0x000)) { RootKey = PressedNotes[Chords[c].Root]; PendingI = LatestPendingI;
  while (PendingI) { if (PendingI->Key < RootKey) { i++; }                PendingI = PendingI->Prev; } V0 = Var | Chords[c].Type | (i<<4) | Chords[c].Root; V1 = v / Chords[c].Num; }} break;
  case  2: if ((i = Key1->Val | Label0->Idx & 0xfff) < LabelNum && !Labels[i].Ret) { if (Key1->Val == Var0) { Var0 = Var1; } else { Var1 = Var0; Var0 = Key1->Val; }} else { Var1 = Var0 = Key1->Val; }
           V0 = (Var = Var0) | Label2->Idx & 0xfff; if (MidiEvent->Label->Ret && V0 < LabelNum && !Labels[V0].Ret && (i = Var | Label1->Idx & 0xfff) < LabelNum) { Label1 = &Labels[i]; V0 = -1; } break;
  case  4: Mute[Key1->Val] ^= 0x08;                                                                                              V0 |= Var; break;
  case  8: if (!(Mute2 = (unsigned char*)Key1->Val)[-1]) { if (Mute2 == MuteA) { MuteA = MuteB; } else { MuteB = MuteA; MuteA = Mute2; } Mute0 = Mute1 = Mute2 = Mute3 = Mute11 = MuteA; }
            else                                         { Mute1 = Mute3 = Mute2; if (!Mute[-1]) { Mute0 = Mute11 = Mute; }}     V0 |= Var; break;
  case 16: Label3 = Label2 = Label1 = Label0 = MidiEvents->Label; IRQ = 0x20;                                                    V0 |= Var; break;
  default:                                                                                                                       V0 |= Var; }
  MyMacro0 RecEvent->EventData = dwParam1; RecEvent = RecEvent->NextEvent; return;

case 0x80: RecEvent->event_time = timeGetTime(); V1 = dwParam1>>16;                            L0x80:   if ((V0 = (dwParam1>>8 & 0x7f)+InOfs) & -128) { return; } Key0 = &Keys[dwParam1 & 0xf][V0]; i = -1;
 while (ThruE = (Thru = &Key0->Thrus[++i])->Pending) { midiOutShortMsg(ThruE->midi_out, Thru->v0[V1]<<16 | Thru->k<<8 | 0x80 | ThruE->Ch); }
 switch (Key0->Zone) { case 1: if ( (PendingI = &PendingEventsI[V0])->Vel) { PendingI->Vel =  0;
  if (PendingI->Prev) { PendingI->Prev->Next = PendingI->Next; }
  if (PendingI->Next) { PendingI->Next->Prev = PendingI->Prev; } else { LatestPendingI = PendingI->Prev; }
  if (!LatestPendingI) { V0 = Label2->Idx & 0xfff; }
  }} V0 |= Var | 0x80;
  MyMacro0 RecEvent->EventData = dwParam1; RecEvent = RecEvent->NextEvent; return;

case 0xa0: case 0xb0: case 0xc0: case 0xd0: case 0xe0: RecEvent->event_time = timeGetTime(); i = -1;
 switch (dwParam1 & 0x7ff0) { case 0x40b0: while (              Thrus[++i]        ) { if (ThruE = *Thrus[i]) { midiOutShortMsg(ThruE->midi_out, dwParam1 & 0xfffffff0 | ThruE->Ch); }} break;
                              case 0x42b0: while (              Thrus[++i]        ) { if (ThruE = *Thrus[i]) { midiOutShortMsg(ThruE->midi_out, dwParam1 & 0xfffffff0 | ThruE->Ch); }}
                                           PedalMid  = dwParam1>>16; if (PedalMid) { Speed0 = 1-(float)PedalLeft/127*.5; } else { Speed0 = 1+(float)PedalLeft/127; }                   break;
                              case 0x43b0: while (              Thrus[++i]        ) { if (ThruE = *Thrus[i]) { midiOutShortMsg(ThruE->midi_out, dwParam1 & 0xfffffff0 | ThruE->Ch); }}
                                           PedalLeft = dwParam1>>16; if (PedalMid) { Speed0 = 1-(float)PedalLeft/127*.5; } else { Speed0 = 1+(float)PedalLeft/127; }                   break;
                              default:     while (ThruE = Key1->Thrus[++i].Pending) {                          midiOutShortMsg(ThruE->midi_out, dwParam1 & 0xfffffff0 | ThruE->Ch);  }        }
 RecEvent->EventData = dwParam1; RecEvent = RecEvent->NextEvent; return;

} return; //switch dwParam1 // MIM_DATA fallthru

case MIM_LONGDATA: V0 = timeGetTime(); i = -1;
 while (++i < ((MIDIHDR*)dwParam1)->dwBytesRecorded) { RecEvent->event_time = V0; RecEvent->EventData = ((*(((MIDIHDR*)dwParam1)->lpData+i)&0xff)<<8) | 0xf0; RecEvent = RecEvent->NextEvent; }
 if (((MIDIHDR*)dwParam1)->dwBytesRecorded) { midiInAddBuffer(hMidiIn, (MIDIHDR*)dwParam1, sizeof(MIDIHDR)); } return; // MIM_LONGDATA
case MIM_OPEN: case MIM_CLOSE: return; // MIM_OPEN|MIM_CLOSE

} printf("%08x %08x %08x %08x %08x\n", hMidiIn, wMsg, dwInstance, dwParam1, dwParam2); } //switch wMsg // CALLBACK fallthru

//----------------------------------------------------------------------------//

static void CALLBACK MidiInProc1(HMIDIIN hMidiIn, unsigned long wMsg, unsigned long dwInstance, unsigned long dwParam1, unsigned long dwParam2) { midiInGetID(hMidiIn, &i1); switch (wMsg) {
case MIM_DATA: if (((i1 = InPortOrder[i1]+(dwParam1&0xf)) < TrkNum) && (ThruE1 = TrkInfo[i1]) && (ThruE1->Out & 0x1)) { midiOutShortMsg(ThruE1->midi_out, dwParam1 & 0xfffffff0 | ThruE1->Ch); } return;
case MIM_LONGDATA: i1 = -1; while (++i1 < ((MIDIHDR*)dwParam1)->dwBytesRecorded) { printf("%02x ", (*(((MIDIHDR*)dwParam1)->lpData+i1)&0xff)); } if (i1) { printf("\n"); midiInAddBuffer(hMidiIn, (MIDIHDR*)dwParam1, sizeof(MIDIHDR)); } return; // MIM_LONGDATA
case MIM_OPEN: case MIM_CLOSE: return; // MIM_OPEN|MIM_CLOSE

} printf("%08x %08x %08x %08x %08x\n", hMidiIn, wMsg, dwInstance, dwParam1, dwParam2); } //switch wMsg // CALLBACK fallthru

//----------------------------------------------------------------------------//

static void CALLBACK MidiOutProc(HMIDIOUT hmo, unsigned long wMsg, unsigned long dwInstance, unsigned long dwParam1, unsigned long dwParam2) { sysex_message_pending = 0; PulseEvent(signalling_object); }

//----------------------------------------------------------------------------//

static BOOL WINAPI HandlerRoutine(DWORD dwCtrlType) { RecEvent->event_time = timeGetTime(); V1 = -1; if (dwCtrlType) { Label3 = Label2 = Label1 = Label0 = EntryLabel; } else { Label3 = Label2 = Label1 = Label0 = ExitLabel; ExitVal |= 4; } V0 = Label0->Idx; IRQ = 0x08; MyMacro1 PulseEvent(signalling_object); return(TRUE); }

//----------------------------------------------------------------------------//

static void saveMidiEventsToFile(signed long *args, unsigned short PPQ, unsigned long Tempo, unsigned long TimeSig, unsigned long KeySig, struct RecEvent *RecEvents, struct RecEvent *RecEvent, unsigned char ExitVal, struct Label *Label0) {
SYSTEMTIME    current_time;
unsigned char filename[1024], tempo[] = {(Tempo>>16)&0xff, (Tempo>>8)&0xff, (Tempo>>0)&0xff}, timeSig[] = {(TimeSig>>24)&0xff, (TimeSig>>16)&0xff, (TimeSig>>8)&0xff, (TimeSig>>0)&0xff}, keySig[] = {(KeySig>>8)&0xff, (KeySig>>0)&0xff};
unsigned long PPQc = PPQ*1000;
float         c = (float)PPQc/(float)(Tempo&0xffffff);

MidiFile_t midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, PPQ);

MidiFileTrack_t track0 = MidiFile_createTrack(midi_file), track1 = MidiFile_createTrack(midi_file);

MidiFileTrack_createMetaEvent(track0, 0, 0x51, 3, tempo);
if (!(TimeSig & 0x00000080)) { MidiFileTrack_createMetaEvent(track0, 0, 0x58, 4, timeSig); }
if (!(KeySig  & 0x00010000)) { MidiFileTrack_createMetaEvent(track0, 0, 0x59, 2, keySig ); }

while (!(ExitVal&4) && (RecEvent-- >= RecEvents) && ((RecEvent->EventData & 0xe0) == 0x80) && (((RecEvent->EventData>>8) & 0x7f) == Label0->Idx)) { RecEvent->EventData = 0x00000000; }

if (!RecEvents[sizeof(RecEvents)/sizeof(struct RecEvent)-1].EventData) { unsigned long i = -1, data_length = 0, t0; unsigned char data_buffer[1024];
 while (RecEvents[++i].EventData) { unsigned long t = (RecEvents[i].event_time-RecEvents[0].event_time)*c;
  switch (RecEvents[i].EventData & 0xf0) {
  case 0x80: { MidiFileTrack_createNoteOffEvent(        track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f , (RecEvents[i].EventData>>16)&0x7f); break; }
  case 0x90: { MidiFileTrack_createNoteOnEvent(         track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f , (RecEvents[i].EventData>>16)&0x7f); break; }
  case 0xa0: { MidiFileTrack_createKeyPressureEvent(    track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f , (RecEvents[i].EventData>>16)&0x7f); break; }
  case 0xb0: { MidiFileTrack_createControlChangeEvent(  track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f , (RecEvents[i].EventData>>16)&0x7f); break; }
  case 0xc0: { MidiFileTrack_createProgramChangeEvent(  track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f                                    ); break; }
  case 0xd0: { MidiFileTrack_createChannelPressureEvent(track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f                                    ); break; }
  case 0xe0: { MidiFileTrack_createPitchWheelEvent(     track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>9)&0x3f80 | (RecEvents[i].EventData>> 8)&0x7f); break; }
  case 0xf0: { if ((RecEvents[i].EventData>>8) == 0xf0) { if (data_length) { MidiFileTrack_createSysexEvent(track1, t0, data_length, data_buffer); } data_length = 0; }
               if (!data_length) { t0 = t; } if (data_length < sizeof(data_buffer)) { data_buffer[data_length++] = RecEvents[i].EventData>>8; }
               if ((RecEvents[i].EventData>>8) == 0xf7) {                    MidiFileTrack_createSysexEvent(track1, t0, data_length, data_buffer);   data_length = 0; } break; }
  }}
 if (data_length) { MidiFileTrack_createSysexEvent(track1, t0, data_length, data_buffer); } //write incomplete sysex w/o f7 termination (should not happen)
 }

GetLocalTime(&current_time); sprintf(filename, "MyMid%d%02d%02d%02d%02d%02d.mid", current_time.wYear, current_time.wMonth, current_time.wDay, current_time.wHour, current_time.wMinute, current_time.wSecond);
if (args[8] != 0x0ff) { MidiFile_save(midi_file, filename); } MidiFile_free(midi_file); return; }

//============================================================================//

signed long main(signed long argc, unsigned char **argv) {
static TIMECAPS          time_caps;
static MIDIINCAPS        midi_i_caps;
static MIDIOUTCAPS       midi_o_caps;
static MIDIHDR           midi_message_header, midi_message_header0;
static struct MidiIn     Port2In[  256];
static struct MidiOut    Port2Out[ 256];
static unsigned char     Port2Port[256], data_buffer[1024];
static MidiFileEvent_t   midi_file_event;
static unsigned long     start_time, WatchDogTimeOut, MutesNum, tick, MutesInv, MutesRet, MutesInv1, MutesInv2, Tempo, TimeSig, KeySig, Tempo0, TimeSig0, KeySig0;
static   signed long     i, j, k, l, *args, TimeOut;
static unsigned char     Permutations1[ 1][1] = {{0}};
static unsigned char     Permutations2[ 2][2] = {{0,1},{1,0}};
static unsigned char     Permutations3[ 6][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
static unsigned char     Permutations4[24][4] = {{0,1,2,3},{0,1,3,2},{0,2,1,3},{0,2,3,1},{0,3,1,2},{0,3,2,1},
                                                 {1,0,2,3},{1,0,3,2},{1,2,0,3},{1,2,3,0},{1,3,0,2},{1,3,2,0},
                                                 {2,0,1,3},{2,0,3,1},{2,1,0,3},{2,1,3,0},{2,3,0,1},{2,3,1,0},
                                                 {3,0,1,2},{3,0,2,1},{3,1,0,2},{3,1,2,0},{3,2,0,1},{3,2,1,0}};
static unsigned char     Intervals0[] = {0      };
static unsigned char     IntervalsA[] = {0,    0};
static unsigned char     Intervals1[] = {0,    7};
static unsigned char     Intervals9[] = {0, 0, 0};
static unsigned char     IntervalsB[] = {0, 7, 0};
static unsigned char     Intervals2[] = {0, 4, 7};
static unsigned char     Intervals3[] = {0, 3, 7};
static unsigned char     Intervals4[] = {0, 2, 7};
static unsigned char     Intervals8[] = {0, 0, 0,  0};
static unsigned char     IntervalsC[] = {0, 7, 0,  7};
static unsigned char     Intervals5[] = {0, 4, 7, 11};
static unsigned char     Intervals6[] = {0, 3, 7, 10};
static unsigned char     Intervals7[] = {0, 4, 7, 10};
static unsigned char     IntervalsD[] = {0, 4, 7, 12};
static unsigned char     IntervalsE[] = {0, 3, 7, 12};

MidiFile_t midi_file = MidiFile_load(argv[1]); args = malloc(argc*sizeof(signed long)); for (i=2; i<_msize(args)/sizeof(signed long); i++) { args[i] = strtol(argv[i], NULL, 0); }

for (i=0; i<(sizeof(Chords)/sizeof(struct Chord)); i++) { Chords[i].Type = -1; Chords[i].Root = 0; Chords[i].Num = 0; j = i; while (j) { if (j&0xf) { Chords[i].Num++; } j >>= 4; }}
for (i=1; i<(sizeof(Chords)/sizeof(struct Chord)); i++) { Chords[i].Root = 11; j = i; while (j) { if ((j&0xf) && ((j&0xf)-1 <= Chords[i].Root)) { Chords[i].Root = (j&0xf)-1; } j >>= 4; }}

for (i=0; i<(sizeof(Port2Port)/sizeof(unsigned char)); i++) { Port2Port[i] = i; }

for (i=0x0; i<=0xf; i++) {
 for (j=0x00; j <= 0x7f; j++) { Keys[i][j].Zone = 0;
  for (k=0; k<(sizeof(Keys[i][j].Thrus)/sizeof(struct Thru)); k++) { Keys[i][j].Thrus[k].Trk = NULL; }
  }
 }

for (i=0; i<=11; i++) {
 for (j=0; j<=0; j++) {
  l = 0; for (k=0; k<=0; k++) { l = (l<<4) + (i+Intervals0[Permutations1[j][k]])%12 + 1; } Chords[l].Type = 0x120; Chords[l].Root = i;
  }
 for (j=0; j<=1; j++) {
  l = 0; for (k=0; k<=1; k++) { l = (l<<4) + (i+IntervalsA[Permutations2[j][k]])%12 + 1; } Chords[l].Type = 0x120; Chords[l].Root = i;
  l = 0; for (k=0; k<=1; k++) { l = (l<<4) + (i+Intervals1[Permutations2[j][k]])%12 + 1; } Chords[l].Type = 0x100; Chords[l].Root = i;
  }
 for (j=0; j<=5; j++) {
  l = 0; for (k=0; k<=2; k++) { l = (l<<4) + (i+Intervals9[Permutations3[j][k]])%12 + 1; } Chords[l].Type = 0x120; Chords[l].Root = i;
  l = 0; for (k=0; k<=2; k++) { l = (l<<4) + (i+IntervalsB[Permutations3[j][k]])%12 + 1; } Chords[l].Type = 0x100; Chords[l].Root = i;
  l = 0; for (k=0; k<=2; k++) { l = (l<<4) + (i+Intervals2[Permutations3[j][k]])%12 + 1; } Chords[l].Type = 0x200; Chords[l].Root = i;
  l = 0; for (k=0; k<=2; k++) { l = (l<<4) + (i+Intervals3[Permutations3[j][k]])%12 + 1; } Chords[l].Type = 0x300; Chords[l].Root = i;
  l = 0; for (k=0; k<=2; k++) { l = (l<<4) + (i+Intervals4[Permutations3[j][k]])%12 + 1; } Chords[l].Type = 0x400; Chords[l].Root = i;
  }
 for (j=0; j<=23; j++) {
  l = 0; for (k=0; k<=3; k++) { l = (l<<4) + (i+Intervals8[Permutations4[j][k]])%12 + 1; } Chords[l].Type = 0x120; Chords[l].Root = i;
  l = 0; for (k=0; k<=3; k++) { l = (l<<4) + (i+IntervalsC[Permutations4[j][k]])%12 + 1; } Chords[l].Type = 0x100; Chords[l].Root = i;
  l = 0; for (k=0; k<=3; k++) { l = (l<<4) + (i+Intervals5[Permutations4[j][k]])%12 + 1; } Chords[l].Type = 0x200; Chords[l].Root = i;
  l = 0; for (k=0; k<=3; k++) { l = (l<<4) + (i+Intervals6[Permutations4[j][k]])%12 + 1; } Chords[l].Type = 0x300; Chords[l].Root = i;
  l = 0; for (k=0; k<=3; k++) { l = (l<<4) + (i+Intervals7[Permutations4[j][k]])%12 + 1; } Chords[l].Type = 0x200; Chords[l].Root = i;
  l = 0; for (k=0; k<=3; k++) { l = (l<<4) + (i+IntervalsD[Permutations4[j][k]])%12 + 1; } Chords[l].Type = 0x200; Chords[l].Root = i;
  l = 0; for (k=0; k<=3; k++) { l = (l<<4) + (i+IntervalsE[Permutations4[j][k]])%12 + 1; } Chords[l].Type = 0x300; Chords[l].Root = i;
  }
 }

i = 0; j = k = -1; MutesNum = MutesInv = MutesRet = 0;
for (midi_file_event = MidiFile_getFirstEvent(midi_file); midi_file_event; midi_file_event = MidiFileEvent_getNextEventInFile(midi_file_event)) {
 if (MidiFileTrack_getNumber(MidiFileEvent_getTrack(midi_file_event)) > k) { k = MidiFileTrack_getNumber(MidiFileEvent_getTrack(midi_file_event)); }
 if (MidiFileEvent_getType(midi_file_event) == MIDI_FILE_EVENT_TYPE_META) {
  if (MidiFileMetaEvent_getNumber(midi_file_event) == 0x03) {                    unsigned char *p0 = MidiFileMetaEvent_getData(midi_file_event), *p1; p1 = p0; //trkname
   while (p0 = strstr(p0, KW2)) { signed long v = strtol(p0+sizeof(KW2)-1, &p0, 0); unsigned long t = 0; if (v == -1) { v = 0; } if (p0 == strstr(p0, "r")) { MutesRet |= v; }                while (v) { t++; v >>= 1; } if (t > MutesNum) { MutesNum = t; }}
   while (p1 = strstr(p1, KW3)) { signed long v = strtol(p1+sizeof(KW3)-1, &p1, 0); unsigned long t = 0; if (v == -1) { v = 0; } if (p1 == strstr(p1, "r")) { MutesRet |= v; } MutesInv |= v; while (v) { t++; v >>= 1; } if (t > MutesNum) { MutesNum = t; }}
   }
  if (MidiFileMetaEvent_getNumber(midi_file_event) == 0x06) { unsigned long t=0; unsigned char *p0 = MidiFileMetaEvent_getData(midi_file_event), *p1; p1 = p0; //marker
   while (p0 = strstr(p0, KW0)) { signed long v = strtol(p0+sizeof(KW0)-1, &p0, 0); t=1; if (v > j) { j = v; }}
   while (p1 = strstr(p1, KW1)) {                 strtol(p1+sizeof(KW1)-1, &p1, 0); t=1; }
   i += t;
   }
  if (MidiFileMetaEvent_getNumber(midi_file_event) == 0x7f) { unsigned long L = MidiFileMetaEvent_getDataLength(midi_file_event); unsigned char *D = MidiFileMetaEvent_getData(midi_file_event);
   if ((L >= 4) && (D[0] == 0x00) && (D[1] == 0xab) && (D[2] == 0xcd)) {
    if ((D[3] == 0x00) && (L >= 8)) { signed long A = D[4]<<24 | D[5]<<16 | D[6]<<8 | D[7], t; if (A < 0) { A += _msize(args)/sizeof(signed long)+1; }
	 if ((A+((L-8)>>2))*sizeof(signed long) > _msize(args)) { args = realloc(args, (A+((L-8)>>2))*sizeof(signed long)); }
	 for (t=8; (t+3)<L; t += 4) { args[A++] = D[t+0]<<24 | D[t+1]<<16 | D[t+2]<<8 | D[t+3]; }
	 }
    }
   }
  } else { i++; }
 }

if (args[10] > j) { j = args[10]; } if (args[11] > j) { j = args[11]; } MutesNum += 2; MutesInv1 = (-1^MutesInv) & ((1<<(MutesNum-2))-1); MutesInv2 = (1<<(MutesNum-1)) | MutesInv; InOfs = args[7];

MidiEvents = malloc((i+1)*sizeof(struct MidiEvent)); Labels = malloc((j+1)*sizeof(struct Label)); PendingEventsO = malloc(1*(k+1)*16*128*sizeof(struct PNoteO)); Mutes = malloc(MutesNum*(k+2)*sizeof(unsigned char)); TrkInfo = malloc((k+1)*sizeof(struct MidiEvent*)); Thrus = malloc((k+2)*sizeof(struct MidiEvent**));

LabelNum = _msize(Labels)/sizeof(struct Label); TrkNum = _msize(TrkInfo)/sizeof(struct MidiEvent*);

for (i=0; i<LabelNum; i++) { Labels[i].Idx = i; Labels[i].Event = NULL; Labels[i].Ret = 0; } EntryLabel = &Labels[args[10]]; ExitLabel = &Labels[args[11]];

for (i=0x0; i<=0xf; i++) { signed long C = args[6], Ck = args[12], Mk = args[13]+1, K2 = Ck-MutesNum, K1 = K2-1, K0 = K1; if (LabelNum>>12) { K0 -= ((LabelNum-1)>>12)+1; } if (Mk-1 < 0) { Mk = Ck+abs(Mk-1); if (Mk > 128) { Mk = 128; }}
 for (j = K0; j < K1; j++) { Keys[i][j].Zone |= 2; Keys[i][j].Val = (K1-j-1)<<12; }
 for (j = K1; j < K2; j++) { Keys[i][j].Zone |= 4; Keys[i][j].Val = K2-j-1; }
 for (j = K2; j < Ck; j++) { Keys[i][j].Zone |= 8; if (j > (Ck-3)) { Keys[i][j].Val = (unsigned long)&Mutes[(j-K2)*(TrkNum+1)+1]; } else { Keys[i][j].Val = (unsigned long)&Mutes[(Ck-3-j)*(TrkNum+1)+1]; }}
 for (j = Ck; j < Mk; j++) { Keys[i][j].Zone |= 1; }
 for (j = args[10]+1; j <= args[10]+1; j++) { Keys[i][j].Zone |= 16; }

 k = 12;
 while (((k+6) < _msize(args)/sizeof(signed long)) && (abs(args[k]) < 0x10000)) { signed long K0 = args[k++], K1 = args[k++]+1, T = args[k++], Delay = args[k++], K = args[k++], V1 = args[k++], V0 = args[k++];
  signed char v0o = V0&0xff, v1o = V1&0xff; float v0s = 1.0, v1s = 1.0; if (V0 > 255) { v0s = ((V0>>8)-1)*.25; } if (V1 > 255) { v1s = ((V1>>8)-1)*.25; }
  if (K0 <= -2) { K0 = Ck; } if (K0 == -1) { K0 = Mk; } if (K1-1 < 0) { K1 = K0+abs(K1-1); if (K1 > 128) { K1 = 128; }} Ck = K0; Mk = K1;
  if ((T < TrkNum) && ((C < 0) || (i == C))) { if (C < -1) { T = (T+i+abs(C+2))%TrkNum; }
   for (j = K0; j < K1; j++) { signed long k = j+K, L = -1, a; while (Keys[i][j].Thrus[++L].Trk) {} if (K > 0x7f) { k = K & 0x7f; } if ((k > 127) || (k < 0)) { k = j; }
    Keys[i][j].Thrus[L].Trk = &TrkInfo[T]; Keys[i][j].Thrus[L].Delay = Delay; Keys[i][j].Thrus[L].k = k;
    for (a = 0; a <= 0x7f; a++) { signed long v = a*v0s; if (v > 127) { v = 127; } v += v0o; if (v > 127) { v = 127; } else if (v < 0) { v = 0; } Keys[i][j].Thrus[L].v0[a] = v; }
    for (a = 0; a <= 0x7f; a++) { signed long v = a*v1s; if (v > 127) { v = 127; } v += v1o; if (v > 127) { v = 127; } else if (v < 1) { v = 1; } Keys[i][j].Thrus[L].v1[a] = v; }
	}
   }
  }
 }

if (args[3] < 0) { args[3] += midiOutGetNumDevs(); } if (args[3] < 0) { args[3] = midiOutGetNumDevs(); }
if (args[4] < 0) { args[4] += midiInGetNumDevs();  } if (args[4] < 0) { args[4] = midiInGetNumDevs();  }

for (i=12; i<_msize(args)/sizeof(signed long); i++) { if ((args[i]>>16) == 1) { Port2Port[(args[i]>>8)&0xff] = args[i]&0xff; } if ((args[i]>>16) == 3) { Port2Port[(args[i]>>8)&0xff] = midiOutGetNumDevs()-1-args[i]&0xff; }}

timeGetDevCaps(&time_caps, sizeof(TIMECAPS)); signalling_object = CreateEvent(NULL, FALSE, FALSE, NULL);

printf("[%d:%d] [%d:%d] %d %d %d %d %x %x %x %x %4.2f %4.2f\n", time_caps.wPeriodMin, time_caps.wPeriodMax, midiInGetNumDevs()-1, midiOutGetNumDevs()-1, MidiFile_getResolution(midi_file), MidiFile_getFileFormat(midi_file), TrkNum, _msize(MidiEvents)/sizeof(struct MidiEvent)-1, LabelNum-1, MutesNum-1, MutesInv2, MutesRet, (float)_msize(MidiEvents)/(1024*1024), (float)_msize(Labels)/(1024*1024));

start:
for (i=0; i<(sizeof(Port2In       )/sizeof(struct MidiIn     )); i++) { Port2In[i].s  = -1; Port2In[i].h  = NULL; }
for (i=0; i<(sizeof(Port2Out      )/sizeof(struct MidiOut    )); i++) { Port2Out[i].s = -1; Port2Out[i].h = NULL; }
for (i=0; i<(_msize(TrkInfo       )/sizeof(struct MidiEvent* )); i++) { TrkInfo[i] = (struct MidiEvent*)(args[3] << 8); }
for (i=0; i<(sizeof(PendingEventsI)/sizeof(struct PNoteI     )); i++) { PendingEventsI[i].Prev = PendingEventsI[i].Next = NULL; PendingEventsI[i].Vel   =    0; PendingEventsI[i].Key = i; PendingEventsI[i].Note = i%12;                                      } LatestPendingI = NULL;
for (i=0; i<(_msize(PendingEventsO)/sizeof(struct PNoteO     )); i++) { PendingEventsO[i].Prev = PendingEventsO[i].Next = NULL; PendingEventsO[i].Event = NULL; PendingEventsO[i].EventOffMsg = 0x400080 ^ (i>>11)/TrkNum*0x400030 | (i&0x7f)<<8 | (i>>7)&0xf; } LatestPendingO = NULL;
for (i=0; i<(_msize(Mutes         )/sizeof(unsigned char     )); i++) { Mutes[i] = 0; }
for (i=1; i<(sizeof(RecEvents     )/sizeof(struct RecEvent   )); i++) { RecEvents[i-1].NextEvent = &RecEvents[i]; RecEvents[i-1].EventData = 0; } RecEvents[i-1].NextEvent = &RecEvents[0]; RecEvents[i-1].EventData = 0; RecEvent = &RecEvents[0];
for (i=0; i<(_msize(Thrus         )/sizeof(struct MidiEvent**)); i++) { Thrus[i] = NULL; }

for (i=0x0; i<=0xf; i++) {
 for (j=0x00; j <= 0x7f; j++) {
  k = -1; while (Keys[i][j].Thrus[++k].Trk) { signed long L = -1; Keys[i][j].Thrus[k].Pending = NULL; while ((Thrus[++L]) && (Thrus[L] != Keys[i][j].Thrus[k].Trk)) {} Thrus[L] = Keys[i][j].Thrus[k].Trk; }
  }
 }

k = args[4];
Port2In[k].s = midiInOpen(&Port2In[k].h, k, (unsigned long)MidiInProc , (unsigned long)NULL, CALLBACK_FUNCTION | MIDI_IO_STATUS);
strcpy(midi_i_caps.szPname, ""); midiInGetDevCaps(k, &midi_i_caps, sizeof(midi_i_caps)); printf("i%2d %x '%s'\n", k, Port2In[k].s, midi_i_caps.szPname);
midi_message_header0.lpData = data_buffer; midi_message_header0.dwBufferLength = sizeof(data_buffer); midi_message_header0.dwFlags = 0;
midiInPrepareHeader(Port2In[k].h, &midi_message_header0, sizeof(MIDIHDR)); midiInAddBuffer(Port2In[k].h, &midi_message_header0, sizeof(MIDIHDR));

for (i=12; i<_msize(args)/sizeof(signed long); i++) { if ((((args[i]>>16) == 2) && (!Port2In[k = args[i]&0xff].h) || ((args[i]>>16) == 4) && (!Port2In[k = midiInGetNumDevs()-1-args[i]&0xff].h)) && (k < midiInGetNumDevs())) {
Port2In[k].s = midiInOpen(&Port2In[k].h, k, (unsigned long)MidiInProc1, (unsigned long)NULL, CALLBACK_FUNCTION | MIDI_IO_STATUS);
strcpy(midi_i_caps.szPname, ""); midiInGetDevCaps(k, &midi_i_caps, sizeof(midi_i_caps)); printf("i%2d %x '%s'\n", k, Port2In[k].s, midi_i_caps.szPname);
midi_message_header0.lpData = data_buffer; midi_message_header0.dwBufferLength = sizeof(data_buffer); midi_message_header0.dwFlags = 0;
midiInPrepareHeader(Port2In[k].h, &midi_message_header0, sizeof(MIDIHDR)); midiInAddBuffer(Port2In[k].h, &midi_message_header0, sizeof(MIDIHDR)); InPortOrder[k] = (args[i]>>8)&0xff; }}

i = j = tick = 0; FirstLabel = LastLabel = NULL; Tempo = Tempo0 = 0x0107a120; TimeSig = TimeSig0 = 0x04020088; KeySig = KeySig0 = 0x00010000;
for (midi_file_event = MidiFile_getFirstEvent(midi_file); midi_file_event; midi_file_event = MidiFileEvent_getNextEventInFile(midi_file_event)) {
 if (MidiFileEvent_getTick(midi_file_event) != tick) { j = i; tick = MidiFileEvent_getTick(midi_file_event); }
 if ((i > j) && (!MidiEvents[i-1].MsgCtl && !MidiEvents[i-1].Rec)) { i--; } else { MidiEvents[i].FlwCtl = 0; MidiEvents[i].JumpEvent = (struct MidiEvent*)-1; }
 MidiEvents[i].NextEvent  = &MidiEvents[i+1];
 MidiEvents[i].event_time = (unsigned long)(MidiFile_getTimeFromTick(midi_file, MidiFileEvent_getTick(midi_file_event)) * 1000);
 MidiEvents[i].Tempo      = Tempo   & 0x00ffffff;
 MidiEvents[i].TimeSigN   = TimeSig >> 24;
 MidiEvents[i].TimeSigD   = TimeSig >> 16;
 MidiEvents[i].Label      = EntryLabel; if (LastLabel) { MidiEvents[i].Label = LastLabel; }
 MidiEvents[i].MsgCtl     = MidiEvents[i].Rec = 0;
 MidiEvents[i].Track      = MidiFileTrack_getNumber(MidiFileEvent_getTrack(midi_file_event));
 MidiEvents[i].Out        = (unsigned long)TrkInfo[MidiEvents[i].Track];
 if (Port2Out[MidiEvents[i].Out>>8].s == -1) { Port2Out[MidiEvents[i].Out>>8].s = midiOutOpen(&Port2Out[MidiEvents[i].Out>>8].h, MidiEvents[i].Out>>8, (unsigned long)MidiOutProc, (unsigned long)NULL, CALLBACK_FUNCTION);
  strcpy(midi_o_caps.szPname, ""); midiOutGetDevCaps(MidiEvents[i].Out>>8, &midi_o_caps, sizeof(midi_o_caps)); printf("o%2d %x '%s'\n", MidiEvents[i].Out>>8, Port2Out[MidiEvents[i].Out>>8].s, midi_o_caps.szPname);
  }
 if (!Port2Out[MidiEvents[i].Out>>8].h) { MidiEvents[i].Out = args[3] << 8; }
 MidiEvents[i].midi_out = Port2Out[MidiEvents[i].Out>>8].h;
 if (MidiFileEvent_getType(midi_file_event) == MIDI_FILE_EVENT_TYPE_META) {
  MidiEvents[i].EventData   = (MidiFileMetaEvent_getNumber(midi_file_event) << 8) | 0x000000ff;
  MidiEvents[i].data_length = MidiFileMetaEvent_getDataLength(midi_file_event);
  MidiEvents[i].data_buffer = MidiFileMetaEvent_getData(midi_file_event);
  if (MidiEvents[i].EventData == 0x000051ff) { Tempo   =                                      (MidiEvents[i].data_buffer[0]<<16) | (MidiEvents[i].data_buffer[1]<<8) | (MidiEvents[i].data_buffer[2]<<0); if (Tempo0   & 0x01000000) { Tempo0   = Tempo;   }}
  if (MidiEvents[i].EventData == 0x000058ff) { TimeSig = (MidiEvents[i].data_buffer[0]<<24) | (MidiEvents[i].data_buffer[1]<<16) | (MidiEvents[i].data_buffer[2]<<8) | (MidiEvents[i].data_buffer[3]<<0); if (TimeSig0 & 0x00000080) { TimeSig0 = TimeSig; }}
  if (MidiEvents[i].EventData == 0x000059ff) { KeySig  =                                                                           (MidiEvents[i].data_buffer[0]<<8) | (MidiEvents[i].data_buffer[1]<<0); if (KeySig0  & 0x00010000) { KeySig0  = KeySig;  }}
  if (MidiEvents[i].EventData == 0x000021ff) { TrkInfo[MidiEvents[i].Track] = (struct MidiEvent*)((unsigned long)TrkInfo[MidiEvents[i].Track] & 0xffff00ff | (Port2Port[MidiEvents[i].data_buffer[0]] << 8) | 0x2); } //port
  if (MidiEvents[i].EventData == 0x000003ff) {                    unsigned char *p0 = MidiEvents[i].data_buffer, *p1; p1 = p0; //trkname
   while (p0 = strstr(p0, KW2)) { signed long v = strtol(p0+sizeof(KW2)-1, &p0, 0); unsigned long t = 0; if (!v) { v = MutesInv;  } if (v == -1) { v = MutesInv1; } while (v) { if (v&1) { Mutes[t*(TrkNum+1)+1+MidiEvents[i].Track] ^= 0x08; } t++; v >>= 1; }}
   while (p1 = strstr(p1, KW3)) { signed long v = strtol(p1+sizeof(KW3)-1, &p1, 0); unsigned long t = 0; if (!v) { v = MutesInv1; } if (v == -1) { v = MutesInv;  } while (v) { if (v&1) { Mutes[t*(TrkNum+1)+1+MidiEvents[i].Track] ^= 0x08; } t++; v >>= 1; }}
   }
  if (MidiEvents[i].EventData == 0x000006ff) { unsigned long t=0; unsigned char *p0 = MidiEvents[i].data_buffer, *p1; p1 = p0; //marker
   while (p0 = strstr(p0, KW0)) { signed long v = strtol(p0+sizeof(KW0)-1, &p0, 0); t=1; MidiEvents[j].FlwCtl &= 1; if ((v&0xfff) == 0xfff) { while (Labels[v].Event) { v--; }} MidiEvents[j].JumpEvent = (struct MidiEvent*)v; Labels[v].Event = &MidiEvents[j]; LastLabel = &Labels[v]; if (!FirstLabel) { FirstLabel = LastLabel; } for (k=j; k<=i; k++) { MidiEvents[k].Label = LastLabel; }}
   while (p1 = strstr(p1, KW1)) { signed long v = strtol(p1+sizeof(KW1)-1, &p1, 0); t=1; MidiEvents[j].FlwCtl |= 3;                                                             MidiEvents[j].JumpEvent = (struct MidiEvent*)v; if (p1 == strstr(p1, ">")) { MidiEvents[j].FlwCtl |= 8; } if (p1 == strstr(p1, "v")) { MidiEvents[j].FlwCtl |= 0x10; }}
   if (i <= j) { i += t; }
   }
  }
  else if (MidiFileEvent_getType(midi_file_event) == MIDI_FILE_EVENT_TYPE_SYSEX) {
  MidiEvents[i].MsgCtl      = 1;
  MidiEvents[i].EventData   = 0x000000f0;
  MidiEvents[i].data_length = MidiFileSysexEvent_getDataLength(midi_file_event);
  MidiEvents[i].data_buffer = MidiFileSysexEvent_getData(midi_file_event);
  i++;
  }
  else { unsigned long EventIdx;
  MidiEvents[i].MsgCtl    = 2;
  MidiEvents[i].EventData = (unsigned long)MidiFileVoiceEvent_getData(midi_file_event); EventIdx = MidiEvents[i].Track<<11 | (MidiEvents[i].EventData & 0x7f00)>>8 | (MidiEvents[i].EventData & 0xf)<<7;
  MidiEvents[i].Out       = (unsigned long)(TrkInfo[MidiEvents[i].Track] = (struct MidiEvent*)((unsigned long)TrkInfo[MidiEvents[i].Track] & 0xffffff0f | ((MidiEvents[i].EventData & 0xf) << 4) | 0x1));
  if ((MidiEvents[i].EventData & 0x7f00f0) == 0x90) { MidiEvents[i].EventData = PendingEventsO[EventIdx].EventOffMsg; }
  MidiEvents[i].MsgCtl   += ((MidiEvents[i].EventData & 0xf0) == 0x80)<<0;
  MidiEvents[i].MsgCtl   += ((MidiEvents[i].EventData & 0xf0) == 0x90)<<1;
  if ((MidiEvents[i].EventData & 0xf0) == 0xb0) { EventIdx += 1*TrkNum*16*128;
   //MidiEvents[i].MsgCtl += ((MidiEvents[i].EventData & 0x407ff0) == 0x0040b0)<<0;
   //MidiEvents[i].MsgCtl += ((MidiEvents[i].EventData & 0x407ff0) == 0x4040b0)<<1;
   }
  MidiEvents[i].EventIdx = &PendingEventsO[EventIdx];
  if ((MidiEvents[i].EventData & (args[9]>>16)) == (args[9] & 0x7fff)) { if ((MidiEvents[i].EventData & 0xf0) != 0x80) { MidiEvents[j].FlwCtl |= 1; } MidiEvents[i].MsgCtl *= (args[9]>>15) & 1; }
  if ((MidiEvents[i].EventData & (args[8]>>16)) == (args[8] & 0x7fff)) {                                                 MidiEvents[i].Rec    |= 1;   MidiEvents[i].MsgCtl *= (args[8]>>15) & 1; }
  if (MidiEvents[i].MsgCtl | MidiEvents[i].FlwCtl & 1 | MidiEvents[i].Rec) { i++; }
  }
 }

MidiEvents[i].event_time = 0; if (i) { MidiEvents[i].event_time = MidiEvents[i-1].event_time; if (!MidiEvents[i-1].FlwCtl && !MidiEvents[i-1].MsgCtl && !MidiEvents[i-1].Rec && ((unsigned long)MidiEvents[i-1].JumpEvent == -1)) { i--; }}
MidiEvents[i].Label = ExitLabel; MidiEvents[i].EventData = 0; ExitLabel->Event = &MidiEvents[i]; MidiEvent = EntryLabel->Event = &MidiEvents[0]; EntryLabel->Ret = 1; k = 0;

MidiEvents[i].Tempo    = Tempo   & 0x00ffffff;
MidiEvents[i].TimeSigN = TimeSig >> 24;
MidiEvents[i].TimeSigD = TimeSig >> 16;

while (--i >= 0) { MidiEvents[i].TrkInfo = &TrkInfo[MidiEvents[i].Track]; MidiEvents[i].Ch = (MidiEvents[i].Out >> 4) & 0xf;
 if (MidiEvents[i].FlwCtl & 2) { k = 0;
  if (((unsigned long)MidiEvents[i].JumpEvent < LabelNum) && (Labels[(unsigned long)MidiEvents[i].JumpEvent].Event)) { MidiEvents[i].JumpEvent = Labels[(unsigned long)MidiEvents[i].JumpEvent].Event; }
   else if ((l = (unsigned long)MidiEvents[i].JumpEvent*-1) >= 4) { l -= 4; j = i; while (l && j) { if (MidiEvents[j].Label->Event != MidiEvents[j-1].Label->Event) { l--; } j--; } if (l) { MidiEvents[i].FlwCtl &= -3; } MidiEvents[i].JumpEvent = MidiEvents[j].Label->Event; }
   else { MidiEvents[i].FlwCtl = 4; k = (unsigned long)MidiEvents[i].JumpEvent; }
  } else if ((unsigned long)MidiEvents[i].JumpEvent != -1) { Labels[(unsigned long)MidiEvents[i].JumpEvent].Ret = k; }
 if (MidiEvents[i].FlwCtl & 0x10) { MidiEvents[i].FlwCtl = 2; }
 if (MidiEvents[i].FlwCtl & 0x08) { MidiEvents[i].FlwCtl = 5; }
 }

for (i=0; i<LabelNum; i++) { j = (i>>8)&0xf; k = (i>>4)&0x3; l = i&0xf;
 if ((j >= 1) && (j <= 4) && (l >= 1) && (l <= 0xb) && (!Labels[i].Event) && (Labels[i&(-1^0xf)].Event)) { Labels[i].Event = Labels[i&(-1^0xf)].Event; }
 }

for (i=0; i<LabelNum; i++) { j = (i>>8)&0xf; k = (i>>4)&0x3; l = i&0xf;
 if       ((j >= 1) && (j <= 1) && (k >= 1) && (k <= 1) && (!Labels[i].Event) && (Labels[i&(-1^0x30)].Event)) { Labels[i].Event = Labels[i&(-1^0x30)].Event; }
  else if ((j >= 2) && (j <= 4) && (k >= 1) && (k <= 2) && (!Labels[i].Event) && (Labels[i&(-1^0x30)].Event)) { Labels[i].Event = Labels[i&(-1^0x30)].Event; }
 }

for (i=0; i<LabelNum ; i++) { if ((!Labels[i].Event) && (Labels[i&0xfff].Event)) { Labels[i].Event = Labels[i&0xfff].Event; }}

for (i=0; i<LabelNum; i++) { if (Labels[i].Event) { unsigned char a = 0;
 for (j=0; j<LabelNum; j++) { if (Labels[i].Event == Labels[j].Event) { a |= Labels[j].Ret; }}
 for (j=0; j<LabelNum; j++) { if (Labels[i].Event == Labels[j].Event) { Labels[j].Ret = a;  }}
 }}

if (!FirstLabel) { FirstLabel = EntryLabel; } if (!LastLabel) { LastLabel = ExitLabel; }

for (j=0; j<MutesNum; j++) { Mutes[j*(TrkNum+1)] |= (MutesRet>>j)&1; for (i=0; i<TrkNum; i++) { Mutes[j*(TrkNum+1)+1+i] ^= ((MutesInv2>>j)&1)<<3; }}

for (i=0; i<(sizeof(Port2Out)/sizeof(struct MidiOut)); i++) { if (Port2Out[i].h) { for (j=0; j<=0xf; j++) {
 midiOutShortMsg(Port2Out[i].h, 0x00007bb0 | j); //all notes off (GM)
 midiOutShortMsg(Port2Out[i].h, 0x000079b0 | j); //reset all controller (GM)
 }}}

for (i=0; i<TrkNum; i++) { TrkInfo[i] = NULL; } Var = Var1 = Var0 = (Label3 = Label2 = Label1 = Label0 = EntryLabel)->Idx&-4096;

FirstMute = EntryMute = &Mutes[(MutesNum-2)*(TrkNum+1)+1]; if (MutesNum > 2) { FirstMute = &Mutes[(0)*(TrkNum+1)+1]; } Mute = Mute0 = Mute1 = Mute2 = Mute3 = Mute11 = MuteA = MuteB = EntryMute;

Key0 = Key1 = &Keys[0x0][0x00]; ExitVal = IRQ = start_time = 0; Dead = 1; Speed = Speed0 = 1; PedalLeft = PedalMid = 0;

if (args[2] >= 0) { timeBeginPeriod(args[2]); } LastTime = WatchDogTimeOut = timeGetTime(); WatchDogTimeOut += args[5]; if ((TimeOut = args[5]) < 0) { TimeOut = WatchDogTimeOut = -1; }

SetConsoleCtrlHandler(HandlerRoutine, TRUE); for (i=0; i<(sizeof(Port2In)/sizeof(struct MidiIn)); i++) { if (Port2In[i].h) { midiInStart(Port2In[i].h); }} printf("%d playing ...", ExitLabel->Event-MidiEvent);
while (MidiEvent->EventData) { register unsigned long t = MidiEvent->event_time*Speed, current_time = timeGetTime(); if (!start_time) { start_time = current_time-t; }
 if ((signed long)(t += start_time-current_time) > 0) { Sleep(t); }

 if (MidiEvent->Rec) { RecEvent->event_time = timeGetTime(); RecEvent->EventData = MidiEvent->EventData; RecEvent = RecEvent->NextEvent; } *(MidiEvent->TrkInfo) = MidiEvent;

 switch (MidiEvent->FlwCtl | IRQ) {
  case 0x09: case 0x0b: case 0x0c: case 0x0d: case 0x11: case 0x13: case 0x14:
   while (LatestPendingO) { midiOutShortMsg(LatestPendingO->Event->midi_out, LatestPendingO->EventOffMsg); LatestPendingO->Event = NULL; LatestPendingO = LatestPendingO->Prev; }
   MidiEvent = Label0->Event;  IRQ = start_time = 0; Mute = Mute0 = Mute2; Mute3 = Mute2 = Mute1 = Mute11; Speed = Speed0; continue;
  case 0x02: case 0x03: case 0x05: case 0x0a: case 0x12:
   while (LatestPendingO) { midiOutShortMsg(LatestPendingO->Event->midi_out, LatestPendingO->EventOffMsg); LatestPendingO->Event = NULL; LatestPendingO = LatestPendingO->Prev; }
   MidiEvent = MidiEvent->JumpEvent; start_time = 0; Mute = Mute0 = Mute1; Mute3 = Mute2 = Mute1 = Mute11; Speed = Speed0; continue;
  case 0x04:
   while (LatestPendingO) { midiOutShortMsg(LatestPendingO->Event->midi_out, LatestPendingO->EventOffMsg); LatestPendingO->Event = NULL; LatestPendingO = LatestPendingO->Prev; }
   MidiEvent = Label1->Event;        start_time = 0; Mute = Mute0 = Mute3; Mute3 = Mute2 = Mute1 = Mute11; Speed = Speed0; continue;
  case 0x22: case 0x23: case 0x24: case 0x25: IRQ = 0; case 0x01: case 0x15: Mute = Mute0; }

 switch (MidiEvent->MsgCtl | Mute[MidiEvent->Track]) {
  case 0x4:           if (!(PendingO = MidiEvent->EventIdx)->Event) { midiOutShortMsg(MidiEvent->midi_out, MidiEvent->EventData); PendingO->Event = MidiEvent;
                      if (LatestPendingO) { LatestPendingO->Next = PendingO; } PendingO->Prev = LatestPendingO; (LatestPendingO = PendingO)->Next = NULL; } break;
  case 0x3: case 0xb: if ( (PendingO = MidiEvent->EventIdx)->Event) { midiOutShortMsg(MidiEvent->midi_out, MidiEvent->EventData); PendingO->Event = NULL;
                      if (PendingO->Prev) { PendingO->Prev->Next = PendingO->Next; }
                      if (PendingO->Next) { PendingO->Next->Prev = PendingO->Prev; } else { LatestPendingO = PendingO->Prev; }} break;
  case 0x2:           midiOutShortMsg(MidiEvent->midi_out, MidiEvent->EventData); break;
  case 0x1:           midi_message_header.lpData         = MidiEvent->data_buffer;
                      midi_message_header.dwBufferLength = MidiEvent->data_length;
                      midi_message_header.dwFlags        = 0;
                      midiOutPrepareHeader(MidiEvent->midi_out, &midi_message_header, sizeof(MIDIHDR));
                      sysex_message_pending = 1;
                      midiOutLongMsg(MidiEvent->midi_out, &midi_message_header, sizeof(MIDIHDR));
                      if (sysex_message_pending) { if (WaitForSingleObject(signalling_object, TimeOut)) { goto Exit2; }}
                      midiOutUnprepareHeader(MidiEvent->midi_out, &midi_message_header, sizeof(MIDIHDR)); }

 MidiEvent = MidiEvent->NextEvent; if (timeGetTime() > WatchDogTimeOut) { WatchDogTimeOut = timeGetTime()+TimeOut; if (Dead) { goto Exit0; } Dead = 1; }
 }
ExitVal |= 1; Exit2: ExitVal |= 2; Exit0: printf(" done. (%x)\n", ExitVal); for (i=0; i<(sizeof(Port2In)/sizeof(struct MidiIn)); i++) { if (Port2In[i].h) { midiInStop(Port2In[i].h); }} SetConsoleCtrlHandler(HandlerRoutine, FALSE); if (args[2] >= 0) { timeEndPeriod(args[2]); }

while (LatestPendingO) { midiOutShortMsg(LatestPendingO->Event->midi_out, LatestPendingO->EventOffMsg); LatestPendingO->Event = NULL; LatestPendingO = LatestPendingO->Prev; }
while (LatestPendingI) {                                                                                                              LatestPendingI = LatestPendingI->Prev; }

for (i=0; i<(sizeof(Port2Out)/sizeof(struct MidiOut)); i++) { if (Port2Out[i].h) { for (j=0; j<=0xf; j++) {
 midiOutShortMsg(Port2Out[i].h, 0x00007bb0 | j); //all notes off (GM)
 midiOutShortMsg(Port2Out[i].h, 0x000079b0 | j); //reset all controller (GM)
 } midiOutClose(Port2Out[i].h); }}

for (i=0; i<(sizeof(Port2In)/sizeof(struct MidiIn)); i++) { if (Port2In[i].h) { midiInReset(Port2In[i].h); midiInUnprepareHeader(Port2In[i].h, &midi_message_header0, sizeof(MIDIHDR)); midiInClose(Port2In[i].h); }} if (ExitVal < 3) { Sleep(TimeOut); } //goto start;

CloseHandle(signalling_object); saveMidiEventsToFile(args, MidiFile_getResolution(midi_file), Tempo0, TimeSig0, KeySig0, RecEvents, RecEvent, ExitVal, Label0);
free(Thrus); free(TrkInfo); free(Mutes); free(PendingEventsO); free(Labels); free(MidiEvents); free(args); MidiFile_free(midi_file);

if       ((ExitVal & 3) < 3)   { return(2); } //error
 else if ( ExitVal & 4     )   { return(3); } //CTRL+C
 else if (Label0 == ExitLabel) { return(4); } //exit
 else if (Label0 == LastLabel) { return(5); } //last

return(0); } //regular

//============================================================================//
