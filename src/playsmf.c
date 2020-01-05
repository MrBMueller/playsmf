//============================================================================//

#include <windows.h>
#include <midifile.h>

#define Msg0 "For further options, features, examples, updates and other information, check the github/sourceforge project repositories.\n"

#define KW0 "Label"
#define KW1 "Jump"
#define KW2 "Mute"
#define KW3 "Solo"

#define SetEntryLabel Var = Var1 = Var0 = (Label4 = Label3 = Label2 = Label1 = Label0 = EntryLabel)->Idx&-4096;
#define SetExitLabel  Var = Var1 = Var0 = (Label4 = Label3 = Label2 = Label1 = Label0 = ExitLabel )->Idx&-4096;

#define SetEntryMute Mute0 = Mute1 = Mute2 = Mute3 = Mute11 = MuteA = MuteB = EntryMute;
#define SetFirstMute Mute0 = Mute1 = Mute2 = Mute3 = Mute11 = MuteA = MuteB = FirstMute;

#define MyMacro1 \
 if       (Label0->Event == MidiEvents || Label0->Event == LastLabel->Event)                                                    { SetEntryMute i = -2;                      c = Mute0[0]; }\
  else if (Mute == EntryMute && (MidiEvent->Label->Event == EntryLabel->Event || MidiEvent->Label->Event == FirstLabel->Event)) { SetFirstMute i =  0;                      c = Mute0[0]; }\
  else                                                                                                                          {              i = (Mute-Mutes)/(TrkNum+1); c = Mute[ 0]; }\
 printf("%4.2f -> %3d %4x %4x %4x %2d %d %d %2d %d %02x => %6.2f (%6.2f %d/%d) -> %6.2f (%6.2f %d/%d)\n", (float)(RecEvent->event_time-LastTime)*1000*(1<<MidiEvent->TimeSigD)/((MidiEvent->Tempo<<2)*MidiEvent->TimeSigN)/Speed0, V1, MidiEvent->Label->Idx, Label4->Idx, Label1->Idx, (signed char)Label4->Ret, Label4->Now, Label4->ReT, i, c, IRQ,\
  (float)MidiEvent->event_time*1000*(1<<MidiEvent->TimeSigD)/((MidiEvent->Tempo<<2)*MidiEvent->TimeSigN), (float)60000000/MidiEvent->Tempo/Speed0, MidiEvent->TimeSigN, 1<<MidiEvent->TimeSigD, (float)Label4->Event->event_time*1000*(1<<Label4->Event->TimeSigD)/((Label4->Event->Tempo<<2)*Label4->Event->TimeSigN), (float)60000000/Label4->Event->Tempo/Speed0, Label4->Event->TimeSigN, 1<<Label4->Event->TimeSigD);\
 LastTime = RecEvent->event_time; if (MidiEvent->Label->Now) { SetEvent(signalling_object0); }

#define MyMacro0 \
 if (V0 < LabelNum && Labels[V0].Event) {\
  if (Labels[V0].Event != Label0->Event) { if (!(Label0 = &Labels[V0])->Ret) { if (Label0 != Label2 || MidiEvent->Label->Ret) {                                                                               Label4 = Label3 = Label2 = Label1 =                                                                          Label0; if (Label4->Event != MidiEvent->Label->Event) { IRQ = 0x10; MyMacro1 }}}\
                    else { Label1 = Label2; if (Label0->Ret&1) { if (!MidiEvent->Label->Ret) { Label3 = &Labels[Label1->Idx&-4096|MidiEvent->Label->Idx&0xfff]; } Label1 = Label3; } Var = Label1->Idx&-4096; Label4 = (i=0x1000+V0)<LabelNum && (V0^Label4->Idx)&0xf7f && Labels[i].Event && Labels[i].Ret ? &Labels[i] : Label0;                                                 IRQ = 0x08; MyMacro1  }}\
   else if (Label0->Ret) { Label1 = Label2; if (Label0->Ret&1) { if (!MidiEvent->Label->Ret) { Label3 = &Labels[Label1->Idx&-4096|MidiEvent->Label->Idx&0xfff]; } Label1 = Label3; } Var = Label1->Idx&-4096; Label4 =                                                                                                     Label0;                                                 IRQ = 0x08; MyMacro1   }\
   else                  { Label4 = Label3 = Label2 = Label1 = Label0 = &Labels[V0]; if (!IRQ) { IRQ = 0x18^Label0->ReT; } if (Label0->ReT) { MyMacro1 }}}

struct MidiEvent { unsigned long     event_time;
                   unsigned long     Track;
                   unsigned short    Out;
                   unsigned long     EventData, OffMsg;
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
                   struct MidiEvent *Event;
                   struct RecEvent  *NextEvent; };

struct PNoteI    { struct PNoteI    *Prev;
                   struct PNoteI    *Next;
                   struct PNoteI    **NoteI;
                   signed char       Key, Note, Vel; };

struct PNoteO    { struct PNoteO    *Prev;
                   struct PNoteO    *Next;
                   unsigned long     Cnt;
                   struct MidiEvent *Event; };

struct Label     {   signed long     Idx;
                   struct MidiEvent *Event;
                   unsigned char     Ret, Now, ReT; };

struct Chord     {   signed short    Type;
                     signed char     Root, Num; };

struct Thru      { struct MidiEvent  **Trk, *Pending;
                   unsigned long     Delay;
                   unsigned char     k, v0[128], v1[128]; };

struct Key       { unsigned char     Zone;
                   unsigned long     Val;
                   struct Thru       Thrus[8+1]; };

struct MidiBuf   { MIDIHDR           h;
                   unsigned char     b[512]; };

struct MidiIn    { unsigned long     s;
                   HMIDIIN           h;
                   struct MidiBuf    b[2]; };

struct MidiOut   { unsigned long     s;
                   HMIDIOUT          h; };

static HANDLE           signalling_object0, signalling_object1;
static struct Label    *Labels, *Label0, *Label1, *Label2, *Label3, *Label4, *FirstLabel, *LastLabel, *EntryLabel, *ExitLabel;
static struct Key       Keys[16][128], *Key0, *Key1;
static struct Thru     *Thru;
static struct PNoteI    PendingEventsI[128], *PendingI, *LatestPendingI, *PressedNotes[12];
static struct Chord     Chords[0xcccc+1];
static struct RecEvent  *RecEvents, *RecEvent;
static unsigned char    RootKey, IRQ, *Mutes, *Mute, *Mute0, *Mute1, *Mute2, *Mute3, *Mute11, *EntryMute, *FirstMute, *MuteA, *MuteB, Dead, ExitVal, PedalLeft, PedalMid, InPortOrder[256];
static unsigned long    V0, V1, c, i, i1, v, LastTime, LabelNum, TrkNum, Var, Var0, Var1;
static struct MidiEvent *MidiEvents, *MidiEvent, **TrkInfo, ***Thrus[16], *ThruE, *ThruE1;
static float            Speed0;
static signed char      InOfs;

//============================================================================//

static void CallMyMacro0() { RecEvent->event_time = timeGetTime(); V1 = -1; MyMacro0 }

//----------------------------------------------------------------------------//

static void CALLBACK MidiInProc(HMIDIIN hMidiIn, unsigned long wMsg, unsigned long dwInstance, unsigned long dwParam1, unsigned long dwParam2) { Dead = 0; switch (wMsg) { case MIM_DATA: switch (dwParam1 & 0xf0) {

case 0x90: RecEvent->event_time = timeGetTime(); V1 = dwParam1>>16; if (!V1) { V1 = 0x40; goto L0x80; } if ((V0 = (dwParam1>>8 & 0x7f)+InOfs) & -128) { return; } Key1 = &Keys[dwParam1 & 0xf][V0]; i = -1;
 while ((Thru = &Key1->Thrus[++i])->Trk) { if (*Thru->Trk && (*Thru->Trk)->Out & 0x1) { ThruE = Thru->Pending = *Thru->Trk; if (Thru->Delay) { Sleep(Thru->Delay); }
  midiOutShortMsg(ThruE->midi_out, Thru->v1[V1]<<16 | Thru->k<<8 | 0x90 | ThruE->Ch); }}
 switch (Key1->Zone) { case 1: if (!(PendingI = &PendingEventsI[V0])->Vel) { PendingI->Vel = V1;
  if (LatestPendingI) { LatestPendingI->Next = PendingI; } PendingI->Prev = LatestPendingI; (LatestPendingI = PendingI)->Next = NULL;
  if (!(*PendingI->NoteI)->Vel || PendingI->Key < (*PendingI->NoteI)->Key) { *(PendingI->NoteI) = PendingI; } c = v = 0;
  while (PendingI) { c = c<<4 | PendingI->Note; v += PendingI->Vel; PendingI = PendingI->Prev; } if (c <= 0xcccc && Chords[c].Type >= 0) { RootKey = PressedNotes[Chords[c].Root]->Key; PendingI = LatestPendingI; i = 0;
  while (PendingI) { if (PendingI->Key < RootKey) { i++; }          PendingI = PendingI->Prev; } V0 = Var | Chords[c].Type | i%Chords[c].Num<<4 | Chords[c].Root; V1 = v / Chords[c].Num; }} break;
  case  2: if ((i = Key1->Val | Label0->Idx & 0xfff) < LabelNum && !Labels[i].Ret) { if (Key1->Val == Var0) { Var0 = Var1; } else { Var1 = Var0; Var0 = Key1->Val; } Var = Var0; } else { Var = Key1->Val; }
           V0 = Var | Label2->Idx & 0xfff; if (MidiEvent->Label->Ret && V0 < LabelNum && !Labels[V0].Ret && (i = Var | Label1->Idx & 0xfff) < LabelNum) { Label1 = &Labels[i]; V0 = -1; } break;
  case  4: Mute[Key1->Val] ^= 0x08;                                                                                              V0 |= Var; break;
  case  8: if (!(Mute2 = (unsigned char*)Key1->Val)[-1]) { if (Mute2 == MuteA) { MuteA = MuteB; } else { MuteB = MuteA; MuteA = Mute2; } Mute0 = Mute1 = Mute2 = Mute3 = Mute11 = MuteA; }
            else                                         { Mute1 = Mute3 = Mute2; if (!Mute[-1]) { Mute0 = Mute11 = Mute; }}     V0 |= Var; break;
  case 16: SetEntryLabel IRQ = 0x20;                                                                                             V0 |= Var; break;
  default:                                                                                                                       V0 |= Var; }
  MyMacro0 RecEvent->EventData = dwParam1; RecEvent = RecEvent->NextEvent; return;

case 0x80: RecEvent->event_time = timeGetTime(); V1 = dwParam1>>16;                            L0x80:   if ((V0 = (dwParam1>>8 & 0x7f)+InOfs) & -128) { return; } Key0 = &Keys[dwParam1 & 0xf][V0]; i = -1;
 while (ThruE = (Thru = &Key0->Thrus[++i])->Pending) { midiOutShortMsg(ThruE->midi_out, Thru->v0[V1]<<16 | Thru->k<<8 | 0x80 | ThruE->Ch); }
 switch (Key0->Zone) { case 1: if ( (PendingI = &PendingEventsI[V0])->Vel) { PendingI->Vel =  0;
  if (PendingI->Prev) { PendingI->Prev->Next = PendingI->Next; }
  if (PendingI->Next) { PendingI->Next->Prev = PendingI->Prev; } else { LatestPendingI = PendingI->Prev; }
  if (!LatestPendingI && !(Label0->Idx & 0x80)) { V0 = Label0->Idx & 0xfff; }
  }} V0 |= Var | 0x80;
  MyMacro0 RecEvent->EventData = dwParam1; RecEvent = RecEvent->NextEvent; return;

case 0xa0: case 0xb0: case 0xc0: case 0xd0: case 0xe0: RecEvent->event_time = timeGetTime(); i = -1;
 switch (dwParam1 & 0x7ff0) { case 0x40b0: while (Thrus[c=dwParam1&0xf][++i]) { if (ThruE = *Thrus[c][i]) {    midiOutShortMsg(ThruE->midi_out, dwParam1 & 0xfffffff0 | ThruE->Ch); }} break;
                              case 0x42b0: while (Thrus[c=dwParam1&0xf][++i]) { if (ThruE = *Thrus[c][i]) {    midiOutShortMsg(ThruE->midi_out, dwParam1 & 0xfffffff0 | ThruE->Ch); }}
                                           PedalMid  = dwParam1>>16; if (PedalMid) { Speed0 = 1-(float)PedalLeft/127*.5; } else { Speed0 = 1+(float)PedalLeft/127; }                   break;
                              case 0x43b0: while (Thrus[c=dwParam1&0xf][++i]) { if (ThruE = *Thrus[c][i]) {    midiOutShortMsg(ThruE->midi_out, dwParam1 & 0xfffffff0 | ThruE->Ch); }}
                                           PedalLeft = dwParam1>>16; if (PedalMid) { Speed0 = 1-(float)PedalLeft/127*.5; } else { Speed0 = 1+(float)PedalLeft/127; }                   break;
                              default:     while (ThruE = Key1->Thrus[++i].Pending) {                          midiOutShortMsg(ThruE->midi_out, dwParam1 & 0xfffffff0 | ThruE->Ch);  }        }
 RecEvent->EventData = dwParam1; RecEvent = RecEvent->NextEvent; return;

default: switch (dwParam1 & 0xff) { case 0xfa: printf("start\n");                                       return;
                                    case 0xfb: printf("cont\n" ); SetEntryLabel IRQ = 0x20;             return;
                                    case 0xfc: printf("stop\n" ); V0 = FirstLabel->Idx; CallMyMacro0(); return;
                                    case 0xff: printf("reset\n"); V0 = EntryLabel->Idx; CallMyMacro0(); return; }

} return; //switch dwParam1 // MIM_DATA fallthru

case MIM_LONGDATA: V0 = timeGetTime(); i = -1;
 while (++i < ((MIDIHDR*)dwParam1)->dwBytesRecorded) { RecEvent->event_time = V0; RecEvent->EventData = (*(((MIDIHDR*)dwParam1)->lpData+i)&0xff)<<8 | 0xf0; RecEvent = RecEvent->NextEvent; }
 if (((MIDIHDR*)dwParam1)->dwBytesRecorded) { midiInAddBuffer(hMidiIn, (MIDIHDR*)dwParam1, sizeof(MIDIHDR)); } return; // MIM_LONGDATA
case MIM_OPEN: case MIM_CLOSE: return; // MIM_OPEN|MIM_CLOSE

} printf("%08x %08x %08x %08x %08x\n", hMidiIn, wMsg, dwInstance, dwParam1, dwParam2); } //switch wMsg // CALLBACK fallthru

//----------------------------------------------------------------------------//

static void CALLBACK MidiInProc1(HMIDIIN hMidiIn, unsigned long wMsg, unsigned long dwInstance, unsigned long dwParam1, unsigned long dwParam2) { midiInGetID(hMidiIn, &i1); i1 = InPortOrder[i1]; switch (wMsg) {
case MIM_DATA: if ((dwParam1&0xff) < 0xf0) { if ((i1 += dwParam1&0xf) < TrkNum && (ThruE1 = TrkInfo[i1]) && ThruE1->Out & 0x1) { midiOutShortMsg(ThruE1->midi_out, dwParam1 & 0xfffffff0 | ThruE1->Ch); }} else { printf("%x\n", dwParam1); } return;
case MIM_LONGDATA: if (((MIDIHDR*)dwParam1)->dwBytesRecorded) {
 i1 = ((MIDIHDR*)dwParam1)->dwBufferLength; ((MIDIHDR*)dwParam1)->dwBufferLength = ((MIDIHDR*)dwParam1)->dwBytesRecorded;
 midiOutLongMsg(ThruE1->midi_out, (MIDIHDR*)dwParam1, sizeof(MIDIHDR)); ((MIDIHDR*)dwParam1)->dwBufferLength = i1;
 midiInAddBuffer(hMidiIn, (MIDIHDR*)dwParam1, sizeof(MIDIHDR)); } return; // MIM_LONGDATA
case MIM_OPEN: case MIM_CLOSE: return; // MIM_OPEN|MIM_CLOSE

} printf("%08x %08x %08x %08x %08x\n", hMidiIn, wMsg, dwInstance, dwParam1, dwParam2); } //switch wMsg // CALLBACK fallthru

//----------------------------------------------------------------------------//

static void CALLBACK MidiOutProc(HMIDIOUT hmo, unsigned long wMsg, unsigned long dwInstance, unsigned long dwParam1, unsigned long dwParam2) { SetEvent(signalling_object1); }

//----------------------------------------------------------------------------//

static BOOL WINAPI HandlerRoutine(DWORD dwCtrlType) { if (dwCtrlType) { V0 = EntryLabel->Idx; } else { V0 = ExitLabel->Idx; ExitVal |= 4; } CallMyMacro0(); SetEvent(signalling_object0); SetEvent(signalling_object1); return(TRUE); }

//----------------------------------------------------------------------------//

static void ExpandLabels(struct Label *Labels) { unsigned long LabelNum = _msize(Labels)/sizeof(struct Label), i, j, k, l;

for (i=0; i<LabelNum; i++) { j = (i>>8)&0xf; k = (i>>4)&0x3; l = i&0xf;
 if ((j >= 1) && (j <= 4) && (l >= 1) && (l <= 0xb) && (!Labels[i].Event) && (Labels[i&(-1^0xf)].Event)) { Labels[i].Event = Labels[i&(-1^0xf)].Event; }
 }

for (i=0; i<LabelNum; i++) { j = (i>>8)&0xf; k = (i>>4)&0x3; l = i&0xf;
 if       ((j >= 1) && (j <= 1) && (k >= 1) && (k <= 1) && (!Labels[i].Event) && (Labels[i&(-1^0x30)].Event)) { Labels[i].Event = Labels[i&(-1^0x30)].Event; }
  else if ((j >= 2) && (j <= 4) && (k >= 1) && (k <= 2) && (!Labels[i].Event) && (Labels[i&(-1^0x30)].Event)) { Labels[i].Event = Labels[i&(-1^0x30)].Event; }
 }

for (i=0; i<LabelNum ; i++) { if ((!Labels[i].Event) && (Labels[i&0xfff].Event)) { Labels[i].Event = Labels[i&0xfff].Event; }}

return; }

//----------------------------------------------------------------------------//

static void AlignLabels(struct Label *Labels) { unsigned long LabelNum = _msize(Labels)/sizeof(struct Label), i, j;

for (i=0; i<LabelNum; i++) { if (Labels[i].Event) { unsigned char a = 0, b = 0, c = 0;
 for (j=0; j<LabelNum; j++) { if (Labels[i].Event == Labels[j].Event) { a |= Labels[j].Ret; b |= Labels[j].Now; c |= Labels[j].ReT; }}
 for (j=0; j<LabelNum; j++) { if (Labels[i].Event == Labels[j].Event) { Labels[j].Ret = a;  Labels[j].Now = b;  Labels[j].ReT = c;  }}
 }}

return; }

//----------------------------------------------------------------------------//

static void saveMidiEventsToFile(signed long *args, unsigned short PPQ, unsigned long Tempo, unsigned long TimeSig, unsigned long KeySig, struct RecEvent *RecEvents, struct RecEvent *RecEvent, unsigned char ExitVal, struct Label *Label0) {
SYSTEMTIME    current_time;
unsigned char filename[1024], tempo[] = {(Tempo>>16)&0xff, (Tempo>>8)&0xff, (Tempo>>0)&0xff}, timeSig[] = {(TimeSig>>24)&0xff, (TimeSig>>16)&0xff, (TimeSig>>8)&0xff, (TimeSig>>0)&0x7f}, keySig[] = {(KeySig>>8)&0xff, (KeySig>>0)&0xff};
unsigned long PPQc = PPQ*1000;
float         c = (float)PPQc/(float)(Tempo&0xffffff);

MidiFile_t midi_file = MidiFile_new(1, MIDI_FILE_DIVISION_TYPE_PPQ, PPQ);

MidiFileTrack_t track0 = MidiFile_createTrack(midi_file), track1 = MidiFile_createTrack(midi_file);

MidiFileTrack_createMetaEvent(track0, 0, 0x02, strlen(GetCommandLineA()), GetCommandLineA());
MidiFileTrack_createMetaEvent(track0, 0, 0x51, 3, tempo);
if (!(TimeSig & 0x00000080)) { MidiFileTrack_createMetaEvent(track0, 0, 0x58, 4, timeSig); }
if (!(KeySig  & 0x00010000)) { MidiFileTrack_createMetaEvent(track0, 0, 0x59, 2, keySig ); }

if (!RecEvent->EventData && !RecEvent->Event) { unsigned long i = -1, data_length = 0, t0; unsigned char data_buffer[1024];
 while (!(ExitVal&4) && (RecEvent-- > RecEvents) && (((RecEvent->EventData & 0xe0) == 0x80) && (((RecEvent->EventData>>8) & 0x7f) == Label0->Idx) || RecEvent->Event)) { RecEvent->EventData = 0x00000000; }
 while (RecEvents[++i].EventData || RecEvents[i].Event) { unsigned long t = (RecEvents[i].event_time-RecEvents[0].event_time)*c; if (RecEvents[i].Event) { RecEvents[i].EventData = RecEvents[i].Event->EventData; }
  switch (RecEvents[i].EventData & 0xf0) {
  case 0x80: { MidiFileTrack_createNoteOffEvent(        track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f , (RecEvents[i].EventData>>16)&0x7f); break; }
  case 0x90: { MidiFileTrack_createNoteOnEvent(         track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f , (RecEvents[i].EventData>>16)&0x7f); break; }
  case 0xa0: { MidiFileTrack_createKeyPressureEvent(    track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f , (RecEvents[i].EventData>>16)&0x7f); break; }
  case 0xb0: { MidiFileTrack_createControlChangeEvent(  track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f , (RecEvents[i].EventData>>16)&0x7f); break; }
  case 0xc0: { MidiFileTrack_createProgramChangeEvent(  track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f                                    ); break; }
  case 0xd0: { MidiFileTrack_createChannelPressureEvent(track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>8)&0x007f                                    ); break; }
  case 0xe0: { MidiFileTrack_createPitchWheelEvent(     track1, t, RecEvents[i].EventData&0xf, (RecEvents[i].EventData>>9)&0x3f80 | (RecEvents[i].EventData>> 8)&0x7f); break; }
  default: { switch (RecEvents[i].EventData & 0xff) {
               case 0xf0: { if (RecEvents[i].Event) { MidiFileTrack_createSysexEvent(track1, t, RecEvents[i].Event->data_length, RecEvents[i].Event->data_buffer); break; }
                            if ((RecEvents[i].EventData>>8) == 0xf0) { if (data_length) { MidiFileTrack_createSysexEvent(track1, t0, data_length, data_buffer); } data_length = 0; }
                            if (!data_length) { t0 = t; } if (data_length < sizeof(data_buffer)) { data_buffer[data_length++] = RecEvents[i].EventData>>8; }
                            if ((RecEvents[i].EventData>>8) == 0xf7) {                    MidiFileTrack_createSysexEvent(track1, t0, data_length, data_buffer);   data_length = 0; } break; }
			   case 0xff: { MidiFileTrack_createMetaEvent(track1, t, RecEvents[i].EventData>>8, RecEvents[i].Event->data_length, RecEvents[i].Event->data_buffer); break; }} break; }
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
static MIDIHDR           midi_message_header;
static struct MidiIn     Port2In[  256];
static struct MidiOut    Port2Out[ 256];
static unsigned char     Port2Port[256], FlwMsk;
static MidiFileEvent_t   midi_file_event;
static unsigned long     start_time, WatchDogTimeOut, MutesNum, tick, tempo_event_tick, MutesInv, MutesRet, MutesInv1, MutesInv2, Tempo, TimeSig, KeySig, Tempo0, TimeSig0, KeySig0;
static   signed long     i, j, k, l, *args, TimeOut, DefIDev, DefODev;
static struct PNoteO     *PendingEventsO, *PendingO, *LatestPendingO;
static float             Speed, tempo_event_time;
static unsigned char     Permutations0[ 1][1] = {{0}};
static unsigned char     Permutations1[ 2][2] = {{0,1},{1,0}};
static unsigned char     Permutations2[ 6][3] = {{0,1,2},{0,2,1},{1,0,2},{1,2,0},{2,0,1},{2,1,0}};
static unsigned char     Permutations3[24][4] = {{0,1,2,3},{0,1,3,2},{0,2,1,3},{0,2,3,1},{0,3,1,2},{0,3,2,1},
                                                 {1,0,2,3},{1,0,3,2},{1,2,0,3},{1,2,3,0},{1,3,0,2},{1,3,2,0},
                                                 {2,0,1,3},{2,0,3,1},{2,1,0,3},{2,1,3,0},{2,3,0,1},{2,3,1,0},
                                                 {3,0,1,2},{3,0,2,1},{3,1,0,2},{3,1,2,0},{3,2,0,1},{3,2,1,0}};

static unsigned short    Intervals0[][2] = {{0x120, 0}};

static unsigned short    Intervals1[][3] = {{0x120, 0, 0},
                                            {0x100, 0, 7}};

static unsigned short    Intervals2[][4] = {{0x120, 0, 0, 0},
                                            {0x100, 0, 0, 7}, {0x100, 0, 7, 7},
                                            {0x200, 0, 4, 7},
                                            {0x300, 0, 3, 7},
                                            {0x400, 0, 2, 7}};

static unsigned short    Intervals3[][5] = {{0x120, 0, 0, 0,  0},
                                            {0x100, 0, 0, 0,  7}, {0x100, 0, 0, 7,  7}, {0x100, 0, 7, 7,  7},
                                            {0x200, 0, 4, 7,  0}, {0x200, 0, 4, 7,  4}, {0x200, 0, 4, 7,  7},
                                            {0x300, 0, 3, 7,  0}, {0x300, 0, 3, 7,  3}, {0x300, 0, 3, 7,  7},
                                            {0x200, 0, 4, 7, 11},
                                            {0x300, 0, 3, 7, 10},
                                            {0x200, 0, 4, 7, 10}};

static signed long       DefArgs[] = {0, 0, -1, 0, 0, -1, -1, 0, 0x0ff, 0x00008000, 21, 22, 36, 59};

MidiFile_t midi_file = NULL; if (argc < 2) { printf("Usage: %s <filename.mid>\n" Msg0, argv[0]); return(1); } if (!(midi_file = MidiFile_load(argv[1]))) { printf("Error: Cannot open \"%s\".\n", argv[1]); return(1); }

i = j = sizeof(DefArgs)/sizeof(signed long); if (argc > i) { i = argc; } args = malloc(i*sizeof(signed long)); for (i=0; i<j; i++) { args[i] = DefArgs[i]; } for (i=2; i<argc; i++) { args[i] = strtol(argv[i], NULL, 0); }

for (i=0; i<(sizeof(Chords)/sizeof(struct Chord)); i++) { Chords[i].Type = -1; Chords[i].Num = 0; j = i; while (j) { if (j&0xf) { Chords[i].Num++; } j >>= 4; }}

for (i=0; i<(sizeof(Port2Port)/sizeof(unsigned char)); i++) { Port2Port[i] = i; }

for (i=0x0; i<=0xf; i++) {
 for (j=0x00; j <= 0x7f; j++) { Keys[i][j].Zone = 0;
  for (k=0; k<(sizeof(Keys[i][j].Thrus)/sizeof(struct Thru)); k++) { Keys[i][j].Thrus[k].Trk = NULL; }
  }
 }

for (i=0; i<=11; i++) { unsigned char m;
 for (j=0; j<= 0; j++) { for (m=0; m<sizeof(Intervals0)/2/sizeof(unsigned short); m++) { l = 0; for (k=0; k<=0; k++) { l = (l<<4) + (i+Intervals0[m][Permutations0[j][k]+1])%12 + 1; } Chords[l].Type = Intervals0[m][0]; Chords[l].Root = i; }}
 for (j=0; j<= 1; j++) { for (m=0; m<sizeof(Intervals1)/3/sizeof(unsigned short); m++) { l = 0; for (k=0; k<=1; k++) { l = (l<<4) + (i+Intervals1[m][Permutations1[j][k]+1])%12 + 1; } Chords[l].Type = Intervals1[m][0]; Chords[l].Root = i; }}
 for (j=0; j<= 5; j++) { for (m=0; m<sizeof(Intervals2)/4/sizeof(unsigned short); m++) { l = 0; for (k=0; k<=2; k++) { l = (l<<4) + (i+Intervals2[m][Permutations2[j][k]+1])%12 + 1; } Chords[l].Type = Intervals2[m][0]; Chords[l].Root = i; }}
 for (j=0; j<=23; j++) { for (m=0; m<sizeof(Intervals3)/5/sizeof(unsigned short); m++) { l = 0; for (k=0; k<=3; k++) { l = (l<<4) + (i+Intervals3[m][Permutations3[j][k]+1])%12 + 1; } Chords[l].Type = Intervals3[m][0]; Chords[l].Root = i; }}
 }

i = l = 0; j = k = -1; MutesNum = MutesInv = MutesRet = 0;
for (midi_file_event = MidiFile_getFirstEvent(midi_file); midi_file_event; midi_file_event = MidiFileEvent_getNextEventInFile(midi_file_event)) { i++;
 if (MidiFileTrack_getNumber(MidiFileEvent_getTrack(midi_file_event)) > k) { k = MidiFileTrack_getNumber(MidiFileEvent_getTrack(midi_file_event)); }
 if (MidiFileEvent_getType(midi_file_event) == MIDI_FILE_EVENT_TYPE_META) {
  if (MidiFileMetaEvent_getNumber(midi_file_event) == 0x03) { unsigned char *p0 = MidiFileMetaEvent_getData(midi_file_event), *p1; p1 = p0; //trkname
   while (p0 = strstr(p0, KW2)) { signed long v = strtol(p0+sizeof(KW2)-1, &p0, 0); unsigned long t = 0; if (v == -1) { v = 0; } if (p0 == strstr(p0, "r")) { MutesRet |= v; }                while (v) { t++; v >>= 1; } if (t > MutesNum) { MutesNum = t; }}
   while (p1 = strstr(p1, KW3)) { signed long v = strtol(p1+sizeof(KW3)-1, &p1, 0); unsigned long t = 0; if (v == -1) { v = 0; } if (p1 == strstr(p1, "r")) { MutesRet |= v; } MutesInv |= v; while (v) { t++; v >>= 1; } if (t > MutesNum) { MutesNum = t; }}
   }
  if (MidiFileMetaEvent_getNumber(midi_file_event) == 0x06) { unsigned char *p0 = MidiFileMetaEvent_getData(midi_file_event), *p1; p1 = p0; //marker
   while (p0 = strstr(p0, KW0)) { signed long v = strtol(p0+sizeof(KW0)-1, &p0, 0); if (v < 0) { v = l & ~0xfff | v & 0xfff; } l = v; if (v > j) { j = v; }}
   while (p1 = strstr(p1, KW1)) { signed long v = strtol(p1+sizeof(KW1)-1, &p1, 0);                                                   if (v > j) { j = v; }}
   }
  if (MidiFileMetaEvent_getNumber(midi_file_event) == 0x7f) { unsigned long L = MidiFileMetaEvent_getDataLength(midi_file_event); unsigned char *D = MidiFileMetaEvent_getData(midi_file_event);
   if ((L >= 4) && (D[0] == 0x00) && (D[1] == 0xab) && (D[2] == 0xcd)) {
    if ((D[3] == 0x00) && (L >= 8)) { signed long A = D[4]<<24 | D[5]<<16 | D[6]<<8 | D[7], t; if (A < 0) { A += _msize(args)/sizeof(signed long)+1; }
	 if ((A+((L-8)>>2))*sizeof(signed long) > _msize(args)) { args = realloc(args, (A+((L-8)>>2))*sizeof(signed long)); }
	 for (t=8; (t+3)<L; t += 4) { args[A++] = D[t+0]<<24 | D[t+1]<<16 | D[t+2]<<8 | D[t+3]; }
	 }
    }
   }
  }
 }

if (args[10] > j) { j = args[10]; } if (args[11] > j) { j = args[11]; } MutesNum += 2; MutesInv1 = (-1^MutesInv) & ((1<<(MutesNum-2))-1); MutesInv2 = (1<<(MutesNum-1)) | MutesInv; InOfs = args[7];

MidiEvents = malloc((i+1)*sizeof(struct MidiEvent)); Labels = malloc((j+1)*sizeof(struct Label)); PendingEventsO = malloc(1*(k+1)*16*128*sizeof(struct PNoteO)); Mutes = malloc(MutesNum*(k+2)*sizeof(unsigned char)); TrkInfo = malloc((k+1)*sizeof(struct MidiEvent*)); Thrus[0] = malloc(16*(k+2)*sizeof(struct MidiEvent**));

RecEvents = malloc((args[8]==0x0ff?1:1024*1024)*sizeof(struct RecEvent)); LabelNum = _msize(Labels)/sizeof(struct Label); TrkNum = _msize(TrkInfo)/sizeof(struct MidiEvent*);

EntryLabel = &Labels[args[10]]; ExitLabel = &Labels[args[11]];

for (i=0x0; i<=0xf; i++) { signed long C = args[6], Ck = args[12], Mk = args[13]+1, K2 = Ck-MutesNum, K1 = K2-1, K0 = K1; if (LabelNum>>12) { K0 -= ((LabelNum-1)>>12)+1; } if (Mk-1 < 0) { Mk = Ck+abs(Mk-1); if (Mk > 128) { Mk = 128; }}
 for (j = K0; j < K1; j++) { Keys[i][j].Zone |= 2; Keys[i][j].Val = (K1-j-1)<<12; }
 for (j = K1; j < K2; j++) { Keys[i][j].Zone |= 4; Keys[i][j].Val = K2-j-1; }
 for (j = K2; j < Ck; j++) { Keys[i][j].Zone |= 8; if (j > (Ck-3)) { Keys[i][j].Val = (unsigned long)&Mutes[(j-K2)*(TrkNum+1)+1]; } else { Keys[i][j].Val = (unsigned long)&Mutes[(Ck-3-j)*(TrkNum+1)+1]; }}
 for (j = Ck; j < Mk; j++) { Keys[i][j].Zone |= 1; }
 for (j = args[10]+1; j <= args[10]+1; j++) { Keys[i][j].Zone |= 16; }

 k = 12; Thrus[i] = &Thrus[0][i*(TrkNum+1)];
 while (((k+6) < _msize(args)/sizeof(signed long)) && (abs(args[k+6]) < 0x10000)) { signed long K0 = args[k++], K1 = args[k++]+1, T = args[k++], Delay = args[k++], K = args[k++], V1 = args[k++], V0 = args[k++];
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

timeGetDevCaps(&time_caps, sizeof(TIMECAPS)); signalling_object0 = CreateEvent(NULL, FALSE, FALSE, NULL); signalling_object1 = CreateEvent(NULL, FALSE, FALSE, NULL);

start:
printf("[%d:%d] [%d:%d] %d %d %d %d %x %x %x %x %4.2f %4.2f\n", time_caps.wPeriodMin, time_caps.wPeriodMax, midiInGetNumDevs()-1, midiOutGetNumDevs()-1, MidiFile_getResolution(midi_file), MidiFile_getFileFormat(midi_file), TrkNum, _msize(MidiEvents)/sizeof(struct MidiEvent)-1, LabelNum-1, MutesNum-1, MutesInv2, MutesRet, (float)_msize(MidiEvents)/(1024*1024), (float)_msize(Labels)/(1024*1024));

DefODev = args[3]; if (DefODev < 0) { DefODev += midiOutGetNumDevs(); } if (DefODev < 0 || DefODev >= midiOutGetNumDevs()) { DefODev = 0; }
DefIDev = args[4]; if (DefIDev < 0) { DefIDev += midiInGetNumDevs();  } if (DefIDev < 0 || DefIDev >= midiInGetNumDevs() ) { DefIDev = 0; }

for (i=12; i<_msize(args)/sizeof(signed long); i++) {
 if ((args[i]>>16) == 1) { Port2Port[(args[i]>>8)&0xff] =                       args[i]&0xff; if (Port2Port[(args[i]>>8)&0xff] >= midiOutGetNumDevs()) { Port2Port[(args[i]>>8)&0xff] = DefODev; }}
 if ((args[i]>>16) == 3) { Port2Port[(args[i]>>8)&0xff] = midiOutGetNumDevs()-1-args[i]&0xff; if (Port2Port[(args[i]>>8)&0xff] >= midiOutGetNumDevs()) { Port2Port[(args[i]>>8)&0xff] = DefODev; }}
 }

for (i=0; i<(sizeof(Port2In       )/sizeof(struct MidiIn     )); i++) { Port2In[i].s  = -1; Port2In[i].h  = NULL; for (j=0; j<sizeof(Port2In[i].b)/sizeof(struct MidiBuf); j++) { Port2In[i].b[j].h.lpData = Port2In[i].b[j].b; Port2In[i].b[j].h.dwBufferLength = sizeof(Port2In[i].b[j].b); Port2In[i].b[j].h.dwFlags = 0; }}
for (i=0; i<(sizeof(Port2Out      )/sizeof(struct MidiOut    )); i++) { Port2Out[i].s = -1; Port2Out[i].h = NULL; }
for (i=0; i<(_msize(TrkInfo       )/sizeof(struct MidiEvent* )); i++) { TrkInfo[i] = (struct MidiEvent*)(DefODev << 8); }
for (i=0; i<(sizeof(PendingEventsI)/sizeof(struct PNoteI     )); i++) { PendingEventsI[i].Prev = PendingEventsI[i].Next = NULL; PendingEventsI[i].Vel = 0; PendingEventsI[i].Key = i; PendingEventsI[i].Note = (i%12)+1; PendingEventsI[i].NoteI = &PressedNotes[i%12]; } LatestPendingI = NULL;
for (i=0; i<(_msize(PendingEventsO)/sizeof(struct PNoteO     )); i++) { PendingEventsO[i].Prev = PendingEventsO[i].Next = NULL; PendingEventsO[i].Cnt = 0; PendingEventsO[i].Event = NULL;                                                                              } LatestPendingO = NULL;
for (i=0; i<(_msize(Mutes         )/sizeof(unsigned char     )); i++) { Mutes[i] = 0; }
for (i=0; i<(_msize(RecEvents     )/sizeof(struct RecEvent   )); i++) { RecEvents[i].NextEvent = &RecEvents[i+1]; RecEvents[i].EventData = 0; RecEvents[i].Event = NULL; } RecEvent = RecEvents[i-1].NextEvent = &RecEvents[0];
for (i=0; i<(_msize(Thrus[0]      )/sizeof(struct MidiEvent**)); i++) { Thrus[0][i] = NULL; }
for (i=0; i<(sizeof(PressedNotes  )/sizeof(struct PNoteI*    )); i++) { PressedNotes[i] = &PendingEventsI[0]; }
for (i=0; i<(_msize(MidiEvents    )/sizeof(struct MidiEvent  )); i++) { MidiEvents[i].NextEvent = &MidiEvents[i+1]; MidiEvents[i].FlwCtl = MidiEvents[i].MsgCtl = MidiEvents[i].Rec = 0; }
for (i=0; i<(_msize(Labels        )/sizeof(struct Label      )); i++) { Labels[i].Idx = i; Labels[i].Event = NULL; Labels[i].ReT = Labels[i].Now = Labels[i].Ret = 0; }

for (i=0x0; i<=0xf; i++) {
 for (j=0x00; j <= 0x7f; j++) {
  k = -1; while (Keys[i][j].Thrus[++k].Trk) { signed long L = -1; Keys[i][j].Thrus[k].Pending = NULL; while ((Thrus[i][++L]) && (Thrus[i][L] != Keys[i][j].Thrus[k].Trk)) {} Thrus[i][L] = Keys[i][j].Thrus[k].Trk; }
  }
 }

k = DefIDev;
Port2In[k].s = midiInOpen(&Port2In[k].h, k, (unsigned long)MidiInProc , (unsigned long)NULL, CALLBACK_FUNCTION | MIDI_IO_STATUS);
strcpy(midi_i_caps.szPname, ""); midiInGetDevCaps(k, &midi_i_caps, sizeof(midi_i_caps)); printf("i%2d %x '%s'\n", k, Port2In[k].s, midi_i_caps.szPname);
for (j=0; j<sizeof(Port2In[k].b)/sizeof(struct MidiBuf); j++) { midiInPrepareHeader(Port2In[k].h, &Port2In[k].b[j].h, sizeof(MIDIHDR)); midiInAddBuffer(Port2In[k].h, &Port2In[k].b[j].h, sizeof(MIDIHDR)); }

for (i=12; i<_msize(args)/sizeof(signed long); i++) { if ((((args[i]>>16) == 2) && (!Port2In[k = args[i]&0xff].h) || ((args[i]>>16) == 4) && (!Port2In[k = midiInGetNumDevs()-1-args[i]&0xff].h)) && (k < midiInGetNumDevs())) { InPortOrder[k] = (args[i]>>8)&0xff;
Port2In[k].s = midiInOpen(&Port2In[k].h, k, (unsigned long)MidiInProc1, (unsigned long)NULL, CALLBACK_FUNCTION | MIDI_IO_STATUS);
strcpy(midi_i_caps.szPname, ""); midiInGetDevCaps(k, &midi_i_caps, sizeof(midi_i_caps)); printf("i%2d %x '%s'\n", k, Port2In[k].s, midi_i_caps.szPname);
for (j=0; j<sizeof(Port2In[k].b)/sizeof(struct MidiBuf); j++) { midiInPrepareHeader(Port2In[k].h, &Port2In[k].b[j].h, sizeof(MIDIHDR)); midiInAddBuffer(Port2In[k].h, &Port2In[k].b[j].h, sizeof(MIDIHDR)); }}}

i = j = tempo_event_tick = tick = 0; tempo_event_time = 0; FirstLabel = LastLabel = NULL; Tempo = Tempo0 = 0x0107a120; TimeSig = TimeSig0 = 0x04021888; KeySig = KeySig0 = 0x00010000;
for (midi_file_event = MidiFile_getFirstEvent(midi_file); midi_file_event; midi_file_event = MidiFileEvent_getNextEventInFile(midi_file_event)) {
 if (MidiFileEvent_getTick(midi_file_event) != tick) { j = i; tick = MidiFileEvent_getTick(midi_file_event); } while ((i > j) && !MidiEvents[i-1].MsgCtl) { i--; }
 MidiEvents[i].event_time = (unsigned long)(tempo_event_time + (float)(tick - tempo_event_tick) * (float)(Tempo & 0x00ffffff) / (MidiFile_getResolution(midi_file)*1000));
 MidiEvents[i].Tempo      = Tempo & 0x00ffffff;
 MidiEvents[i].TimeSigN   = TimeSig >> 24;
 MidiEvents[i].TimeSigD   = TimeSig >> 16;
 MidiEvents[i].Label      = EntryLabel; if (LastLabel) { MidiEvents[i].Label = LastLabel; }
 MidiEvents[i].Track      = MidiFileTrack_getNumber(MidiFileEvent_getTrack(midi_file_event));
 MidiEvents[i].Out        = (unsigned long)TrkInfo[MidiEvents[i].Track];
 if (Port2Out[MidiEvents[i].Out>>8].s == -1) { Port2Out[MidiEvents[i].Out>>8].s = midiOutOpen(&Port2Out[MidiEvents[i].Out>>8].h, MidiEvents[i].Out>>8, (unsigned long)MidiOutProc, (unsigned long)NULL, CALLBACK_FUNCTION);
  strcpy(midi_o_caps.szPname, ""); midiOutGetDevCaps(MidiEvents[i].Out>>8, &midi_o_caps, sizeof(midi_o_caps)); printf("o%2d %x '%s'\n", MidiEvents[i].Out>>8, Port2Out[MidiEvents[i].Out>>8].s, midi_o_caps.szPname);
  }
 if (!Port2Out[MidiEvents[i].Out>>8].h) { MidiEvents[i].Out = DefODev << 8; }
 MidiEvents[i].midi_out = Port2Out[MidiEvents[i].Out>>8].h;
 if (MidiFileEvent_getType(midi_file_event) == MIDI_FILE_EVENT_TYPE_META) {
  MidiEvents[i].EventData   = (MidiFileMetaEvent_getNumber(midi_file_event) << 8) | 0x000000ff;
  MidiEvents[i].data_length = MidiFileMetaEvent_getDataLength(midi_file_event);
  MidiEvents[i].data_buffer = MidiFileMetaEvent_getData(midi_file_event);
  if ((MidiEvents[i].EventData & (args[9]>>16)) == (args[9] & 0x7fff)) { MidiEvents[j].FlwCtl |= 1; }
  if (MidiEvents[i].EventData == 0x000051ff) { tempo_event_time += (float)(tick - tempo_event_tick) * (float)(Tempo & 0x00ffffff) / (MidiFile_getResolution(midi_file)*1000); tempo_event_tick = tick;
                                               Tempo   =                                      (MidiEvents[i].data_buffer[0]<<16) | (MidiEvents[i].data_buffer[1]<<8) | (MidiEvents[i].data_buffer[2]<<0); if (Tempo0   & 0x01000000) { Tempo0   = Tempo;   }}
  if (MidiEvents[i].EventData == 0x000058ff) { TimeSig = (MidiEvents[i].data_buffer[0]<<24) | (MidiEvents[i].data_buffer[1]<<16) | (MidiEvents[i].data_buffer[2]<<8) | (MidiEvents[i].data_buffer[3]<<0); if (TimeSig0 & 0x00000080) { TimeSig0 = TimeSig; }}
  if (MidiEvents[i].EventData == 0x000059ff) { KeySig  =                                                                           (MidiEvents[i].data_buffer[0]<<8) | (MidiEvents[i].data_buffer[1]<<0); if (KeySig0  & 0x00010000) { KeySig0  = KeySig;  }}
  if (MidiEvents[i].EventData == 0x000021ff) { TrkInfo[MidiEvents[i].Track] = (struct MidiEvent*)((unsigned long)TrkInfo[MidiEvents[i].Track] & 0xffff00ff | (Port2Port[MidiEvents[i].data_buffer[0]] << 8) | 0x2); } //port
  if (MidiEvents[i].EventData == 0x000003ff) { unsigned char *p0 = MidiEvents[i].data_buffer, *p1; p1 = p0; //trkname
   while (p0 = strstr(p0, KW2)) { signed long v = strtol(p0+sizeof(KW2)-1, &p0, 0); unsigned long t = 0; if (!v) { v = MutesInv;  } if (v == -1) { v = MutesInv1; } while (v) { if (v&1) { Mutes[t*(TrkNum+1)+1+MidiEvents[i].Track] ^= 0x08; } t++; v >>= 1; }}
   while (p1 = strstr(p1, KW3)) { signed long v = strtol(p1+sizeof(KW3)-1, &p1, 0); unsigned long t = 0; if (!v) { v = MutesInv1; } if (v == -1) { v = MutesInv;  } while (v) { if (v&1) { Mutes[t*(TrkNum+1)+1+MidiEvents[i].Track] ^= 0x08; } t++; v >>= 1; }}
   }
  if (MidiEvents[i].EventData == 0x000006ff) { unsigned char *p0 = MidiEvents[i].data_buffer, *p1; p1 = p0; //marker
   while (p0 = strstr(p0, KW0)) { signed long v = strtol(p0+sizeof(KW0)-1, &p0, 0); MidiEvents[j].FlwCtl |= 4; if ((v&0xfff) == 0xfff || v < 0) { v = (LastLabel?LastLabel->Idx:0) & ~0xfff | v & 0xfff; while (Labels[v].Event) { v--; }} (LastLabel = &Labels[v])->Event = &MidiEvents[j]; if (!FirstLabel) { FirstLabel = LastLabel; } for (k=j; k<=i; k++) { MidiEvents[k].Label = LastLabel; } if (p0 == strstr(p0, "i")) { LastLabel->Now = 1; p0++; } if (p0 == strstr(p0, "r")) { LastLabel->ReT = 8; }}
   while (p1 = strstr(p1, KW1)) { signed long v = strtol(p1+sizeof(KW1)-1, &p1, 0); MidiEvents[j].FlwCtl |= 2; if (v == -3) { v++; } MidiEvents[j].JumpEvent = (struct MidiEvent*)v; if (p1 == strstr(p1, ">|>")) { MidiEvents[j].FlwCtl |= 0x40; } else if (p1 == strstr(p1, ">>")) { MidiEvents[j].FlwCtl |= 0x20; } else if (p1 == strstr(p1, ">")) { MidiEvents[j].FlwCtl |= 8; } else if (p1 == strstr(p1, "v")) { MidiEvents[j].FlwCtl |= 0x10; }}
   }
  }
  else if (MidiFileEvent_getType(midi_file_event) == MIDI_FILE_EVENT_TYPE_SYSEX) {
  MidiEvents[i].MsgCtl      = 1;
  MidiEvents[i].EventData   = 0x000000f0;
  MidiEvents[i].data_length = MidiFileSysexEvent_getDataLength(midi_file_event);
  MidiEvents[i].data_buffer = MidiFileSysexEvent_getData(midi_file_event);
  if ((MidiEvents[i].EventData & (args[9]>>16)) == (args[9] & 0x7fff)) { MidiEvents[j].FlwCtl |= 1; MidiEvents[i].MsgCtl *= (args[9]>>15) & 1; }
  }
  else { unsigned long EventIdx;
  MidiEvents[i].MsgCtl    = 2;
  MidiEvents[i].EventData = (unsigned long)MidiFileVoiceEvent_getData(midi_file_event); EventIdx = MidiEvents[i].Track<<11 | (MidiEvents[i].EventData & 0x7f00)>>8 | (MidiEvents[i].EventData & 0xf)<<7;
  MidiEvents[i].Out       = (unsigned long)(TrkInfo[MidiEvents[i].Track] = (struct MidiEvent*)((unsigned long)TrkInfo[MidiEvents[i].Track] & 0xffffff0f | ((MidiEvents[i].EventData & 0xf) << 4) | 0x1));
  if       (((MidiEvents[i].EventData & 0xf0) == 0x80) || ((MidiEvents[i].EventData & 0x7f00f0) == 0x90)) { MidiEvents[i].MsgCtl += 1; }
   else if ( (MidiEvents[i].EventData & 0xf0) == 0x90                                                   ) { MidiEvents[i].MsgCtl += 2; }
  if ((MidiEvents[i].EventData & 0xf0) == 0xb0) { EventIdx += 1*TrkNum*16*128;
   //MidiEvents[i].MsgCtl += ((MidiEvents[i].EventData & 0x407ff0) == 0x0040b0)*1;
   //MidiEvents[i].MsgCtl += ((MidiEvents[i].EventData & 0x407ff0) == 0x4040b0)*2;
   }
  MidiEvents[i].EventIdx = &PendingEventsO[EventIdx];
  if ((MidiEvents[i].EventData & (args[9]>>16)) == (args[9] & 0x7fff)) { if ((MidiEvents[i].EventData & 0xf0) != 0x80) { MidiEvents[j].FlwCtl |= 1; } MidiEvents[i].MsgCtl *= (args[9]>>15) & 1; }
  }
 while (MidiEvents[i].MsgCtl | MidiEvents[i].FlwCtl) { i++; }
 }

MidiEvents[i].event_time = MidiEvents[i].EventData = 0; if (i) { MidiEvents[i].event_time = MidiEvents[i-1].event_time; }
MidiEvents[i].Label = ExitLabel; ExitLabel->Event = &MidiEvents[i]; MidiEvent = EntryLabel->Event = &MidiEvents[0]; k = 0;

MidiEvents[i].Tempo    = Tempo & 0x00ffffff;
MidiEvents[i].TimeSigN = TimeSig >> 24;
MidiEvents[i].TimeSigD = TimeSig >> 16;

ExpandLabels(Labels);

while (--i >= 0) { MidiEvents[i].TrkInfo = &TrkInfo[MidiEvents[i].Track]; MidiEvents[i].Ch = (MidiEvents[i].Out >> 4) & 0xf;
 if ((MidiEvents[i].EventData & (args[8]>>16)) == (args[8] & 0x7fff)) { MidiEvents[i].Rec |= 1; MidiEvents[i].MsgCtl *= (args[8]>>15) & 1; }
 if (MidiEvents[i].FlwCtl & 2) { MidiEvents[i].FlwCtl &= ~4; MidiEvents[i].FlwCtl |= 1;
  if (((unsigned long)MidiEvents[i].JumpEvent < LabelNum) && (Labels[(unsigned long)MidiEvents[i].JumpEvent].Event)) { if (!(k & 0x100)) { k = 0; } MidiEvents[i].JumpEvent = Labels[(unsigned long)MidiEvents[i].JumpEvent].Event; }
   else if ((l = (unsigned long)MidiEvents[i].JumpEvent*-1) >= 4) { l -= 4; j = i; while (l && j) { if (MidiEvents[j].Label->Event != MidiEvents[j-1].Label->Event) { l--; } j--; } if (l) { MidiEvents[i].FlwCtl = 1; } if (!(k & 0x100)) { k = 0; } MidiEvents[i].JumpEvent = MidiEvents[j].Label->Event; }
   else { MidiEvents[i].FlwCtl = 4; k = (unsigned long)MidiEvents[i].JumpEvent; }
  } else if (MidiEvents[i].FlwCtl & 4) { MidiEvents[i].FlwCtl &= ~4; k &= ~0x100; }
 MidiEvents[i].Label->Ret = k;
 if (MidiEvents[i].FlwCtl & 0x10) { MidiEvents[i].FlwCtl = 2; }
 if (MidiEvents[i].FlwCtl & 0x08) { MidiEvents[i].FlwCtl = 5; }
 if (MidiEvents[i].FlwCtl & 0x20) { MidiEvents[i].FlwCtl = 6; }
 if (MidiEvents[i].FlwCtl & 0x40) { MidiEvents[i].FlwCtl = 7; }
 MidiEvents[i].OffMsg = (MidiEvents[i].EventData & ~0x007f0010) | 0x00400000; if ((MidiEvents[i].EventData & 0xf0) == 0xb0) { MidiEvents[i].OffMsg = MidiEvents[i].EventData & ~0x007f0000; }
 if  (MidiEvents[i].MsgCtl == 3)                                     { MidiEvents[i].EventIdx->Event = &MidiEvents[i];                           }
 if ((MidiEvents[i].MsgCtl == 4) && (MidiEvents[i].EventIdx->Event)) { MidiEvents[i].OffMsg          = MidiEvents[i].EventIdx->Event->EventData; }
 if (MidiEvents[i].Label->Now && !MidiEvents[i].FlwCtl) { MidiEvents[i].FlwCtl = 1; }
 }

EntryLabel->Ret = -1; AlignLabels(Labels);

for (i=0; i<LabelNum; i++) { if (Labels[i].Event) { if (i <= 0x7f) { for (j=0; j<=0xf; j++) { Keys[j][i].Zone &= ~16; }} if (Labels[i].Event->FlwCtl == 4) { j = Labels[i].Event-MidiEvents;
 k = 0; while (MidiEvents[j+k+1].EventData && (MidiEvents[j+k+1].event_time == MidiEvents[j].event_time)) { k++; }
 l = MidiEvents[j+k].FlwCtl; MidiEvents[j+k].FlwCtl = MidiEvents[j].FlwCtl; MidiEvents[j].FlwCtl = l; MidiEvents[j+k].JumpEvent = MidiEvents[j].JumpEvent;
 }}}

if (!FirstLabel) { FirstLabel = EntryLabel; } if (!LastLabel) { LastLabel = ExitLabel; }

for (j=0; j<MutesNum; j++) { Mutes[j*(TrkNum+1)] |= (MutesRet>>j)&1; for (i=0; i<TrkNum; i++) { Mutes[j*(TrkNum+1)+1+i] ^= ((MutesInv2>>j)&1)<<3; }}

for (i=0; i<(sizeof(Port2Out)/sizeof(struct MidiOut)); i++) { if (Port2Out[i].h) { for (j=0; j<=0xf; j++) {
 for (k=12; k<_msize(args)/sizeof(signed long); k++) { if (((args[k]>>24) == 1) || ((args[k]>>24) == 3)) { midiOutShortMsg(Port2Out[i].h, args[k]&0x7f7fff | j); }}
 }}}

for (i=0; i<TrkNum; i++) { TrkInfo[i] = NULL; } SetEntryLabel

FirstMute = EntryMute = &Mutes[(MutesNum-2)*(TrkNum+1)+1]; if (MutesNum > 2) { FirstMute = &Mutes[(0)*(TrkNum+1)+1]; } Mute = SetEntryMute

Key0 = Key1 = &Keys[0x0][0x00]; PedalLeft = PedalMid = FlwMsk = IRQ = ExitVal = start_time = 0; Dead = 1; Speed = Speed0 = 1; (ThruE1 = ExitLabel->Event)->midi_out = Port2Out[DefODev].h;

if (args[2] >= 0) { timeBeginPeriod(args[2]); } LastTime = WatchDogTimeOut = timeGetTime(); WatchDogTimeOut += args[5]; if ((TimeOut = args[5]) < 0) { TimeOut = WatchDogTimeOut = -1; }

SetConsoleCtrlHandler(HandlerRoutine, TRUE); for (i=0; i<(sizeof(Port2In)/sizeof(struct MidiIn)); i++) { if (Port2In[i].h) { midiInStart(Port2In[i].h); }} printf("%d playing ...", ExitLabel->Event-MidiEvent);
while (MidiEvent->EventData) { register unsigned long t = MidiEvent->event_time*Speed, current_time = timeGetTime(); if (!start_time) { start_time = current_time-t; }
 if ((signed long)(t += start_time-current_time) > 0) { WaitForSingleObject(signalling_object0, t); }

 switch (MidiEvent->FlwCtl^FlwMsk | IRQ) {
  case 0x09: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f: case 0x11: case 0x13: case 0x14: case 0x16:
   while (LatestPendingO) { while (LatestPendingO->Cnt) { midiOutShortMsg(LatestPendingO->Event->midi_out, LatestPendingO->Event->OffMsg); LatestPendingO->Cnt--; } LatestPendingO = LatestPendingO->Prev; }
   if ((MidiEvent = Label4->Event       )->FlwCtl > 2) { FlwMsk = MidiEvent->FlwCtl^1; } IRQ = start_time = 0; Mute = Mute0 = Mute2; Mute3 = Mute2 = Mute1 = Mute11; Speed = Speed0; continue;
  case 0x02: case 0x03: case 0x05: case 0x06: case 0x07: case 0x0a: case 0x12: case 0x1a: case 0x1b: case 0x1d:
   while (LatestPendingO) { while (LatestPendingO->Cnt) { midiOutShortMsg(LatestPendingO->Event->midi_out, LatestPendingO->Event->OffMsg); LatestPendingO->Cnt--; } LatestPendingO = LatestPendingO->Prev; }
   if ((MidiEvent = MidiEvent->JumpEvent)->FlwCtl > 2) { FlwMsk = MidiEvent->FlwCtl^1; }       start_time = 0; Mute = Mute0 = Mute1; Mute3 = Mute2 = Mute1 = Mute11; Speed = Speed0; continue;
  case 0x04: case 0x1c:
   while (LatestPendingO) { while (LatestPendingO->Cnt) { midiOutShortMsg(LatestPendingO->Event->midi_out, LatestPendingO->Event->OffMsg); LatestPendingO->Cnt--; } LatestPendingO = LatestPendingO->Prev; }
   if ((MidiEvent = Label1->Event       )->FlwCtl > 2) { FlwMsk = MidiEvent->FlwCtl^1; }       start_time = 0; Mute = Mute0 = Mute3; Mute3 = Mute2 = Mute1 = Mute11; Speed = Speed0; continue;
  case 0x1e: case 0x1f: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: IRQ = 0; case 0x01: case 0x15: case 0x17: case 0x19: case 0x21: Mute = Mute0; }

 if (MidiEvent->Rec) { RecEvent->event_time = timeGetTime(); RecEvent->Event = MidiEvent; RecEvent = RecEvent->NextEvent; } *(MidiEvent->TrkInfo) = MidiEvent;

 switch (MidiEvent->MsgCtl | Mute[MidiEvent->Track]) {
  case 0x4:           midiOutShortMsg(MidiEvent->midi_out, MidiEvent->EventData); if (!(PendingO = MidiEvent->EventIdx)->Cnt) { PendingO->Event = MidiEvent;
                      if (LatestPendingO) { LatestPendingO->Next = PendingO; } PendingO->Prev = LatestPendingO; (LatestPendingO = PendingO)->Next = NULL; } PendingO->Cnt++; break;
  case 0x3: case 0xb: if ((PendingO = MidiEvent->EventIdx)->Cnt) { midiOutShortMsg(MidiEvent->midi_out, MidiEvent->EventData); PendingO->Cnt--; if (!PendingO->Cnt) {
                      if (PendingO->Prev) { PendingO->Prev->Next = PendingO->Next; }
                      if (PendingO->Next) { PendingO->Next->Prev = PendingO->Prev; } else { LatestPendingO = PendingO->Prev; }}} break;
  case 0x2:           midiOutShortMsg(MidiEvent->midi_out, MidiEvent->EventData); break;
  case 0x1:           midi_message_header.lpData         = MidiEvent->data_buffer;
                      midi_message_header.dwBufferLength = MidiEvent->data_length;
                      midi_message_header.dwFlags        = 0;
                      midiOutPrepareHeader(MidiEvent->midi_out, &midi_message_header, sizeof(MIDIHDR));
                      midiOutLongMsg(MidiEvent->midi_out, &midi_message_header, sizeof(MIDIHDR)); if (WaitForSingleObject(signalling_object1, TimeOut)) { goto Exit2; }
                      midiOutUnprepareHeader(MidiEvent->midi_out, &midi_message_header, sizeof(MIDIHDR)); }

 MidiEvent = MidiEvent->NextEvent; FlwMsk = 0; if (timeGetTime() > WatchDogTimeOut) { WatchDogTimeOut = timeGetTime()+TimeOut; if (Dead) { goto Exit0; } Dead = 1; }
 }
ExitVal |= 1; Exit2: ExitVal |= 2; Exit0: printf(" done. (%x)\n", ExitVal); for (i=0; i<(sizeof(Port2In)/sizeof(struct MidiIn)); i++) { if (Port2In[i].h) { midiInStop(Port2In[i].h); }} SetConsoleCtrlHandler(HandlerRoutine, FALSE); if (args[2] >= 0) { timeEndPeriod(args[2]); }

while (LatestPendingO) { while (LatestPendingO->Cnt) { midiOutShortMsg(LatestPendingO->Event->midi_out, LatestPendingO->Event->OffMsg); LatestPendingO->Cnt--; } LatestPendingO = LatestPendingO->Prev; }
while (LatestPendingI) {                                                                                                                                         LatestPendingI = LatestPendingI->Prev; }

for (i=0; i<(sizeof(Port2Out)/sizeof(struct MidiOut)); i++) { if (Port2Out[i].h) { for (j=0; j<=0xf; j++) {
 for (k=12; k<_msize(args)/sizeof(signed long); k++) { if (((args[k]>>24) == 2) || ((args[k]>>24) == 3)) { midiOutShortMsg(Port2Out[i].h, args[k]&0x7f7fff | j); }}
 } midiOutClose(Port2Out[i].h); }}

for (i=0; i<(sizeof(Port2In)/sizeof(struct MidiIn)); i++) { if (Port2In[i].h) { midiInReset(Port2In[i].h); for (j=0; j<sizeof(Port2In[i].b)/sizeof(struct MidiBuf); j++) { midiInUnprepareHeader(Port2In[i].h, &Port2In[i].b[j].h, sizeof(MIDIHDR)); } midiInClose(Port2In[i].h); }} if (ExitVal < 3) { Sleep(TimeOut); } //goto start;

CloseHandle(signalling_object0); CloseHandle(signalling_object1); saveMidiEventsToFile(args, MidiFile_getResolution(midi_file), Tempo0, TimeSig0, KeySig0, RecEvents, RecEvent, ExitVal, Label0);

                                               i = 0;   //regular
if       ((ExitVal & 3) < 3                ) { i = 2; } //error
 else if ( ExitVal & 4                     ) { i = 3; } //CTRL+C
 else if (Label0->Event == ExitLabel->Event) { i = 4; } //exit
 else if (Label0->Event == LastLabel->Event) { i = 5; } //last

free(RecEvents); free(Thrus[0]); free(TrkInfo); free(Mutes); free(PendingEventsO); free(Labels); free(MidiEvents); free(args); MidiFile_free(midi_file); return(i); }

//============================================================================//
