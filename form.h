#ifndef FORM_H
#define FORM_H

#include <QWidget>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <mutex>
#include <vector>
#include <QApplication>
#include "listservices.pb.h"
#include "listservices.grpc.pb.h"
#include <QListWidget>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using servicecheck::ListRequest;
using servicecheck::ListResponse;
using servicecheck::Registry;

namespace Ui {
class Form;
}

class Form : public QWidget
{
	Q_OBJECT

public:
	explicit Form(QWidget *parent = nullptr);
	~Form();
	Ui::Form *ui;

private slots:


	void on_pushButtonListServices_clicked();


	void on_listWidget_name_currentRowChanged(int currentRow);

	void on_pushButton_clicked();

	void on_listWidget_nodesName_currentRowChanged(int currentRow);

	void on_pushButton_Tags_clicked();

	//	void on_pushButton_meta_clicked();

	void on_lineEdit_services_editingFinished();

	void on_listWidget_uuid_currentRowChanged(int currentRow);

	void on_pushButton_meta_clicked();

private:
	std::unique_ptr<servicecheck::Registry::Stub> stub_;
	std::map<std::string, std::map<std::string, std::string>> devices_map;
	std::string target = "10.5.176.243:3456";
};

#endif // FORM_H
