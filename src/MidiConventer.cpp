//
// Programmer:    Payne Zheng <photosynthesi@outlook.com>
// Creation Date: Wed Dec 29 02:02:10 UTC 2021
// Last Modified: Thu Jan  6 07:17:45 UTC 2022
// Filename:      midiediter/src/MidiConventer.cpp
// Syntax:        C++11
// Code           UTF-8
//

#include "MidiConventer.h"
#include "MidiNote.h"
#include "smflog.h"
#include <stdlib.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <set>


namespace smf 
{

MidiConventer::MidiConventer(MidiFile midifile, ChordProgression chord_progression, int duration)
    :m_midifile(midifile), m_chord_progression(chord_progression), m_duration(duration) 
{

}

MidiConventer::MidiConventer(std::string file_url, ChordProgression chord_progression, int duration)
    :m_chord_progression(chord_progression), m_duration(duration) 
{
    m_midifile.read(file_url);
    m_midifile.doTimeAnalysis();
    m_midifile.linkNotePairs();
}

void MidiConventer::Reset() 
{
    m_midifile.clear();
    m_chord_progression.Clear();
    m_duration = 0;
}

void MidiConventer::Clear()
{
    Reset();
}

double MidiConventer::GetBeat(int tick) 
{
    return double(tick)/m_midifile.getTicksPerQuarterNote();
}

void MidiConventer::QuantifyTrack(int track) 
{
    std::cout << "\nQuantifyTrack " << track << std::endl;
    MidiEventList& midi_events = m_midifile[track];
    std::cout<<std::endl;
    for (int event=0; event< midi_events.size(); event++) 
    {
        // TODO: 需要移动两个事件(on && off)
        // 一个note不能越过小节线
        // 一个note不能跨和弦
        if (midi_events[event].isNoteOn()) 
        {
            double move = QuantifyEvent(midi_events[event], m_duration, 0);
            MidiEvent* offevent = midi_events[event].getLinkedEvent();
            // 移动on也要移动off，保持音长不变
            // QuantifyEvent(*offevent, m_duration, 0);
            if(offevent != nullptr) 
            {
                offevent->tick = offevent->tick + move;
                // CuttingNote(midi_events[event], *offevent);
            }
            else
            {
                // printf("")
            }
        }
    }
    // m_midifile.sortTrack(track);
}

/**
 * @brief 量化一个on/off event, 每个事件会移动到最近的节点上, 所以有可能会导致跨小节跨和弦(丢失信息)的问题
 * 
 * @param midievent 
 * @param //unit_size 量化单位, 2,4,8等分别表示二/四/十六分 读取m_duration
 * @param direction 量化方向,-1向左移动,默认移动到最近节点
 * @return 100 000 000: 错误 | 其余表示向左(负数)/右(正数)的位移量(单位: tick)
 */
double MidiConventer::QuantifyEvent(MidiEvent& midievent, int unit_size, int direction) 
{
    int tpq = m_midifile.getTicksPerQuarterNote();
    std::cout<< std::dec << GetBeat(midievent.tick);
    double left_beat = 0;
    double right_beat = 0;
    switch(unit_size) 
    {
        case 4:
            left_beat = (int)GetBeat(midievent.tick);
            right_beat = left_beat + 1;
            break;
        case 8:
            left_beat = (int)(GetBeat(midievent.tick)/0.5) * 0.5;
            right_beat = left_beat + 0.5;
            break;
        case 16:
            left_beat = (int)(GetBeat(midievent.tick)/0.25) * 0.25;
            right_beat = left_beat + 0.25;
            break;
        default:
            return 100000000;
    }
    int tmp = midievent.tick;
    // 向左移
    if (direction == -1) 
    {
        midievent.tick = int((left_beat - 4.0 / unit_size)*tpq);
        return midievent.tick - tmp;
    }
    else if(direction == 1) 
    {
        // TODO:
    }
    int left_tick = int(left_beat*tpq);
    int right_tick = int(right_beat*tpq);
    if (midievent.tick - left_tick > right_tick - midievent.tick)
        midievent.tick = right_tick;
    else
        midievent.tick = left_tick;
    std::cout<< '\t' << GetBeat(midievent.tick) << std::endl;
    return midievent.tick - tmp;
}

bool MidiConventer::IsChordInterior(const MidiEvent& midievent)
{
    return m_chord_progression.IsChordInterior((int)GetBeat(midievent.tick), midievent.getKeyNumber());
}

void MidiConventer::CleanChordVoiceover(int track) 
{
    std::cout << "\nCleanChordVoiceover Track " << track << std::endl;
    MidiEventList& midi_events = m_midifile[track];
    for (int event=0; event< midi_events.size(); event++) 
    {
        if (midi_events[event].isNoteOn()) 
        {
            if (midi_events[event].isNote() && !IsChordInterior(midi_events[event])) 
            {
                MidiEvent* offevent = midi_events[event].getLinkedEvent();
                midi_events[event].clear();
                if (offevent != nullptr) 
                    offevent->clear();
            }
        }
    }
    m_midifile.removeEmpties();
    m_midifile.sortTrack(track);
}
/**
 * @brief 当前逻辑: 遍历音符点,先遍历的音符点保留，后遍历的音符与先遍历的有重叠的则删除之
 * MidiEventList::eventcompare 保证了MidiEventList的时间序
 * TODO: 保留变化音
 * @param track 
 */
void MidiConventer::CleanRecurNotes(int track)
{
    std::cout << "\nCleanRecurNotes Track " << track << std::endl;
    MidiEventList midi_events = m_midifile[track];
    int block_length = 4.0/m_duration*(m_midifile.getTicksPerQuarterNote());
    std::set<MidiNote*> notes;                              // [notes] 表示所以音
    std::map<int, std::vector<MidiNote*>> block_index;      // block_id -> [note], block_id: 0 -- n-1
    // 将所有的事件读到note中, 并保存每个区块包含哪些音
    for (int event=0; event< midi_events.size(); event++) 
    {
        if (midi_events[event].isNoteOn()) {
            MidiEvent* offevent = midi_events[event].getLinkedEvent();
            if (offevent != nullptr && offevent->tick > midi_events[event].tick) 
            {
                MidiNote* new_note = new MidiNote(midi_events[event], (*offevent));
                notes.insert(new_note);
                int begin_tick  = new_note->GetBeginTick();
                int end_tick    = new_note->GetEndTick();
                while(begin_tick < end_tick) 
                {
                    int block_id = begin_tick/block_length;
                    begin_tick += block_length;
                    block_index[block_id].push_back(new_note);
                } 
            }
            else 
            {
                std::cout<< "MidiConventer::CleanRecurNotes off_event nullptr, MidiEvent seq:" << midi_events[event].seq << std::endl;
            }
        }
    }

    // 删除过后每个区间只有一个音, 上一block出现过的音本block不再出现
    MidiNote* last_note = nullptr;
    int last_block_id = -1;
    for (auto iter : block_index) 
    {
        if (iter.second.size() > 1) 
        {
            // 当上一小节有音出现
            if (last_note != nullptr && last_block_id == iter.first-1)
            {   
                auto the_same_note = find(iter.second.begin(), iter.second.end(), last_note);
                if (the_same_note != iter.second.end())
                {
                    iter.second.erase(the_same_note);
                }
            }
            // 随机保留一个音,其他的音全部删除
            int save_note_index = rand() % iter.second.size();
            MidiNote* tmp = iter.second[save_note_index];
            iter.second.clear();
            iter.second.push_back(tmp);
            // 临时变量表示此block使用的音和block_id
            last_note = tmp;
            last_block_id = iter.first;
        }
        else if (iter.second.size() == 1)
        {
            MidiNote* tmp = iter.second[0];
            last_note = tmp;
            last_block_id = iter.first;
        }
        else
        {
            last_note = nullptr;
            last_block_id = iter.first;
        }
    }
    std::vector<MidiNote*>  tmp_notes;
    std::set<MidiNote*>     new_notes;
    for (auto note : notes) 
    {
        tmp_notes.clear();
        MidiNote::CutNote(note, block_index, tmp_notes, block_length);
        for(auto it:tmp_notes) 
        {
            new_notes.insert(it);
        }
    }
    // 写回midifile
    m_midifile[track].clear();
    m_midifile.addTrack();
    for (auto iter : new_notes)
    {   
        m_midifile.addEvent(m_midifile.getNumTracks() -1, iter->GetBeginEvent());
        m_midifile.addEvent(m_midifile.getNumTracks() -1, iter->GetEndEvent());
    }
    m_midifile.doTimeAnalysis();
    m_midifile.sortTrack(m_midifile.getNumTracks() -1);
}


/**
 * @brief 延音功能
 * 
 * @param track 
 */
void MidiConventer::ProlongNotes(int track) 
{
    std::cout << "TicksPerQuarterNote(TPQ): " << m_midifile.getTicksPerQuarterNote() << std::endl;
    std::cout << "QuantifyTrack " << track << std::endl;
    MidiEventList& midi_events = m_midifile[track];
    for (int event=0; event< midi_events.size(); event++) 
    {
        if (midi_events[event].isNoteOff()) 
        {
            if (event+1 < midi_events.size() && midi_events[event+1].isNoteOn())
            {
                midi_events[event].tick = midi_events[event+1].tick;
            }
        }
    }
    m_midifile.sortTrack(track);
    // m_midifile.removeEmpties();
}

void MidiConventer::PrintMidifile(MidiFile m_midifile) 
{
    int tracks = m_midifile.getTrackCount();
    SMF_LOG_INFO("TicksPerQuarterNote(TPQ): %d\n", m_midifile.getTicksPerQuarterNote());
    if (tracks > 1) 
    {
        SMF_LOG_INFO("Total Tracks: %d\n", tracks);
    }
    for (int track=0; track<tracks; track++) 
    {
        SMF_LOG_INFO("\nCur Track: %d\n", track);
        SMF_LOG_INFO("Tick\tSeconds\tDur\tMessage\n");
        const MidiEventList& midi_events = m_midifile[track];
        for (int event=0; event< midi_events.size(); event++) 
        {
            SMF_LOG_INFO("%d\t", midi_events[event].tick);
            SMF_LOG_INFO("%f\t", midi_events[event].seconds);
            if (midi_events[event].isNoteOn())
            {
                SMF_LOG_INFO("%f\t", midi_events[event].getDurationInSeconds());
                // std::cout << midi_events[event].getDurationInSeconds();
            }

            SMF_LOG_INFO("\t");
            for (auto i=0; i<midi_events[event].size(); i++)
            {
                SMF_LOG_INFO("%x ", (int)midi_events[event][i]);
            }

            // std::cout<< std::dec << midi_events[event].getKeyNumber();   // 输出音符
            SMF_LOG_INFO("\t\n");
        }
        SMF_LOG_INFO("\n");
    }
}

// void MidiConventer::PrintEvent(MidiEvent midi_event) {}

bool MidiConventer::CheckNoteValid(const MidiEvent& on, const MidiEvent& off) 
{
    double on_beat = GetBeat(on.tick);
    double off_beat = GetBeat(off.tick);
    // 检查跨和弦
    int on_chord_seq = m_chord_progression.GetChordSeq(on_beat, 0);
    int off_chord_seq = m_chord_progression.GetChordSeq(off_beat, 1);
    if (on_chord_seq != off_chord_seq) 
    {
        return false;
    }
    // 检查跨小节 (先按4拍一小节处理..)
    // auto DoubleMod = [beat = m_chord_progression.m_beats](double x) -> int {
    //     int section_seq = 1;
    //     while (x >= beat) {

    //     }
    //     return x+y+v1+(*v2);
    // };
    int on_section_seq = int (on_beat / 4);
    int off_section_seq = int (off_beat / 4);
    if (on_section_seq != off_section_seq) 
    {
        return false;
    }
    return true;
}

/**
 * @brief 处理跨和弦和跨小节, 需要先量化event on/off到最近的节点上
 * 跨和弦, 直接将off事件的事件挪到on事件和弦的末尾  (暂时逻辑)
 * 跨小节, 直接将off事件的事件挪到on事件小节的末尾  (暂时逻辑)
 * @param on 
 * @param off 
 */
void MidiConventer::CuttingNote(MidiEvent& on, MidiEvent& off) 
{
    std::cout<< "Cutting Note begin" << std::endl; 
    double on_beat = GetBeat(on.tick);
    double off_beat = GetBeat(off.tick);

    // 处理跨和弦
    int on_chord_seq = m_chord_progression.GetChordSeq(on_beat, 0);
    int off_chord_seq = m_chord_progression.GetChordSeq(off_beat, 1);
    if (on_chord_seq != off_chord_seq)
    {
        auto chord_tuple = m_chord_progression.GetChord(on_chord_seq);
        int end_tick = (std::get<1>(chord_tuple) + std::get<2>(chord_tuple) - 1) * m_midifile.getTicksPerQuarterNote();
        off.tick = end_tick;
    }
    // 处理跨小节
    // TODO: payne
    std::cout<< "Cutting Note end" << std::endl; 
}

} // end namespace smf

