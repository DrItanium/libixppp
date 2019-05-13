/* C Implementation copyright ©2006-2010 Kris Maglione <maglione.k at Gmail>
 * C++ Implementation copyright (c)2019 Joshua Scoggins
 * See LICENSE file for license details.
 */
#ifndef LIBJYQ_STAT_H__
#define LIBJYQ_STAT_H__
#include "types.h"
#include "qid.h"
namespace jyq {
    struct Msg;
    /* stat structure */
    struct Stat {
        private:
            uint16_t	_type;
            uint32_t	_dev;
            Qid         _qid;
            uint32_t	_mode;
            uint32_t	_atime;
            uint32_t	_mtime;
            uint64_t	_length;
        public:
            char*	name;
            char*	uid;
            char*	gid;
            char*	muid;
            uint16_t    size() noexcept;
            ~Stat();
            void packUnpack(Msg& msg) noexcept;
            constexpr auto getType() const noexcept { return _type; }
            void setType(uint16_t value) noexcept { _type = value; }
            constexpr auto getDev() const noexcept { return _dev; }
            void setDev(uint32_t value) noexcept { _dev = value; }
            Qid& getQid() noexcept { return _qid; }
            const Qid& getQid() const noexcept { return _qid; }
            constexpr auto getMode() const noexcept { return _mode; }
            void setMode(uint32_t value) noexcept { _mode = value; }
            constexpr auto getAtime() const noexcept { return _atime; }
            void setAtime(uint32_t value) noexcept { _atime = value; }
            constexpr auto getMtime() const noexcept { return _mtime; }
            void setMtime(uint32_t value) noexcept { _mtime = value; }
            constexpr auto getLength() const noexcept { return _length; }
            void setLength(uint64_t value) noexcept { _length = value; }
    };
} // end namespace jyq
#endif // end LIBJYQ_STAT_H__
