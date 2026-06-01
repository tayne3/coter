#include <stdio.h>
#include <stdlib.h>

#include "coter/ini/ini.h"

int main(void) {
    ct_ini_t* ini = ct_ini_empty();
    if (!ini) {
        fprintf(stderr, "Failed to create ini object\n");
        return 1;
    }

    ct_ini_section_t* section = ct_ini_get_section(ini, "section");
    ct_ini_section_add_key(section, "key", "1");
    ct_ini_section_add_key(section, "enabled", "true");
    ct_ini_section_add_key(section, "timeout", "30.5");
    ct_ini_section_add_key(section, "name", "coter");

    ct_ini_key_t* key = ct_ini_get_key(ini, "section", "key");
    printf("Int value: %d\n", ct_ini_key_get_int(key, 0));
    printf("Bool value: %d\n", ct_ini_key_get_bool(ct_ini_find_key(ini, "section", "enabled"), false));
    ct_ini_section_t* s = ct_ini_find_section(ini, "section");
    printf("Double value: %.1f\n", ct_ini_key_get_double(ct_ini_section_find_key(s, "timeout"), 0.0));
    printf("String value: %s\n", ct_ini_key_get_string(ct_ini_find_key(ini, "section", "name"), "default"));

    size_t section_count = 0;
    for (ct_ini_section_t* s = ct_ini_first_section(ini); s; s = ct_ini_section_next(s)) { section_count++; }
    printf("\nTotal sections: %zu\n", section_count);

    for (ct_ini_section_t* s = ct_ini_first_section(ini); s; s = ct_ini_section_next(s)) {
        printf("[%s]\n", ct_ini_section_name(s));
        for (ct_ini_key_t* k = ct_ini_section_first_key(s); k; k = ct_ini_key_next(k)) {
            printf("  %s = %s\n", ct_ini_key_name(k), ct_ini_key_get_value(k));
        }
    }

    ct_ini_destroy(ini);
    return 0;
}
