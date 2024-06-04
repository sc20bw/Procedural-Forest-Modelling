#pragma once

#include <QMainWindow>
#include "ui_User_Interface.h"

class User_Interface : public QMainWindow
{
	Q_OBJECT

public:
	User_Interface(QWidget *parent = nullptr);
	~User_Interface();

private:
	Ui::User_InterfaceClass ui;
};
