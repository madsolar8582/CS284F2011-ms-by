/***
|* Created by Brian Yarbrough & Madison Solarana
|* Created on 11/10/2011
|* Last Modified on 11/12/2011
***/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// ### INCLUDES ################################################################
#include <QMainWindow>
#include <QTcpSocket>
#include <QtGui>
#include <QtCore>

// ### GLOBAL CONSTANTS ########################################################
const unsigned short DEFAULT_PORT = 1300;

// ### NAMESPACES ##############################################################
namespace Ui
{
    class MainWindow;
    class connectionInfoDialog;
}

// ### CLASS DECLARATIONS ######################################################
/***
|* @class	MainWindow
|* @desc	The main window for the application.
|*
|* @priv	QTcpSocket *				socket			TCP socket used to communicate with the server.
|* @priv	QDialog *					connectDialog	Popup Dialog used to gather connection details.
|* @priv	Ui::MainWindow *			ui				MainWindow's user interface.
|* @priv	Ui::connectionInfoDialog *	connectionInfo	connectDialog's user interface.
***/

/**
|*	@func	MainWindow
|*	@desc	Class constructor that sets the class up to be used properly.
**/

/**
|*	@func	~MainWindow
|*	@desc	Class deconstructor that properly cleans up the resources used by the class.
**/

/**
|*	@func	about
|*	@desc	Displays a popup containing information about the program.
**/

/**
|*	@slot	connectToServer
|*	@desc	Handles connecting to the server.
**/

/**
|*	@slot	sendToServer
|*	@desc	Handles sending data to the server.
**/

/**
|*	@slot	readFromServer
|*	@desc	Handles reading data to the server.
**/
class MainWindow : public QMainWindow
{
	Q_OBJECT
	public:
		/* Class Constructor / Deconstructor */
		explicit MainWindow(QWidget * parent = 0);
		~MainWindow();

	private:
		/* Private Class Functions */
		void about();

		/* Private Class Variables */
		QTcpSocket * socket;
		QDialog * connectDialog;
		Ui::MainWindow * ui;
		Ui::connectionInfoDialog * connectionInfo;

	public slots:
		/* Public Class Slots */
		void connectToServer();

	private slots:
		/* Private Class Slots */
		void sendToServer();
		void readFromServer();
};

#endif
