#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using filesystem::path;

path operator""_p(const char *data, std::size_t sz) {
    return path(data, data + sz);
}

enum class SearchType {
    LOCAL,
    GLOBAL
};

bool FindIncludeDirective(const string &src_line, string &result_include, const SearchType search_type) {
    static const regex include_local_reg(R"/(\s*#\s*include\s*"([^"]*)"\s*)/");
    static const regex include_global_reg(R"/(\s*#\s*include\s*<([^>]*)>\s*)/");
    smatch m;
    bool regex_found = false;

    if (search_type == SearchType::LOCAL) {
        if (regex_match(src_line, m, include_local_reg)) {
            result_include = string(m[1]);
            regex_found = true;
        }
    } else {
        if (regex_match(src_line, m, include_global_reg)) {
            result_include = string(m[1]);
            regex_found = true;
        }
    }
    return regex_found;
}

bool PasteIncludeGlobalFile(fstream &dst_file, const path &incl_file_path, const vector<path> &include_directories);
bool PasteIncludeLocalFile(fstream &dst_file, const path &incl_file_path, const vector<path> &include_directories);
/*
 * dst_file - ссылка на открытый файл для записи
 * incl_file - ссылка на открытый файл-источник с директивами #include
 * incl_file_path - сылка на путь до файла-источника с директивами #include
 * include_directories - список каталогов, где искать файлы #include
 */
bool CopyLines(fstream &dst_file, fstream &incl_file, const path &incl_file_path, const vector<path> &include_directories) {
    string incl_line;
    string include_string;
    int line_count = 0;

    while (incl_file.good()) {
        getline(incl_file, incl_line, '\n');
        ++line_count;

        if (FindIncludeDirective(incl_line, include_string, SearchType::LOCAL)) {
            /* Нашли директиву #include "some_path" */
            path paste_loc_file = incl_file_path.parent_path() / path(include_string);
            if (!PasteIncludeLocalFile(dst_file, paste_loc_file, include_directories)) {
                if (!PasteIncludeGlobalFile(dst_file, path(include_string), include_directories)) {
                    cout << "unknown include file "s << include_string << " at file "s << incl_file_path.string() << " at line "s << line_count << endl;
                    return false;
                }
            }
        } else if (FindIncludeDirective(incl_line, include_string, SearchType::GLOBAL)) {
            /* Нашли директиву #include <some_path> */
            path paste_glob_file = path(include_string);
            if (!PasteIncludeGlobalFile(dst_file, paste_glob_file, include_directories)) {
                cout << "unknown include file "s << include_string << " at file "s << incl_file_path.string() << " at line "s << line_count << endl;
                return false;
            }
        } else {
            std::error_code err;
            // if(file_size(incl_file_path, err) > (static_cast<size_t>(incl_file.tellg()) - 1)) {
            try {
                dst_file << incl_line << endl;
            } catch (const std::ios_base::failure &err) {
                return false;
            }
            //}
        }
    }
    return true;
}

bool PasteIncludeLocalFile(fstream &dst_file, const path &incl_file_path, const vector<path> &include_directories) {
    fstream incl_file;
    bool found_file = false;

    incl_file.open(incl_file_path, ios::in);
    if (incl_file.is_open()) {
        found_file = true;
        if (!CopyLines(dst_file, incl_file, incl_file_path, include_directories)) {
            return false;
        }
    }
    return found_file;
}

bool PasteIncludeGlobalFile(fstream &dst_file, const path &incl_file_path, const vector<path> &include_directories) {
    fstream incl_file;
    bool found_file = false;

    for (auto incl_dir : include_directories) {
        path p = incl_dir / incl_file_path;
        incl_file.open(p, ios::in);
        if (incl_file.is_open()) {
            found_file = true;

            if (!CopyLines(dst_file, incl_file, incl_dir / incl_file_path, include_directories)) {
                return false;
            }
            break;
        }
    }
    return found_file;
}

// напишите эту функцию
bool Preprocess(const path &in_file, const path &out_file, const vector<path> &include_directories) {
    string include_string;
    string src_line;

    fstream src_file(in_file, ios::in);
    if (!src_file.is_open()) {
        return false;
    }

    fstream dst_file(out_file, ios::out);
    if (!dst_file.is_open()) {
        src_file.close();
        return false;
    }

    if (!CopyLines(dst_file, src_file, in_file, include_directories)) {
        return false;
    }
    return true;
}

string GetFileContents(string file) {
    ifstream stream(file);

    // конструируем string по двум итераторам
    return {(istreambuf_iterator<char>(stream)), istreambuf_iterator<char>()};
}

void Test() {
    error_code err;
    filesystem::remove_all("sources"_p, err);
    filesystem::create_directories("sources"_p / "include2"_p / "lib"_p, err);
    filesystem::create_directories("sources"_p / "include1"_p, err);
    filesystem::create_directories("sources"_p / "dir1"_p / "subdir"_p, err);

    {
        ofstream file("sources/a.cpp");
        file << "// this comment before include\n"
                "#include \"dir1/b.h\"\n"
                "// text between b.h and c.h\n"
                "#include \"dir1/d.h\"\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"
                "#   include<dummy.txt>\n"
                "}\n"s;
    }
    {
        ofstream file("sources/dir1/b.h");
        file << "// text from b.h before include\n"
                "#include \"subdir/c.h\"\n"
                "// text from b.h after include"s;
    }
    {
        ofstream file("sources/dir1/subdir/c.h");
        file << "// text from c.h before include\n"
                "#include <std1.h>\n"
                "// text from c.h after include\n"s;
    }
    {
        ofstream file("sources/dir1/d.h");
        file << "// text from d.h before include\n"
                "#include \"lib/std2.h\"\n"
                "// text from d.h after include\n"s;
    }
    {
        ofstream file("sources/include1/std1.h");
        file << "// std1\n"s;
    }
    {
        ofstream file("sources/include2/lib/std2.h");
        file << "// std2\n"s;
    }

    assert((!Preprocess("sources"_p / "a.cpp"_p, "sources"_p / "a.in"_p,
                        {"sources"_p / "include1"_p, "sources"_p / "include2"_p})));

    ostringstream test_out;
    test_out << "// this comment before include\n"
                "// text from b.h before include\n"
                "// text from c.h before include\n"
                "// std1\n"
                "// text from c.h after include\n"
                "// text from b.h after include\n"
                "// text between b.h and c.h\n"
                "// text from d.h before include\n"
                "// std2\n"
                "// text from d.h after include\n"
                "\n"
                "int SayHello() {\n"
                "    cout << \"hello, world!\" << endl;\n"s;

    cout << GetFileContents("sources/a.in"s);
    cout << test_out.str();
    cout.flush();
    assert(GetFileContents("sources/a.in"s) == test_out.str());
}

int main() {
    Test();
}