/* Written by Kris Maglione */
/* Public domain */
#include <stdlib.h>
#include "ixp_local.h"

/* Edit s/^([a-zA-Z].*)\n([a-z].*) {/\1 \2;/g  x/^([^a-zA-Z]|static|$)/-+d  s/ (\*map|val|*str)//g */

struct MapEnt {
	ulong		hash;
	const char*	key;
	void*		val;
	MapEnt*		next;
};

MapEnt *NM;

static void
insert(MapEnt **e, ulong val, const char *key) {
	MapEnt *te;
	
	te = (MapEnt*)ixp::emallocz(sizeof *te);
	te->hash = val;
	te->key = key;
	te->next = *e;
	*e = te;
}

static MapEnt**
map_getp(IxpMap *map, ulong val, bool create, bool *exists) {
	MapEnt **e;

	e = &map->bucket[val%map->nhash];
	for(; *e; e = &(*e)->next)
		if((*e)->hash >= val) break;
	if(exists)
		*exists = *e && (*e)->hash == val;

    if (!(*e) || (*e)->hash != val) {
		if(create)
			insert(e, val, nullptr);
		else
			e = &NM;
	}
	return e;
}

void
ixp_mapfree(IxpMap *map, std::function<void(void*)> destroy) {
	int i;
	MapEnt *e;

	thread->wlock(&map->lock);
	for(i=0; i < map->nhash; i++)
		while((e = map->bucket[i])) {
			map->bucket[i] = e->next;
			if(destroy)
				destroy(e->val);
			free(e);
		}
	thread->wunlock(&map->lock);
	thread->rwdestroy(&map->lock);
}

void
ixp_mapexec(IxpMap *map, std::function<void(void*, void*)> run, void* context) {
	int i;
	MapEnt *e;

	thread->rlock(&map->lock);
	for(i=0; i < map->nhash; i++)
		for(e=map->bucket[i]; e; e=e->next)
			run(context, e->val);
	thread->runlock(&map->lock);
}

void
ixp_mapinit(IxpMap *map, MapEnt **buckets, int nbuckets) {

	map->bucket = buckets;
	map->nhash = nbuckets;

	thread->initrwlock(&map->lock);
}

bool
ixp_mapinsert(IxpMap *map, ulong key, void *val, bool overwrite) {
	MapEnt *e;
	bool existed, res;
	
	res = true;
	thread->wlock(&map->lock);
	e = *map_getp(map, key, true, &existed);
	if(existed && !overwrite)
		res = false;
	else
		e->val = val;
	thread->wunlock(&map->lock);
	return res;
}

void*
ixp_mapget(IxpMap *map, ulong val) {
	MapEnt *e;
	void *res;
	
	thread->rlock(&map->lock);
	e = *map_getp(map, val, false, nullptr);
	res = e ? e->val : nullptr;
	thread->runlock(&map->lock);
	return res;
}

void*
ixp_maprm(IxpMap *map, ulong val) {
	MapEnt **e, *te;
	void *ret;
	
	ret = nullptr;
	thread->wlock(&map->lock);
	e = map_getp(map, val, false, nullptr);
	if(*e) {
		te = *e;
		ret = te->val;
		*e = te->next;
		thread->wunlock(&map->lock);
		free(te);
	}
	else
		thread->wunlock(&map->lock);
	return ret;
}

