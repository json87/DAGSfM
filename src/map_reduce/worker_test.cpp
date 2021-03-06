// BSD 3-Clause License

// Copyright (c) 2020, Chenyu
// All rights reserved.

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:

// 1. Redistributions of source code must retain the above copyright notice,
// this
//    list of conditions and the following disclaimer.

// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.

// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.

// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#include "map_reduce/worker.h"

#include <rpc/this_session.h>

#include <chrono>
#include <thread>

using namespace DAGSfM;

int main() {
  SfMWorker sfm_worker;
  // sfm_worker.RunServer();

  bool in_progress = false;
  bool completed = false;
  size_t total_image_num = 100;
  size_t reg_image_num = 0;

  sfm_worker.SetTotalImageNum(total_image_num);
  sfm_worker.BindServer();
  sfm_worker.BindSfMBaseFuncs();
  // sfm_worker.SetIP("localhost");

  rpc::server* server = sfm_worker.Server();
  bool exit = false;

  server->bind("HelloWorld", []() { std::cout << "Hello World!\n\n"; });

  server->bind("Exit", [&]() {
    LOG(INFO) << "Exit";
    rpc::this_session().post_exit();
    exit = true;
  });

  server->async_run(2);

  // while (!exit) {
  //   // do nothing.
  //   LOG(INFO) << "exit: " << exit;
  //   std::this_thread::sleep_for(std::chrono::seconds(2));
  // }
  // return 0;

  for (int i = 0; i < 100; i++) {
    std::cout << "Registered image num: "
              << sfm_worker.GetRunningInfo().registered_image_num << "\n";
    reg_image_num += 2;

    if (reg_image_num == total_image_num) {
      completed = true;
      in_progress = false;
      sfm_worker.SetCompleted(completed);
    }
    // sfm_worker.SetInprogress(in_progress);
    sfm_worker.SetRegImageNum(reg_image_num);

    std::this_thread::sleep_for(std::chrono::seconds(12));
  }
}