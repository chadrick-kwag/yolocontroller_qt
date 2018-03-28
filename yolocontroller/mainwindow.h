#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QRegularExpression>
#include <QFile>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString DARKFLOW_BASEDIR = "/home/chadrick/prj/darkflow";
    QString CFG_DIR=DARKFLOW_BASEDIR+"/cfg";
    QString CKPT_DIR=DARKFLOW_BASEDIR+"/ckpt";
    QString DATA_DIR=DARKFLOW_BASEDIR+"/data";

    QString MODEL_FILE_DEFAULT_VALUE="/home/chadrick/prj/darkflow/cfg/tiny-yolo-face1.cfg";
    int EPOCH_DEFAULT_VALUE = 1000;

    std::list<int> prevckptlist;
    int selected_prevckpt_num;
    QString prevckptprefix;

    std::list<QString> ckptcheck_extensions = {".meta",".profile",".index",".data-00000-of-00001"};


    void load_defaults();
    int setup_prevckpt_select(QString);
    int parse_prevckpt_checkpoint_file(QString);
    bool check_ckptnum_files_ready(int ckptnum);

public slots:
    void openfilebrowser_findmodel();
    void openfilebrowser_findannot();
    void openfilebrowser_findimagesdir();
    void openfilebrowser_findprevckptdir();
    void openfilebrowser_finddatasetdir();
    void check_ckptnumchanged();

};

#endif // MAINWINDOW_H
