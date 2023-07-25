#include "mainwindow.h"
#include <QApplication>
#include <QScreen>

int main(int argc, char** argv) {
	QApplication app(argc, argv);
	MainWindow mw;

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenGeometry = screen->geometry();
	int height = screenGeometry.height();
	int width = screenGeometry.width();

	mw.resize(width / 2, height / 2);
	mw.move((width - mw.width()) / 2, (height - mw.height()) / 2);
	mw.show();

	return app.exec();
}
