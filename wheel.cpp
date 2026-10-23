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

#include "wheel.h"

#include <cmath>
#include <QPainter>
#include <QColor>
#include <QPalette>
#include <QDebug>

const int OFFSET = 3;

static void init_wheel(QList<bool> &wheel, int segments, int pos, int offset)
{
  for (int n = 0; n < segments; n++) {
    int str_pos = (n + pos) % segments;
    int idx = str_pos + offset;
    if (2 * n < segments + 1) {
      wheel.replace(idx, true);
    } else {
      wheel.replace(idx, false);
    }
  }
}

Wheel::Wheel(QWidget *parent) 
  : QWidget(parent),
    currentAngle(0.0f),
    segments(2 * 2 * 2 * 3 * 5), // 120
    rings(4),
    block_w(5),
    block_h(20),
    wheelFreq(0.0f)
{
  setFixedSize(QSize(block_w * segments + OFFSET, block_h * rings + OFFSET));

  segState = QList< QList<float> >(rings, QList<float>(segments, 0.0f));

  setPalette(QPalette(QColor(250, 250, 200)));
  setAutoFillBackground(false);

  wheelPositions = QList< QList< QList<bool> > >();
  for (int r = 0; r < rings; r++) {
    wheelPositions.append(QList< QList<bool> >());
    for (int p = 0; p < segments; p++) {
      wheelPositions[r].append(QList<bool>());
      for (int s = 0; s < segments; s++) {
        wheelPositions[r][p].append(false);
      }
    }
  }

  for (int ring = 0; ring < rings; ring++) {
    for (int pos = 0; pos < segments; pos++) {
      int idx = segments - 1 - pos;
      int ring_divisions = ring + 2;
      int division_segments = segments / ring_divisions;

      for (int division = 0; division < ring_divisions; division++) {
        int offset = division * division_segments;
        init_wheel(wheelPositions[ring][idx], division_segments, pos, offset);
      }
    }
  }
}

Wheel::~Wheel()
{
}

void Wheel::calcBrightness(float timeDelta, bool allFalse)
{
  const float lightTC = -10e-3f;
  const float strobeLength = 750e-6f;

  int wheelPos = static_cast<int>(std::floor(segments * currentAngle / (2.0f * static_cast<float>(M_PI))));
  
  for (int ring = 0; ring < rings; ring++) {
    for (int n = 0; n < segments; n++) {
      bool state = wheelPositions[ring][wheelPos][n];
      float b0 = segState[ring][n];

      if (!allFalse && state) {
        segState[ring][n] = (1.0f - b0) * (1.0f - std::exp(strobeLength / lightTC)) + b0;
      } else {
        segState[ring][n] = b0 * std::exp(timeDelta / lightTC);
      }
    }
  }
}

void Wheel::setAngle(int angle)
{
  double angle_radians = (static_cast<double>(angle) / static_cast<double>(segments)) * 2.0 * M_PI;
  currentAngle = static_cast<float>(std::fmod(angle_radians, 2.0 * M_PI));
  calcBrightness(1.0f / 100.0f, false);
  update();
}

void Wheel::setAnglef(float angle, float deltaT)
{
  bool allFalse = (angle < -2.0f * static_cast<float>(M_PI));
  if (!allFalse) {
    currentAngle = std::fmod(angle, 2.0f * static_cast<float>(M_PI));
  }
  calcBrightness(deltaT, allFalse);
  update();
}

void Wheel::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);
  QPainter painter(this);

  for (int ring = 0; ring < rings; ring++) {
    for (int n = 0; n < segments; n++) {
      int green = static_cast<int>(255.0f * segState[ring][n]);
      if (green > 255 || green < 0) {
        qDebug() << "Error: Green = " << green << "state=" << segState[ring][n];
        segState[ring][n] = 0.5f;
        green = 127;
      }

      int x = block_w * n + OFFSET;
      int y = block_h * ring + OFFSET;
      painter.setBrush(QColor(0, green, 0));
      painter.drawRect(QRect(x, y, block_w, block_h));
    }
  }
}