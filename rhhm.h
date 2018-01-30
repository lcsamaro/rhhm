#ifndef Q4_RHHM_H
#define Q4_RHHM_H

/* robin hood hashmap data structure */

#include <stdint.h>

struct rhhm_value {
	const char *key;
	int32_t     value;
	uint32_t    hash;
};

struct rhhm {
	struct rhhm_value *table;
	uint32_t length;
};

int     rhhm_init(struct rhhm *hm, uint32_t length);
void    rhhm_destroy(struct rhhm *hm);

void    rhhm_insert(struct rhhm *hm, const char *key, int32_t value);
int32_t rhhm_get(struct rhhm *hm, const char *key);
void    rhhm_remove(struct rhhm *hm, const char *key);

typedef void(*rhhm_visit_callback)(const struct rhhm_value *value);
void rhhm_visit(struct rhhm *hm, rhhm_visit_callback callback);

#endif
