#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemWatcher>

#include "NotifyCenter.h"
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
	GETSET(Simulator*, sim, Sim);					// Simulator engine
	GETSET(IO*, io, Io);							// Input/Output channel
	GETSET(Atari*, hw, Hw);							// Atari model with BIOS etc
	GET(uint32_t, address);							// Where to run from
	GET(QFileSystemWatcher, fsWatcher);				// Monitors XEX files

	private:
		Ui::MainWindow *ui;

		/*********************************************************************\
		|* Notification: a binary was just loaded
		\*********************************************************************/
		void _binaryLoaded(NotifyData &nd);

		/*********************************************************************\
		|* Notification: simulation over
		\*********************************************************************/
		void _simulationDone(NotifyData &nd);

		/*********************************************************************\
		|* UI : Load an XEX file
		\*********************************************************************/
		void _toolbarLoadXEX(void);

		/*********************************************************************\
		|* UI : Run the simulator
		\*********************************************************************/
		void _toolbarRunSim(void);

		/*********************************************************************\
		|* UI : Stop the simulator
		\*********************************************************************/
		void _toolbarStopSim(void);


	public:
	  MainWindow(QWidget *parent = nullptr);
	  ~MainWindow();


	private slots:

	  /*********************************************************************\
	  |* UI : we clicked on the load button
	  \*********************************************************************/
	  void _toolbarAction(QAction *a);

	  /*********************************************************************\
	  |* UI : we clicked on the load button
	  \*********************************************************************/
	  void _xexChanged(const QString &path);
	};
#endif // MAINWINDOW_H
