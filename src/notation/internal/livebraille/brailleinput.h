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

    BieSequencePatternType parseBraille();

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
    bool slur();
    bool tie();    

    void setAccidental(const AccidentalType accidental);
    void setNoteName(const NoteName notename);
    void setCurrentDuration(const DurationType duration);
    void setNoteDurations(const std::vector<DurationType> durations);
    void setArticulation(const SymbolId articulation);
    void setOctave(const int octave);
    void setDots(const int dots);
    void setAddedOctave(const int octave);
    void setVoice(const voice_idx_t voice);
    void setSlur(const bool s);
    void setTie(const bool s);
    void setNoteGroup(const NoteGroup g);

private:
    AccidentalType _accidental = AccidentalType::NONE;
    NoteName _note_name = NoteName::C;
    SymbolId _articulation = SymbolId::noSym;
    int _octave = 4;
    int _added_octave = -1;
    int _dots = 0;
    voice_idx_t _voice = 0;
    QString _input_buffer;
    int _code_num = 0;
    bool _slur, _tie;
    DurationType _current_duration;
    std::vector<DurationType> _note_durations;
    NoteGroup _note_group = NoteGroup::Undefined;
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

}
#endif // BRAILLEINPUT_H
