#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include <string.h>
#include "db.h"

static sqlite3 *db = NULL;

int open_db(const char *filename) {
    return sqlite3_open(filename, &db);
}

void close_db() {
    if (db) sqlite3_close(db);
}

void insert_region_autocommit() {
    const char *sql = "INSERT INTO region (oblast_name, oblast_center, population_total, population_men, population_women, territory_area, head_of_administration, country_id) VALUES ('АвтоКоммитская область', 'АвтоГород', 100000, 48000, 52000, 12345, 'АвтоНачальник', 1);";
    char *errmsg = 0;
    if (sqlite3_exec(db, sql, 0, 0, &errmsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errmsg);
        sqlite3_free(errmsg);
    } else {
        printf("Вставка в режиме autocommit выполнена.\n");
    }
}

void insert_region_transaction() {
    char *errmsg = 0;
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, &errmsg);

    const char *sql1 = "INSERT INTO region (oblast_name, oblast_center, population_total, population_men, population_women, territory_area, head_of_administration, country_id) VALUES ('Транзактовская область', 'ТранзГород', 200000, 95000, 105000, 54321, 'ТранзНачальник', 2);";
    sqlite3_exec(db, sql1, 0, 0, &errmsg);

    sqlite3_exec(db, "COMMIT;", 0, 0, &errmsg);
    printf("Вставка в транзакции выполнена.\n");
}

void select_by_id(int id) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, oblast_name, oblast_center, population_total, population_men, population_women, territory_area, head_of_administration, country_id FROM region WHERE id = ?";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("ID: %d\n", sqlite3_column_int(stmt, 0));
            printf("Область: %s\n", sqlite3_column_text(stmt, 1));
            printf("Центр: %s\n", sqlite3_column_text(stmt, 2));
            printf("Население: %d (муж: %d, жен: %d)\n",
                   sqlite3_column_int(stmt, 3),
                   sqlite3_column_int(stmt, 4),
                   sqlite3_column_int(stmt, 5));
            printf("Площадь: %.1f км²\n", sqlite3_column_double(stmt, 6));
            printf("Руководитель: %s\n", sqlite3_column_text(stmt, 7));
            printf("ID страны: %d\n", sqlite3_column_int(stmt, 8));
        } else {
            printf("Запись с ID %d не найдена.\n", id);
        }
        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
    }
}

void select_by_pattern(const char *field, const char *pattern) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT id, oblast_name, oblast_center FROM region WHERE %s LIKE ?", field);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        char like_pattern[128];
        snprintf(like_pattern, sizeof(like_pattern), "%%%s%%", pattern);
        sqlite3_bind_text(stmt, 1, like_pattern, -1, SQLITE_STATIC);

        int found = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            found = 1;
            printf("ID: %d | %s (%s)\n",
                   sqlite3_column_int(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2));
        }

        if (!found) {
            printf("Ничего не найдено.\n");
        }

        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
    }
}

void select_by_field(const char *field, const char *value) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT id, oblast_name, oblast_center FROM region WHERE %s = ?", field);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, value, -1, SQLITE_STATIC);

        int found = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            found = 1;
            printf("ID: %d | %s (%s)\n",
                   sqlite3_column_int(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_text(stmt, 2));
        }

        if (!found) {
            printf("Ничего не найдено.\n");
        }

        sqlite3_finalize(stmt);
    } else {
        fprintf(stderr, "Ошибка подготовки запроса: %s\n", sqlite3_errmsg(db));
    }
}

void export_country_photo_by_id(int id, const char *filename) {
    const char *sql = "SELECT photo FROM country WHERE id = ?";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const void *data = sqlite3_column_blob(stmt, 0);
            int size = sqlite3_column_bytes(stmt, 0);

            FILE *f = fopen(filename, "wb");
            if (f) {
                fwrite(data, 1, size, f);
                fclose(f);
                printf("Фото экспортировано в %s\n", filename);
            }
        }
    }
    sqlite3_finalize(stmt);
}

void insert_country_photo(const char *filename, int country_id) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Cannot open image file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    unsigned char *buffer = malloc(size);
    fread(buffer, 1, size, file);
    fclose(file);

    const char *sql = "UPDATE country SET photo = ? WHERE id = ?";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_blob(stmt, 1, buffer, size, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, country_id);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Ошибка вставки фото в country: %s\n", sqlite3_errmsg(db));
        } else {
            printf("Фото вставлено в страну с id = %d\n", country_id);
        }
    }
    sqlite3_finalize(stmt);
    free(buffer);
}