//
// Programmer:    Payne Zheng <photosynthesi@outlook.com>
// Creation Date: Wed std::dec 29 02:02:10 UTC 2021
// Last Modified: Wed std::dec 29 09:37:18 UTC 2021
// Filename:      midiediter/src/MidiConventer.cpp
// Syntax:        C++11
// Code           UTF-8
//

#include "MidiConventer.h"
#include <stdlib.h>
#include <iostream>


namespace smf {

MidiConventer::MidiConventer(MidiFile* midifile, ChordProgression* chord_progression, int duration)
    :m_midifile(midifile), m_chord_progression(chord_progression), m_duration(duration) {
}

void MidiConventer::Reset() {
    m_midifile          = nullptr;
    m_duration          = 0;
    m_chord_progression = nullptr;
}

bool MidiConventer::QuantifyTrack(int track, int duration) {
    std::cout << "TPQ: " << m_midifile->getTicksPerQuarterNote() << std::endl;
    std::cout << "\nQuantifyTrack " << track << std::endl;
    std::cout << "Tick\tSeconds\tDur\tMessage\tnew_Tick\tnew_Seconds\tnew_Dur\t" << std::endl;
    MidiEventList midi_events = (*m_midifile)[track];
    for (int event=0; event< midi_events.size(); event++) {
        std::cout << std::dec << midi_events[event].tick;
        std::cout << '\t' << std::dec << midi_events[event].seconds;
        std::cout << '\t';
        if (midi_events[event].isNoteOn())
        std::cout << midi_events[event].getDurationInSeconds();
        std::cout << '\t' << std::hex;
        for (auto i=0; i<midi_events[event].size(); i++)
        std::cout << (int)midi_events[event][i] << ' ';
        std::cout<< '\t';
        std::cout<< std::dec << midi_events[event].getKeyNumber();   // 输出音符
        std::cout << std::endl;
    }
    return true;
}

bool QuantifyNote(MidiEvent& midievent, int direction) {
    
}

} // end namespace smf

