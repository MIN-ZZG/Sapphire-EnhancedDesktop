#include "sinputdialog.h"
#include"sunit.h"
#include "sfile.h"
#include "SysFunctions.h"
#include "QWidget"
#include"QProcess"
#include "filefunc.h"
#include "qboxlayout.h"
#include"SysFunctions.h"
#include "qevent.h"
#include "qfileinfo.h"
#include "qlabel.h"
#include"QDebug"
#include"QUrl"
#include"QDesktopServices"
#include"QTextCodec"
#include "qmimedata.h"
#include"mainwindow.h"
#include "qpainter.h"
#include"QGraphicsDropShadowEffect"
#include"ContextMenu/contextmenu.h"
#include"QFileDialog"
#include"stooltip.h"
#include"snotice.h"
#define SET_ANCTION(NAME,TEXT,FUCTION)\
QAction *NAME = new QAction(#TEXT);\
    myMenu->addAction(NAME);\
    connect(NAME, &QAction::triggered, this, [=]()FUCTION);

SFile::SFile(SLayout *dis, int sizex, int sizey,QString filePath):SMultiFunc(dis,sizex,sizey)
{
    if(filePath!="")
    loadFromPath(filePath,true);

    SET_ANCTION(actOpenFileProperty,属性,{
        OpenFileProperty(this->filePath);
    })

    SET_ANCTION(actRename,重命名,{
        qDebug()<<fullName();
        SInputDialog* sip = SInputDialog::showInput("请输入新名字",fullName(),mapToGlobal(geometry().topRight())+QPoint(10,0));
        connect(sip,&SInputDialog::finalText,this,&SFile::renameFile);
    })
    SET_ANCTION(actSoftDelet,软删除,{
        SMultiFunc::Remove();
    })

}


void SFile::mouse_enter_action(){
    SMultiFunc::mouse_enter_action();
    QFileInfo fileInfo(filePath);
    qDebug() << "File info: " << fileInfo.filePath()<<scale<<gv->m_scale<<gv->displaySize<<gv->actualSize; // 调试信息
}


void SFile::double_click_action(QMouseEvent *event){
    //最终双击执行
    QString cmd= QString("file:///")+filePath;
    qDebug("cmd = %s",qPrintable(cmd));
    QDesktopServices::openUrl(QUrl(cmd));
}


QJsonObject SFile::to_json()
{
    QJsonObject rootObject = SMultiFunc::to_json();
    rootObject.insert("path",filePath);
    rootObject.insert("useFileIcon",useFileIcon);
    return rootObject;
}

void SFile::load_json(QJsonObject rootObject)
{
    SMultiFunc::load_json(rootObject);
    filePath =(rootObject.value("path").toString());
    useFileIcon =rootObject.value("useFileIcon").toBool();
    loadFromPath(filePath,false);
}


void SFile::processorTip()
{
    if(isDir)
        SToolTip::Tip("移动至“"+name+"”");
    else
        SToolTip::Tip("用“"+name+"”打开");;
}

void SFile::onShiftContextMenu(QContextMenuEvent *event)
{
    qDebug()<<filePath;
    ContextMenu::show(QStringList() << filePath, (void *)pmw->winId(), event->globalPos());
}


bool SFile::ProcessPath(QString path)
{
    qDebug()<<path<<filePath;
    bool removed = false;
    if(isDir){
        QString newName =  filePath+"/"+QFileInfo(path).fileName();
        removed = QFile::rename(path,newName);
        qDebug()<<"new name"<<newName;
    }
    else{
        QProcess* CalcPro = new QProcess(this);;
        CalcPro->start("cmd",QStringList()<<"/c"<<filePath<<path);
    }
    return removed;
}

void SFile::loadAimIcon(MyFileInfo info)
{
    SMultiFunc::setPix(info.aimIcon());
}

void SFile::setPix(QString pixPath, bool save)
{
    SMultiFunc::setPix(pixPath,save);

    if(save)
        useFileIcon = false;
}

void SFile::renameFile(QString newNameWithDot)
{
    QString folder = QFileInfo(filePath).path();
    QString newName =  folder+"/"+newNameWithDot;
    qDebug()<<"From"<<filePath<<"to"<<newName;
    qDebug()<<QFileInfo(filePath).isReadable();
    bool renamed = QFile::rename(filePath,newName);
    if(renamed){
        QString oldSuffix = suffix();

        nowExits.remove(filePath);
        nowExits[newName] = this;
        filePath = newName;
        setName(baseName());
        QString newSuffix =suffix();
        if(oldSuffix!=newSuffix){
            //重新加载图标
            setPix(filePath,false);
        }
    }
    SNotice::notice(QStringList()<<newName+":"+((renamed)?"成功":"失败"),"重命名文件");
}



void SFile::Remove()
{
    if(nowExits.contains(filePath)){
        nowExits.remove(filePath);
    }

    bool res = QFile::moveToTrash(filePath);
    if(!res){
    qDebug()<<"Unable to delete";
    QString cmd = "del \""+filePath+"\"";
    system(cmd.toStdString().c_str());
    }
    else{
        qDebug()<<"Deleted";
    }

    SUnit::Remove();
}


void SFile::loadFromMyFI(MyFileInfo info,bool init){

    filePath = info.filePath;
    if(QFileInfo(filePath).isDir()) isDir = true;
    nowExits[filePath] = this;


    if(init)
    setName(info.name);



    if(useFileIcon)
    loadAimIcon(info);

    if(info.okForAim()&&init)
    switch (default_steam_icon_type) {
    case 0:
        break;
    case 1:
        setBlockSize(1,2);
        setFullShow(true);
        break;
    case 2:
        setBlockSize(2,1);
        setFullShow(true);
        break;
    default:
        break;
    }


    gv->limitInisde = true;
    lb->setText(elidedLineText(lb, 3, name));
    if(init){
        useFileIcon = true;
        pixPath = "";
    }

}

void SFile::loadFromPath(QString filepath,bool init)
{
    qDebug()<<"Loading form path:"<<filepath;

    MyFileInfo info = path2MyFI(filepath);
    loadFromMyFI(info,init);
}

