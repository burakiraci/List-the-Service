#include <fstream>
#include "form.h"
#include "ui_form.h"
#include <QFontComboBox>
#include <QDebug>
#include <QSet>
#include <QClipboard>
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using servicecheck::ListRequest;
using servicecheck::ListResponse;
using servicecheck::Registry;


Form::Form(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Form)
{
	ui->setupUi(this);

	connect(this, &Form::updateform, this, &Form::processform);
	write_ipaddress();
	take_ipaddress();
}

Form::~Form()
{
	delete ui;
}

void Form::on_pushButtonListServices_clicked() {
	take_ipaddress();
	ListRequest request;
	ClientContext context;
	ListResponse response;
	QStringList nodes ;
	ui->listWidget_name->clear();
	ui->label_status->setText("Connecting....");
	auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	auto stub = Registry::NewStub(channel);
	auto deadline = std::chrono::system_clock::now() + std::chrono::milliseconds(4000);
	context.set_deadline(deadline);
	Status status = stub->ListServices(&context, request, &response);
	allservices.clear();
	if (status.ok()) {
		ui->label_status->setText("Connected");
	} else {
		ui->label_status->setText(QString::fromStdString(status.error_message()));
		return;
	}

	for(const auto& service : response.services()){
		allservices[service.serviceuuid()] = service;
	}
	ui->lcdNumber->display((int)allservices.size());
	emit this->updateform();
}



void Form::on_listWidget_name_currentRowChanged(int currentRow)
{
	ui->listWidget_uuid->clear();
	ui->listWidget_meta->clear();
	ui->listWidget_tag->clear();
	ui->listWidget_ep->clear();
	QListWidgetItem* item = ui->listWidget_name->currentItem();
	if (item != nullptr) {
		QString serviceName = item->text();
		int count = 0;
		for (const auto& service : filteredService) {
			if (QString::fromStdString(service.second.name()) == serviceName) {
				QString uuid = QString::fromStdString(service.first);
				ui->listWidget_uuid->addItem(uuid);
				count++;
			}
			ui->lcdNumber->display(count);
		}
	}
}

void Form::on_lineEdit_services_editingFinished()
{
	ui->listWidget_uuid->clear();
	ui->listWidget_meta->clear();
	ui->listWidget_tag->clear();
	ui->listWidget_name->clear();
	ui->listWidget_ep->clear();
	int count = 0;
	QString search_key = ui->lineEdit_services->text();

	QStringList keywords = search_key.split('&');

	for (const auto& service : filteredService) {
		std::string uuid = service.first;
		std::string name = service.second.name();

		bool has_keywords = true;

		for (const QString& keyword : keywords) {
			bool has_key = QString::fromStdString(name).contains(keyword, Qt::CaseInsensitive);

			if (!has_key) {
				for (auto it = service.second.meta().begin(); it != service.second.meta().end(); ++it) {
					std::string key = it->first;
					std::string value = it->second;

					has_key = QString::fromStdString(value).contains(keyword, Qt::CaseInsensitive);
					if (has_key) {
						break;
					}
				}
			}

			if (!has_key) {
				has_keywords = false;
				break;
			}
		}

		if (has_keywords) {
			filteredService[uuid] = service.second;
			count++;
		}
	}

	std::set<QString> uniqueNames;

	for (const auto& name_uuid : filteredService) {
		QString name = QString::fromStdString(name_uuid.second.name());
		if (!uniqueNames.count(name)) {
			uniqueNames.insert(name);
			ui->listWidget_name->addItem(name);
		}
	}

	for (const auto& name_uuid : filteredService) {
		ui->listWidget_uuid->addItem(QString::fromStdString(name_uuid.first));
	}

	ui->lcdNumber->display(count);

	this->processform();
}

void Form::on_listWidget_uuid_currentRowChanged(int currentRow)
{

	if (!ui->listWidget_uuid->currentItem()) {
		return;
	}

	QString uuid = ui->listWidget_uuid->currentItem()->text();
	ui->listWidget_ep->clear();
	ui->listWidget_meta->clear();
	ui->listWidget_tag->clear();
	int count = 0;
	for (const auto& service : filteredService) {
		if (service.first != uuid.toStdString()) {
			continue;
		}

		for (const auto& it : allservices) {
			if (it.first == uuid.toStdString()) {
				for (const auto& ep_it : it.second.endpoints()) {
					const std::string& endpointName = ep_it.first;
					const servicecheck::Endpoint& endpoint = ep_it.second;
					ui->listWidget_ep->addItem(QString::fromStdString(endpointName));
					ui->listWidget_ep->addItem(QString::fromStdString(endpoint.ipaddress()) + ":" + QString::number(endpoint.port()));
				}
				break;
			}
		}

		count++;

		for (const auto& meta_it : service.second.meta()) {
			std::string key = meta_it.first;
			std::string value = meta_it.second;

			QString item = QString::fromStdString(key) + ": " + QString::fromStdString(value);
			if (key != "app" && key != "tag") {
				ui->listWidget_meta->addItem(item);
			}
		}

		for (auto tag_it = service.second.tags().begin(); tag_it != service.second.tags().end(); tag_it++) {
			for (auto tag_ite = tag_it->begin(); tag_ite != tag_it->end(); tag_ite++) {
				ui->listWidget_tag->addItem(QString::fromStdString(tag_ite.base()));
				break;
			}
		}
		ui->textEdit_status->clear();
		int error_code = service.second.status().errorcode();
		if(error_code == 0) {
			ui->textEdit_status->append("RUNNING");
		}
		else if (error_code == 1) {
			ui->textEdit_status->append("WARNING");
		}
		else if (error_code == 2) {
			ui->textEdit_status->append("CRITICAL");
		}
		ui->textEdit_state->clear();
		for (auto it = service.second.status().metadata().begin() ; it != service.second.status().metadata().end() ; it++) {
			ui->textEdit_state->append(QString::fromStdString(it->second));
		}

		break;
	}

}

void Form::on_lineEdit_connect_editingFinished()
{
	ui->listWidget_meta->clear();
	ui->textEdit_status->clear();
	ui->listWidget_ep->clear();
	ui->listWidget_tag->clear();
	ui->listWidget_name->clear();
	ui->textEdit_nodename->clear();
	ui->listWidget_uuid->clear();
	QString connect = ui->lineEdit_connect->text();
	target = connect.toStdString();
	if(!ui->lineEdit_connect->text().isEmpty()){

		write_ipaddress();
		take_ipaddress();
	}
}

void Form::processform()
{
	// TODO:
	ui->listWidget_meta->clear();
	ui->textEdit_status->clear();
	ui->listWidget_ep->clear();
	ui->listWidget_tag->clear();
	ui->listWidget_name->clear();
	ui->textEdit_nodename->clear();
	ui->listWidget_uuid->clear();
	QString s = ui->lineEdit_services->text();
	filterServices(s);
	QSet<QString> names;
	QStringList nodes;
	QSet<QString> statuss;
	for(const auto& s: filteredService) {
		names.insert(QString::fromStdString(s.second.name()));
		ui->listWidget_uuid->addItem(QString::fromStdString(s.first));
		QString nodeName = QString::fromStdString(s.second.node().name());
		if(!nodes.contains(nodeName)){
			ui->textEdit_nodename->append(QString::fromStdString(s.second.node().name()));
			ui->textEdit_nodename->append(QString::fromStdString(s.second.node().nodeuuid()));
			nodes.append(QString::fromStdString(s.second.node().name()));
		}
		for (auto it = s.second.status().metadata().begin(); it != s.second.status().metadata().end(); it++) {
			QString statusName = QString::number(s.second.status().errorcode());
			if (ui->comboBox->findText(statusName) == -1) {
				ui->comboBox->addItem(statusName);
				statuss.insert(statusName);
			}
		}
	}
	ui->listWidget_name->addItems(names.values());
}

void Form::filterServices(QString s)
{
	filteredService.clear();

	if (s.isEmpty())
		filteredService = allservices;
	else {
		QStringList keywords = s.split(" & ");

		for (const auto& sp: allservices) {
			bool has_keywords = true;

			for (const QString& keyword : keywords) {
				bool has_key = false;
				const std::string& name = sp.second.name();
				const std::string& uuid = sp.first;

				has_key = QString::fromStdString(name).contains(keyword, Qt::CaseInsensitive) ||
						QString::fromStdString(uuid).contains(keyword, Qt::CaseInsensitive);

				if (!has_key) {
					for (const auto& meta : sp.second.meta()) {
						const std::string& key = meta.first;
						const std::string& value = meta.second;
						has_key = QString::fromStdString(key).contains(keyword, Qt::CaseInsensitive) ||
								QString::fromStdString(value).contains(keyword, Qt::CaseInsensitive);
						if (has_key)
							break;
					}
					if (!has_key) {
						for (const auto& tag : sp.second.tags()) {
							for (auto t = tag.begin() ; t != tag.end() ; t++) {
								has_key = QString::fromStdString(t.base()).contains(keyword, Qt::CaseInsensitive);
								if (has_key)
									break;
							}
							if (has_key)
								break;
						}
					}
				}
				if (!has_key) {
					has_keywords = false;
					break;
				}
			}
			if (has_keywords)
				filteredService.insert(sp);
		}
	}
}

void Form::on_comboBox_activated(int index)
{
	QString selectedError = ui->comboBox->itemText(index);
	filteredService.clear();

	for (const auto& service : allservices) {
		int error_code = service.second.status().errorcode();
		if (error_code == selectedError.toInt()) {
			filteredService[service.first] = service.second;
		}
	}

	ui->listWidget_name->clear();
	ui->listWidget_uuid->clear();
	ui->listWidget_meta->clear();
	ui->listWidget_tag->clear();
	ui->listWidget_ep->clear();

	QSet<QString> names;
	for (const auto& s : filteredService) {
		names.insert(QString::fromStdString(s.second.name()));
		ui->listWidget_uuid->addItem(QString::fromStdString(s.first));
	}

	ui->listWidget_name->addItems(names.values());
}

void Form::on_listWidget_meta_currentRowChanged(int currentRow)
{
	ui->lineEdit_services->clear();
	QListWidgetItem* currentItem = ui->listWidget_meta->currentItem();
	if (currentItem != nullptr) {
		QString meta = currentItem->text();
		QString newString_value = meta.mid(meta.indexOf(": ") + 2);
		QApplication::clipboard()->setText(newString_value);

	}
}

void Form::on_listWidget_tag_currentRowChanged(int currentRow)
{
	ui->lineEdit_services->clear();
	QListWidgetItem* currentItem = ui->listWidget_tag->currentItem();
	if (currentItem != nullptr) {
		QString tag = currentItem->text();
		QApplication::clipboard()->setText(tag);
	}
}


void Form::on_listWidget_ep_currentRowChanged(int currentRow)
{
	ui->lineEdit_services->clear();
	QListWidgetItem* currentItem = ui->listWidget_ep->currentItem();
	if(currentItem != nullptr) {
		QString ep = currentItem->text();
		QApplication::clipboard()->setText(ep);
	}
}

void Form::applyFilters()
{
	ui->listWidget_uuid->clear();
	ui->listWidget_name->clear();
	int count = 0;
	for (const auto& service : filteredService) {
		ui->listWidget_uuid->addItem(QString::fromStdString(service.first));

		QString name = QString::fromStdString(service.second.name());
		if (!ui->listWidget_name->findItems(name, Qt::MatchExactly).isEmpty()) {
			continue;
		}

		ui->listWidget_name->addItem(name);
		count++;
	}

	ui->lcdNumber->display(count);
}

void Form::write_ipaddress()
{
	std::ofstream take_ip("ip.txt", std::ios::out | std::ios::app);
	if (take_ip.is_open()) {
		if(!ui->lineEdit_connect->text().isEmpty()){
		take_ip << ui->lineEdit_connect->text().toStdString() << std::endl;
		take_ip.close();
		take_ipaddress();
		}
	}
}

void Form::take_ipaddress()
{
	std::ifstream ip_file("ip.txt");
	std::string ip;

	while (std::getline(ip_file, ip))
	{
		if (ui->comboBox_ip->findText(QString::fromStdString(ip)) == -1) {
			ui->comboBox_ip->addItem(QString::fromStdString(ip));
		}
	}

	ip_file.close();

}

void Form::on_comboBox_ip_currentTextChanged(const QString &arg1)
{
	target = arg1.toStdString();
}

void Form::on_listWidget_meta_itemDoubleClicked(QListWidgetItem *item)
{
	ui->lineEdit_services->clear();
	QListWidgetItem* currentItem = item;
	if (currentItem != nullptr) {
		QString meta = currentItem->text();
		QString newString_value = meta.mid(meta.indexOf(": ") + 2);
		ui->lineEdit_services->setText(newString_value);
		filterServices(newString_value);
		applyFilters();

	}
}

void Form::on_listWidget_tag_itemDoubleClicked(QListWidgetItem *item)
{
	ui->lineEdit_services->clear();
	QListWidgetItem* currentItem = item;
	if(currentItem != nullptr) {
		QString tag = currentItem->text();
		ui->lineEdit_services->setText(tag);
		filterServices(tag);
		applyFilters();
	}
}

void Form::on_listWidget_ep_itemDoubleClicked(QListWidgetItem *item)
{
	ui->lineEdit_services->clear();
	QListWidgetItem* currentItem = item;
	if(currentItem != nullptr) {
		QString ep = currentItem->text();
		ui->lineEdit_services->setText(ep);
		filterServices(ep);
		applyFilters();
	}
}

