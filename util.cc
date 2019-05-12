/* Written by Kris Maglione <maglione.k at Gmail> */
/* C++ Implementation copyright (c)2019 Joshua Scoggins */
/* Public domain */
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <list>
#include <string>
#include <sstream>
#include "PrintFunctions.h"
#include "util.h"
#include "argv.h"
//#include "jyq.h"

namespace jyq {
static const std::string&
_user() {
    static std::string user;
	struct passwd *pw;

	if(user.empty()) {
		pw = getpwuid(getuid());
		if(pw) {
			user = strdup(pw->pw_name);
        }
	}
	if(user.empty()) {
		user = "none";
    }
	return user;
}

static bool 
rmkdir(const std::string& path, int mode) {
    auto tokens = tokenize(path, '/');
    for (const auto& pathComponent: tokens) {
        if (auto ret = mkdir(pathComponent.c_str(), mode); (ret == -1) && (errno != EEXIST)) {
            std::ostringstream msg;
            print(msg, "Can't create path '", path, "': ", errbuf());
            auto str = msg.str();
            throw str; // TODO throw an actual exception
        }
    }
    return true;

}

static std::string
ns_display() {
	struct stat st;
    std::string displayVariable;
    std::string newPath;

	if (auto disp = std::getenv("DISPLAY"); !disp || disp[0] == '\0') {
        wErrorString("$DISPLAY is unset");
        return "";
	} else {
        displayVariable = disp;
        if (auto subComponent = displayVariable.substr(displayVariable.length() - 2); 
                (displayVariable != subComponent) && 
                (!std::strcmp(subComponent.c_str(), ".0"))) {
            // TODO strcmp must be replaced!
            displayVariable = displayVariable.substr(0, displayVariable.length() - 2);
        }
        newPath = smprint("/tmp/ns.", _user(), ".", disp);
    }

	if(!rmkdir(newPath.c_str(), 0700)) {

    } else if(stat(newPath.c_str(), &st)) {
        wErrorString("Can't stat Namespace path '", newPath, "': ", errbuf());
    } else if(getuid() != st.st_uid) {
        wErrorString("Namespace path '", newPath, "' exists but is not owned by you");
    } else if((st.st_mode & 077) && chmod(newPath.c_str(), st.st_mode & ~077)) {
        wErrorString("Namespace path '", newPath, "' exists, but has wrong permissions: ", errbuf());
    } else {
		return newPath;
    }
    return "";
}

/**
 * Function: namespace
 *
 * Returns the path of the canonical 9p namespace directory.
 * Either the value of $NAMESPACE, if it's set, or, roughly,
 * /tmp/ns.${USER}.${DISPLAY:%.0=%}. In the latter case, the
 * directory is created if it doesn't exist, and it is
 * ensured to be owned by the current user, with no group or
 * other permissions.
 *
 * Returns:
 *	A statically allocated string which must not be freed
 *	or altered by the caller. The same value is returned
 *	upon successive calls.
 * Bugs:
 *	This function is not thread safe until after its first
 *	call.
 */
/* Not especially threadsafe. */
std::string
getNamespace() {
    static std::string _namespace;
    if (_namespace.empty()) {
        if (auto ev = std::getenv("NAMESPACE"); !ev) {
            _namespace = ns_display();
        } else {
            _namespace = ev;
        }
    }
	return _namespace;
}

/* Can't malloc */
static void
mfatal(const char *name, uint size) {
    std::cerr << "libjyq: fatal: Could not " << name << "() " << size << " bytes\n";
	exit(1);
}

/**
 * Function: erealloc
 * Function: estrdup
 *
 * These functions act like their stdlib counterparts, but print
 * an error message and exit the program if allocation fails.
 */
void*
erealloc(void *ptr, uint size) {
	if (void *ret = realloc(ptr, size); !ret) {
		mfatal("realloc", size);
        throw "SHOULDN'T GET HERE!";
    } else {
        return ret;
    }
}

char*
estrdup(const char *str) {
	if (void *ret = strdup(str); !ret) {
		mfatal("strdup", strlen(str));
        throw "SHOULDN'T GET HERE!";
    } else {
        return (char*)ret;
    }
}

std::list<std::string>
tokenize(const std::string& input, char delim) {
    std::list<std::string> tokens;
    std::istringstream ss(input);
    for (std::string line; std::getline(ss, line, delim);) {
        if (!line.empty()) {
            tokens.emplace_back(line);
        }
    }
    return tokens;
}

} // end namespace jyq

char* argv0 = nullptr;
