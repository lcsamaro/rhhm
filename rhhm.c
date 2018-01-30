#include "rhhm.h"

#include <stdlib.h>
#include <string.h>

/* TODO: resizing */

static uint32_t djb2(const unsigned char *s) {
    uint32_t hash = 5381;
    int c;
    while (c = *s++) hash = ((hash << 5) + hash) + c;
    return hash;
}

static int rhhm_value_empty(struct rhhm_value *v) {
	return !v->value && !v->hash;
}

#define DISTANCE(i, h) (h >= i ? h-i : h + (hm->length - i))

int rhhm_init(struct rhhm *hm, uint32_t length) {
	hm->length = length;
	hm->table = malloc(hm->length * sizeof(struct rhhm_value));
	if (!hm->table) return 1;
	memset(hm->table, 0, hm->length * sizeof(struct rhhm_value));
	return 0;
}

void rhhm_destroy(struct rhhm *hm) {
	if (hm) free(hm->table);
}

void rhhm_insert(struct rhhm *hm, const char *key, int32_t value) {
	uint32_t h = djb2(key) % hm->length;

	struct rhhm_value entry, tmp;
	entry.key = key;
	entry.value = value;
	entry.hash = h;

	uint32_t i = entry.hash;
	while (DISTANCE(i, entry.hash) <= DISTANCE(i, hm->table[i].hash)) {
		if (rhhm_value_empty(hm->table+i)) break;
		/* this is only needed on first entry, we know other entries are not repeated */
		if (hm->table[i].hash == entry.hash &&
			!strcmp(hm->table[i].key, entry.key)) {
			hm->table[i].value = entry.value; /* replace */
			return;
		}
		if (++i >= hm->length) i = 0;
	}
	// if distance of entry > than distance at current then swap
	tmp = entry;
	entry = hm->table[i];
	hm->table[i] = tmp;

	while (!rhhm_value_empty(&entry)) {
		uint32_t i = entry.hash;
		while (DISTANCE(i, entry.hash) <= DISTANCE(i, hm->table[i].hash)) {
			if (rhhm_value_empty(hm->table+i)) break;
			if (++i >= hm->length) i = 0;
		}
		// if distance of entry > than distance at current then swap
		tmp = entry;
		entry = hm->table[i];
		hm->table[i] = tmp;
	}
}

int32_t rhhm_get(struct rhhm *hm, const char *key) {
	uint32_t h = djb2(key) % hm->length;
	uint32_t i = h;
	while (!rhhm_value_empty(hm->table+i)) {
		if (!strcmp(hm->table[i].key, key)) return hm->table[i].value;
		if (hm->table[i].hash < h) return -1;
		if (++i >= hm->length) i = 0;
	}
	return -1;
}

void rhhm_remove(struct rhhm *hm, const char *key) {
	uint32_t h = djb2(key) % hm->length;
	uint32_t i = h;
	while (!rhhm_value_empty(hm->table+i)) {
		if (!strcmp(hm->table[i].key, key)) {
			uint32_t j = i;
			do {
				if (++j >= hm->length) j = 0;
				hm->table[i] = hm->table[j];
				i = j;
			} while (!rhhm_value_empty(hm->table+j) &&
               DISTANCE(j, hm->table[j].hash) != 0);
			return;
		}
		if (hm->table[i].hash < h) return;
		if (++i >= hm->length) i = 0;
	}
}

void rhhm_visit(struct rhhm *hm, rhhm_visit_callback callback) {
	uint32_t i;
	for (i = 0; i < hm->length; i++)
		if (!rhhm_value_empty(hm->table + i))
			callback(hm->table + i);
}
