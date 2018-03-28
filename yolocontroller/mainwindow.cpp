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
                QRegularExpression re("^model_checkpoint_path.*");
                QRegularExpressionMatch match = re.match(line);
                bool hasMatch = match.hasMatch();
                if(!hasMatch){
                    // the first line of checkpoint_file is unexpected. abort
                    qDebug() << "the first line of checkpoint_file is unexpected. abort";
                    return -1;
                }


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



