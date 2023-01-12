#ifndef LOUIS_HPP_INCLUDED
#define LOUIS_HPP_INCLUDED

#include <string>

std::string get_louis_version();
std::string braille_translate(const char *table_name, std::string txt);
int check_tables(const char *tables);
char * setTablesDir(const char *tablesdir);
char * getTablesDir();

#endif // LOUIS_HPP_INCLUDED
