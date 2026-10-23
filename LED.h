#ifndef LED_H
#define LED_H

#include <QWidget>
#include <QColor>
#include <QPaintEvent>
#include <QMouseEvent>

class LED : public QWidget
{
  Q_OBJECT

public:
  explicit LED(QWidget *parent = nullptr);
  ~LED() override;

public slots:
  void setState(bool newState);
  void toggle();

signals:
  void clicked();

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;

private:
  bool state;
  QColor onColor;
  QColor offColor;
  int width;
  int height;
};

#endif // LED_H