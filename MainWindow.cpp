#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "FramelessHelper.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QPainter>
#include <QScreen>
#include <QSettings>
#include <QTimer>
#include <QWindow>
MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint), ui(new Ui::MainWindow()) {
  ui->setupUi(this);

  setWindowTitle("Qt Widgets Inside");

  auto helper = new FramelessHelper(this);
  helper->setDraggableMargins(3, 3, 3, 3);
  helper->setMaximizedMargins(0, 0, 0, 0);
  helper->setTitleBarHeight(32);

  helper->addExcludeItem(ui->minimizeButton);
  helper->addExcludeItem(ui->maximizeButton);
  helper->addExcludeItem(ui->closeButton);

  connect(ui->minimizeButton, &QPushButton::clicked, helper,
          &FramelessHelper::triggerMinimizeButtonAction);
  connect(ui->maximizeButton, &QPushButton::clicked, helper,
          &FramelessHelper::triggerMaximizeButtonAction);
  connect(ui->closeButton, &QPushButton::clicked, helper,
          &FramelessHelper::triggerCloseButtonAction);

  connect(helper, &FramelessHelper::maximizedChanged, this,
          &MainWindow::updateMaximizeButton);

  ui->maximizeButton->setIcon(
      QIcon(QStringLiteral(":/res/maximize-button1.png")));

  QTimer::singleShot(100, this, &MainWindow::syncPosition);
}

MainWindow::~MainWindow() {
  const QFileInfo fileInfo(QCoreApplication::applicationFilePath());
  const QString iniFileName = fileInfo.completeBaseName() + ".ini";
  const QString iniFilePath = fileInfo.canonicalPath() + u'/' + iniFileName;
  QSettings(iniFilePath, QSettings::IniFormat)
      .setValue(this->objectName() + "/Geometry", geometry());
  QSettings(iniFilePath, QSettings::IniFormat)
      .setValue(this->objectName() + "/DevicePixelRatio", devicePixelRatioF());
  delete ui;
}

void MainWindow::updateMaximizeButton(bool maximized) {
  if (maximized) {
    ui->maximizeButton->setIcon(
        QIcon(QStringLiteral(":/res/maximize-button2.png")));
    ui->maximizeButton->setToolTip(tr("Restore"));
  } else {
    ui->maximizeButton->setIcon(
        QIcon(QStringLiteral(":/res/maximize-button1.png")));
    ui->maximizeButton->setToolTip(tr("Maximize"));
  }
  ui->maximizeButton->setAttribute(Qt::WA_UnderMouse, false);
}

void MainWindow::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter painter(this);
  QImage backgroundImage(QStringLiteral(":/res/background.png"));
  painter.drawImage(contentsRect(), backgroundImage);

#if 1
  painter.setPen(Qt::red);
  painter.drawRect(rect().adjusted(0, 0, -1, -1));
  painter.setPen(Qt::blue);
  painter.drawRect(rect().adjusted(4, 4, -5, -5));
#endif
}

void MainWindow::syncPosition() {
  // 窗口创建之后才能修改大小
  const QFileInfo fileInfo(QCoreApplication::applicationFilePath());
  const QString iniFileName = fileInfo.completeBaseName() + ".ini";
  const QString iniFilePath = fileInfo.canonicalPath() + u'/' + iniFileName;
  const auto savedGeometry =
      QSettings(iniFilePath, QSettings::IniFormat)
          .value(this->objectName() + "/Geometry", geometry())
          .toRect();
  auto const rec = QApplication::desktop()->availableGeometry(this);
  if (rec.width() <= savedGeometry.width() ||
      rec.height() <= savedGeometry.height()) {
//    return;
  }
  if (savedGeometry.isValid() && !parent()) {
    const auto savedDpr = QSettings(iniFilePath, QSettings::IniFormat)
                              .value(this->objectName() + "/DevicePixelRatio",
                                     devicePixelRatioF())
                              .toReal();
    // Qt doesn't support dpr < 1.
    const qreal oldDpr = std::max(savedDpr, qreal(1));
    const qreal scale = (devicePixelRatioF() / oldDpr);
    setGeometry(
        {savedGeometry.topLeft() * scale, savedGeometry.size() * scale});
  }
}
