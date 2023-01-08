#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "properties.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class Simulator;
class IO;
class Atari;

class MainWindow : public QMainWindow
	{
    Q_OBJECT

	public:
		const QString LOAD_XEX_SETTING	= "paths/loadXex";

	/*************************************************************************\
	|* Properties
	\*************************************************************************/
	GETSET(Simulator*, sim, Sim);		// Simulator engine
	GETSET(IO*, io, Io);				// Input/Output channel
	GETSET(Atari*, atari, Atari);		// Atari model with BIOS etc

	public:
	  MainWindow(QWidget *parent = nullptr);
	  ~MainWindow();

	private slots:

	  void on_actionLoad_XEX_triggered();

	private:
	 Ui::MainWindow *ui;
	};
#endif // MAINWINDOW_H
