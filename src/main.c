#include <stdio.h>
#include <stdlib.h>
#include "db.h"

int main() {
    if (open_db("regions.db") != 0) {
        fprintf(stderr, "Не удалось открыть БД\n");
        return 1;
    }

    // Вставляем фото стран
    insert_country_photo("data/bel.jpg", 1);
    insert_country_photo("data/ua.jpg", 2);
    insert_country_photo("data/rus.jpg", 3);

    int choice, id;
    char input[128];

    while (1) {
        printf("\nМеню:\n");
        printf("1. Выборка по ID\n");
        printf("2. Выборка по шаблону\n");
        printf("3. Выборка по точному полю\n");
        printf("4. Вставка региона (autocommit)\n");
        printf("5. Вставка региона (транзакция)\n");
        printf("6. Вставка фото в страну\n");
        printf("7. Экспорт фото из страны\n");
        printf("8. Выход\n> ");

        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                printf("Введите ID: ");
                scanf("%d", &id);
                select_by_id(id);
                break;
            case 2:
                printf("Введите поле и шаблон (через пробел): ");
                scanf("%s %s", input, input + 64);
                select_by_pattern(input, input + 64);
                break;
            case 3:
                printf("Введите поле и значение (через пробел): ");
                scanf("%s %s", input, input + 64);
                select_by_field(input, input + 64);
                break;
            case 4:
                insert_region_autocommit();
                break;
            case 5:
                insert_region_transaction();
                break;
            case 6:
                printf("Введите ID страны и путь к фото: ");
                scanf("%d %s", &id, input);
                insert_country_photo(input, id);
                break;
            case 7:
                printf("Введите ID страны и путь для сохранения: ");
                scanf("%d %s", &id, input);
                export_country_photo_by_id(id, input);
                break;
            case 8:
                close_db();
                return 0;
        }
    }
}