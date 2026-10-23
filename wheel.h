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

#ifndef WHEEL_H
#define WHEEL_H

#include <QWidget>
#include <QList>
#include <QPaintEvent>

class Wheel : public QWidget
{
  Q_OBJECT

public:
  explicit Wheel(QWidget *parent = nullptr);
  ~Wheel() override;

public slots: 
  void setAngle(int angle);
  void setAnglef(float angle, float deltaT);

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  void calcBrightness(float timeDelta, bool allFalse);

  float currentAngle;
  int segments;
  int rings;
  int block_w;
  int block_h;

  float wheelFreq;

  QList< QList<float> > segState;
  QList< QList< QList<bool> > > wheelPositions;
};

#endif // WHEEL_H