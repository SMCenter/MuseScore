#include "brailleinput.h"
#include "braille.h"

#include "log.h"

using namespace mu::notation;

namespace mu::notation {

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

    code = findRest(p);
    if(code != NULL) {
        res.type = BraillePatternType::Rest;
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

NoteName getNoteName(const braille_code* code)
{
    char c = code->tag[0];
    switch (c) {
        case 'A': case 'a':
            return NoteName::A;
        case 'B': case 'b':
            return NoteName::B;
        case 'C': case 'c':
            return NoteName::C;
        case 'D': case 'd':
            return NoteName::D;
        case 'E': case 'e':
            return NoteName::E;
        case 'F': case 'f':
            return NoteName::F;
        case 'G': case 'g':
            return NoteName::G;
    }
    return NoteName::C;
}

std::vector<DurationType> getNoteDurations(const braille_code* code)
{
    for(int i=0; i < 7; i++) {
        if(code->code == Braille_wholeNotes[i]->code) {
            //return {DurationType::V_WHOLE, DurationType::V_16TH, DurationType::V_256TH};
            return {DurationType::V_WHOLE, DurationType::V_16TH};
        }
    }

    for(int i=0; i < 7; i++) {
        if(code->code == Braille_halfNotes[i]->code) {
            //return {DurationType::V_HALF, DurationType::V_32ND, DurationType::V_512TH};
            return {DurationType::V_HALF, DurationType::V_32ND};
        }
    }

    for(int i=0; i < 7; i++) {
        if(code->code == Braille_quarterNotes[i]->code) {
            //return {DurationType::V_QUARTER, DurationType::V_64TH, DurationType::V_1024TH};
            return {DurationType::V_QUARTER, DurationType::V_64TH};
        }
    }

    for(int i=0; i < 7; i++) {
        if(code->code == Braille_8thNotes[i]->code) {
            return {DurationType::V_EIGHTH, DurationType::V_128TH};
        }
    }

    return {};
}

std::vector<DurationType> getRestDurations(const braille_code* code)
{
    if(code->code == Braille_RestWhole.code) {
        return {DurationType::V_WHOLE, DurationType::V_16TH};
    }

    if(code->code == Braille_RestHalf.code) {
        return {DurationType::V_HALF, DurationType::V_32ND};
    }

    if(code->code == Braille_RestQuarter.code) {
        return {DurationType::V_QUARTER, DurationType::V_64TH};
    }

    if(code->code == Braille_Rest8th.code) {
        return {DurationType::V_EIGHTH, DurationType::V_128TH};
    }

    return {};
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

QString fromNoteName(NoteName notename) {
    switch (notename) {
        case NoteName::A:
            return "A";
        case NoteName::B:
            return "B";
        case NoteName::C:
            return "C";
        case NoteName::D:
            return "D";
        case NoteName::E:
            return "E";
        case NoteName::F:
            return "F";
        case NoteName::G:
            return "G";
        default:
            return "";
    }
}

AccidentalType getAccidentalType(const braille_code* code)
{
    if(code->tag == "NaturalAccidental") {
        return AccidentalType::NATURAL;
    }

    if(code->tag == "SharpAccidental") {
        return AccidentalType::SHARP;
    }

    if(code->tag == "FlatAccidental") {
        return AccidentalType::FLAT;
    }

    return AccidentalType::NONE;
}

SymbolId getArticulation(const braille_code* code)
{
    static std::map<std::string, SymbolId> articulations = {
        {"Finger0", SymbolId::fingering0},
        {"Finger1", SymbolId::fingering1},
        {"Finger2", SymbolId::fingering2},
        {"Finger3", SymbolId::fingering3},
        {"Finger4", SymbolId::fingering4},
        {"Finger5", SymbolId::fingering5},
    };

    if (articulations.find(code->tag) != articulations.end()) {
        return articulations[code->tag];
    }

    return SymbolId::noSym;
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
    LOGD() << "...";
    _accidental = AccidentalType::NONE;
    _note_name = NoteName::C;
    _articulation = SymbolId::noSym;
    _octave = 4;
    _voice = 0;
    _input_buffer.clear();
    _code_num = 0;
    _slur = _tie = false;
    _note_group = NoteGroup::Group1;
}

void BrailleInputState::reset()
{
    _accidental = AccidentalType::NONE;
    _note_name = NoteName::C;
    _articulation = SymbolId::noSym;
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
            AccidentalType acc = getAccidentalType(btp.data.front());
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

    if(btp.type != BraillePatternType::Unrecognized
       && btp.type != BraillePatternType::Note
       && btp.type != BraillePatternType::Rest) {
        return btp.type;
    }

    QStringList lst = _input_buffer.split(QString::fromStdString("-"));

    btp = recognizeBrailleInput(lst.back());

    switch (btp.type) {
    case BraillePatternType::Note: {
        setNoteName(getNoteName(btp.data.front()));
        setNoteDurations(getNoteDurations(btp.data.front()));

        int cursor = lst.length() - 2;

        if(cursor >= 0) {
            btp = recognizeBrailleInput(lst.at(cursor));
            if(btp.type == BraillePatternType::Octave) {
                int octave = getOctave(btp.data.front());
                setAddedOctave(octave);
                cursor--;
            }
        }

        if(cursor >= 0) {
            btp = recognizeBrailleInput(lst.at(cursor));
            if(btp.type == BraillePatternType::Accidental) {
                AccidentalType acc = getAccidentalType(btp.data.front());
                setAccidental(acc);
                cursor--;
            }
        }

        return BraillePatternType::Note;
    }
    case BraillePatternType::Rest: {
        setNoteDurations(getRestDurations(btp.data.front()));
        return BraillePatternType::Rest;
        break;
    }
    default: {
        return BraillePatternType::Unrecognized;
    }
    }
}

QString BrailleInputState::buffer()
{
    return _input_buffer;
}

AccidentalType BrailleInputState::accidental()
{
    return _accidental;
}

NoteName BrailleInputState::noteName()
{
    return _note_name;
}

NoteGroup BrailleInputState::noteGroup()
{
    return _note_group;
}

DurationType BrailleInputState::currentDuration()
{
    return _current_duration;
}

std::vector<DurationType> BrailleInputState::noteDurations()
{
    return _note_durations;
}

bool BrailleInputState::isDurationMatch()
{
    return std::find(_note_durations.begin(), _note_durations.end(), _current_duration) != _note_durations.end();
}

DurationType BrailleInputState::getCloseDuration()
{    
    switch(_note_group) {
        case NoteGroup::Group1: {
            return _note_durations[0];
            break;
        }
        case NoteGroup::Group2: {
            return _note_durations[1];
            break;
        }
        case NoteGroup::Group3: {
            return _note_durations[2];
            break;
        }
        default: {
            if(isDurationMatch()) {
                return _current_duration;
            } else {
                for(auto d : _note_durations) {
                    if(d > _current_duration) {
                        return d;
                    }
                }
            }
        }
     }
    return _note_durations.back();
}

SymbolId BrailleInputState::articulation()
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

voice_idx_t BrailleInputState::voice()
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

void BrailleInputState::setAccidental(const AccidentalType accidental)
{
    _accidental = accidental;
}

void BrailleInputState::setNoteName(const NoteName notename)
{
    _note_name = notename;
}

void BrailleInputState::setNoteGroup(const NoteGroup notegroup)
{
    LOGD() << (int) notegroup;
    _note_group = notegroup;
}

void BrailleInputState::setCurrentDuration(const DurationType duration)
{
    _current_duration = duration;
}

void BrailleInputState::setNoteDurations(const std::vector<DurationType> durations)
{
    _note_durations = durations;
}

void BrailleInputState::setArticulation(const SymbolId articulation)
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

void BrailleInputState::setVoicce(const voice_idx_t voice)
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
}
