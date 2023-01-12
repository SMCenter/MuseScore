#ifndef MU_NOTATION_LIVEBRAILLE_LOUIS_H
#define MU_NOTATION_LIVEBRAILLE_LOUIS_H

#include <string>

std::string get_louis_version();
std::string braille_translate(const char* table_name, std::string txt);
int check_tables(const char* tables);
char* setTablesDir(const char* tablesdir);
char* getTablesDir();

#endif // MU_NOTATION_LIVEBRAILLE_LOUIS_H
