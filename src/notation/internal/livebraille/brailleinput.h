#ifndef BRAILLEINPUT_H
#define BRAILLEINPUT_H

#include "notationtypes.h"
#include "braille.h"
#include "brailleinputparser.h"

namespace mu::notation {

enum class NoteGroup
{
    Undefined = 0,
    Group1, // whole, half, quarter, 8th
    Group2, // 16th, 32nd, 64th, 128th
    Group3  // 256th, 512th, 1024th, 2048th
};

enum class IntervalDirection
{
    Up,
    Down
};

class BrailleInputState
{
public:
    BrailleInputState();
    ~BrailleInputState();

    void initialize();
    void reset();
    void resetBuffer();    
    void insertToBuffer(const QString);
    QString buffer();

    BieSequencePatternType parseBraille(IntervalDirection direction);

    AccidentalType accidental();
    NoteName noteName();
    NoteGroup noteGroup();

    DurationType currentDuration();
    std::vector<DurationType> noteDurations();
    bool isDurationMatch();
    DurationType getCloseDuration();

    SymbolId articulation();
    int octave();
    int dots();
    int addedOctave();
    voice_idx_t voice();
    bool noteSlur();
    bool longSlurStart();
    bool longSlurStop();
    bool tie();

    void setAccidental(const AccidentalType accidental);
    void setNoteName(const NoteName notename, const bool chord_base = true);
    void setCurrentDuration(const DurationType duration);
    void setNoteDurations(const std::vector<DurationType> durations);
    void setArticulation(const SymbolId articulation);
    void setOctave(const int octave);
    void setDots(const int dots);
    void setAddedOctave(const int octave);
    void setVoice(const voice_idx_t voice);    

    void setNoteGroup(const NoteGroup g);

    std::vector<int> intervals();
    void clearIntervals();
    void addInterval(const int interval);

    void setTie(const bool s);
    Note * tieStartNote();
    void setTieStartNote(Note *);
    void clearTie();

    void setNoteSlur(const bool s);
    void setLongSlurStart(const bool s);
    void setLongSlurStop(const bool s);
    Note * slurStartNote();
    Note * longSlurStartNote();
    void setSlurStartNote(Note *);
    void setLongSlurStartNote(Note *);
    void clearSlur();
    void clearLongSlur();

private:
    AccidentalType _accidental = AccidentalType::NONE;
    NoteName _note_name = NoteName::C;
    NoteName _chordbase_note_name = NoteName::C;
    SymbolId _articulation = SymbolId::noSym;
    int _octave = 4;
    int _added_octave = -1;
    int _dots = 0;
    voice_idx_t _voice = 0;
    QString _input_buffer;
    int _code_num = 0;

    DurationType _current_duration;
    std::vector<DurationType> _note_durations;
    NoteGroup _note_group = NoteGroup::Undefined;

    std::vector<int> _intervals;

    bool _tie;
    Note * _tie_start_note =  NULL;

    bool _note_slur, _long_slur_start, _long_slur_stop;
    Note * _slur_start_note =  NULL;
    Note * _long_slur_start_note =  NULL;
};

QString parseBrailleKeyInput(QString keys);

NoteName getNoteName(const braille_code* code);
std::vector<DurationType> getNoteDurations(const braille_code* code);
std::vector<DurationType> getRestDurations(const braille_code* code);
int getInterval(const braille_code* code);
bool isNoteName(const braille_code* code);
QString fromNoteName(NoteName);
AccidentalType getAccidentalType(const braille_code* code);
SymbolId getArticulation(const braille_code* code);
int getOctave(const braille_code* code);
int getOctaveDiff(NoteName source, NoteName dest);
int getOctaveDiff(IntervalDirection direction, NoteName source, int interval);
NoteName getNoteNameForInterval(IntervalDirection direction, NoteName source, int interval);

}
#endif // BRAILLEINPUT_H
