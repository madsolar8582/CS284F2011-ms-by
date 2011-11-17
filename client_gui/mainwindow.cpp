/***
|* Created by Brian Yarbrough & Madison Solarana
|* Created on 11/10/2011
|* Last Modified on 11/12/2011
***/

// ### INCLUDES ################################################################
#include <QInputDialog>
#include <QMessageBox>
#include <QTcpSocket>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_connectioninfodialog.h"

// ### CLASS DEFINITIONS #######################################################
/**
|*	@func	MainWindow
|*	@desc	Class constructor that sets the class up to be used properly.
**/
MainWindow::MainWindow(QWidget * parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	// Add the applications UI
	ui->setupUi(this);
	ui->centralWidget->setEnabled(false);

	// Watch the input for returns, and the submit button for clicks
	connect(ui->input, SIGNAL(returnPressed()), this, SLOT(sendToServer()));
	connect(ui->submit, SIGNAL(clicked()), this, SLOT(sendToServer()));

	// Initialize the socket for later use
	socket = new QTcpSocket(this);
	connect(socket, SIGNAL(readyRead()), this, SLOT(readFromServer()));

	// Add the connection info dialog
	connectDialog = new QDialog(this, Qt::WindowTitleHint);
	connectionInfo->setupUi(connectDialog);
	connectionInfo->port->setValue(DEFAULT_PORT);

	// Watch for the connection info dialog to be closed
	connect(connectDialog, SIGNAL(accepted()), this, SLOT(connectToServer()));
	connect(connectDialog, SIGNAL(rejected()), this, SLOT(close()));

	// Open the connection info dialog
	connectDialog->show();
}

/**
|*	@func	~MainWindow
|*	@desc	Class deconstructor that properly cleans up the resources used by the class.
**/
MainWindow::~MainWindow()
{
	// Close the socket and delete it
	if (socket->state() == QAbstractSocket::ConnectedState)
	{
		socket->disconnectFromHost();
		socket->waitForDisconnected();
	}
	delete socket;

	// Delete both UI elements
	delete ui;
	delete connectDialog;

	// delete connectionInfo; Should be handled by the deletion of connectDialog
}

/**
|*	@func	about
|*	@desc	Displays a popup containing information about the program.
**/
void MainWindow::about()
{
	QMessageBox::about(
		this,
		"About Simple Chat",
		"Authors: Brian Yarbrough & Madison Solarana\n"
		"Version: 1.0.0\n\n" 
		"Icon by Yusuke Kamiyamane (http://p.yusukekamiyamane.com/)"
	);

	return;
}

/**
|*	@slot	connectToServer
|*	@desc	Handles connecting to the server.
**/
void MainWindow::connectToServer()
{
	// Check for valid connection settings
	if (connectionInfo->server->text().isEmpty() || connectionInfo->nick->text().isEmpty())
	{
		QMessageBox::critical(this, "Error", "Invalid connection settings provided!");

		// Open the connection info dialog
		connectDialog->show();

		return;
	}

	// Let the user know we are connecting to the server
	ui->responses->appendPlainText("* Connecting to " + connectionInfo->server->text() + ":" + connectionInfo->port->text() + " as " + connectionInfo->nick->text() + "... *");

	// Connect to the server
	socket->connectToHost(connectionInfo->server->text(), connectionInfo->port->value());

	// Check if we actually got connected
	if (!socket->waitForConnected())
	{
		ui->responses->appendPlainText("* Connection Failed! (" + socket->errorString() + ") *");
		return;
	}

 	// Let the user know we got connected
	ui->centralWidget->setEnabled(true);
	ui->responses->appendPlainText("* Connection Successful! *");
	socket->write(QString(connectionInfo->nick->text()).toLatin1());
}

/**
|*	@slot	sendToServer
|*	@desc	Handles sending data to the server.
**/
void MainWindow::sendToServer()
{
	// Get the input to send and clear out the input box
	QString text = ui->input->text();
	ui->input->clear();

	// Check if they are wanting to see the about popup
	if (text == "/about")
	{
		about();
	}

	// Check if they are wanting to quit the application
	else if (text == "/exit" || text == "/quit" || text == "/part")
	{
		close();
	}

	// Else, send the input to the server and show it locally
	else
	{
		socket->write(text.toLatin1());
		ui->responses->appendPlainText(text.prepend(": ").prepend(connectionInfo->nick->text()));
	}
}

/**
|*	@slot	readFromServer
|*	@desc	Handles reading data to the server.
**/
void MainWindow::readFromServer()
{
	// Get the text from the server
	QString text = socket->readLine(256);

	// Check for server codes
	if (text.length() == 11 && text.startsWith("* CODE ") && text.endsWith(" *"))
	{
		if (text == "* CODE 00 *")
		{
			// Server is shutting down, let the user know
			text = "* Server shutting down! *";
		}
		else if (text == "* CODE 01 *")
		{
			// Connection was closed becuase too many clients have connected already
			text = "* Connection closed! (Max Clients Exceeded) *";
		}
		else if (text == "* CODE 02 *")
		{
			// Connection was closed becuase someone else is already using the nick requested
			text = "* Connection closed! (Nick Already In Use) *";
			QMessageBox::critical(this, "Error", "Nick already in use!");

			// Open the connection info dialog again to allow them to reconnect easily with a new nick
			connectDialog->show();
		}
		else if (text == "* CODE 03 *")
		{
			// Connection was closed becuase of server failure
			text = "* Connection closed! (Internal Server Failure) *";
		}
		else
		{
			// Should never hit this, but just in case, let the user know what code was sent
			text = "* Unknown Server Code Received! (" + text.replace("* ", "").replace(" *", "") + ") *";
		}

		// Close the socket and disable the cental widget (All known server codes will result in the server closing the connection with the client)
		socket->close();
		ui->centralWidget->setEnabled(false);
	}

	// Show the text
	ui->responses->appendPlainText(text);
}
