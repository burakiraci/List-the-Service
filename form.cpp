#include "form.h"
#include "ui_form.h"
#include <QFontComboBox>
#include <QDebug>


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
}

Form::~Form()
{
	delete ui;
}

class ServiceCheck {
public:
	ServiceCheck(Form* f) : form_(f) {}

	void allService(const ListResponse& response) {
		QStringList names;
		for (const servicecheck::Service& service : response.services()) {
			for(auto it = service.endpoints().begin(); it != service.endpoints().end(); it++){
				std::string endp = it->first;
				form_->ui->textEdit_Endpoint->setText(QString::fromStdString(endp));
			}

			QString name = QString::fromStdString(service.name());

			if (!names.contains(name)) {
				form_->ui->listWidget_name->addItem(name);
				names.append(name);
			}


		}
	}

	void nodes(const ListResponse& response) {
		ListRequest request;
		ClientContext context;
		QStringList nodes ;
		for(const servicecheck::Service& service : response.services()){
			QString nodeName = QString::fromStdString(service.node().name());
			if(!nodes.contains(nodeName)){
				form_->ui->listWidget_nodesName->addItem(nodeName);
				nodes.append(nodeName);
			}
		}
	}

private:
	Form* form_;
};

void Form::on_pushButtonListServices_clicked() {
	ListRequest request;
	ClientContext context;
	ListResponse response;
	ui->listWidget_name->clear();
	auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	auto stub = Registry::NewStub(channel);
	Status status = stub->ListServices(&context, request, &response);

	ServiceCheck client(this);
	client.allService(response);
}



void Form::on_listWidget_name_currentRowChanged(int currentRow)
{
	ui->listWidget_uuid->clear();
	auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	auto stub = Registry::NewStub(channel);
	ListRequest request;
	ClientContext context;
	ListResponse response;
	QString search_key = ui->lineEdit_services->text();
	Status status = stub->ListServices(&context, request, &response);
	int count = 0;
	QListWidgetItem *item = ui->listWidget_name->currentItem();
	if (item != nullptr) {
		QString serviceName = item->text();
		for (const servicecheck::Service& service : response.services()) {
			QString uuids = QString::fromStdString(service.serviceuuid());
			if (QString::fromStdString(service.name()) == serviceName) {
				std::string uuid = service.serviceuuid();
				bool found_key = false;
				for (auto it = service.meta().begin(); it != service.meta().end(); ++it) {
					std::string key = it->first;
					std::string value = it->second;
					if (devices_map.find(uuid) == devices_map.end()) {
						devices_map[uuid] = std::map<std::string, std::string>();
					}
					devices_map[uuid][key] = value;
					if (QString::fromStdString(value).contains(search_key, Qt::CaseInsensitive)) {
						found_key = true;
					}
				}
				for (auto it = service.tags().begin(); it != service.tags().end(); it++) {
					for (auto ite = it->begin(); ite != it->end(); ite++) {
						std::string tag = ite.base();
						if (devices_map.find(uuid) == devices_map.end()) {
							devices_map[uuid] = std::map<std::string, std::string>();
						}
						devices_map[uuid]["tag"] = tag;
						if (QString::fromStdString(tag).contains(search_key, Qt::CaseInsensitive)) {
							found_key = true;
						}
						break;
					}
				}
				if (found_key) {
					count++;
					ui->listWidget_uuid->addItem(uuids);
					ui->lcdNumber->display(count);
				}
			}
		}
	}
}

void Form::on_pushButton_clicked()
{
	ui->listWidget_nodesName->clear();
	auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	auto stub = Registry::NewStub(channel);
	ListRequest request;
	ClientContext context;
	ListResponse response;
	Status status = stub->ListServices(&context, request, &response);

	ServiceCheck client(this);
	client.nodes(response);
}


void Form::on_listWidget_nodesName_currentRowChanged(int currentRow)
{
	auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	auto stub = Registry::NewStub(channel);
	ListRequest request;
	ClientContext context;
	ListResponse response;
	QStringList nodess;
	int count = 0;
	Status status = stub->ListServices(&context, request, &response);
	ui->listWidget_uuid->clear();
	QListWidgetItem *item = ui->listWidget_nodesName->currentItem();
	if( item != nullptr){
		QString nodeName = item->text();
		for(const servicecheck::Service& service: response.services()){
			QString uuids = QString::fromStdString(service.node().nodeuuid());
			if(QString::fromStdString(service.node().name()) == nodeName) {
				if(!nodess.contains(uuids)){
					nodess.append(uuids);
					count++;
					ui->listWidget_uuid->addItem(uuids);
					ui->lcdNumber->display(count);
				}
			}
		}
	}
}


void Form::on_pushButton_Tags_clicked()
{
	auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	auto stub = Registry::NewStub(channel);
	QStringList tags;
	ListRequest request;
	ClientContext context;
	ListResponse response;
	int count = 0;
	Status status = stub->ListServices(&context, request, &response);
	for(const servicecheck::Service& service: response.services()){
		for (auto it = service.tags().begin(); it != service.tags().end(); it++){
			for(auto ite = it->begin(); ite != it->end(); ite++){
				QString tag = QString::fromStdString(ite.base());
				if(!tags.contains(tag)){
					count++;
					ui->textEdit_tag->append(tag);
					tags.append(tag);
					ui->lcdNumber->display(count);

				}
				break;
			}
		}
	}
}



void Form::on_lineEdit_services_editingFinished()
{
	ui->listWidget_uuid->clear();
	ui->textEdit_meta->clear();
	ui->textEdit_tag->clear();
	ui->listWidget_name->clear();

	auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	auto stub = Registry::NewStub(channel);

	ListRequest request;
	ListResponse response;
	ClientContext context;
	Status status = stub->ListServices(&context, request, &response);

	QString search_key = ui->lineEdit_services->text();

	std::map<std::string, std::vector<std::string>> names_to_uuids;

	for (const servicecheck::Service& service : response.services()) {
		std::string uuid = service.serviceuuid();
		std::string name = service.name();

		bool has_key = QString::fromStdString(name).contains(search_key, Qt::CaseInsensitive);

		if (!has_key) {
			for (auto it = service.meta().begin(); it != service.meta().end(); ++it) {
				std::string key = it->first;
				std::string value = it->second;

				has_key = QString::fromStdString(value).contains(search_key, Qt::CaseInsensitive);
				if (has_key) {
					break;
				}
			}

			if (!has_key) {
				for (auto it = service.tags().begin(); it != service.tags().end(); it++){
					for (auto ite = it->begin(); ite != it->end(); ite++){
						std::string tag = ite.base();

						has_key = QString::fromStdString(tag).contains(search_key, Qt::CaseInsensitive);
						if (has_key) {
							break;
						}
					}
					if (has_key) {
						break;
					}
				}
			}
		}

		if (has_key) {
			names_to_uuids[name].push_back(uuid);
		}
	}

	for (const auto& name_uuid_pair : names_to_uuids) {
		QString name = QString::fromStdString(name_uuid_pair.first);
		ui->listWidget_name->addItem(name);

		for (const std::string& uuid : name_uuid_pair.second) {
			ui->listWidget_uuid->addItem(QString::fromStdString(uuid));
		}
	}
}


void Form::on_listWidget_uuid_currentRowChanged(int currentRow)
{
	if (!ui->listWidget_uuid->currentItem()) {
		return;
	}
	auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	auto stub = Registry::NewStub(channel);
	ListRequest request;
	ClientContext context;
	ListResponse response;
	Status status = stub->ListServices(&context, request, &response);
	QString uuid = ui->listWidget_uuid->currentItem()->text();

	ui->textEdit_meta->clear();
	std::map<std::string, std::string> device = devices_map[uuid.toStdString()];
	for(const servicecheck::Service& service: response.services()){
		std::string name = service.name();
		for (auto it = device.begin(); it != device.end(); ++it) {
			std::string key = it->first;
			std::string value = it->second;
			if(key == "app"){
				ui->textEdit_meta->append("Service Name: " + QString::fromStdString(value));
				ui->textEdit_meta->append("\n\nMeta Data : \n");
			}
			if(key == "tag"){
				ui->textEdit_tag->setText(QString::fromStdString(value));
			}


			QString item = QString::fromStdString(key) + ": " + QString::fromStdString(value);
			if(key != "app" && key!= "tag"){
				ui->textEdit_meta->append(item);
			}
		}
		break;
	}

}

void Form::on_pushButton_meta_clicked()
{
	auto channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
	auto stub = Registry::NewStub(channel);
	ListRequest request;
	ClientContext context;
	ListResponse response;
	Status status = stub->ListServices(&context, request, &response);

	ui->textEdit_meta->append(" ");
	for(const servicecheck::Service& service: response.services()){
		for (auto it = service.meta().begin(); it != service.meta().end(); ++it) {
			ui->textEdit_meta->append(QString::fromStdString(it->first + ": " + it->second));
		}
	}
}
