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
#include <cstdint>

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/supplyfinder.grpc.pb.h"
#else
#include "supplyfinder.grpc.pb.h"
#endif

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using supplyfinder::HelloRequest;
using supplyfinder::HelloReply;
using supplyfinder::Supplier;
using supplyfinder::Vendor;
using supplyfinder::FoodID;
using supplyfinder::VendorInfo;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> supplier_channel,
    std::shared_ptr<Channel> vendor_channel)
      : supplier_stub_(Supplier::NewStub(supplier_channel)),
      vendor_stub_(Vendor::NewStub(vendor_channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string SayHelloToSupplier(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = supplier_stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string SayHelloToVendor(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = vendor_stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }
  
  VendorInfo InquireVendorInfo (uint32_t food_id) {
    FoodID request;
    request.set_food_id(food_id);
    
    VendorInfo info;
    
    ClientContext context;
    
    Status status = supplier_stub_->CheckVendor(&context, request, &info);
    
    if (status.ok()) {
      return info;
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      info.set_url("RPC failed");
      return info;
    }
  }
  

 private:
  std::unique_ptr<Supplier::Stub> supplier_stub_;
  std::unique_ptr<Vendor::Stub> vendor_stub_;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  std::string target_str;
  std::string arg_str("--target");
  if (argc > 1) {
    std::string arg_val = argv[1];
    size_t start_pos = arg_val.find(arg_str);
    if (start_pos != std::string::npos) {
      start_pos += arg_str.size();
      if (arg_val[start_pos] == '=') {
        target_str = arg_val.substr(start_pos + 1);
      } else {
        std::cout << "The only correct argument syntax is --target=" << std::endl;
        return 0;
      }
    } else {
      std::cout << "The only acceptable argument is --target=" << std::endl;
      return 0;
    }
  } else {
    target_str = "localhost:50051";
  }
  std::string vendor_target_string("localhost:50052");
  GreeterClient greeter(grpc::CreateChannel(
      target_str, grpc::InsecureChannelCredentials()), grpc::CreateChannel(
          vendor_target_string, grpc::InsecureChannelCredentials()));
  // std::string user("supplier");
  // std::string reply = greeter.SayHelloToSupplier(user);
  // std::cout << "Greeter received: " << reply << std::endl;
  // user = "vendor";
  // reply = greeter.SayHelloToVendor(user);
  // std::cout << "Greeter received: " << reply << std::endl;
  
  std::cout << "========== Testing Supplier Server ==========" << std::endl;
  std::cout << "Queryiny FoodID = 1 (exist)" << std::endl;
  VendorInfo info = greeter.InquireVendorInfo(1);
  std::cout << "Vendor info received: " << info.url() << std::endl;
  std::cout << "Queryiny FoodID = 2 (doesn't exist)" << std::endl;
  info = greeter.InquireVendorInfo(2);
  std::cout << "Vendor info received: " << info.url() << std::endl;
  

  return 0;
}
