/*
 * motiondialog.h
 *
 * Author: Manu Sutela
 * Copyright (C) 2018 Aalto University
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MOTIONDIALOG_H
#define MOTIONDIALOG_H

#include <QDialog>

class MainWindow;

namespace Ui {
class MotionDialog;
}

/*!
 * MotionDialog is only visible when checked in the file menu of MainWindow.
 * It is used to display the 'differential image' produced by a
 * MotionDetectorEvent, which can be in turn used to adjust the threshold value
 * for motion detection.
 */
class MotionDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MotionDialog(MainWindow *parent = nullptr);
    ~MotionDialog();

public slots:
    void setPixmap(const QPixmap&);

private:
    MotionDialog(const MotionDialog&);
    MotionDialog& operator=(const MotionDialog&);

    void closeEvent(QCloseEvent*);

    Ui::MotionDialog *ui;
    MainWindow* window_;
};

#endif // MOTIONDIALOG_H
