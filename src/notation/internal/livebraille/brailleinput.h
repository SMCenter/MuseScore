#ifndef BRAILLEINPUT_H
#define BRAILLEINPUT_H

#include "notationtypes.h"
#include "braille.h"

namespace mu::notation {

#define MAX_CODE_NUM 20

enum class BraillePatternType
{
    Unrecognized,
    Accidental,
    Octave,
    Note,
    Slur,
    LongSlurStart,
    LongSlurStop,
    NoteTie,
    ChordTie,
    Articulation,
    Tuplet,
    Finger,
    InAccord,
    Interval
};

struct BraillePattern
{
    BraillePatternType type = BraillePatternType::Unrecognized;
    std::vector<braille_code *> data;
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

    BraillePatternType parseBraille();

    AccidentalType accidental();
    NoteName noteName();

    DurationType currentDuration();
    std::vector<DurationType> noteDurations();
    bool isDurationMatch();
    DurationType getCloseDuration();

    SymbolId articulation();
    int octave();
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
    void setAddedOctave(const int octave);
    void setVoicce(const voice_idx_t voice);
    void setSlur(const bool s);
    void setTie(const bool s);

private:
    AccidentalType _accidental = AccidentalType::NONE;
    NoteName _note_name = NoteName::C;
    SymbolId _articulation = SymbolId::noSym;
    int _octave = 4;
    int _added_octave = -1;
    voice_idx_t _voice = 0;
    QString _input_buffer;
    int _code_num = 0;
    bool _slur, _tie;
    DurationType _current_duration;
    std::vector<DurationType> _note_durations;
};

QString parseBrailleKeyInput(QString keys);
BraillePattern recognizeBrailleInput(QString pattern);

NoteName getNoteName(const braille_code* code);
std::vector<DurationType> getNoteDurations(const braille_code* code);
int getInterval(const braille_code* code);
bool isNoteName(const braille_code* code);
QString fromNoteName(NoteName);
AccidentalType getAccidentalType(const braille_code* code);
SymbolId getArticulation(const braille_code* code);
int getOctave(const braille_code* code);
}
#endif // BRAILLEINPUT_H
