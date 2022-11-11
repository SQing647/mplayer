#include "widget.h"
#include "ui_widget.h"
QVector<lyric> txt_lrc;  //歌词容器
QVector<QString> vec_music; //播放列表容器
int ret;                    //有名管道的文件修饰符
int i = 0;                  //静音的判断符
int _pause = 0;         //暂停的判断符
int arr[2] = {0};       //无名管道的文件修饰符
int row;                    //歌曲列表当前的焦点行数
int music_num=0;    //歌曲列表中的歌曲总数
char song_name[50] = {0};
pthread_t pth1,pth2,pth3,pth4;  /*三个线程
pth1:  music_start   获取总时间
pth2:  music_other  歌名 作者 专辑
pth3:  cur_pos  当前播放位置
pth4:  lrc_roll  歌词的滚动
*/


//查找歌词文件
QString FindFile(const QString &strFilePath, const QString &strNameFilters)
{
    if (strFilePath.isEmpty() || strNameFilters.isEmpty())
    {
        return QString();
    }

    QDir dir;
    QStringList filters;
    filters << strNameFilters;
    dir.setPath(strFilePath);
    dir.setNameFilters(filters);
    QDirIterator iter(dir,QDirIterator::Subdirectories);
    while (iter.hasNext())
    {
        iter.next();
        QFileInfo info=iter.fileInfo();
        if (info.isFile())
        {
            return info.absoluteFilePath().replace('/', '\\');
        }
    }
    return QString();
}

//切割排序歌词并存入容器
void lrc_in_vec(QString &s){
    QFile qf(s);
    int p;
    char *str = new char[200];
    if(qf.open(QIODevice::ReadOnly)){
        qDebug() << "文件打开失败!!";
        return ;
    }
    qDebug() << "begin";
    while(!qf.atEnd()){
        qDebug() << "join";
        memset(str,0,200);
        qDebug() << p++;
        qf.readLine(str,200);
        qDebug() << str;
    }
    qf.close();
}

//设置歌曲当前时间
void Widget::curtime(int n){
    ui->music_long->setValue(n);
    char ch[5] = {0};
    sprintf(ch,"%2d:%2d",n/60,n%60);
    ui->nowtime->setText(ch);
}

//设置歌曲时间总长度
void Widget::music_length(int n){
    char str[7] = {0};
    sprintf(str,"/ %2d:%2d",n/60,n%60);
    ui->music_long->setMaximum(n);
    ui->time->setText(str);
}

//设置歌曲名
void Widget::music_name(char *s){
    ui->name->setText(s);
    ui->label->setText(s);
}

//设置歌手名
void Widget::music_singer(char *s){
    if(0==strlen(s)){
        ui->singer->setText("未知");
    }else{
        ui->singer->setText(s);
    }
}

//设置专辑名
void Widget::music_album(char *s){
    if(0==strlen(s)){
        ui->album->setText("未知");
    }else{
        ui->album->setText(s);
    }
}

//获取总长度并显示在屏幕上
void *music_start(void *arg){
    int length;
    char str[200] = {0};
    char *str1 = (char *)calloc(200,1);
    read(arr[0],str,200);
    write(ret,"get_time_length\n",strlen("get_time_length\n"));
    do {
        read(arr[0],str,200);
        str1 = strstr(str,"ANS_LENGTH=");
    }while(!str1);
    sscanf(str1,"ANS_LENGTH=%d.",&length);
    (*(Widget *)arg).music_length(length);
    usleep(100*1000);
    return NULL;
}

//获取 歌名 作者 专辑
void *music_other(void *arg){
    char *str = (char *)calloc(100,1);
    char *ans = str;
    int len;
    memset(song_name,0,100);
    //歌名
    write(ret,"get_file_name\n",strlen("get_file_name\n"));
    read(arr[0],ans,100);
    ans +=14;
    len = strlen(ans);
    *(ans+len-6) = 0;
    strncpy(song_name,ans,len-6);
    (*(Widget *)arg).music_name(ans);
    //作者
    ans = str;
    memset(str,0,100);
    write(ret,"get_meta_artist\n",strlen("get_meta_artist\n"));
    read(arr[0],ans,100);
    ans +=17;
    len = strlen(ans);
    *(ans+len-2) = 0;
    (*(Widget *)arg).music_singer(ans);
    //专辑
    ans = str;
    memset(str,0,100);
    write(ret,"get_meta_album\n",strlen("get_meta_album\n"));
    read(arr[0],ans,50);
    ans +=16;
    len = strlen(ans);
    *(ans+len-2) = 0;
    (*(Widget *)arg).music_album(ans);
    free(str);
    //找歌词文件
    strcat(song_name,".lrc");
    qDebug() << song_name;
    QString lrc_name = FindFile("/home/lsq/share/lrc",song_name);
    qDebug() << "lrc_name=" << lrc_name;
    if(!lrc_name.isEmpty()){
        lrc_in_vec(lrc_name);
    }else{
        qDebug() << "未找到歌词文件";
    }
    return NULL;
}

//获取播放时间
void *cur_pos(void *arg){
    pthread_detach(pthread_self());
    int cur_time;
    char str[200] = {0};
    char *str1 = (char *)calloc(200,1);
    while(1){
        if(_pause==0){
            write(ret,"get_time_pos\n",strlen("get_time_pos\n"));
            do {
                read(arr[0],str,200);
                str1 = strstr(str,"ANS_TIME_POSITION=");
            }while(!str1);
            sscanf(str1,"ANS_TIME_POSITION=%d.",&cur_time);
            (*(Widget *)arg).curtime(cur_time);
            sleep(1);
        }
    }
    return NULL;
}

//暂停按钮
void Widget::on_pause_clicked()
{
    static int x = 0;
    if(0 == x){
        _pause = 1;
        x = 1;
        ui->pause->setText("继续");
    }else{
        x = 0;
        _pause = 0;
        ui->pause->setText("暂停");
    }
    write(ret,"pause\n",strlen("pause\n"));
}

//静音按钮
void Widget::on_mute_clicked()
{
    _pause = 1;
    static int last_volume;
    if(0==i){
        i = 1;
        last_volume = ui->volume->value();
        write(ret,"mute 1\n",7);
        ui->mute->setText("取消静音");
        ui->volume->setValue(0);
    }else{
        i = 0;
        write(ret,"mute 0\n",7);
        ui->mute->setText("静音");
        ui->volume->setValue(last_volume);
    }
    _pause = 0;
}

//拖动音量条
void Widget::on_volume_valueChanged(int value)
{
    QString str;
    _pause = 1;
    if(0==value){
        i = 1;
        ui->mute->setText("取消静音");
    }else{
        i = 0;
        ui->mute->setText("静音");
    }
    str.sprintf("volume %3d%% 1\n",value);
    write(ret,str.toStdString().c_str(),strlen(str.toStdString().c_str()));
    _pause = 0;
}

//上一首
void Widget::on_deal_back_clicked()
{
    QString temp;
    if(0 == row){
        row = vec_music.size()-1;
    }else{
        row--;
    }
    ui->listWidget->setCurrentRow(row);
    temp.sprintf("loadfile \'%s\'\n",vec_music[row].toStdString().c_str());
    write(ret,temp.toStdString().c_str(),strlen(temp.toStdString().c_str()));
    pthread_cancel(pth3);
    pthread_create(&pth1,NULL,music_start,this);
    pthread_join(pth1,NULL);
    pthread_create(&pth2,NULL,music_other,this);
    pthread_join(pth2,NULL);
    pthread_create(&pth3,NULL,cur_pos,this);
}

//下一首
void Widget::on_deal_next_clicked()
{
    QString temp;
    QString str;
    if(vec_music.size() - 1 == row){
        row = 0;
    }else{
        row++;
    }
    ui->listWidget->setCurrentRow(row);
    temp.sprintf("loadfile \'%s\'\n",vec_music[row].toStdString().c_str());
    write(ret,temp.toStdString().c_str(),strlen(temp.toStdString().c_str()));
    pthread_cancel(pth3);
    pthread_create(&pth1,NULL,music_start,this);
    pthread_join(pth1,NULL);
    pthread_create(&pth2,NULL,music_other,this);
    pthread_join(pth2,NULL);
    pthread_create(&pth3,NULL,cur_pos,this);
}

//声音界面按钮
void Widget::on_voice_clicked()
{   static bool _volume = true;
    if(_volume){
        _volume = false;
        ui->widget->show();
    }else{
        _volume =true;
        ui->widget->hide();
    }
}

//添加音乐
void Widget::on_add_music_clicked()
{
    char *deal_name = (char *)calloc(100,1);
    char *str = deal_name;
    char ans[50] = {0};
    QString Path ="/home/lsq/share/mp3";
    QString dlgTitle="选择音频文件";
    QString filter="mp3文件(*.mp3);;wav文件(*.wav);;wma文件(*.wma);;所有文件(*.*)";
    QStringList fileList = QFileDialog::getOpenFileNames(this,dlgTitle,Path,filter);
    if(fileList.count()<1)
        return;
    music_num +=fileList.count();
    for(int i = 0;i < fileList.count();i++){
        memset(str,0,100);
        memset(ans,0,50);
        deal_name = str;
        vec_music.push_back(fileList.at(i));
        strncpy(deal_name,fileList.at(i).toStdString().c_str(),strlen(fileList.at(i).toStdString().c_str()));
        strtok(deal_name,"/");
        while (deal_name = strtok(NULL,"/")) {
            strncpy(ans,deal_name,strlen(deal_name));
        }
        qDebug() << vec_music[i];
        qDebug() << ans;
        ui->listWidget->insertItem(i,ans);
    }
    qDebug() << fileList.count();
    qDebug() << vec_music.size();
}

//拖动进度条
void Widget::on_music_long_sliderMoved(int position)
{
    QString str;
    _pause = 1;
    str.sprintf("seek %d 2\n",position);
    write(ret,str.toStdString().c_str(),strlen(str.toStdString().c_str()));
    _pause = 0;
}

//双击播放
void Widget::on_listWidget_doubleClicked(const QModelIndex &index)
{
    QString temp = "loadfile \'";
    row = index.row();
    static int degree = 0;
    if(0==degree){
        if(0 == fork()){
            execlp("mplayer","mplayer","-slave", "-quiet","-idle","-input", "file=/home/lsq/my_lab/mplayer/mod",
                   vec_music[row].toStdString().c_str(), NULL);
        }
    }else{
        pthread_cancel(pth3);
        qDebug() << vec_music.size();
        _pause = 0;
        temp = temp + vec_music[row] +"\'\n";
        qDebug() << temp;
        write(ret,temp.toStdString().c_str(),strlen(temp.toStdString().c_str()));
    }
    pthread_create(&pth1,NULL,music_start,this);
    pthread_join(pth1,NULL);
    pthread_create(&pth2,NULL,music_other,this);
    pthread_join(pth2,NULL);
    pthread_create(&pth3,NULL,cur_pos,this);
    degree++;
}
//歌词界面切换
void Widget::on_words_clicked()
{
    static int _x_ = 0;
    if(0 == _x_){
        _x_ = 1;
        ui->add_music->hide();
        ui->listWidget->hide();
        ui->lyric->show();
    }else{
        _x_ = 0;
        ui->add_music->show();
        ui->listWidget->show();
        ui->lyric->hide();
    }
}
//构造函数
Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->widget->hide();
    pipe(arr);
    dup2(1,10);
    dup2(arr[1],1);
    ret = open("/home/lsq/my_lab/mplayer/mod", O_RDWR);
    ui->lyric->hide();
}
//析构函数
Widget::~Widget()
{
    pthread_cancel(pth3);
    write(ret,"quit\n",strlen("quit\n"));
    delete ui;
}

