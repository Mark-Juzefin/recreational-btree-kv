//
// Created by Illia on 18.04.2026.
//


#include "person.h"

#include <stdlib.h>
#include <string.h>


kv_result_t person_create(const char *name, const char *email, person_t **out) {
    if (name == NULL || email == NULL || out == NULL) {
        return KV_ERR_ARG;
    }

    person_t *p = malloc(sizeof(*p));
    if (p == NULL) {
        return KV_ERR_NOMEM; // нічого ще не виділено — простий return
    }

    p->name = strdup(name); // strdup = malloc + strcpy
    if (p->name == NULL) {
        goto fail_after_struct; // виділено p; звільнити p і вийти
    }

    p->email = strdup(email);
    if (p->email == NULL) {
        goto fail_after_name; // виділено p + name; звільнити обидва
    }

    *out = p;
    return KV_OK;


fail_after_name:
    free(p->name);
fail_after_struct:
    free(p);
    return KV_ERR_NOMEM;
}

void person_free(person_t *p) {
    if (p == NULL) return;
    free(p->name);            // free(NULL) безпечний, тому if не потрібен
    free(p->email);
    free(p);
}

kv_result_t person_rename(person_t *p, const char *new_name) {
    if (p == NULL || new_name == NULL) {
        return KV_ERR_ARG;
    }

    char *dup = strdup(new_name);
    if (dup == NULL) {
        return KV_ERR_NOMEM;   // p->name лишається недоторканим — важлива гарантія
    }

    free(p->name);
    p->name = dup;
    return KV_OK;
}

const char *person_name(const person_t *p) {
    return p->name;            // просто віддаємо внутрішній вказівник
}
