
#include<io.h>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<queue>

#include<string>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>
#include<regex>
#include <json/json.h>
#include <functional>

#ifdef _WIN32
#include <windows.h>
#else
#include <iconv.h>
#endif

#include "mytools.h"

// C:/Users/10729/Desktop/测试

bool mytools::tranverseFiles(string& base_path, vector<string>& ans, string& file_ext,const function<bool(string&)>& f){
    queue<string> q;
    long long handle;
    _finddata_t file_info;
    string path; //也可以用/*来匹配所有

    q.push("");

    while(!q.empty()){
        path = q.front();

        q.pop();
        handle=_findfirst((base_path + '/'+ path + "/*.*").c_str(),&file_info);
        do{
            if(-1!=handle){
                if(file_info.attrib==_A_SUBDIR){
                    string name = file_info.name;
                    if(f(name))
                        continue;
                    if(strcmp(file_info.name,"..")!=0 && strcmp(file_info.name,".")!=0){
                        q.push(path+'/'+file_info.name);
                    }
                }else{
                    string filename = file_info.name;
                    if(filename.substr(filename.find_last_of('.') + 1) == file_ext){
                        //ans.emplace_back(GbkToUtf8((path+'/'+filename).c_str()));

                        ans.emplace_back(((path+'/'+filename).c_str()));
                        //cout << path+'/'+filename << endl;
                    }

                }
            }
        }while(!_findnext(handle,&file_info));
        _findclose(handle);
    }

    return true;
}

string mytools::simplifyPath(string& path) {
    //cout << path << endl;
    stringstream is(path);
    vector<string> strs;
    string res = "", tmp = "";
    while(getline(is, tmp, '/')) {
        if(tmp == "" || tmp == ".")
            continue;
        else if(tmp == ".." && !strs.empty())
            strs.pop_back();
        else if(tmp != "..")
            strs.push_back(tmp);
    }
    for(string str:strs)
        res +=  "/" + str;
    if(res.empty())
        return "/";
    //cout << res << endl;
    return res;   // 有没有斜杠？
}

void mytools::splitString(string path, vector<string>& wd, char c){
    stringstream is(path);
    string tmp ;
    while(getline(is, tmp, c)) {
        if(tmp == "")   continue;
        wd.push_back(tmp);
    }
}

string mytools::splitPath(string s, int mode){
    if(mode == 0)   return s.substr(0, s.rfind("."));   // 返回不带后缀的文件名
    string::size_type iPos = s.find_last_of('/') + 1;
    if(mode == 1)
        return s.substr(iPos, s.length() - iPos);       // 返回文件名
    else if(mode == 2)
        return s.substr(0, iPos - 1);                   // 返回去除文件名后的路径
    else return "";
}

string mytools::getRelativePath(string path, string refpath)
{
    vector<string> wds, wdfs;
    splitString(path, wds, '/');
    splitString(refpath, wdfs, '/');

    string wd, wdf, ans;
    int k = 0, sz_wd = wds.size(), sz_wdf = wdfs.size();

    for(k = 0; (k < sz_wd && k < sz_wdf && (wds[k] == wdfs[k])); k++);
    for(int i = 0; i < sz_wd-k; i++)    ans+="../";
    for(int i = k; i < sz_wdf; i++)   ans += wdfs[i] + "/";

    if(ans[0] != '.')   ans = "./" + ans;
    return ans;
}

#ifdef _WIN32

string mytools::GbkToUtf8(const char *src_str)
{
    int len = MultiByteToWideChar(CP_ACP, 0, src_str, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, src_str, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    string strTemp = str;
    if (wstr) delete[] wstr;
    if (str) delete[] str;
    return strTemp;
}

string mytools::Utf8ToGbk(const char *src_str)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, src_str, -1, NULL, 0);
    wchar_t* wszGBK = new wchar_t[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, src_str, -1, wszGBK, len);
    len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
    char* szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
    string strTemp(szGBK);
    if (wszGBK) delete[] wszGBK;
    if (szGBK) delete[] szGBK;
    return strTemp;
}


time_t mytools::getFileCreateLocalTimet(string path){
    WCHAR wszClassName[256];
    memset(wszClassName,0,sizeof(wszClassName));
    MultiByteToWideChar(CP_ACP,0,path.c_str(),strlen(path.c_str())+1,wszClassName,sizeof(wszClassName)/sizeof(wszClassName[0]));

    HANDLE hFile = CreateFile(wszClassName, FILE_WRITE_ATTRIBUTES | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE){
        cout << "getFileCreateLocalTimet: file open failed - " << path << endl;
        return 0;
    }
    FILETIME fCreateTime, fAccessTime, fWriteTime, localTime;
    SYSTEMTIME st;
    GetFileTime(hFile, &fCreateTime, &fAccessTime, &fWriteTime);

    FileTimeToLocalFileTime(&fCreateTime,&localTime);//将文件时间转换为本地文件时间
    FileTimeToSystemTime(&localTime, &st);//将文件时间转换为本地系统时间

    struct tm gm = {st.wSecond, st.wMinute, st.wHour, st.wDay, st.wMonth-1, st.wYear-1900, st.wDayOfWeek, 0, 0};
    CloseHandle(hFile);
    return mktime(&gm);
}

bool mytools::chgFileCreateLocalTimet(string path, time_t timet){
    WCHAR wszClassName[256];
    memset(wszClassName,0,sizeof(wszClassName));
    MultiByteToWideChar(CP_ACP,0,path.c_str(),strlen(path.c_str())+1,wszClassName,sizeof(wszClassName)/sizeof(wszClassName[0]));

    HANDLE hFile = CreateFile(wszClassName, FILE_WRITE_ATTRIBUTES | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE){
        cout << "chgFileCreateLocalTimet: file open failed" << endl;
        return false;
    }
    FILETIME fCreateTime, fAccessTime, fWriteTime;
    GetFileTime(hFile, &fCreateTime, &fAccessTime, &fWriteTime);

    tm temptm = *gmtime(&timet);
    SYSTEMTIME st  = { 1900 + temptm.tm_year
                    , 1 + temptm.tm_mon
                    , temptm.tm_wday
                    , temptm.tm_mday
                    , temptm.tm_hour
                    , temptm.tm_min
                    , temptm.tm_sec
                    , 0};
    SystemTimeToFileTime(&st, &fCreateTime);
    SetFileTime(hFile, &fCreateTime, &fAccessTime, &fWriteTime);
    CloseHandle(hFile);
    return true;
}


#else

int GbkToUtf8(char *str_str, size_t src_len, char *dst_str, size_t dst_len)
{
    iconv_t cd;
    char **pin = &str_str;
    char **pout = &dst_str;

    cd = iconv_open("utf8", "gbk");
    if (cd == 0)
        return -1;
    memset(dst_str, 0, dst_len);
    if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
        return -1;
    iconv_close(cd);
    *pout = '\0';

    return 0;
}

int Utf8ToGbk(char *src_str, size_t src_len, char *dst_str, size_t dst_len)
{
    iconv_t cd;
    char **pin = &src_str;
    char **pout = &dst_str;

    cd = iconv_open("gbk", "utf8");
    if (cd == 0)
        return -1;
    memset(dst_str, 0, dst_len);
    if (iconv(cd, pin, &src_len, pout, &dst_len) == -1)
        return -1;
    iconv_close(cd);
    *pout = '\0';

    return 0;
}


#endif


