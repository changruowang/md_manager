

QString getFullPath(QModelIndex &index){
    QString str;

    vector<QString> files;

    QModelIndex p = index;
    while(p.parent().isValid()){
        files.push_back(p.data().toString());
        p = p.parent();
    }
    reverse(files.begin(), files.end());
    for(auto n : files)
        str += ("/" + n);
    return str;
}
