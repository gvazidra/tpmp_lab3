#ifndef DB_H
#define DB_H

int open_db(const char *filename);
void close_db();

void insert_region_autocommit();
void insert_region_transaction();
void select_by_id(int id);
void select_by_pattern(const char *field, const char *pattern);
void select_by_field(const char *field, const char *value);
void export_country_photo_by_id(int id, const char *filename);
void insert_country_photo(const char *filename, int country_id);

#endif