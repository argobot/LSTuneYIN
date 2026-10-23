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

#include "LED.h"

#include <QPainter>

// --------------------------------------------
// Tamaño lógico de referencia en puntos/píxeles virtuales de interfaz
const int LOGICAL_DIAMETER_PX = 19; 
// --------------------------------------------

LED::LED(QWidget *parent) 
  : QWidget(parent),
    state(false),
    onColor(0, 255, 0),
    offColor(0, 0, 0),
    width(LOGICAL_DIAMETER_PX),
    height(LOGICAL_DIAMETER_PX)
{
  setFixedSize(width, height);
  setAutoFillBackground(false);
}

LED::~LED()
{
}

void LED::setState(bool newState)
{
  if (state != newState) {
    state = newState;
    update();
  }
}

void LED::toggle()
{
  setState(!state);
}

void LED::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  if (state) {
    painter.setBrush(onColor);
  } else {
    painter.setBrush(offColor);
  }

  painter.drawEllipse(1, 1, width - 2, height - 2);
}

void LED::mousePressEvent(QMouseEvent *event) 
{
  Q_UNUSED(event);
  emit clicked();
}