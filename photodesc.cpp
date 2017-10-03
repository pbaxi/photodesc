/*
    Copyright (C) 2017  Peter Baxendale

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "photodesc.h"
#include "ui_photodesc.h"
#include <QFileSystemModel>
#include <QMessageBox>
#include <exiv2/exiv2.hpp>


photoDesc::photoDesc(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::photoDesc)
{
    ui->setupUi(this);

    model.setRootPath("");

    loadFileList();

    orientations << "no rotation" << "rotate left" << "rotate right" << "invert";
    ui->comboBox_oientation->insertItems(0,orientations);
    ui->comboBox_oientation->setCurrentIndex(0);
    orientation = 1;
    orientation_old = 1;

    scene = NULL;
    imageObject = NULL;
    picture = NULL;
    saved = false;

    comment_previous = "";
    description_previous = "";

    showDialogs(false);     // hide entry areas
}

photoDesc::~photoDesc()
{
    delete ui;
}


// show the file selection dialog
void photoDesc::loadFileList()
{

   ui->fileTreeView->setModel(&model);

   QString rootPath = QDir::homePath();
   const QModelIndex rootIndex = model.index(QDir::cleanPath(rootPath));
   if (rootIndex.isValid())
         ui->fileTreeView->setRootIndex(rootIndex);
   ui->fileTreeView->setColumnHidden(1,true);
   ui->fileTreeView->setColumnHidden(2,true);
   ui->fileTreeView->setColumnHidden(3,true);
   ui->fileTreeView->setHeaderHidden(true);
   ui->fileTreeView->header()->setResizeMode(QHeaderView::ResizeToContents);
   ui->fileTreeView->header()->setStretchLastSection(false);
}



/*
 * Check if any changes have been made to
 * description, comment or orientation.
 * If they have, open dialog to offer save, discard or cancel.
 * Returns fals for cancel, true otherwise.
*/
bool photoDesc::checkForChanges()
{

    QString cmt = ui->textEdit_comment->toPlainText();
    QString dsc = ui->lineEdit_title->text();

    // check if any changes made
    if( (
         ( description_old != ui->lineEdit_title->text() ) ||
         ( comment_old != ui->textEdit_comment->toPlainText() ) ||
         ( orientation_old != orientation)
        ) && (saved == false)
       )
    {
        QMessageBox msgBox;
        msgBox.setInformativeText("Save your changes?");
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        int ret = msgBox.exec();

        switch (ret) {
          case QMessageBox::Save:
              // Save was clicked
              saveExifData();
              break;
          case QMessageBox::Discard:
              // Don't Save was clicked
              break;
          case QMessageBox::Cancel:
              // Cancel was clicked
              return false;
              break;
          default:
              // should never be reached
              break;
        }
    }

    return true;
}


// show image and exif data for selected file
void photoDesc::nextImage(const QModelIndex &index)
{
    // get filename as qstring and C++ string and show in status bar
    fnam = model.filePath(index);

    QFileInfo info(fnam);       // ignore if it's a directory
    if(info.isDir()) return;

    fnam_utf8 = fnam.toUtf8().constData();
    ui->statusBar->showMessage(fnam);

    // if not an exiv2 supported file type, show dialog and return
    int ftype = Exiv2::ImageFactory::getType(fnam_utf8);
    if(ftype == Exiv2::ImageType::none)
    {
        showDialogs(false);
        QMessageBox errmsg;
        errmsg.setText("Unsuported File Type for file:\n"+fnam);
        errmsg.setIcon(QMessageBox::Critical);
        errmsg.exec();
        return;
    }

    // save the previous description and comment
    comment_previous = ui->textEdit_comment->toPlainText();
    description_previous = ui->lineEdit_title->text();

    // clear the current diplayed exif data
    ui->label_datetaken->clear();
    ui->textEdit_comment->clear();
    ui->lineEdit_title->clear();
    ui->comboBox_oientation->setCurrentIndex(0);
    saved = false;

    // load the meta data
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(fnam_utf8);
    if(image.get() != 0)
    {
      image->readMetadata();
      exif = image->exifData();
      if (!exif.empty())
      {
        showDate();
        showDescription();
        showComment();
        showOrientation();
      }
      else
      {
          ui->statusBar->showMessage("no exif data: "+fnam);
          return;
      }
    }
    else
    {
        ui->statusBar->showMessage("failed to open file: "+fnam);
        return;
    }

    // show the picture

    delete scene;       // delete previous instances (if any)
    delete imageObject;
    delete picture;

    scene = new QGraphicsScene();
    imageObject = new QImage;

    imageObject->load(fnam);
    picture = new QPixmap(QPixmap::fromImage(*imageObject));

    ui->pushButton->setEnabled(true);
    ui->restore_button->setEnabled(true);
    ui->comboBox_oientation->setEnabled(true);
    ui->textEdit_comment->setEnabled(true);
    ui->lineEdit_title->setEnabled(true);

    showDialogs(true);  // show the exif data and controls
    displayImage();     // show the graphics box before scaling to fit
}


// file selected: load the image and exif data
void photoDesc::on_fileTreeView_doubleClicked(const QModelIndex &index)
{
    // check if there are any changes to save and deal with them
    if(!checkForChanges())
    {
        // if user chose cancel, revert file selected to previous one
        ui->fileTreeView->selectionModel()->setCurrentIndex(currentFileIndex, QItemSelectionModel::SelectCurrent);
        return;
    }

    currentFileIndex = index;
    nextImage(index);       // display the image and exif data

}



// hide or show all the relevant dialogs etc
void photoDesc::showDialogs(bool show)
{
    ui->pushButton->setVisible(show);
    ui->restore_button->setVisible(show);
    ui->comboBox_oientation->setVisible(show);
    ui->textEdit_comment->setVisible(show);
    ui->lineEdit_title->setVisible(show);
    ui->label_datetaken->setVisible(show);
    ui->comboBox_oientation->setVisible(show);
    ui->label_orientation->setVisible(show);
    ui->clear_button->setVisible(show);
    ui->clear_comment_button->setVisible(show);
    ui->graphicsView->setVisible(show);
    ui->label->setVisible(show);
    ui->label_2->setVisible(show);
    ui->pushButton_next->setVisible(show);
    ui->pushButton_previous->setVisible(show);
    ui->pushButton_copy_comment->setVisible(show);
    ui->pushButton_copy_desc->setVisible(show);
    ui->pushButton_undo_comment->setVisible(show);
    ui->pushButton_undo_desc->setVisible(show);
}



// show the date the photo was taken (exif Photo.DateTimeOriginal)
void photoDesc::showDate()
{
    Exiv2::ExifKey key("Exif.Photo.DateTimeOriginal");
    Exiv2::ExifData::iterator pos = exif.findKey(key);
    if (! (pos == exif.end()))
    {
        std::string s = pos->getValue()->toString();
        const char * str = s.c_str();
        QString d(str);
        ui->label_datetaken->setText("Date taken: "+d);
    }
    else ui->label_datetaken->setText("(date taken not available)");
}


// show the image.description exif data
void photoDesc::showDescription()
{
    Exiv2::ExifKey key("Exif.Image.ImageDescription");
    Exiv2::ExifData::iterator pos = exif.findKey(key);
    if (! (pos == exif.end()))
    {
        std::string s = pos->getValue()->toString();
        const char * str = s.c_str();
        QString d(str);
        ui->lineEdit_title->setText(d);
        description_old = d;
    }
    else description_old = "";
}


// show the photo.comment exif data
void photoDesc::showComment()
{
    Exiv2::ExifKey key("Exif.Photo.UserComment");
    Exiv2::ExifData::iterator pos = exif.findKey(key);
    if (! (pos == exif.end()))
    {
        std::string s = pos->getValue()->toString();
        const char * str = s.c_str();
        QString d(str);
        if(d.startsWith("charset=\"Ascii\" "))
        {
          d.remove("charset=\"Ascii\" ");
        }

        ui->textEdit_comment->setText(d);
        comment_old = d;
    }
    else comment_old = "";
}



// Get the exif orientation value.
// If none present, set to no rotation.
// Set combo box to show the current orientation.

void photoDesc::showOrientation()
{
    Exiv2::ExifKey key("Exif.Image.Orientation");
    Exiv2::ExifData::iterator pos = exif.findKey(key);

    if(! (pos == exif.end())) orientation = pos->toLong(); // get exif orientation value
    else orientation = 1;                                  // no exif value - set to no rotation

    ui->comboBox_oientation->setCurrentIndex(exifToIndex(orientation));
    orientation_old = orientation;  // so we can go back to original value
}

/*
 * Map exif orientation to combo box index
 * Only exif orientation values likely to be useful for a photo are considered.
*/
int photoDesc::exifToIndex(int exifValue)
{
    switch(exifValue)
    {
    case 1:
        return 0;
    case 8:
        return 1;
    case 3:
        return 3;
    case 6:
        return 2;
    default:
        return 0;
    }
}

// "Save" button pressed
void photoDesc::on_pushButton_clicked()
{   
   saveExifData();
   saved = true;
}


// save exif data to file
void photoDesc::saveExifData()
{
    // get the new comment, trim spaces, prepend with char set
    QString qsComment = ui->textEdit_comment->toPlainText().trimmed();
    qsComment.prepend("charset=\"Ascii\" ");

    // set the UserComment tag to the new comment
    exif["Exif.Photo.UserComment"] = qsComment.toStdString();

    // set the new description (title)
    std::string desc = ui->lineEdit_title->text().trimmed().toStdString();
    exif["Exif.Image.ImageDescription"] = desc;

    //set the new orientation
    exif["Exif.Image.Orientation"] = orientation;

    // write the new metadata to file
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(fnam_utf8);
    image->setExifData(exif);
    image->writeMetadata();
}

// clear description
void photoDesc::on_clear_button_clicked()
{
    ui->lineEdit_title->clear();
}

// clear comment
void photoDesc::on_clear_comment_button_clicked()
{
    ui->textEdit_comment->clear();
}

// restore original exif settings
void photoDesc::on_restore_button_clicked()
{
    if(picture != NULL)
    {
        ui->textEdit_comment->setText(comment_old);
        ui->lineEdit_title->setText(description_old);
        if(orientation != orientation_old)
        {
            orientation = orientation_old;
            ui->comboBox_oientation->setCurrentIndex(exifToIndex(orientation));
            displayImage();
        }
    }
    saved = false;
}


// display an image
void photoDesc::displayImage()
{
    int angle=0;

    switch(orientation)
    {
    case 8:
        angle = 270;
        break;
    case 3:
        angle = 180;
        break;
    case 6:
        angle = 90;
    }

    scene->clear();

    if(angle != 0)
    {
        QTransform transform;
        QTransform trans = transform.rotate(angle);
        QPixmap transImage = QPixmap(picture->transformed(trans));
        scene->addPixmap(transImage);
        scene->setSceneRect(transImage.rect());
    }

    else
    {
        scene->addPixmap(*picture);
        scene->setSceneRect(picture->rect());
    }

    ui->graphicsView->setScene(scene);
    ui->graphicsView->fitInView(ui->graphicsView->sceneRect(),Qt::KeepAspectRatio);

}


// user selects new orientation
void photoDesc::on_comboBox_oientation_activated(int index)
{
    int neworientation = orientation_map[index];
    if(neworientation == orientation) return;   // nothing to do if no change

    orientation = neworientation;
   // if(orientation != 0) displayImage();
    displayImage();
}


void photoDesc::on_pushButton_next_clicked()
{
    if(!checkForChanges()) return;

    int currentRow = ui->fileTreeView->currentIndex().row();
    QModelIndex p = ui->fileTreeView->currentIndex().parent();
    QModelIndex index = ui->fileTreeView->model()->index(currentRow+1,0,p);
    if(index.isValid())
    {
        ui->fileTreeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
        ui->fileTreeView->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        nextImage(index);
    }
}

void photoDesc::on_pushButton_previous_clicked()
{
    if(!checkForChanges()) return;

    int currentRow = ui->fileTreeView->currentIndex().row();
    QModelIndex p = ui->fileTreeView->currentIndex().parent();
    QModelIndex index = ui->fileTreeView->model()->index(currentRow-1,0,p);
    if(index.isValid())
    {
        ui->fileTreeView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
        ui->fileTreeView->selectionModel()->select(index,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        nextImage(index);
    }
}

void photoDesc::on_pushButton_undo_desc_clicked()
{
   ui->lineEdit_title->setText(description_old);
}

void photoDesc::on_pushButton_undo_comment_clicked()
{
   ui->textEdit_comment->setText(comment_old);
}

void photoDesc::on_pushButton_copy_desc_clicked()
{
    ui->lineEdit_title->setText(description_previous);
}

void photoDesc::on_pushButton_copy_comment_clicked()
{
    ui->textEdit_comment->setText(comment_previous);
}
