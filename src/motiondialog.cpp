/*
 * motiondialog.cpp
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

#include "motiondialog.h"
#include "ui_motiondialog.h"
#include "mainwindow.h"

MotionDialog::MotionDialog(MainWindow *parent) :
    QDialog(parent),
    ui(new Ui::MotionDialog), window_(parent)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
}

MotionDialog::~MotionDialog()
{
    delete ui;
}

void MotionDialog::setPixmap(const QPixmap &p)
{
    ui->label->setPixmap(p);
}

void MotionDialog::closeEvent(QCloseEvent *)
{
    window_->toggleMotionDialogChecked(false);
}
