#ifndef LOUIS_HPP_INCLUDED
#define LOUIS_HPP_INCLUDED

#include <string>

extern std::string table_ascii_to_unicode;
extern std::string table_unicode_to_ascii;
extern std::string table_for_literature;
//extern std::string table_for_general;

std::string get_louis_version();
std::string braille_translate(const char *table_name, std::string txt);
int check_tables(const char *tables);
char * setTablesDir(const char *tablesdir);
char * getTablesDir();

std::string braille_long_translate(const char *table_name, std::string txt);
std::string braille_multi_line_translate(const char *table_name, std::string txt);
int get_braille_text_length(const char *table_name, std::string txt);

#endif // LOUIS_HPP_INCLUDED
