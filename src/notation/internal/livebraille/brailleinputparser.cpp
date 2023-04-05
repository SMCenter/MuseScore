#include "brailleinputparser.h"
#include <log.h>

namespace mu::notation {

static BiePattern pattern_accord = {"accord", {&Braille_FullMeasureAccord}};

static BiePattern pattern_accidentals = {"accidental",
    {&Braille_NaturalAccidental, &Braille_SharpAccidental, &Braille_FlatAccidental}};

static BiePattern pattern_octaves = {"octave",
    {&Braille_Octave0, &Braille_Octave1, &Braille_Octave2,
     &Braille_Octave3, &Braille_Octave4, &Braille_Octave5,
     &Braille_Octave6, &Braille_Octave7, &Braille_Octave8}};


static BiePattern pattern_notes ={"note",
    {&Braille_aWhole, &Braille_aHalf, &Braille_aQuarter, &Braille_a8th,
     &Braille_bWhole, &Braille_bHalf, &Braille_bQuarter, &Braille_b8th,
     &Braille_cWhole, &Braille_cHalf, &Braille_cQuarter, &Braille_c8th,
     &Braille_dWhole, &Braille_dHalf, &Braille_dQuarter, &Braille_d8th,
     &Braille_eWhole, &Braille_eHalf, &Braille_eQuarter, &Braille_f8th,
     &Braille_fWhole, &Braille_fHalf, &Braille_fQuarter, &Braille_e8th,
     &Braille_gWhole, &Braille_gHalf, &Braille_gQuarter, &Braille_g8th}};

static BiePattern pattern_dot = {"dot", {&Braille_Dot}};
static BiePattern pattern_tie = {"tie", {&Braille_NoteTie}};
static BiePattern pattern_note_slur = {"note-slur", {&Braille_NoteSlur}};
static BiePattern pattern_slur_start = {"slur-start", {&Braille_LongSlurOpenBracket}};
static BiePattern pattern_slur_stop = {"slur-stop", {&Braille_LongSlurCloseBracket}};

static BiePattern pattern_rests ={"rest",
    {&Braille_RestWhole, &Braille_RestHalf, &Braille_RestQuarter, &Braille_Rest8th}};

static BiePattern pattern_intervals ={"interval",
    {&Braille_Interval2, &Braille_Interval3, &Braille_Interval4, &Braille_Interval5,
     &Braille_Interval6, &Braille_Interval7, &Braille_Interval8}};

static BiePattern pattern_fingerings ={"fingering",
    {&Braille_Finger0, &Braille_Finger1, &Braille_Finger2,
     &Braille_Finger3, &Braille_Finger4, &Braille_Finger5}};

static BiePattern pattern_tuplet3 ={"tuplet3", {&Braille_Tuplet3}};
static BiePattern pattern_tuplet_prefix ={"tuplet-prefix", {&Braille_TupletPrefix}};

static BiePattern pattern_numbers ={"number",
    {&Braille_Upper0, &Braille_Upper1, &Braille_Upper2, &Braille_Upper3, &Braille_Upper4,
     &Braille_Upper5, &Braille_Upper6, &Braille_Upper7, &Braille_Upper8, &Braille_Upper9}};

static int maxPatternLength(BiePattern* pattern)
{
    int max = 0;
    for(auto code : pattern->codes) {
        if(code->cells_num > max) {
            max = code->cells_num;
        }
    }
    return max;
}

static int matchPattern(BiePattern* pattern, std::string braille)
// result: 0: doesn't match, otherwise return num braille cells of match
{
    for(auto code : pattern->codes) {
        if(code->braille == braille) {
            return code->cells_num;
        }
    }
    return 0;
}

BieSequencePattern::BieSequencePattern(BieSequencePatternType t, std::string sequence)
{
    _type = t;
    patterns.clear();
    max_cell_length = 0;

    int len = sequence.length();
    QString key = QString();
    bool mandatory = false;
    char openning = ' ';
    int i = 0;
    while(i < len) {
        char c = sequence.at(i);
        //LOGD() << i << " c=" << c << " open=" << openning;
        switch(c) {
        case '(': {
            if(openning != ' ') {
                _valid = false;
                return;
            }
            mandatory = true;
            openning = '(';
            break;
        }
        case ')': {
            if(openning != '(') {
                _valid = false;
                return;
            }
            openning = ' ';
            break;
        }
        case '[': {
            if(openning != ' ') {
                _valid = false;
                return;
            }
            mandatory = false;
            openning = '[';
            break;
        }
        case ']': {
            if(openning != '[') {
                _valid = false;
                return;
            }
            openning = ' ';
            break;
        }
        case '{': {
            // ignore
            break;
        }
        case '}': {
            // ignore
            break;
        }
        default: {
            key.push_back(sequence.at(i));
            break;
        }
        }

        if(openning == ' ' && !key.isEmpty()) {
            LOGD() << key << " " << mandatory;
            BiePattern * pattern = NULL;
            if(key == "accord") {
                pattern = &pattern_accord;
            } else if(key == "accord") {
                pattern = &pattern_accord;
            } else if(key == "accidental") {
                pattern = &pattern_accidentals;
            } else if(key == "octave") {
                pattern = &pattern_octaves;
            } else if(key == "note") {
                pattern = &pattern_notes;
            } else if(key == "dot") {
                pattern = &pattern_dot;
            } else if(key == "tie") {
                pattern = &pattern_tie;
            } else if(key == "note-slur") {
                pattern = &pattern_note_slur;
            } else if(key == "slur-start") {
                pattern = &pattern_slur_start;
            } else if(key == "slur-stop") {
                pattern = &pattern_slur_stop;
            } else if(key == "rest") {
                pattern = &pattern_rests;
            } else if(key == "interval") {
                pattern = &pattern_intervals;
            } else if(key == "fingering") {
                pattern = &pattern_fingerings;
            } else if(key == "tuplet3") {
                pattern = &pattern_tuplet3;
            } else if(key == "tuplet-prefix") {
                pattern = &pattern_tuplet_prefix;
            } else if(key == "number") {
                pattern = &pattern_numbers;
            }
            if(pattern != NULL) {
                patterns.push_back({pattern->name, pattern->codes, mandatory});
                int n = maxPatternLength(pattern);
                if(max_cell_length < n ) {
                    max_cell_length = n;
                }
            }
            mandatory = false;
            key = QString();
        }
        i++;
    }
    _valid = true;
}

BieSequencePattern::~BieSequencePattern()
{
    // TODO
}

BieSequencePatternType BieSequencePattern::type()
{
    return _type;
}

bool BieSequencePattern::recognize(std::string braille)
{
    _res.clear();

    int pos = 0;
    size_t cursor = 0;

    while(cursor < braille.length()) {
        //LOGD() << "cursor: " << cursor;
        bool match = false;
        for(auto code : patterns[pos].codes) {
            code->print();
            int len = code->cells_num;
            if(cursor + len <= braille.length()) {
                std::string bxt = braille.substr(cursor, len);
                if(bxt == code->braille) {
                    match = true;
                    _res[patterns[pos].name] = code;
                    pos++;
                    cursor += len;                    
                    break;
                }
            }
        }
        if(!match) {
            if(patterns[pos].mandatory) {
                return false;
            } else {
                pos++;
            }
        }
    }
    for(std::map<std::string, braille_code *>::iterator it = _res.begin(); it != _res.end(); ++it) {
        LOGD() << "Key: " << it->first << " value: " << it->second->tag;
    }
    return true;
}

std::map<std::string, braille_code*> BieSequencePattern::res()
{
    return _res;
}

braille_code* BieSequencePattern::res(std::string key)
{
    return _res[key];
}

bool BieSequencePattern::valid()
{
    return _valid;
}

BieSequencePattern* BieRecognize(std::string braille) {
    static std::string note_input_seq = "{[accidental][octave](note)[dot][fingering][note-slur][slur-start][slur-end][tie]}";
    static BieSequencePattern bie_note_input(BieSequencePatternType::Note, note_input_seq);

    static std::string rest_input_seq = "{(rest)[dot][slur]}";
    static BieSequencePattern bie_rest_input(BieSequencePatternType::Rest, rest_input_seq);

    static std::string interval_input_seq = "{[accidental][octave](interval)[fingering][slur-start][slur-end][tie]}";
    static BieSequencePattern bie_interval_input(BieSequencePatternType::Interval, interval_input_seq);

    static std::string accord_seq = "{(accord)}";
    static BieSequencePattern bie_accord(BieSequencePatternType::Accord, accord_seq);

    static std::string tuplet3_seq = "{(tuplet3)}";
    static BieSequencePattern bie_tuplet3(BieSequencePatternType::Tuplet3, tuplet3_seq);

    static std::string tuplet_seq = "{(tuplet-prefix)(number)}";
    static BieSequencePattern bie_tuplet(BieSequencePatternType::Tuplet, tuplet_seq);

    BieSequencePattern* res = NULL;

    if(bie_note_input.valid() && bie_note_input.recognize(braille)) {
        res = &bie_note_input;
    } else if(bie_rest_input.valid() && bie_rest_input.recognize(braille)) {
        res = &bie_rest_input;
    } else if(bie_interval_input.valid() && bie_interval_input.recognize(braille)) {
        res = &bie_interval_input;
    } else if(bie_accord.valid() && bie_accord.recognize(braille)) {
        res = &bie_accord;
    } else if(bie_tuplet3.valid() && bie_tuplet3.recognize(braille)) {
        res = &bie_tuplet3;
    } else if(bie_tuplet.valid() && bie_tuplet.recognize(braille)) {        
        res = &bie_tuplet;
    }

    return res;
}
}
