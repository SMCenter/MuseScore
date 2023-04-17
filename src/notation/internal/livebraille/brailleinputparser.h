#ifndef BRAILLEINPUTPARSER_H
#define BRAILLEINPUTPARSER_H

#include "braille.h"
#include "notationtypes.h"

namespace mu::notation {

struct BiePattern {
    std::string name;
    std::vector<braille_code*> codes;
    bool mandatory = false;
};

enum class BieSequencePatternType {
    Undefined,
    Note,
    Interval,
    Rest,
    Tuplet3,
    Tuplet,    
    Dot,
    Tie,
    NoteSlur,
    LongSlurStop
};

class BieSequencePattern {
public:
    BieSequencePattern(BieSequencePatternType t, std::string sequence);
    ~BieSequencePattern();

    BieSequencePatternType type();
    bool recognize(std::string braille);
    std::map<std::string, braille_code *> res();
    braille_code* res(std::string key);
    bool valid();
private:
    BieSequencePatternType _type;
    std::vector<BiePattern> patterns;
    bool _valid;
    std::map<std::string, braille_code *> _res;
    int max_cell_length;
    int _mandatories = 0;
};

BieSequencePattern* BieRecognize(std::string braille);

}

#endif // BRAILLEINPUTPARSER_H
