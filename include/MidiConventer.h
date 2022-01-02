//
// Programmer:    Payne Zheng <photosynthesi@outlook.com>
// Creation Date: Wed Dec 29 02:02:10 UTC 2021
// Last Modified: Wed Dec 29 09:37:18 UTC 2021 
// Filename:      midiediter/include/MidiConventer.h
// Syntax:        C++11
// Code           UTF-8
//

#pragma once
#include "MidiFile.h"
#include "MidiMessage.h"
#include "ChordProgression.h"
#include <vector>
#include <map>

namespace smf {

class MidiConventer {
public:
	MidiConventer (MidiFile* midifile, ChordProgression* chord_progression, int duration);
	void		Reset();
	bool		Convent(int track, int bpm, int chords);									// all in one
	
	double		GetBeat(int tick, int tpq);
	// 量化 
	void		QuantifyTrack(int track, int duration);
	void 		QuantifyEvent(MidiEvent& midievent, int unit_size, int tpq, int direction);			// 向前/后 量化一个事件

	bool		IsChordInterior(const MidiEvent& midievent);
	void		CleanChordVoiceover(int track);

	void		CleanRecurNotes(int track);
	
	void		ProlongNotes(int track);
	// 延音
	// 去掉和弦外音和重复音


private:
	MidiFile* 			m_midifile = nullptr;
	ChordProgression*	m_chord_progression	= nullptr;
	int 				m_duration;			// 量化一个事件的最小单位
};

} // end of namespace smf

