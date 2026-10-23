/*   Copyright 2010 Craig Eaton
 *
 *   This file is part of LSTune.
 *
 *    LSTune is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) 
 * any later version.
 *
 *   LSTune is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 *   You should have received a copy of the GNU General Public License along
 * with LSTune. If not, see http://www.gnu.org/licenses/.
 */ 

#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QAudioDevice>

#include "tuner.h"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  auto *mainWin = new QMainWindow();
  auto *mWidget = new Tuner(mainWin);

  mainWin->setWindowTitle(QObject::tr("LSTune"));
  mainWin->setGeometry(100, 100, 650, 380);
  mainWin->setCentralWidget(mWidget);

  QMenu *fileMenu = mainWin->menuBar()->addMenu(QObject::tr("&File"));
  QMenu *inputMenu = fileMenu->addMenu(QObject::tr("&Audio Input"));
  
  QAction *quitAction = fileMenu->addAction(QObject::tr("&Quit"));
  QObject::connect(quitAction, &QAction::triggered, &app, &QCoreApplication::quit);

  const QList<QAudioDevice> devices = mWidget->getDevices();
  for (int i = 0; i < devices.size(); ++i) {
    QAction *action = inputMenu->addAction(devices.at(i).description());
    QObject::connect(action, &QAction::triggered, mWidget, [mWidget, i]() {
      mWidget->switchDevice(i);
    });
  }

  QMenu *helpMenu = mainWin->menuBar()->addMenu(QObject::tr("&Help"));
  QAction *aboutAction = helpMenu->addAction(QObject::tr("&About"));
  QObject::connect(aboutAction, &QAction::triggered, mWidget, &Tuner::about);
  
  mainWin->show();
  return app.exec();
}