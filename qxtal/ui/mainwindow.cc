#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QResource>
#include <QSettings>
#include <QVBoxLayout>

#include "sim/atari.h"
#include "sim/io.h"
#include "sim/simulator.h"

#include "NotifyCenter.h"
#include "notifications.h"


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
	|* Configure the UI object
	\*************************************************************************/
	ui->midVL->setSpacing(0);

	/*************************************************************************\
	|* Create the simulator
	\*************************************************************************/
	_io		= new IO();
	_sim	= new Simulator(0x10000, this);
	_atari	= Atari::instance(_sim, _io, true);

	//_sim->setDebug(Simulator::DBG_TRACE);
	_sim->setDebug(Simulator::DBG_MESSAGE);

	/*************************************************************************\
	|* Announce to who cares what the object pointers are
	\*************************************************************************/
	auto nc = NotifyCenter::defaultNotifyCenter();
	nc->notify(NTFY_SIM_AVAILABLE, _atari);

	/*************************************************************************\
	|* Tell the trace widget to talk to the memory widget
	\*************************************************************************/
	QObject::connect(ui->traceWidget, &TraceWidget::updateMemory,
					 ui->memoryWidget, &MemoryWidget::updateState);

	}

/*****************************************************************************\
|* Destructor
\*****************************************************************************/
MainWindow::~MainWindow()
	{
    delete ui;
	}



/*****************************************************************************\
|* Toolbar action - we want to load an XEX
\*****************************************************************************/
void MainWindow::on_actionLoad_XEX_triggered()
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
		_atari->load(files.at(0).toStdString());
	}

