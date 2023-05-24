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
	ListResponse response;
private slots:


	void on_pushButtonListServices_clicked();


	void on_listWidget_name_currentRowChanged(int currentRow);



	void on_lineEdit_services_editingFinished();

	void on_listWidget_uuid_currentRowChanged(int currentRow);



	void on_lineEdit_connect_editingFinished();

	void processform();
//	void on_comboBox_currentIndexChanged(int index);

	void on_comboBox_activated(int index);

	void on_listWidget_meta_currentRowChanged(int currentRow);

	void on_listWidget_tag_currentRowChanged(int currentRow);
	void applyFilters();

	void on_listWidget_ep_currentRowChanged(int currentRow);

	void write_ipaddress();

	void take_ipaddress();


	void on_comboBox_ip_currentTextChanged(const QString &arg1);

	void on_listWidget_meta_itemDoubleClicked(QListWidgetItem *item);

	void on_listWidget_tag_itemDoubleClicked(QListWidgetItem *item);

	void on_listWidget_ep_itemDoubleClicked(QListWidgetItem *item);

signals:
	void updateform();

private:
	std::map<std::string, servicecheck::Service> allservices;
	std::map<std::string, servicecheck::Service> filteredService;
	std::map<std::string, servicecheck::Service> errorService;
	std::unique_ptr<servicecheck::Registry::Stub> stub_;
//	std::string target = "10.5.190.206:3456";
	std::string target = "";
	void filterServices(QString s);

};

#endif // FORM_H
