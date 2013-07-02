//Modified version of newFileChooserWidget.cxx as modified by Joshua Murphy
// on 2 July 2013.  This file will be needed only until the bug is fixed in the
// master paraview file.

/*=========================================================================

   Program: ParaView
   Module:    newFileChooserWidget.cxx

   Copyright (c) 2005-2008 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/


#include "newFileChooserWidget.h"

// Qt includes
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>

// ParaView
#include "pqFileDialog.h"

newFileChooserWidget::newFileChooserWidget(QWidget* p)
  : QWidget(p), Server(NULL)
{
  this->ForceSingleFile = false;
  this->UseDirectoryMode = false;
  this->UseFilenameList = false;

  QHBoxLayout* l = new QHBoxLayout(this);
  l->setMargin(0);
  l->setSpacing(2);
  this->LineEdit = new QLineEdit(this);
  this->LineEdit->setObjectName("FileLineEdit");

  QToolButton* tb = new QToolButton(this);
  tb->setObjectName("FileButton");
  tb->setText("...");
  QObject::connect(tb, SIGNAL(clicked(bool)),
                   this, SLOT(chooseFile()));

  l->addWidget(this->LineEdit);
  l->addWidget(tb);

  QObject::connect(this->LineEdit,
                   SIGNAL(textChanged(const QString&)),
                   this, SLOT(handleFileLineEditChanged(const QString&)));

}

newFileChooserWidget::~newFileChooserWidget()
{
}


QStringList newFileChooserWidget::filenames() const
{
  if (this->UseFilenameList)
    {
    return this->FilenameList;
    }
  else
    {
    return newFileChooserWidget::splitFilenames(this->LineEdit->text());
    }
}

void newFileChooserWidget::setFilenames(const QStringList& files)
{
  this->UseFilenameList = false;
  this->LineEdit->setEnabled(true);

  if(this->UseDirectoryMode)
    {
    if (!files.isEmpty())
      {
        //this has been modified from the origanal... was .path() now .filePath()
        //.path() was return the incorrect results
      this->LineEdit->setText(QFileInfo(files[0]).filePath());
      }
    else
      {
      this->LineEdit->setText("");
      }
    }
  else if (this->ForceSingleFile)
    {
    if (!files.isEmpty())
      {
      this->LineEdit->setText(files[0]);
      }
    else
      {
      this->LineEdit->setText("");
      }
    }
  else if (files.size() > 1)
    {
    this->UseFilenameList = true;
    this->LineEdit->setEnabled(false);
    this->LineEdit->setText(files[0] + ";...");
    this->FilenameList = files;
    this->emitFilenamesChanged(files);
    }
  else
    {
    this->LineEdit->setText(newFileChooserWidget::joinFilenames(files));
    }
}

QString newFileChooserWidget::singleFilename() const
{
  QStringList files = this->filenames();
  if (files.isEmpty())
    {
    return "";
    }
  else
    {
    return files[0];
    }
}

void newFileChooserWidget::setSingleFilename(const QString &file)
{
  this->setFilenames(QStringList(file));
}

QString newFileChooserWidget::extension()
{
  return this->Extension;
}

void newFileChooserWidget::setExtension(const QString& ext)
{
  this->Extension = ext;
}

pqServer* newFileChooserWidget::server()
{
  return this->Server;
}

void newFileChooserWidget::setServer(pqServer* s)
{
  this->Server = s;
}

void newFileChooserWidget::chooseFile()
{
  QString filters = this->Extension;
  filters += ";;All files (*)";

  pqFileDialog* dialog = new pqFileDialog(this->Server,
    this, tr("Open File:"), QString(), filters);
  if(this->UseDirectoryMode)
    {
    dialog->setFileMode(pqFileDialog::Directory);
    }
  else if (this->forceSingleFile())
    {
    dialog->setFileMode(pqFileDialog::ExistingFile);
    }
  else
    {
    dialog->setFileMode(pqFileDialog::ExistingFiles);
    }
  if(QDialog::Accepted == dialog->exec())
    {
    QStringList files;
    // The file browser has a list of selected files, each of which could
    // be a group of files.  Condense them all into a single list.
    QStringList selectedFiles;
    foreach(selectedFiles, dialog->getAllSelectedFiles())
      {
      files << selectedFiles;
      }
    if(files.size())
      {
      this->setFilenames(files);
      }
    }
}

void newFileChooserWidget::handleFileLineEditChanged(const QString &fileString)
{
  if (this->UseFilenameList)
    {
    // Ignoring string from line edit.  Ignore this signal.
    return;
    }
  QStringList fileList = newFileChooserWidget::splitFilenames(fileString);
  this->emitFilenamesChanged(fileList);
}

void newFileChooserWidget::emitFilenamesChanged(const QStringList &fileList)
{
  emit this->filenamesChanged(fileList);
  if (!fileList.empty())
    {
    emit this->filenameChanged(fileList[0]);
    }
  else
    {
    emit this->filenameChanged("");
    }
}
