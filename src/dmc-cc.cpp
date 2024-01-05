/**
 *  @file   dmc-cc.cpp
 *  @brief  Convert and pass gcc-like command line arguments to dmc.
 *  @author Masashi Kitamura (tenka@6809.net)
 *  @date   2024-01-03
 *  @license    Boost Software License, Version 1.0
 *  @note
 */
#include <utility>
#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <io.h>
#include <process.h>

//namespace {

template< unsigned int F=3, typename C=char>
class cmd_line_args {
    enum flag { use_opt_next_arg = 1, enable_short_opt = 2 };
public:
    typedef C char_type;

    cmd_line_args(int argc, char_type* argv[])
        : argv_(argv), arg_(&nil_), arg_0_(0),argc_(argc), index_(1)
        , alloc_(false), enable_opt_(true), sub_opt_(false)
        , short_idx_(0), pre_short_idx_(0), nil_(0)
    { }

    ~cmd_line_args() {}

    int         argc() const { return argc_; }
    char_type** argv() { return argv_; }

    bool has_arg() const { return (index_ < argc_); }

    void disable_opt() { enable_opt_ = false; }

    // @return true:-option false:other
    bool prepare_get();

    char_type* get_arg() { return arg_; }
    char_type* get_arg_0() { return arg_0_; }

    bool get_opt(char_type const* opt) {
        if (F & enable_short_opt) {
            if (short_idx_)
                return false;
        }
        char_type* p = get_opt1(opt);
        return p != NULL && *p == 0;
    }

    bool get_opt(char_type const* opt, bool& b) {
        char_type* p = get_opt1(opt);
        if (p) b = *p != '-';
        return p != NULL;
    }

    template<class U>
    bool get_opt(char_type const* opt, U& u) { return get_opt(opt, u, true); }

    template<class U>
    bool get_opt(char_type const* opt, U& u, bool next_arg) {
        char_type* p = get_opt1(opt);
        if (p)
            u = get_opt_arg(p, next_arg);
        return p != NULL;
    }

    bool get_opt(char_type c);

    bool get_opt(char_type c, bool& b) {
        if (F & enable_short_opt) {
            if (get_opt(c)) {
                b = *arg_ != '-';
                if (!b)
                    ++arg_;
                return true;
            }
        } else {
            assert(F & enable_short_opt);
        }
        return false;
    }

    template<class U>
    bool get_opt(char_type c, U& u) { return get_opt(c, u, true); }

    template<class U>
    bool get_opt(char_type c, U& u, bool next_arg) {
        if (F & enable_short_opt) {
            if (get_opt(c)) {
                if (*arg_ == C('=')) ++arg_;
                u = get_opt_arg(arg_, next_arg);
                short_idx_ = 0;
                arg_ = &nil_;
                return true;
            }
        } else {
            assert(F & enable_short_opt);
        }
        return false;
    }

    bool get_opt2(char_type c, char_type const* opt) {
        if (F & enable_short_opt) {
            return get_opt(c) || get_opt(opt);
        } else {
            assert(F & enable_short_opt);
            return false;
        }
    }

    template<typename U>
    bool get_opt2(char_type c, char_type const* opt, U& u) {
		return get_opt2(c, opt, u, true);
	}

    template<typename U>
    bool get_opt2(char_type c, char_type const* opt, U& u, bool next_arg) {
        return get_opt(c, u, next_arg) || get_opt(opt, u, next_arg);
    }

    bool get_opt2(char_type const* opt1, char_type const* opt2) {
        return get_opt(opt1) || get_opt(opt2);
    }

    template<typename U>
    bool get_opt2(char_type const* opt1, char_type const* opt2, U& u) {
		return get_opt2(opt1, opt2, u, true);
	}

    template<typename U>
    bool get_opt2(char_type const* opt1, char_type const* opt2, U& u, bool next_arg) {
        return get_opt(opt1, u, next_arg) || get_opt(opt2, u, next_arg);
    }

    void reset() {
        index_ = 1;
    }

    bool get_first_ch(C ch) {
        if (*arg_ == ch) {
            ++arg_;
            return true;
        }
        return false;
    }

private:
    C*      get_opt1(C const* opt);
    C*      get_opt_arg(C* opt_arg, bool next_arg);

private:
    char_type**     argv_;
    char_type*      arg_;
    char_type*      arg_0_;
    int             argc_;
    int             index_;
    bool            alloc_;
    bool            enable_opt_;
    bool            sub_opt_;
    unsigned char   short_idx_;
    unsigned char   pre_short_idx_;
    char_type       nil_;
};

template<unsigned int F, typename C>
bool cmd_line_args<F,C>::prepare_get() {
    assert(index_ < argc_);
    if (short_idx_) {
        if (*arg_) {
            if (pre_short_idx_ < short_idx_) {
                pre_short_idx_ = short_idx_;
                return true;
            }
            assert(pre_short_idx_ < short_idx_);
        }
        short_idx_ = 0;
    }
    sub_opt_ = false;
    pre_short_idx_ = short_idx_;
    arg_ = arg_0_ = argv_[index_++];
    bool rc = enable_opt_ && arg_ && *arg_ == '-';
    return  rc;
}

template<unsigned int F, typename C>
bool cmd_line_args<F,C>::get_opt(char_type c) {
    if (F & enable_short_opt) {
        if (!c)
            return false;
        if (short_idx_) {
            if (*arg_ == c) {
                if (short_idx_ < 255)
                    ++short_idx_;
                ++arg_;
                return true;
            }
        } else if (*arg_ == C('-') && arg_[1] == c) {
            short_idx_ = 1;
            arg_ += 2;
            return true;
        }
    } else {
        assert(F & enable_short_opt);
    }
    return false;
}

template<unsigned int F, typename C>
C* cmd_line_args<F,C>::get_opt1(C const* opt) {
    if (opt == NULL)
        return NULL;
    std::size_t opt_len = std::char_traits<C>::length(opt);
    if (std::char_traits<C>::compare(arg_, opt, opt_len) == 0) {
        C* p = arg_ + opt_len;
        if (*p == C('=')) ++p;
        return p;
    }
    return NULL;
}

template<unsigned int F, typename C>
C* cmd_line_args<F,C>::get_opt_arg(C* opt_arg, bool next_arg) {
    assert(opt_arg != 0);
    if (F & use_opt_next_arg) {
        sub_opt_ = false;
        if (next_arg && *opt_arg == 0 && index_ < argc_) {
            sub_opt_ = true;
            opt_arg = argv_[index_++];
        }
    }
    return opt_arg;
}


char*   fname_base(char const* p) {
    char const *adr = p;
    while (*p) {
        char c = *p++;
        if (c == ':' || c == '/' || c == '\\')
            adr = p;
    }
    return (char*)adr;
}

char const* fname_ext(char const* p) {
    p = fname_base(p);
    char const* e = strrchr(p, '.');
    return e ? e : "";
}

template<typename S>
void str_fsl_to_bsl(S& str) {
    typedef typename S::value_type C;
    C* s = (C*)str.data();
    C* e = s + str.size();
    while (s < e) {
        if (*s == '/')
            *s = '\\';
        ++s;
    }
}

bool file_exist(char const* fpath) {
    return ::access(fpath, 0) == 0;
}

//}

using namespace std;

class Program {
    vector<string>      opts_;
    vector<string>      files_;
    vector<string>      libs_;
    vector<char const*> dst_args_;
    string              exepath_;
    string              bindir_;
    char const*         ccpath_;
    bool                print_args_;
	bool				verbose_;

public:
    Program() : ccpath_(NULL), print_args_(false), verbose_(false) {}

    int main(int argc, char* argv[], char** env) {
        ccpath_ = argv[0];
        if (argc < 2)
            return usage();

        get_dmcpath(ccpath_);

        opts_.reserve(512);
        files_.reserve(512);
        libs_.reserve(64);
        if (conv_gcc_to_native_args(argc, argv) != 0)
            return 1;

        char** dst_argv = (char**)&dst_args_[0];

		if (print_args(dst_argv) == 0)
			return 0;

        int rc = execve(exepath_.c_str(), (char**)&dst_args_[0], env);
        return rc;
    }

    void get_dmcpath(char const* exepath) {
        char buf[_MAX_PATH*2] = {0};
        strncpy(buf, exepath, sizeof(buf)-1);
        char* b = fname_base(buf);
        strcpy(b, "dmc.exe");
        exepath_ = buf;
        if (!file_exist(buf)) {
            char const* envdir = getenv("DMC_DIR");
            if (!envdir || !file_exist(envdir))
                envdir = getenv("DMC");
            if (!envdir || !file_exist(envdir)) {
                if (file_exist("c:\\dmc"))
                    envdir = "c:\\dmc";
                else //if (file_exist("c:\\dm"))
                    envdir = "c:\\dm";
            }
            //printf("envdir=%s\n", envdir);
            b = buf;
            b += _snprintf(buf, (sizeof buf)-1-8, "%s\\bin\\", envdir);
            strcpy(b, "dmc.exe");
            exepath_ = buf;
        }
        *b = '\0';
        bindir_ = buf;
        str_fsl_to_bsl(bindir_);
        //printf("dir=%s\n", buf);
        //printf("exe=%s\n", exepath_.c_str());
    }

    int conv_gcc_to_native_args(int argc, char* argv[]) {
        cmd_line_args<> args(argc, argv);
        string          str;
        bool            cxx = false;
        bool            gccmode = true;
        bool            opt_linker = false;
        while (args.has_arg()) {
            if (args.prepare_get()) {  // option.
                if (args.get_opt("--help")) {
                    return usage();
                } else if (args.get_opt("--CC-print-args", print_args_)) {
                    continue;
                } else if (args.get_opt("--GCC")) {
                    gccmode = true;
                    continue;
                } else if (args.get_opt("--NATIVE") || args.get_opt("--DMC")) {
                    gccmode = false;
                    continue;
                }
                if (gccmode) {
                    if (args.get_opt('D', str, false)) {
                        opts_.push_back("-D");
                        opts_.back() += str;
                    } else if (args.get_opt("--define-macro", str)) {
                        opts_.push_back("-D");
                        opts_.back() += str;
                    } else if (args.get_opt('U', str, false)) {
                        opts_.push_back("-U");
                        opts_.back() += str;
                    } else if (args.get_opt("--undefine-macro", str)) {
                        opts_.push_back("-U");
                        opts_.back() += str;
                    } else if (args.get_opt2('I', "--include-directory", str)) {
                        opts_.push_back("-I");
                        //str_fsl_to_bsl(str);
                        opts_.back() += str;
                    } else if (args.get_opt("--include", str)) {
                        opts_.push_back("-HI");
                        //str_fsl_to_bsl(str);
                        opts_.back() += str;
                    } else if (args.get_opt('c')) {
                        opts_.push_back("-c");
                    } else  if (args.get_opt2('o', "--output", str)) {
                        opts_.push_back("-o");
                        str_fsl_to_bsl(str);
                        opts_.back() += str;
                    } else  if (args.get_opt2('L', "--library-path", str)) {
                        opts_.push_back("-L/");
                        str_fsl_to_bsl(str);
                        opts_.back() += str;
                    } else  if (args.get_opt2('l', "--library", str)) {
                        libs_.push_back("lib" + str + ".lib");
                    } else if (args.get_opt("-Wall")) {
                        opts_.push_back("-w");
                    } else if (args.get_opt("-Werror")) {
                        opts_.push_back("-wx");
                    } else if (args.get_opt("--std=c++",str) || args.get_opt("--std=gnu++",str)) {
                        opts_.push_back("-cpp");
                        cxx = true;
                    } else if (args.get_opt("--std=c",str) || args.get_opt("--std=gnu",str)) {
                        cxx = false;
                    } else if (args.get_opt("-g")) {
                        opts_.push_back("-g");
                    } else if (args.get_opt("--debug")) {
                        opts_.push_back("-g");
                    } else if (args.get_opt("-S")) {
                        opts_.push_back("-cod");
                    } else if (args.get_opt("-O0")) {
                        opts_.push_back("-o+none");
                    } else if (args.get_opt("-O1") || args.get_opt("-O2") || args.get_opt("-O3")) {
                        opts_.push_back("-o+all");
                    } else if (args.get_opt("-Ofast")) {
                        opts_.push_back("-o+speed");
                    } else if (args.get_opt("-Os") || args.get_opt("-Oz")) {
                        opts_.push_back("-o+space");
                    } else if (args.get_opt("-frtti")) {
                        opts_.push_back("-Ar");
                    } else if (args.get_opt("-fexceptions")) {
                        opts_.push_back("-Ae");
                    } else if (args.get_opt("-v2")) {
                        opts_.push_back("-v2");
                        verbose_ = true;
                    } else if (args.get_opt2('v', "--verbose")) {
                        opts_.push_back("-v1");
                        verbose_ = true;
                    } else if (args.get_opt("-fstack-check", str)) {
                        if (str != "no")
                            opts_.push_back("-s");
                    } else if (args.get_opt("-funsigned-char")) {
                        opts_.push_back("-J");
                    } else if (args.get_opt("-fsigned-char")) {
                    } else if (args.get_opt("-shared")) {
                        opts_.push_back("-WD");
                    } else if (args.get_opt("-mdll")) {
                        opts_.push_back("-WD");
                    } else if (args.get_opt("--ansi")) {
                        opts_.push_back("-A");
                    } else {
                        //if (verbose_)
                        fprintf(stderr, "Ignore option %s\n", args.get_arg());
                    }
                } else {    // dmc
                    if (args.get_opt("-o+", str, false)) {
                        opts_.push_back("-o+");
                        opts_.back() += str;
                    } else if (args.get_opt("-o-", str, false)) {
                        opts_.push_back("-o-");
                        opts_.back() += str;
                    } else  if (args.get_opt("-o", str, false)) {
                        opts_.push_back("-o");
                        str_fsl_to_bsl(str);
                        opts_.back() += str;
                    } else  if (args.get_opt("-I", str, false)) {
                        opts_.push_back("-I");
                        //str_fsl_to_bsl(str);
                        opts_.back() += str;
                    } else  if (args.get_opt("-L/", str, false)) {
                        opts_.push_back("-L/");
                        opts_.back() += str;
                    } else  if (args.get_opt("-L", str, false)) {
                        opts_.push_back("-L");
                        if (str.size() > 0) {
                            str_fsl_to_bsl(str);
                            opts_.back() += str;
                            if (str != "link")
                                opt_linker = true;
                        }
                    } else if (args.get_opt("-v0")) {
                        opts_.push_back("-v0");
                        verbose_ = false;
                    } else if (args.get_opt("-v1") || args.get_opt("-v2")) {
                        opts_.push_back(args.get_arg_0());
                        verbose_ = true;
                    } else {
                        opts_.push_back(args.get_arg_0());
                    }
                }
            } else { // file.
                files_.push_back(args.get_arg());
                str_fsl_to_bsl(files_.back());
                char const* a = files_.back().c_str();
                if (strcmp(fname_ext(a), ".cpp") == 0
                 || strcmp(fname_ext(a), ".cxx") == 0
                 || strcmp(fname_ext(a), ".cc") == 0)
                {
                    cxx = true;
                }
            }
        }
        if (cxx) {
            opts_.push_back("-Aa");
            opts_.push_back("-Ab");
        }
        if (!opt_linker) {
            opts_.push_back("-L" + bindir_ + "optlink.exe");
        }
        size_t num = opts_.size() + files_.size();
        dst_args_.reserve(num + 1);
        dst_args_.push_back(exepath_.c_str());
        for (size_t i = 0; i < opts_.size(); ++i)
            dst_args_.push_back(opts_[i].c_str());
        for (size_t i = 0; i < files_.size(); ++i)
            dst_args_.push_back(files_[i].c_str());
        for (size_t i = 0; i < libs_.size(); ++i)
            dst_args_.push_back(libs_[i].c_str());
        dst_args_.push_back(NULL);
        return 0;
    }

	int print_args(char** dst_argv) {
        if (print_args_) {
            for (size_t i = 0; dst_argv[i]; ++i)
                printf("argv[%d]=%s\n", i, dst_args_[i]);
            return 0;
        }
        if (verbose_) {
			printf("[verbose] ");
            for (size_t i = 0; dst_argv[i]; ++i)
				printf("%s ", dst_argv[i]);
			printf("\n");
		}
		return 1;
	}

    int usage() {
        printf("usage> %s [-options] filename(s)\n", fname_base(ccpath_));
        printf("      Convert and pass gcc-like command line arguments to dmc.\n"
               "      filename convert '/' to '\\'.\n"
               "  --help    help.\n"
               "  --NATIVE  Afterwards dmc option.\n"
               "  --GCC     Afterwards gcc option.\n"
               " (gcc)                   (dmc)\n"
               "  --define-macro M[=S]    -D[M[=S]]\n"
               "  -D[MACRO[=STR]]         -D[MACRO[=STR]]\n"
               "  --undefine-macro MACRO  -U[MACRO]\n"
               "  -U[MACRO]               -U[MACRO]\n"
               "  --include-directory DIR -I[DIR]\n"
               "  -I DIR                  -I[DIR]\n"
               "  --include FILE          -HI[FILE]\n"
               "  --output FILE           -o[FILE]\n"
               "  -o FILE                 -o[FILE]\n"
               "  --library NAME          lib[NAME].lib\n"
               "  -l NAME                 lib[NAME].lib\n"
               "  --library-path DIR      -L/DIR\n"
               "  -L DIR                  -L/DIR\n"
               "  -S                      -cod\n"
               "  -shared                 -WD\n"
               "  -mdll                   -WD\n"
               "  --debug                 -g\n"
               "  -g                      -g\n"
               "  -Wall                   -w\n"
               "  -Werror                 -wx\n"
               "  -O0                     -o+none\n"
               "  -O1 -O2 -O3             -o+all\n"
               "  -Ofast                  -o+speed\n"
               "  -Os                     -o+space\n"
               "  -Oz                     -o+space\n"
               "  --std=c++??             -cpp\n"
               "  --std=gnu++??           -cpp\n"
               "  --std=c??               \n"
               "  --std=gnu??             \n"
               "  -frtti                  -Ar\n"
               "  -fexceptions            -Ae\n"
               "  -funsigned-char         -J\n"
               "  -fsigned-char           \n"
               "  -fstack-check-generic   -s\n"
               "  -fstack-check-specific  -s\n"
               "  --ansi                  -A\n"
               "  -v                      -v1\n"
        );
        return 1;
    }
};


int main(int argc, char* argv[], char** env) {
    int rc = Program().main(argc, argv, env);
    return rc;
}
