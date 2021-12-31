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

Chord::Chord(int chord_base, int chord_name):m_chord_base(chord_base), m_chord_name(chord_name) {
    int chord_interior_base = chord_base % 12;
    switch(m_chord_name){
        case EN_CHORD_NAME__MAJOR:
            m_notes.push_back(chord_interior_base);
            m_notes.push_back((chord_interior_base + 4) % 12);     // 根音到三音中间间隔四个半音
            m_notes.push_back((chord_interior_base + 7) % 12);     // 根音到五音中间间隔7个半音
            break;
        case EN_CHORD_NAME__MINOR:
            m_notes.push_back(chord_interior_base);
            m_notes.push_back((chord_interior_base + 3) % 12);
            m_notes.push_back((chord_interior_base + 7) % 12);
            break;
        case EN_CHORD_NAME__MAJOR_SEVENTH:
            m_notes.push_back(chord_interior_base);
            m_notes.push_back((chord_interior_base + 4) % 12);
            m_notes.push_back((chord_interior_base + 7) % 12);
            m_notes.push_back((chord_interior_base + 11) % 12);
            break;
        case EN_CHORD_NAME__MINOR_SEVENTH:
            m_notes.push_back(chord_interior_base);
            m_notes.push_back((chord_interior_base + 3) % 12);
            m_notes.push_back((chord_interior_base + 7) % 12);
            m_notes.push_back((chord_interior_base + 10) % 12);
            break;
        case EN_CHORD_NAME__SUS4_SEVENTH:
            m_notes.push_back(chord_interior_base);
            m_notes.push_back((chord_interior_base + 5) % 12);
            m_notes.push_back((chord_interior_base + 7) % 12);
            m_notes.push_back((chord_interior_base + 10) % 12);
        default:
            return;
    }
}

bool Chord::IsChordInterior(int key) {
    const int key_base = key % 12;
    for (auto i: m_notes) {
        if(i == key_base)
            return true;
    }
    return false;
}

ChordProgression::ChordProgression(int chord_progression_id) {
    switch(chord_progression_id){
        case EN_CHORD_PROGRESSIONS_TYPE__C_BLUES:
        case EN_CHORD_PROGRESSIONS_TYPE__Dm7_G7_CM7_Am7:
            m_chords.push_back(Chord(EN_NOTE__D, EN_CHORD_NAME__MINOR_SEVENTH));
            m_chords.push_back(Chord(EN_NOTE__G, EN_CHORD_NAME__SUS_SEVENTH));
            m_chords.push_back(Chord(EN_NOTE__C, EN_CHORD_NAME__MAJOR_SEVENTH));
            m_chords.push_back(Chord(EN_NOTE__A, EN_CHORD_NAME__MINOR_SEVENTH));
            break;
        default:
            return;
    }
}

void ChordProgression::Init(std::vector<std::tuple<int, int>>& chords) {
    // TODO
}	

void ChordProgression::Reset() {
    m_chords.clear();
}

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
    std::cout << "\nTrack " << track << std::endl;
    std::cout << "Tick\tSeconds\tDur\tMessage" << std::endl;
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

} // end namespace smf

