/* Copyright ©2008-2010 Kris Maglione <maglione.k at Gmail>
 * See LICENSE file for license details.
 */
#include <assert.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ixp_local.h"


namespace ixp {
/**
 * Function: ixp_msec
 *
 * Returns the time since the Epoch in milliseconds.
 */
uint64_t
msec() {
	timeval tv;

	gettimeofday(&tv, 0);
	return (uint64_t)tv.tv_sec*1000 + (uint64_t)tv.tv_usec/1000;
}

/**
 * Function: ixp_settimer
 *
 * Params:
 *	msec: The timeout in milliseconds.
 *	fn:   The function to call after P<msec> milliseconds
 *	      have elapsed.
 *	aux:  An arbitrary argument to pass to P<fn> when it
 *	      is called.
 * 
 * Initializes a callback-based timer to be triggerred after
 * P<msec> milliseconds. The timer is passed its id number
 * and the value of P<aux>.
 *
 * Returns:
 *	Returns the new timer's unique id number.
 * See also:
 *	F<ixp_unsettimer>, F<ixp_serverloop>
 */
long
settimer(IxpServer *srv, long msec, std::function<void(long, const std::any&)> fn, void *aux) {
    /* 
     * This really needn't be threadsafe, as it has little use in
     * threaded programs, but it nonetheless is.
     */

    static long	lastid = 1;
	Timer **tp;
	Timer *t;
	uint64_t time;

	time = ixp_msec() + msec;

	t = (decltype(t))emallocz(sizeof *t);
	thread->lock(&srv->lk);
	t->id = lastid++;
	t->msec = time;
	t->fn = fn;
	t->aux = aux;

	for(tp=&srv->timer; *tp; tp=&tp[0]->link)
		if(tp[0]->msec < time)
			break;
	t->link = *tp;
	*tp = t;
	thread->unlock(&srv->lk);
	return t->id;
}

/**
 * Function: ixp_unsettimer
 *
 * Params:
 *	id: The id number of the timer to void.
 *
 * Voids the timer identified by P<id>.
 *
 * Returns:
 *	Returns true if a timer was stopped, false
 *	otherwise.
 * See also:
 *	F<ixp_settimer>, F<ixp_serverloop>
 */
bool
unsettimer(IxpServer *srv, long id) {
	Timer **tp;
	Timer *t;

	thread->lock(&srv->lk);
	for(tp=&srv->timer; (t=*tp); tp=&t->link)
		if(t->id == id)
			break;
	if(t) {
		*tp = t->link;
		free(t);
	}
	thread->unlock(&srv->lk);
	return t != nullptr;
}

/*
 * Function: ixp_nexttimer
 *
 * Triggers any timers whose timeouts have ellapsed. This is
 * primarily intended to be called from libixp's select
 * loop.
 *
 * Returns:
 *	Returns the number of milliseconds until the next
 *	timer's timeout.
 * See also:
 *	F<ixp_settimer>, F<ixp_serverloop>
 */
long
nexttimer(IxpServer *srv) {
	Timer *t;
	uint64_t time;
	long ret;

	SET(time);
	thread->lock(&srv->lk);
	while((t = srv->timer)) {
		time = ixp_msec();
		if(t->msec > time)
			break;
		srv->timer = t->link;

		thread->unlock(&srv->lk);
		t->fn(t->id, t->aux);
		free(t);
		thread->lock(&srv->lk);
	}
	ret = 0;
	if(t)
		ret = t->msec - time;
	thread->unlock(&srv->lk);
	return ret;
}
} // end namespace ixp

