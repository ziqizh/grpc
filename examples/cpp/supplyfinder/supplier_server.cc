/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <cstdint>

#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>

#ifdef BAZEL_BUILD
#include "examples/protos/supplyfinder.grpc.pb.h"
#else
#include "supplyfinder.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using supplyfinder::HelloRequest;
using supplyfinder::HelloReply;
using supplyfinder::FoodID;
using supplyfinder::VendorInfo;
using supplyfinder::Supplier;

// Logic and data behind the server's behavior.
class SupplierServiceImpl final : public Supplier::Service {
  
  std::unordered_map<uint32_t, VendorInfo> vendor_db;
  void InitDB () {
    VendorInfo vendor1;
    vendor1.set_url("localhost:10933");
    vendor1.set_name("Kroger");
    vendor1.set_location("Ann Arbor, MI");
    vendor_db[1] = vendor1;
  }
  Status SayHello(ServerContext* context, const HelloRequest* request,
                  HelloReply* reply) override {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    return Status::OK;
  }

  Status CheckVendor(ServerContext* context, const FoodID* request,
                  VendorInfo* reply) override {
    if (vendor_db.find(request->food_id()) == vendor_db.end()) {
      return Status::CANCELLED;
    }

    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  SupplierServiceImpl service;
  // service.InitDB();

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}
