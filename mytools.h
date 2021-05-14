#ifndef MYTOOLS_H
#define MYTOOLS_H

#include<time.h>
#include<iostream>
#include<string>
#include<vector>
#include <functional>


using namespace std;

namespace mytools{
    bool tranverseFiles(string& base_path, vector<string>& ans, string& file_ext,const function<bool(string&)>& f);
    string simplifyPath(string& path);
    string GbkToUtf8(const char *src_str);
    string Utf8ToGbk(const char *src_str);
    bool chgFileCreateLocalTimet(string path, time_t t);
    time_t getFileCreateLocalTimet(string path);
    string getRelativePath(string path, string refpath);
    void splitString(string path, vector<string>& wd, char c);
    string splitPath(string s, int mode);
}

#endif // MYTOOLS_H
