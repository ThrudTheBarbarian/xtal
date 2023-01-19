#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "./ui_preferences.h"

#include <QFileDialog>
#include <QResource>
#include <QSettings>
#include <QToolBar>
#include <QVBoxLayout>

#include "sim/atari.h"
#include "sim/io.h"
#include "sim/simulator.h"
#include "sim/worker.h"

#include "notifications.h"
#include "commands.h"

/*****************************************************************************\
|* Constructor
\*****************************************************************************/
MainWindow::MainWindow(QWidget *parent)
		   :QMainWindow(parent)
		   ,ui(new Ui::MainWindow)
		   ,prefs(new Ui::Preferences)
	{
	/*************************************************************************\
	|* Set up the main UI
	\*************************************************************************/
	QResource::registerResource("resources.qrc");
	ui->setupUi(this);

	/*************************************************************************\
	|* Set up the preferences dialog
	\*************************************************************************/
	_prefsDialog = new QDialog(this);
	prefs->setupUi(_prefsDialog);
	connect(prefs->buttons, &QDialogButtonBox::accepted,
			this, &MainWindow::_prefsAccepted);

	/*************************************************************************\
	|* Set up the defaults parameters
	\*************************************************************************/
	QCoreApplication::setOrganizationName("MoebiusTechLLC");
	QCoreApplication::setOrganizationDomain("https://github.com/ThrudTheBarbarian/xtal");
	QCoreApplication::setApplicationName("qxtal");

	/*************************************************************************\
	|* Connect the QFilesystemWatcher to monitor the current XEX
	\*************************************************************************/
	connect(&_fsWatcher, &QFileSystemWatcher::fileChanged,
			this, &MainWindow::_xexChanged);

	/*************************************************************************\
	|* Listen for binary-loaded notifications
	\*************************************************************************/
	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->addObserver([=](NotifyData &nd){_binaryLoaded(nd);}, NTFY_BINARY_LOADED);
	nc->addObserver([=](NotifyData &nd){_simulationDone(nd);}, NTFY_SIM_DONE);

	/*************************************************************************\
	|* Configure the UI object
	\*************************************************************************/
	ui->midVL->setSpacing(0);

	/*************************************************************************\
	|* Configure the memory widget
	\*************************************************************************/
	ui->memoryWidget->setMemStartEditor(ui->memStart);

	/*************************************************************************\
	|* Tell the trace widget to talk to the memory widget
	\*************************************************************************/
	QObject::connect(ui->traceWidget, &TraceWidget::updateMemory,
					 ui->memoryWidget, &MemoryWidget::updateState);

	/*************************************************************************\
	|* Configure the toolbar
	\*************************************************************************/
	QObject::connect(ui->toolBar, &QToolBar::actionTriggered,
					 this, &MainWindow::_toolbarAction);
	ui->actionSimulate->setEnabled(false);
	ui->actionStop->setEnabled(false);

	/*************************************************************************\
	|* Connect up the menu for what counts/pages to display
	\*************************************************************************/
	QObject::connect(ui->countType, &QComboBox::currentIndexChanged,
					 ui->memoryWidget, &MemoryWidget::_countTypeChanged);


	/*************************************************************************\
	|* Create the simulator
	\*************************************************************************/
	_io		= new IO();
	_sim	= new Simulator(0x10000, this);
	_hw		= Atari::instance(_sim, _io, true);

	//_sim->setDebug(Simulator::DBG_TRACE);
	_sim->setDebug(Simulator::DBG_MESSAGE);

	/*************************************************************************\
	|* Announce to who cares what the object pointers are
	\*************************************************************************/
	nc->notify(NTFY_SIM_AVAILABLE, _hw);
	}

/*****************************************************************************\
|* Destructor
\*****************************************************************************/
MainWindow::~MainWindow()
	{
	delete ui;
	}



#pragma mark -- Toolbar


/*****************************************************************************\
|* Tell the world that the file has changed
\*****************************************************************************/
void MainWindow::_xexChanged(const QString& path)
	{
	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->notify(NTFY_XEX_CHANGED, path.toStdString());
	}

/*****************************************************************************\
|* Toolbar action - we want to load an XEX
\*****************************************************************************/
void MainWindow::_toolbarAction(QAction *a)
	{
	if (a->text() == "Load XEX")
		_toolbarLoadXEX();
	else if (a->text() == "Simulate")
		_toolbarRunSim();
	else if (a->text() == "Stop")
		_toolbarStopSim();
	else if (a->text() == "Settings")
		_toolbarSettings();
	}

/*****************************************************************************\
|* Toolbar action - we want to load an XEX
\*****************************************************************************/
void MainWindow::_toolbarLoadXEX(void)
	{
	QSettings settings;
	QString defaultPath = getenv("HOME");
	QString xexPath = settings.value(LOAD_XEX_SETTING,
									 defaultPath).toString();

	QFileDialog chooser(this);
	chooser.setFileMode(QFileDialog::ExistingFile);
	chooser.setDirectory(xexPath);
	chooser.setNameFilter(tr("Binaries (*.bin *.com *.xex)"));
	chooser.setViewMode(QFileDialog::Detail);

	QStringList files;
	if (chooser.exec())
		{
		files = chooser.selectedFiles();
		settings.setValue(LOAD_XEX_SETTING,
						  chooser.directory().absolutePath());
		}

	if (files.length() > 0)
		{
		QString path = files.at(0);
		_hw->load(path.toStdString());

		if (_fsWatcher.files().size() > 0)
			_fsWatcher.removePaths(_fsWatcher.files());
		_fsWatcher.addPath(path);
		}
	}

/*****************************************************************************\
|* Toolbar action - launch the settings dialog
\*****************************************************************************/
void MainWindow::_toolbarSettings(void)
	{
	_prefsDialog->show();
	}

/*****************************************************************************\
|* Toolbar action - we want to run the simulator
\*****************************************************************************/
void MainWindow::_toolbarRunSim(void)
	{
	ui->actionStop->setEnabled(true);

	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->notify(NTFY_SIM_START, (int)_address);

	_hw->worker()->schedule(CMD_RESET, _address);
	fprintf(stderr, "Running from address: $%04x\n", _address);
	_hw->worker()->schedule(CMD_PLAY_FORWARD, _address);
	}


/*****************************************************************************\
|* Toolbar action - we want to run the simulator
\*****************************************************************************/
void MainWindow::_toolbarStopSim(void)
	{
	_hw->worker()->stop();
	}



#pragma mark -- Notifications


/*****************************************************************************\
|* A binary was loaded
\*****************************************************************************/
void MainWindow::_binaryLoaded(NotifyData& nd)
	{
	_address = nd.integerValue();
	ui->actionSimulate->setEnabled(true);
	}


/*****************************************************************************\
|* Simulation is complete
\*****************************************************************************/
void MainWindow::_simulationDone(NotifyData& nd)
	{
	ui->actionStop->setEnabled(false);
	}




#pragma mark -- Preferences

/*****************************************************************************\
|* We want to change the preferences
\*****************************************************************************/
void MainWindow::_prefsAccepted(void)
	{
	_prefVals.cycleLimit = prefs->cyclesLimit->text().toInt();

	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->notify(NTFY_PREFS_CHANGED, &_prefVals);
	}




