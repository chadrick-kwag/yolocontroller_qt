#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    load_defaults();

    connect(ui->findmodel_btn,SIGNAL(clicked(bool)),this,SLOT(openfilebrowser_findmodel()));
    connect(ui->annot_find_btn,SIGNAL(clicked(bool)),this,SLOT(openfilebrowser_findannot()));
    connect(ui->imagesdir_find_btn,SIGNAL(clicked(bool)),this,SLOT(openfilebrowser_findimagesdir()));
    connect(ui->previousckpt_find_btn,SIGNAL(clicked(bool)),this,SLOT(openfilebrowser_findprevckptdir()));
    connect(ui->traindir_find_btn,SIGNAL(clicked(bool)),this,SLOT(openfilebrowser_finddatasetdir()));
    connect(ui->prevckptnum_dropdown,SIGNAL(currentTextChanged(QString)),this,SLOT(check_ckptnumchanged()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::load_defaults(){
    ui->model_le->setText(MODEL_FILE_DEFAULT_VALUE);
    ui->epoch_le->setText(QString::number(EPOCH_DEFAULT_VALUE));
}


void MainWindow::openfilebrowser_findmodel(){
    qDebug() << "button clicked";
    QString filename = QFileDialog::getOpenFileName(this,"open model file",CFG_DIR,"All files(*.*)");
    qDebug() << filename + "selected";
    ui->model_le->setText(filename);
}

void MainWindow::openfilebrowser_findannot(){
    QString dirpath = QFileDialog::getExistingDirectory(this,"open annotation dir",DARKFLOW_BASEDIR,QFileDialog::ShowDirsOnly);
    qDebug() << dirpath + "selected";
    ui->annot_le->setText(dirpath);
}

void MainWindow::openfilebrowser_findimagesdir(){
    QString dirpath = QFileDialog::getExistingDirectory(this,"open images dir",DARKFLOW_BASEDIR,QFileDialog::ShowDirsOnly);
    ui->imagesdir_le->setText(dirpath);
}

void MainWindow::openfilebrowser_findprevckptdir(){
    QString dirpath = QFileDialog::getExistingDirectory(this,"open previous checkpoint dir",CKPT_DIR,QFileDialog::ShowDirsOnly);
    ui->previousckpt_le->setText(dirpath);


    QString checkpoint_path = dirpath + "/checkpoint";


    int setup_return = setup_prevckpt_select(checkpoint_path);

    if(setup_return){
        qDebug() << "error occured while setting up previous checkpoint";
    }


}

int MainWindow::setup_prevckpt_select(QString filepath){

    QFile checkpoint_file(filepath);
    if(checkpoint_file.exists()){
        qDebug() << "checkpoint file exists";

        int retval = parse_prevckpt_checkpoint_file(filepath);

        if(retval){
            return -1;
        }

        // populate dropdown menu
        ui->prevckptnum_dropdown->clear();

        for(auto const& i : prevckptlist){
            ui->prevckptnum_dropdown->addItem(QString::number(i));
        }


        return 0;
    }
    else{
        qDebug() << "checkpoint file doesn't exist";
        return -1;
    }

}

int MainWindow::parse_prevckpt_checkpoint_file(QString checkpoint_path){
    // assume that checkpoint_file exist check has already been done
    QFile checkpoint_file(checkpoint_path);

    //empty the prevckptlist
    prevckptlist.clear();

    if(checkpoint_file.open(QIODevice::ReadOnly)){
        QTextStream in(&checkpoint_file);
        bool firstread=true;

        while(!in.atEnd()){
            QString line = in.readLine();

            if(firstread){
                qDebug() << "inside first read";
                firstread=false;
                QRegularExpression re("^model_checkpoint_path.*\"(.+)-\\d+\"");
                QRegularExpressionMatch match = re.match(line);
                bool hasMatch = match.hasMatch();
                if(!hasMatch){
                    // the first line of checkpoint_file is unexpected. abort
                    qDebug() << "the first line of checkpoint_file is unexpected. abort";
                    return -1;
                }

                prevckptprefix = match.captured(1);



            }
            else{
                //
                qDebug() << line;

                QRegularExpression re("^all_model_checkpoint_paths.*-(\\d+)\\\"$");

                QRegularExpressionMatch match = re.match(line);

                qDebug() << "match.captured(1)=";
                qDebug() << match.captured(1);


                QString captured_ckptnum = match.captured(1);
                if(!captured_ckptnum.isEmpty()){
//                    prevckptlist.insert(captured_ckptnum.toInt(),prevckptlist.size());

                    int converted = captured_ckptnum.toInt();

                    prevckptlist.push_back(converted);
                }
            }
        }

        checkpoint_file.close();

    }

    //debug print all prevckptlist

    for(auto const& i : prevckptlist){
        qDebug() << i;
    }

    return 0;
}


void MainWindow::openfilebrowser_finddatasetdir(){
    QString dirpath = QFileDialog::getExistingDirectory(this,"open dataset dir",DATA_DIR,QFileDialog::ShowDirsOnly);

    if(dirpath.isEmpty() || dirpath.isNull()){
        return;
    }
    // try to extract the name for this dataset dir
    QDir datadir(dirpath);

    QString dirname = datadir.dirName();

    ui->traindir_le->setText(dirpath);


    // set the dirname as the suggested train session name
    ui->newsessiontitle_le->setText(dirname);

    // try to detect and setup annotation dir and images dir.
    QDir annotationdir(dirpath+"/annotations");
    if( annotationdir.exists()){
        ui->annot_le->setText(annotationdir.absolutePath());
    }

    QDir imagesdir(dirpath+"/images");
    if( imagesdir.exists()){
        ui->imagesdir_le->setText(imagesdir.absolutePath());
    }

}



bool MainWindow::check_ckptnum_files_ready(int ckptnum){
    QDir basedir(ui->previousckpt_le->text());

    // check if basedir exists
    if(!basedir.exists()){
        qDebug() << "basedir not found. abort";
        return false;
    }

    QString filebase = prevckptprefix+"-"+QString::number(ckptnum);
    qDebug() << "filebase="+filebase;

    for(auto &i : ckptcheck_extensions){
        QString filefullname=filebase+i;
        qDebug() << filefullname;
        QFile targetfile(basedir.absolutePath()+"/"+filefullname);
        if(!targetfile.exists()){
            qDebug() <<  targetfile.fileName() + " doesn't exist";
            return false;
        }
        else{
            qDebug() << targetfile.fileName() + " exists";
        }
    }

    return true;

}


void MainWindow::check_ckptnumchanged(){
    bool checkreturn = check_ckptnum_files_ready((ui->prevckptnum_dropdown->currentText()).toInt());
    if(checkreturn){
        qDebug() << "check passed. "+ QDir::currentPath()+"/images/ok.png";
        // show positive sign

        QPixmap passimage(QDir::currentPath()+"/images/ok.png");

        ui->checkimglabel->setPixmap(passimage);
        ui->checkimglabel->setScaledContents(true);
        //ui->checkimglabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);


    }
    else{

        qDebug() << "check failed";
        // show negative sign
        QPixmap failimage(QDir::currentPath()+"/images/no.png");
        ui->checkimglabel->setPixmap(failimage);
        ui->checkimglabel->setScaledContents(true);

    }
}
