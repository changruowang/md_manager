#ifndef MDEDIT_H
#define MDEDIT_H
#include <vector>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include<time.h>

using namespace std;

struct FileInfo{
    //FileInfo(string p, bool h, unordered_set<time_t>& l): path(p), head(h), link(l){};
    FileInfo(string p, bool h): path(p), head(h){};
    FileInfo(): path(""), head(false){};
    bool head;
    string path;
    unordered_set<time_t> link;
    unordered_set<time_t> linked;
};

class MdEdit
{
public:

    MdEdit(string path): m_filePath(path), m_fileInfoMap(nullptr){
        m_cmds.clear();
        m_cmds.push_back("cd hexo_bolg_files\ncall hexo clean\ncall hexo d");
        m_cmds.push_back("start /b git pull\ncd hexo_bolg_files\ncall hexo clean\ncd ..\ngit add .\ngit commit -m \"Sync\"\ngit push");
    };
    ~MdEdit(){
        if(m_fileInfoMap){
            m_fileInfoMap->clear();
            delete m_fileInfoMap;
        }

    }
    string m_filePath = "E:/mynote";          // 根目录
    string m_yamInform = "abstract: \ntags: \npassword: 1997\ncategories: \n";
    bool m_autoInsertYaml = true;

    bool dirFilter(string& name);
    void makeJsonCathe();
    
    void setFilePath(string path);
    void setYamlHead(string head);
    string getFilePath();
    string getYamlHead();

    void freshFileIDs();

    bool readJsonCathe();
    void makeMemoryCathe();
    void removeCatheFile();
    void chgFileLinks(string filename);
    string getFileLink(string path_src, string path_dst);
    void insertYamlHeadUTF8(const char* filefullpath);
    vector<string>& getPushCmds();
private:
    void insertYamlHead(string filename);
    void chgFileHead(time_t id, string new_filename,bool hasYaml);
    auto mergTwoMap(unordered_map<time_t, FileInfo>* new_map) ->decltype (new_map);
    bool traverseFolder(string& base_path, string name, vector<string>& ans);
    bool matchOneFile(string filename, vector<string>& ans);
    void insertOneRecord(string file, unordered_map<time_t, FileInfo>* m_cathe);
    time_t getFileId(string file);
    void setFileId(string file, time_t id);
    void chgFileLinks(time_t id, string new_filename);
    bool replaceFileString(string filename, string str1, string str2);
    string creatTmpFile(string filename);

    vector<string> m_cmds;
    unordered_map<time_t, FileInfo>* m_fileInfoMap;

};

#endif // MDEDIT_H
