/* Public Domain --Kris Maglione */
#include <unistd.h>
#include "ixp_local.h"

namespace ixp::concurrency {

ssize_t 
Thread::read(int fd, void* buf, size_t count) {
    return ::read(fd, buf, count);
}
ssize_t 
Thread::write(int fd, const void* buf, size_t count) {
    return ::write(fd, buf, count);
}
int 
Thread::select(int fd, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, timeval* timeout) {
    return ::select(fd, readfds, writefds, exceptfds, timeout);
}
char*
NoThreadImpl::errbuf() {
    static char errbuf[IXP_ERRMAX];
    return errbuf;
}
void
NoThreadImpl::sleep(IxpRendez*) {
    throw "unimplemented";
}
std::unique_ptr<Thread> threadModel = std::make_unique<NoThreadImpl>();
} // end namespace ixp::concurrency
