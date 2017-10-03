#ifndef PHOTODESC_H
#define PHOTODESC_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QGraphicsScene>
#include <exiv2/exiv2.hpp>

namespace Ui {
class photoDesc;
}

class photoDesc : public QMainWindow
{
    Q_OBJECT

public:
    explicit photoDesc(QWidget *parent = 0);
    ~photoDesc();

private slots:
    void on_fileTreeView_doubleClicked(const QModelIndex &index);

    void on_pushButton_clicked();

    void on_clear_button_clicked();

    void on_clear_comment_button_clicked();

    void on_restore_button_clicked();

    void on_comboBox_oientation_activated(int index);

    void on_pushButton_next_clicked();

    void on_pushButton_previous_clicked();

    void on_pushButton_undo_desc_clicked();

    void on_pushButton_undo_comment_clicked();

    void on_pushButton_copy_desc_clicked();

    void on_pushButton_copy_comment_clicked();

private:
    Ui::photoDesc *ui;
    QFileSystemModel model;
    QModelIndex currentFileIndex;

    QImage  *imageObject;
    QGraphicsScene *scene;
    QPixmap *picture;

    Exiv2::ExifData exif;
    int orientation;
    QStringList orientations;
    QString fnam;
    std::string fnam_utf8;

    QString comment_old;
    QString description_old;
    int orientation_old;

    QString comment_previous;
    QString description_previous;

    bool saved;

    void loadFileList();
    void showDate();
    void showDescription();
    void showComment();
    void showOrientation();
    void displayImage();
    int exifToIndex(int exifValue);
    void showDialogs(bool show);
    void enableDialogs(bool enable);
    bool checkForChanges();
    void saveExifData();
    void nextImage(const QModelIndex &index);

    int orientation_map[4] = {1,8,6,3};

};

#endif // PHOTODESC_H
