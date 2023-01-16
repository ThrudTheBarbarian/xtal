#include "mainwindow.h"
#include "./ui_mainwindow.h"

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
	{
	QResource::registerResource("resources.qrc");
	ui->setupUi(this);

	/*************************************************************************\
	|* Set up the defaults parameters
	\*************************************************************************/
	QCoreApplication::setOrganizationName("MoebiusTechLLC");
	QCoreApplication::setOrganizationDomain("https://github.com/ThrudTheBarbarian/xtal");
	QCoreApplication::setApplicationName("qxtal");

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
		_hw->load(files.at(0).toStdString());
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

