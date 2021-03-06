/* strand.cpp */
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>
#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

boost::mutex global_stream_lock;

void WorkerThread(boost::shared_ptr<boost::asio::io_service> iosvc, int counter)
{
  global_stream_lock.lock();
//  std::cout << "Thread " << counter << " Start.\n";
  LOG(INFO) << "Thread: " << counter << " Start.\n";
  global_stream_lock.unlock();

  iosvc->run();

  global_stream_lock.lock();
//  std::cout << "Thread " << counter << " End.\n";
  LOG(INFO) << "Thread: " << counter << " End.\n";
  global_stream_lock.unlock();
}

void Print(int number)
{
  LOG(INFO) << "Number: " << number << std::endl;
//  std::cout << "Number: " << number << std::endl;
}

int main(int argc, char* argv[])
{
  // 解析命令行参数
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  // 初始化日志库
  google::InitGoogleLogging(argv[0]);

  boost::shared_ptr<boost::asio::io_service> io_svc(
    new boost::asio::io_service
  );

  boost::shared_ptr<boost::asio::io_service::work> worker(
    new boost::asio::io_service::work(*io_svc)
  );

  boost::asio::io_service::strand strand(*io_svc);

  global_stream_lock.lock();
//  std::cout << "The program will exit once all work has finished.\n";
  LOG(INFO) << "The program will exit once all work has finished.\n";
  global_stream_lock.unlock();

  boost::thread_group threads;
  for(int i=1; i<=5; i++)
    threads.create_thread(boost::bind(&WorkerThread, io_svc, i));

  boost::this_thread::sleep(boost::posix_time::milliseconds(500));

//  strand.post(boost::bind(&Print, 1));
//  strand.post(boost::bind(&Print, 2));
//  strand.post(boost::bind(&Print, 3));
//  strand.post(boost::bind(&Print, 4));
//  strand.post(boost::bind(&Print, 5));

  for (int j = 0; j < 100; ++j) {
//    io_svc->post(boost::bind(&Print, j));
    strand.post(boost::bind(&Print, j));
  }
  worker.reset();

  threads.join_all();

  return 0;
}