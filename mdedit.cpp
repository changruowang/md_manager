#include<string>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fstream>
#include<regex>
//#include <tchar.h>
#include <json/json.h>
//#include <unordered_set>
#include <functional>

#include "mytools.h"
#include "mdedit.h"


void MdEdit::removeCatheFile(){
    remove("cathe.json");
}

vector<string>& MdEdit::getPushCmds(){
    return m_cmds;
}

bool MdEdit::matchOneFile(string filename, vector<string>& ans){
    ifstream ifs;

    ifs.open(m_filePath + "/" + filename, ios::in);

    if (!ifs.is_open())
         return false;
    string buf;
    regex pattern("\\[(.*?)\\]\\((.*?\\.md)\\)");  // 匹配md连接
    bool st_line = true;
    bool hasYaml = false;
    while (getline(ifs, buf))
    {
        if(st_line && (buf == "---"))               //第一行是否有 yaml 头信息
            hasYaml = true;
        st_line = false;
        smatch result;
        if(regex_search(buf, result, pattern)){
            for(auto &it : result){
                string tmp = it.str();
                if(tmp.substr(tmp.size()-3) == ".md"){
                    ans.emplace_back(mytools::Utf8ToGbk(tmp.c_str()));
                }
            }
        }
    }
    ifs.close();
    return hasYaml;
}

string MdEdit::creatTmpFile(string filename){
    time_t id = getFileId(filename);   // 修改内容的时候采用创建临时文件的方法 因此要记录 id
    string tmp_filename = mytools::splitPath(filename, 0) + "_tmp.md";
    string tmp_filepath = m_filePath + "/" + tmp_filename;
    ofstream outstream(tmp_filepath);
    outstream.close();
    setFileId(tmp_filename, id);
    return tmp_filepath;
}

string MdEdit::getFileLink(string file_src, string file_dst){
    string path_src = mytools::splitPath(file_src, 2);
    string path_dst = mytools::splitPath(file_dst, 2);
    string filename = mytools::splitPath(file_dst, 1);

    string linkpath = mytools::getRelativePath(path_src, path_dst) + filename;
    filename = filename.substr(0, filename.size()-2);

    return "[" + filename + "]" + "(" + linkpath  + ")";
}

void MdEdit::insertYamlHeadUTF8(const char* filefullpath){
    string tmp = mytools::Utf8ToGbk(filefullpath);
    insertYamlHead(tmp);
}

void MdEdit::insertYamlHead(string filefullpath){

    string head = m_yamInform;
    string filename = mytools::getRelativePath(m_filePath, filefullpath);
    filename = filename.substr(1, filename.size()-2);  //去掉首尾的 '/'

    bool &hasHead = m_fileInfoMap->at(getFileId(filename)).head;

    if(hasHead) return;   // 已经有了yaml头

    vector<string> cat;
    mytools::splitString(filename, cat, '/');
    string title = cat.back();
    head = "---\ntitle: " + title.substr(0, title.size()-3) + "\n" + head;
    for(unsigned long long i = 0; i < cat.size()-1; i++){
        head += ("- " + cat[i] + "\n");
    }
    head += "---\n\n\n<!--more-->\n";

    string tmp_filepath = creatTmpFile(filename);
    string filepath = m_filePath + "/" + filename, str;

    ifstream insfile(filepath);
    if (!insfile.is_open()){
        remove(tmp_filepath.c_str());
        return;
    }

    ofstream outfile(tmp_filepath);

    outfile << mytools::GbkToUtf8(head.c_str()) << endl;
    while (getline(insfile, str))
        outfile << str << endl;

    insfile.close();
    outfile.close();
    remove(filepath.c_str());
    rename(tmp_filepath.c_str(), filepath.c_str());
    hasHead = true;
}

bool MdEdit::replaceFileString(string filename, string str1, string str2){
    str1 = mytools::GbkToUtf8(str1.c_str());   //  md 文件中的字符编码使用的utf8  所以匹配字符串也得是 utf8 否则找不到
    str2 = mytools::GbkToUtf8(str2.c_str());

    string tmp_filepath = creatTmpFile(filename);
    string filepath = m_filePath + "/" + filename;

    ifstream instream(filepath);
    ofstream outstream(tmp_filepath);
    if (!instream.is_open())
         return false;

    string str;
    size_t pos = 0;
    while (getline(instream, str))
    {
        pos = str.find(str1); // 查找每一行中的"Tom"
        if (pos != string::npos)
        {
            str = str.replace(pos, str1.size(), str2); //
            outstream << str << endl;
            continue;
        }
        outstream << str << endl;

    }
    instream.close();
    outstream.close();
    remove(filepath.c_str());
    rename(tmp_filepath.c_str(), filepath.c_str());

    return true;
}

bool MdEdit::dirFilter(string& name){
    return (name == "hexo_bolg_files" || name == ".git");
}


void MdEdit::chgFileLinks(string filename){
    time_t id = getFileId(filename);
    chgFileLinks(id, filename);
    m_fileInfoMap->at(id).path = filename;
}

void MdEdit::chgFileLinks(time_t id, string new_filename){
    if((m_fileInfoMap == nullptr) || (m_fileInfoMap->find(id) == m_fileInfoMap->end()))    return;
    FileInfo& info = m_fileInfoMap->at(id);

    string use_filename, use_path;
    string old_filename = info.path;  // 这个路径是没修改之前 内存缓存 中的路径
    string old_path = mytools::splitPath(old_filename, 2);
    string old_name = mytools::splitPath(old_filename, 1);
    string new_path = mytools::splitPath(new_filename, 2);
    string new_name = mytools::splitPath(new_filename, 1);
    // 修改引用了它的文件中的路径
    for(auto &id : info.linked){
        use_filename = m_fileInfoMap->at(id).path;
        use_path =  mytools::splitPath(use_filename, 2);

        string old_str =  mytools::getRelativePath(use_path, old_path) + old_name;
        string new_str =  mytools::getRelativePath(use_path, new_path) + new_name;

        //cout << "1: " << old_str << " " << new_str << endl;

        if(!replaceFileString(use_filename, old_str, new_str))
            cout << "文件打开失败 :" <<  use_filename << endl;
    }
    // 修该该文件中自己的引用，因为当自身路径变了，它里面引用别的文件的引用也会失效
    for(auto &id : info.link){
        use_filename = m_fileInfoMap->at(id).path;
        use_path =  mytools::splitPath(use_filename, 2);
        old_name =  mytools::splitPath(use_filename, 1);

        string old_str =  mytools::getRelativePath(old_path, use_path) + old_name;
        string new_str =  mytools::getRelativePath(new_path, use_path) + old_name;

        if(!replaceFileString(new_filename, old_str, new_str))
            cout << "文件打开失败 :" <<  old_filename << endl;
    }
}

void MdEdit::chgFileHead(time_t id, string new_filename, bool hasYaml){
    if(!hasYaml)  return;  //没有头 m_autoInsertYaml
    if((m_fileInfoMap == nullptr) || (m_fileInfoMap->find(id) == m_fileInfoMap->end()))    return;


    FileInfo& info = m_fileInfoMap->at(id);
    string old_path = mytools::splitPath(info.path, 2);
    string new_path = mytools::splitPath(new_filename, 2);
    if(old_path == new_path)    return; // 路径没变  不用改变类别

    vector<string> cats;
    mytools::splitString(new_path, cats, '/');
    int n_cats = (int)cats.size();
    string tmp_filepath = creatTmpFile(new_filename);
    string filepath = m_filePath + "/" + new_filename;

    ifstream instream(filepath);
    ofstream outstream(tmp_filepath);
    if (!instream.is_open())
         return;

    string str;
    int state  = 0;
    while (getline(instream, str))
    {
        if(state == 0){
            if(str.find("categories") != string::npos){
                state = 1;
                str = "categories: ";
            }
        }else if(state <= n_cats){
            do{
                str = mytools::GbkToUtf8(("- " + cats[state - 1]).c_str());
                state++;
                outstream << str << endl;
            }while(state <= n_cats);
            continue;
        }else if((state == n_cats+1) && (str[0] == '-') && (str[1] == ' ')){
            continue;
        }else if((state == n_cats+1) && !((str[0] == '-') && (str[1] == ' '))){
            state++;
        }else{}
        outstream << str << endl;
    }
    instream.close();
    outstream.close();
    remove(filepath.c_str());
    rename(tmp_filepath.c_str(), filepath.c_str());
}

auto MdEdit::mergTwoMap(unordered_map<time_t, FileInfo>* new_map) ->decltype (new_map){
    if(m_fileInfoMap == nullptr)  return new_map;

    vector<pair<time_t, string>> chgFiles;

    for(auto &it : (*new_map)){
        if(m_fileInfoMap->find(it.first) != m_fileInfoMap->end()){
            FileInfo& info = m_fileInfoMap->at(it.first);
            if(info.path != it.second.path){
                chgFileHead(it.first, it.second.path, it.second.head);
                chgFileLinks(it.first, it.second.path);  // 被修改过文件名 或者是 被移动过
                info.path = it.second.path;
            }
        }
    }
    new_map->clear();
    return m_fileInfoMap;
}


bool MdEdit::readJsonCathe(){

    Json::Reader reader;
    Json::Value root;

    //从文件中读取，保证当前文件有demo.json文件
    ifstream in("cathe.json", ios::binary);

    if (!in.is_open()){
        cout << "no cathe file \n";
        return false;
    }

    if (reader.parse(in, root)){

        m_filePath = root["base_path"].asString();

        m_fileInfoMap = new unordered_map<time_t, FileInfo>();
        auto members = root["items"].getMemberNames();
        for(auto &key : members ){
            time_t id = atoll(key.c_str());
            FileInfo info(root["items"][key]["path"].asString(), (root["items"][key]["head"].asString() == "true"));

            auto son_members = root["items"][key].getMemberNames();
            for(auto k : son_members){
                if(k == "link"){
                    for(auto &id : root["items"][key]["link"])
                        info.link.insert(atoll(id.asString().c_str()));
                }
                else if(k == "linked")
                    for(auto &id : root["items"][key]["linked"])
                        info.linked.insert(atoll(id.asString().c_str()));
            }
            m_fileInfoMap->insert({id, info});
        }

        m_yamInform = root["yaml_head"].asString();
        m_filePath = root["base_path"].asString();

        if(!root["push_cmds"].empty()){
            m_cmds.clear();
            for(auto c : root["push_cmds"])
                m_cmds.push_back(c.asString());
        }

    }
    in.close();
    return true;
}

time_t MdEdit::getFileId(string file){
    return mytools::getFileCreateLocalTimet(m_filePath + '/' + file);
}

void MdEdit::setFileId(string file, time_t id){
    mytools::chgFileCreateLocalTimet(m_filePath + '/' + file, id);
}

void MdEdit::insertOneRecord(string file, unordered_map<time_t, FileInfo>* m_cathe){
    time_t id = getFileId(file);
    if(id == 0){
        cout << "insertOneRecord: open files erro -- "<< file << endl;
        return;
    }
    if(m_cathe->find(id) != m_cathe->end()){
        id = time(nullptr);
        while(m_cathe->find(id) != m_cathe->end())
            id++;
        setFileId(file, id);
    }
    FileInfo info;
    info.path = file;
    vector<string> mdlinks;
    info.head = matchOneFile(file, mdlinks);

    for(auto& md_file : mdlinks){
        string abpath = mytools::splitPath(file, 2) + '/' + md_file;
        string re = mytools::simplifyPath(abpath);
        time_t id_ = getFileId(re);
        if(id_ != 0)    info.link.insert(id_);
        else    cout << re <<  " is a invalid link in " << file << endl;
    }
    m_cathe->insert({id, info});
}

void MdEdit::makeJsonCathe(){
    Json::Value root;
    Json::Value child;

    for(auto &it : (*m_fileInfoMap)){
        string idstr = to_string(it.first);
        Json::Value son;
        son["head"] = it.second.head;
        son["path"] = it.second.path;
        for(auto& lk : it.second.link)
            son["link"].append(to_string(lk));
        for(auto& lk : it.second.linked)
            son["linked"].append(to_string(lk));
        child[idstr] = Json::Value(son);
    }
    root["items"] = Json::Value(child);
    root["base_path"] =  m_filePath.c_str();
    root["yaml_head"] = m_yamInform.c_str();

    for(auto &cmd : m_cmds){
        root["push_cmds"].append(cmd);
    }

    ofstream os;
    os.open("cathe.json", std::ios::out | std::ios::trunc);
    Json::StyledWriter sw;
    os << sw.write(root);
    os.close();
}

void MdEdit::makeMemoryCathe(){
    auto pMap = new unordered_map<time_t, FileInfo>();   // 在mergTwoMap函数中释放
    auto m_fileIDMap = unordered_map<string, time_t>();
    // 遍历文件夹
    string ext = "md";
    auto filter = bind(&MdEdit::dirFilter, this, std::placeholders::_1);
    vector<string> mdfiles;
    mytools::tranverseFiles(m_filePath, mdfiles, ext, filter);

    // 新增一条记录
    for(auto& file : mdfiles){
        insertOneRecord(file, pMap);
    }
    for(auto &data : (*pMap)){
        for(auto id : data.second.link)
            pMap->at(id).linked.insert(data.first);
    }

    // 将新读出的缓冲区的结果和文件中记载的结果做对比，如果文件有变动，修改文件中的链接，修改之后，重新扫描建立缓冲区
    // 为了逻辑简单  第一次第一次只对比修改文件中的引用，第二次扫描重新建立缓冲区
    m_fileInfoMap = mergTwoMap(pMap);
    if(pMap != m_fileInfoMap){
        delete pMap;
        delete m_fileInfoMap;
        m_fileInfoMap = nullptr;
        makeMemoryCathe();
    }
}


void MdEdit::setFilePath(string path){
    m_filePath = mytools::Utf8ToGbk(path.c_str());
}
void MdEdit::setYamlHead(string head){
    m_yamInform = mytools::Utf8ToGbk(head.c_str());
}
string MdEdit::getFilePath(){
   return  mytools::GbkToUtf8(m_filePath.c_str());
}
string MdEdit::getYamlHead(){
    return mytools::GbkToUtf8(m_yamInform.c_str());
}
