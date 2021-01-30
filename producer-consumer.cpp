#include <iostream>
#include <chrono>
#include <queue>

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>
#include <boost/bind/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <gflags/gflags.h>
#include <glog/logging.h>

class Runner {
public:
  Runner (boost::shared_ptr<boost::asio::io_service> io_service_):
    id(0),
    jq(),
    io_service(io_service_),
    pi(1000),
    ci(5000){
    strand = boost::make_shared<boost::asio::io_service::strand>(*io_service);
    producer = boost::make_shared<boost::asio::steady_timer>(*io_service, this->pi);
    consumer = boost::make_shared<boost::asio::steady_timer>(*io_service, this->ci);
  }

  void exec () {
    this->producer->async_wait(this->strand->wrap(std::bind(&Runner::ph, this, std::placeholders::_1)));
    this->consumer->async_wait(this->strand->wrap(std::bind(&Runner::ch, this, std::placeholders::_1)));
    this->io_service->run();
  }

private:
  void ph (const boost::system::error_code & /*ec*/) {
    int tmp = this->id++;
    this->jq.push(tmp);
    LOG(INFO) << "push " << tmp << std::endl;
    this->producer->expires_from_now(this->pi);
    this->producer->async_wait(this->strand->wrap(std::bind(&Runner::ph, this, std::placeholders::_1)));
  }

  void ch (const boost::system::error_code & /*ec*/) {
    while (!this->jq.empty()) {
      int tmp = this->jq.front();
      this->jq.pop();
      LOG(INFO) << "pop " << tmp << std::endl;
    }
    this->consumer->expires_from_now(this->ci);
    this->consumer->async_wait(this->strand->wrap(std::bind(&Runner::ch, this, std::placeholders::_1)));
  }

  int id;
  std::queue<int> jq;
  std::chrono::milliseconds pi;
  boost::shared_ptr<boost::asio::steady_timer> producer;
  std::chrono::milliseconds ci;
  boost::shared_ptr<boost::asio::steady_timer> consumer;
  boost::shared_ptr<boost::asio::io_service::strand> strand;
  boost::shared_ptr<boost::asio::io_service> io_service;
};


int main(int argc, char* argv[])
{
  FLAGS_alsologtostderr = 1;
  // 解析命令行参数
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // 初始化日志库
  google::InitGoogleLogging(argv[0]);
//  google::SetLogDestination(google::GLOG_INFO, "./myInfo");

  boost::shared_ptr<boost::asio::io_service> io_service = boost::make_shared<boost::asio::io_service>();
  boost::shared_ptr<boost::asio::io_service::work> work(
    new boost::asio::io_service::work(*io_service)
  );
  Runner runner(io_service);
  runner.exec();

  return 0;

}
