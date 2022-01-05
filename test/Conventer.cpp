//
// Programmer:    Payne Zheng <photosynthesi@outlook.com>
// Creation Date: Sun Jan  2 10:10:51 CST 2022
// Last Modified: Sun Jan  2 10:10:51 CST 2022
// Filename:      midiediter/test/Conventer.cpp
// Syntax:        C++11
// Code           UTF-8
// Usage:         interface
//


#include "MidiFile.h"
#include "MidiConventer.h"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace smf;

int main(int argc, char** argv) {
    if (argc != 4)
    {
        cout<< "--> param num error <--"<<endl;
        cout<< "./Conventer input_midifile_chord input_midifile 1/2" <<endl;
        return -1;
    }
    MidiFile* chord = new MidiFile(argv[1]);
    MidiFile* input_user = new MidiFile(argv[2]);
    chord->doTimeAnalysis();
    input_user->doTimeAnalysis();
    chord->linkNotePairs();
    input_user->linkNotePairs();

    if (chord->getTicksPerQuarterNote() != input_user->getTicksPerQuarterNote()) {
        cout<< "file tpq not the same" <<endl;
        return -1;
    }
    int param_chord_progression = atoi(argv[3]); // 先默认一个1625
    ChordProgression* chord_progression;
    int tracks = input_user->getTrackCount();
    if (param_chord_progression == 1625)
        chord_progression = new ChordProgression(EN_CHORD_PROGRESSIONS_TYPE__CM7_Am7_Dm7_G7);    // 1625
    else
        chord_progression = new ChordProgression(EN_CHORD_PROGRESSIONS_TYPE__1_4M7_6m7_5sus4_5);
    MidiConventer* midi_conventer = new MidiConventer(input_user, chord_progression, 1);

    for (int track = 0; track < tracks; track++) {
        // midi_conventer->QuantifyTrack(track, 1);
        // midi_conventer->CleanChordVoiceover(track);
        midi_conventer->CleanRecurNotes(track);
        input_user->sortTracks();
    }
    input_user->write(argv[2]);

}