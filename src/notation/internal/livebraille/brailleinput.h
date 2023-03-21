#ifndef BRAILLEINPUT_H
#define BRAILLEINPUT_H

#include "notationtypes.h"
#include "braille.h"

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

    mu::notation::AccidentalType accidental();
    mu::notation::NoteName notename();
    mu::notation::SymbolId articulation();
    int octave();
    mu::notation::voice_idx_t voice();
    bool slur();
    bool tie();

    void setAccidental(const mu::notation::AccidentalType accidental);
    void setNoteName(const mu::notation::NoteName notename);
    void setArticulation(const mu::notation::SymbolId articulation);
    void setOctave(const int octave);
    void setVoicce(const mu::notation::voice_idx_t voice);
    void setSlur(const bool s);
    void setTie(const bool s);

private:
    mu::notation::AccidentalType _accidental = mu::notation::AccidentalType::NONE;
    mu::notation::NoteName _note_name = mu::notation::NoteName::C;
    mu::notation::SymbolId _articulation = mu::notation::SymbolId::noSym;
    int _octave = 4;
    mu::notation::voice_idx_t _voice = 0;
    QString _input_buffer;
    int _code_num = 0;
    bool _slur, _tie;
};

QString parseBrailleKeyInput(QString keys);
BraillePattern recognizeBrailleInput(QString pattern);

mu::notation::NoteName getNoteName(const braille_code* code);
int getInterval(const braille_code* code);
bool isNoteName(const braille_code* code);
QString fromNoteName(mu::notation::NoteName);
mu::notation::AccidentalType getAccidentalType(const braille_code* code);
mu::notation::SymbolId getArticulation(const braille_code* code);
int getOctave(const braille_code* code);

#endif // BRAILLEINPUT_H
