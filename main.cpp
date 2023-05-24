
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <grpcpp/grpcpp.h>
#include <mutex>
#include <vector>
#include <QApplication>
#include "form.h"
#ifdef BAZEL_BUILD
//#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "listservices.pb.h"
#include "listservices.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using servicecheck::ListRequest;
using servicecheck::ListResponse;
using servicecheck::Registry;


int main(int argc, char *argv[]){
	QApplication a(argc, argv);
	Form w;
	w.show();
	std::string target = "10.5.192.140:3456";
//	ServiceCheck client (grpc::CreateChannel(target,grpc::InsecureChannelCredentials()));
//	client.ListServices();
	return a.exec();
	return 0;
}
