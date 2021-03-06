/* C Implementation copyright ©2006-2010 Kris Maglione <maglione.k at Gmail>
 * C++ Implementation copyright (c)2019 Joshua Scoggins
 * See LICENSE file for license details.
 */
#ifndef LIBJYQ_CLIENT_H__
#define LIBJYQ_CLIENT_H__
#include <string>
#include <functional>
#include <list>
#include <memory>
#include "types.h"
#include "Msg.h"
#include "Fcall.h"
#include "Rpc.h"
#include "stat.h"
#include "socket.h"

namespace jyq {
    struct CFid;
    struct Client {
        public:
            static std::unique_ptr<Client> mount(const char*);
            static auto mountfd(int fd) { return mountfd(Connection(fd)); }
            static std::unique_ptr<Client> mountfd(const Connection& c);
            static std::unique_ptr<Client> nsmount(const char*);

            static auto mount(const std::string& str) { return mount(str.c_str()); }
            static auto nsmount(const std::string& str) { return nsmount(str.c_str()); }
            Client(int fd);
            Client(const Connection& c);
            ~Client();
            Connection& getConnection() noexcept { return fd; }
            const Connection& getConnection() const noexcept { return fd; }
            Msg& getWmsg() noexcept { return _wmsg; }
            const Msg& getWmsg() const noexcept { return _wmsg; }
            [[nodiscard]] Lock getLock() { return Lock(_lk); }
            [[nodiscard]] Lock getReadLock() { return Lock(_rlock); }
            [[nodiscard]] Lock getWriteLock() { return Lock(_wlock); }
        private:
            //int     fd;
            Connection fd;
            uint    _lastfid;
            uint    _freetag;
            uint    _msize;
            uint    _nwait;
            uint    _mwait;
            std::list<std::shared_ptr<CFid>> _freefid;
            Msg     _rmsg;
            Msg     _wmsg;
            mutable Mutex	_lk;
            mutable Mutex	_rlock;
            mutable Mutex	_wlock;
            mutable Rendez	_tagrend;
        public:
            std::vector<Rpc> wait;
            Rpc::weak_type muxer;
            Rpc         sleep;
        private:
            int		_mintag;
            int		_maxtag;
        public:
            bool remove(const char*);
            std::shared_ptr<CFid> create(const char*, uint perm, uint8_t mode);
            std::shared_ptr<CFid> open(const char*, uint8_t);
            std::shared_ptr<Stat> stat(const char*);
            bool remove(const std::string& str) noexcept { return remove(str.c_str()); }
            auto create(const std::string& str, uint perm, uint8_t mode) { return create(str.c_str(), perm, mode); }
            auto create(const std::string& str, uint perm, OMode mode) { return create(str.c_str(), perm, uint8_t(mode)); }
            auto create(const char* str, uint perm, OMode mode) { return create(str, perm, uint8_t(mode)); }
            auto open(const std::string& str, uint8_t val) { return open(str.c_str(), val); }
            auto open(const std::string& str, OMode mode) { return open(str, uint8_t(mode)); }
            auto open(const char* str, OMode mode) { return open(str, uint8_t(mode)); }
            auto stat(const std::string& str) { return stat(str.c_str()); }
            std::shared_ptr<Fcall> muxrpc(Fcall& tx);
            std::shared_ptr<CFid> getFid();
            std::shared_ptr<CFid> walk(const std::string&);
            std::shared_ptr<CFid> walkdir(char *path, const char **rest);
            std::shared_ptr<Fcall> dofcall(Fcall& fcall);
            void enqueue(Rpc&);
            void dequeue(Rpc&);
            void putfid(std::shared_ptr<CFid> cfid);
            void clunk(std::shared_ptr<CFid> fid);
            inline DoFcallFunc getDoFcallLambda() noexcept { return [this](auto& ptr) { return dofcall(ptr); }; }
            int gettag(Rpc* r, Lock& lock) { return gettag(*r, lock); }
            int gettag(Rpc& r, Lock& lock);
            void puttag(Rpc& r);
            void puttag(Rpc* r) { return puttag(*r); }
            bool sendrpc(Rpc& r, Fcall& f);
        private:
            std::unique_ptr<Fcall> muxrecv();
            void electmuxer();
            void dispatchandqlock(std::shared_ptr<Fcall> f, Lock&);
            void allocmsg(int n);
    };
} // end namespace jyq
#endif // end LIBJYQ_CLIENT_H__
