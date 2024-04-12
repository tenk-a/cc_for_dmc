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
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <process.h>

#define ZATU_UNUSE_WCHAR_T
#define ZATU_USE_CMD_LINE_ARGS_UTIL
#include "cmd_line_args.hpp"

using namespace std;
using namespace zatu;
using namespace zatu::cmd_line_args_util;


class Program {
    vector<string>      opts_;
    vector<string>      files_;
    vector<string>      libs_;
    vector<char const*> dst_args_;
    string              bindir_;
    string              exepath_;
    char const*         ccpath_;
    bool                print_args_;
    bool                verbose_;

public:
    Program() : ccpath_(NULL), print_args_(false), verbose_(false) {}

    int main(int argc, char* argv[], char** env) {
        ccpath_ = argv[0];
        if (argc < 2)
            return usage();

        get_exepath(ccpath_);

        opts_.reserve(512);
        files_.reserve(512);
        libs_.reserve(64);
        if (conv_gcc_to_native_args(argc, argv) != 0)
            return 1;

        char** dst_argv = (char**)&dst_args_[0];

        if (print_args(dst_argv) == 0)
            return 0;

        int rc = execve(exepath_.c_str(), dst_argv, env);
        return rc;
    }

private:
    void get_exepath(char const* exepath) {
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
                if (file_exist("c:\\dm\\bin"))
                    envdir = "c:\\dm";
                else if (file_exist("c:\\DMC\\dm\\bin"))
                    envdir = "c:\\dmc\\dm";
                else //if (file_exist("c:\\dmc\\bin"))
                    envdir = "c:\\dmc";
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

        // ini file load.
        string str = ccpath_;
        char*  ext = (char*)fname_ext(str.c_str());
        if (strlen(ext) >= 4) {
            strcpy(ext, ".ini");
            if (file_exist(str.c_str()))
                args.insert_response_str(file_load<string>(str.c_str()));
        }

        bool cxx = false;
        bool gccmode = true;
        bool opt_linker = false;

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
            } else if (*args.get_arg() == '@') {    // response file.
                args.replace_response_str(file_load<string>(args.get_arg()+1));
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
		 #if defined USE_WLINK
			string wlink = ccpath_;
			wlink.resize(fname_base(wlink.c_str()) - wlink.c_str());
			wlink += "wlink.exe";
			if (file_exist(wlink.c_str()))
            	opts_.push_back("-L" + wlink);
			else
		 #endif
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

    template<class S>
    void str_fsl_to_bsl(S& s) {
        str_replace(s, '/', '\\');
    }

    int usage() {
        printf("usage> %s [-options] filename(s)\n", fname_base(ccpath_));
        printf("      Convert and pass gcc-like command line arguments to dmc.\n"
               "      Filename convert '/' to '\\'.\n"
               "  @FILE     Input response FILE.\n"
               "  --help    Help.\n"
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
