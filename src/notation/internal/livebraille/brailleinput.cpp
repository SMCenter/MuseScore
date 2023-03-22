#include "brailleinput.h"
#include "braille.h"

#include "log.h"

BrailleInputState brailleInputState;

QString parseBrailleKeyInput(QString keys)
{
    static std::map<std::string, std::string > braille_input_keys = {
        {"S", "3"}, {"D", "2"}, {"F", "1"},
        {"J", "4"}, {"K", "5"}, {"L", "6"}
    };

    QStringList lst = keys.split(QString::fromStdString("+"));

    std::vector<std::string> nlst;
    for(int i = 0; i < lst.size(); i++) {
        std::string key = lst.at(i).toStdString();

        if (braille_input_keys.find(key) != braille_input_keys.end()) {
            nlst.push_back(braille_input_keys[key]);
        }
    }    
    std::sort(nlst.begin(), nlst.end());

    QString buff = QString();
    for(auto n : nlst) {
        buff.append(QString::fromStdString(n));
    }

    return buff;
}

BraillePattern recognizeBrailleInput(QString pattern)
{
    BraillePattern res;
    std::string p = pattern.toStdString();

    braille_code* code = findNote(p);
    if(code != NULL) {
        res.type = BraillePatternType::Note;
        res.data = {code};
        return res;
    }

    code = findOctave(p);
    if(code != NULL) {
        res.type = BraillePatternType::Octave;
        res.data = {code};
        return res;
    }

    code = findAccidental(p);
    if(code != NULL) {
        res.type = BraillePatternType::Accidental;
        res.data = {code};
        return res;
    }

    code = findFinger(p);
    if(code != NULL) {
        res.type = BraillePatternType::Finger;
        res.data = {code};
        return res;
    }

    code = findInterval(p);
    if(code != NULL) {
        res.type = BraillePatternType::Interval;
        res.data = {code};
        return res;
    }

    if(p == Braille_FullMeasureAccord.code) {
        res.type = BraillePatternType::InAccord;
        res.data = {&Braille_FullMeasureAccord};
        return res;
    }

    if(p == Braille_NoteTie.code) {
        res.type = BraillePatternType::NoteTie;
        res.data = {&Braille_NoteTie};
        return res;
    }

    if(p == Braille_ChordTie.code) {
        res.type = BraillePatternType::ChordTie;
        res.data = {&Braille_ChordTie};
        return res;
    }

    if(p == Braille_NoteSlur.code) {
        res.type = BraillePatternType::Slur;
        res.data = {&Braille_NoteSlur};
        return res;
    }

    if(p == Braille_LongSlurOpenBracket.code) {
        res.type = BraillePatternType::LongSlurStart;
        res.data = {&Braille_LongSlurOpenBracket};
        return res;
    }

    if(p == Braille_LongSlurCloseBracket.code) {
        res.type = BraillePatternType::LongSlurStop;
        res.data = {&Braille_LongSlurCloseBracket};
        return res;
    }

    return res;
}

mu::notation::NoteName getNoteName(const braille_code* code)
{
    char c = code->tag[0];
    switch (c) {
        case 'A': case 'a':
            return mu::notation::NoteName::A;
        case 'B': case 'b':
            return mu::notation::NoteName::B;
        case 'C': case 'c':
            return mu::notation::NoteName::C;
        case 'D': case 'd':
            return mu::notation::NoteName::D;
        case 'E': case 'e':
            return mu::notation::NoteName::E;
        case 'F': case 'f':
            return mu::notation::NoteName::F;
        case 'G': case 'g':
            return mu::notation::NoteName::G;
    }
    return mu::notation::NoteName::C;
}

int getInterval(const braille_code* code)
{
    char c = code->tag.back();
    if(c < '2' || c > '8') {
        return -1;
    } else {
        return c - '1';
    }
}

bool isNoteName(const braille_code* code)
{
    static std::vector<std::string> note_names = {
        "aMaxima", "aLonga", "aBreve", "aWhole", "aHalf",
        "aQuarter", "a8th", "a16th", "a32nd", "a64th", "a128th",
        "a256th", "a512th", "a128th", "a2048th", "aBreveAlt",

        "bMaxima", "bLonga", "bBreve", "bWhole", "bHalf",
        "bQuarter", "b8th", "b16th", "b32nd", "b64th", "b128th",
        "b256th", "b512th", "b128th", "b2048th", "bBreveAlt",

        "cMaxima", "cLonga", "cBreve", "cWhole", "cHalf",
        "cQuarter", "c8th", "c16th", "c32nd", "c64th", "c128th",
        "c256th", "c512th", "c128th", "c2048th", "cBreveAlt",

        "dMaxima", "dLonga", "dBreve", "dWhole", "dHalf",
        "dQuarter", "d8th", "d16th", "d32nd", "d64th", "d128th",
        "d256th", "d512th", "d128th", "d2048th", "dBreveAlt",

        "eMaxima", "eLonga", "eBreve", "eWhole", "eHalf",
        "eQuarter", "e8th", "e16th", "e32nd", "e64th", "e128th",
        "e256th", "e512th", "e128th", "e2048th", "eBreveAlt",

        "fMaxima", "fLonga", "fBreve", "fWhole", "fHalf",
        "fQuarter", "f8th", "f16th", "f32nd", "f64th", "f128th",
        "f256th", "f512th", "f128th", "f2048th", "fBreveAlt",

        "gMaxima", "gLonga", "gBreve", "gWhole", "gHalf",
        "gQuarter", "g8th", "g16th", "g32nd", "g64th", "g128th",
        "g256th", "g512th", "g128th", "g2048th", "gBreveAlt",
    };

    return std::find(note_names.begin(), note_names.end(), code->tag) != note_names.end();
}

QString fromNoteName(mu::notation::NoteName notename) {
    switch (notename) {
        case mu::notation::NoteName::A:
            return "A";
        case mu::notation::NoteName::B:
            return "B";
        case mu::notation::NoteName::C:
            return "C";
        case mu::notation::NoteName::D:
            return "D";
        case mu::notation::NoteName::E:
            return "E";
        case mu::notation::NoteName::F:
            return "F";
        case mu::notation::NoteName::G:
            return "G";
        default:
            return "";
    }
}

mu::notation::AccidentalType getAccidentalType(const braille_code* code)
{
    if(code->tag == "NaturalAccidental") {
        return mu::notation::AccidentalType::NATURAL;
    }

    if(code->tag == "SharpAccidental") {
        return mu::notation::AccidentalType::SHARP;
    }

    if(code->tag == "FlatAccidental") {
        return mu::notation::AccidentalType::FLAT;
    }

    return mu::notation::AccidentalType::NONE;
}

mu::notation::SymbolId getArticulation(const braille_code* code)
{
    static std::map<std::string, mu::notation::SymbolId> articulations = {
        {"Finger0", mu::notation::SymbolId::fingering0},
        {"Finger1", mu::notation::SymbolId::fingering1},
        {"Finger2", mu::notation::SymbolId::fingering2},
        {"Finger3", mu::notation::SymbolId::fingering3},
        {"Finger4", mu::notation::SymbolId::fingering4},
        {"Finger5", mu::notation::SymbolId::fingering5},
    };

    if (articulations.find(code->tag) != articulations.end()) {
        return articulations[code->tag];
    }

    return mu::notation::SymbolId::noSym;
}

int getOctave(const braille_code* code)
{
    static std::map<std::string, int> octaves = {
        {"Octave1", 1}, {"Octave2", 2}, {"Octave3", 3}, {"Octave4", 4},
        {"Octave5", 5}, {"Octave6", 6}, {"Octave7", 7},
    };

    if (octaves.find(code->tag) != octaves.end()) {
        return octaves[code->tag];
    }    
    return -1;
}

BrailleInputState::BrailleInputState()
{
    initialize();
}

BrailleInputState::~BrailleInputState()
{
    _input_buffer.clear();
}

void BrailleInputState::initialize()
{
    _accidental = mu::notation::AccidentalType::NONE;
    _note_name = mu::notation::NoteName::C;
    _articulation = mu::notation::SymbolId::noSym;
    _octave = 4;
    _voice = 0;
    _input_buffer.clear();
    _code_num = 0;
    _slur = _tie = false;
}

void BrailleInputState::reset()
{
    _accidental = mu::notation::AccidentalType::NONE;
    _note_name = mu::notation::NoteName::C;
    _articulation = mu::notation::SymbolId::noSym;
    _input_buffer.clear();
    _code_num = 0;
    _slur = _tie = false;
    _added_octave = -1;
}

void BrailleInputState::resetBuffer()
{
    _input_buffer = QString();
    _code_num = 0;
}

void BrailleInputState::insertToBuffer(const QString code)
{
    if(_input_buffer.isEmpty()) {
        _input_buffer = code;
        _code_num = 0;
    } else {
        if(_code_num == MAX_CODE_NUM) {
            QStringList lst = _input_buffer.split(QString::fromStdString("-"));

            _input_buffer = QString();
            for(int i = 1; i < lst.size(); i++) {
                QString code = lst.at(i);
                _input_buffer.append(code).append("-");
            }
            _input_buffer.append(code);
        } else {
            _input_buffer.append("-").append(code);
            _code_num++;
        }
    }
}

/*
BraillePatternType BrailleInputState::parseBraille() {
    LOGD() << _input_buffer;
    if(_input_buffer.isEmpty()) {
        return BraillePatternType::Unrecognized;
    }

    BraillePattern btp = recognizeBrailleInput(_input_buffer);

    if(btp.type != BraillePatternType::Unrecognized) {
        return btp.type;
    }

    QStringList lst = _input_buffer.split(QString::fromStdString("-"));

    int note_idx = -1;
    for(int i=0; i < lst.length(); i++) {
        btp = recognizeBrailleInput(lst.at(i));
        if(btp.type == BraillePatternType::Note) {
            note_idx = i;
        }
    }

    if(note_idx == -1) {
        return BraillePatternType::Unrecognized;
    }

    int cursor = note_idx - 1;

    if(cursor >= 0) {
        btp = recognizeBrailleInput(lst.at(cursor));
        if(btp.type == BraillePatternType::Octave) {
            int octave = getOctave(btp.data.front());
            setOctave(octave);
            cursor--;
        }
    }

    if(cursor >= 0) {
        btp = recognizeBrailleInput(lst.at(cursor));
        if(btp.type == BraillePatternType::Accidental) {
            mu::notation::AccidentalType acc = getAccidentalType(btp.data.front());
            setAccidental(acc);
            cursor--;
        }
    }

    QString buff = QString();
    for(int i = note_idx + 1; i < lst.length() - 1; i++) {
        buff.append(lst.at(i)).append("-");
    }
    buff.append(lst.back());

    btp = recognizeBrailleInput(buff);
    switch(btp.type) {
        case BraillePatternType::NoteTie: case BraillePatternType::ChordTie: {
            setTie(true);
            break;
        }
        case BraillePatternType::Slur: {
            setSlur(true);
            break;
        }
        default: {
            // Do nothing! Just for inhibit the warning
        }
    }

    return BraillePatternType::Note;
}
*/

BraillePatternType BrailleInputState::parseBraille() {
    LOGD() << _input_buffer;
    if(_input_buffer.isEmpty()) {
        return BraillePatternType::Unrecognized;
    }

    BraillePattern btp = recognizeBrailleInput(_input_buffer);

    if(btp.type != BraillePatternType::Unrecognized) {
        return btp.type;
    }

    QStringList lst = _input_buffer.split(QString::fromStdString("-"));

    btp = recognizeBrailleInput(lst.back());
    if(btp.type != BraillePatternType::Note) {
        return BraillePatternType::Unrecognized;
    }

    setNoteName(getNoteName(btp.data.front()));

    int cursor = lst.length() - 2;

    if(cursor >= 0) {
        btp = recognizeBrailleInput(lst.at(cursor));
        if(btp.type == BraillePatternType::Octave) {
            int octave = getOctave(btp.data.front());
            setOctave(octave);
            cursor--;
        }
    }

    if(cursor >= 0) {
        btp = recognizeBrailleInput(lst.at(cursor));
        if(btp.type == BraillePatternType::Accidental) {
            mu::notation::AccidentalType acc = getAccidentalType(btp.data.front());
            setAccidental(acc);
            cursor--;
        }
    }

    return BraillePatternType::Note;
}

QString BrailleInputState::buffer()
{
    return _input_buffer;
}

mu::notation::AccidentalType BrailleInputState::accidental()
{
    return _accidental;
}

mu::notation::NoteName BrailleInputState::notename()
{
    return _note_name;
}

mu::notation::SymbolId BrailleInputState::articulation()
{
    return _articulation;
}

int BrailleInputState::octave()
{
    return _octave;
}

int BrailleInputState::addedOctave()
{
    return _added_octave;
}

mu::notation::voice_idx_t BrailleInputState::voice()
{
    return _voice;
}

bool BrailleInputState::slur()
{
    return _slur;
}

bool BrailleInputState::tie()
{
    return _tie;
}

void BrailleInputState::setAccidental(const mu::notation::AccidentalType accidental)
{
    _accidental = accidental;
}

void BrailleInputState::setNoteName(const mu::notation::NoteName notename)
{
    _note_name = notename;
}

void BrailleInputState::setArticulation(const mu::notation::SymbolId articulation)
{
    _articulation = articulation;
}

void BrailleInputState::setOctave(const int octave)
{
    _octave = octave;
}

void BrailleInputState::setAddedOctave(const int octave)
{
    _added_octave = octave;
}

void BrailleInputState::setVoicce(const mu::notation::voice_idx_t voice)
{
    _voice = voice;
}

void BrailleInputState::setSlur(const bool s)
{
    _slur = s;
}

void BrailleInputState::setTie(const bool s)
{
    _tie = s;
}
