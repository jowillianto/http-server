import moderna.http;
import moderna.io;
import moderna.thread_plus;
import moderna.http.routing;
#include <format>
#include <print>

int main(int argc, char **argv) {
  int port = std::atoi(argv[1]);
  auto pool = moderna::thread_plus::pool{10};
  auto listener = moderna::io::listener_sock_file::create_tcp_listener(port, 10).value();
  for (size_t i = 0; i < 10; i += 1) {
    static_cast<void>(pool.add_task([&]() {
      while (true) {
        auto router = moderna::http::router{moderna::http::dir_router::create_router("/").value()};
        static_cast<void>(
          listener.accept()
            .transform_error([](auto &&e) {
              return e.template cast_to<moderna::http::request::error_t>();
            })
            .and_then([&](auto &&acceptor) {
              return moderna::http::request::make(acceptor.io).and_then([&](auto &&request) {
                auto resp = router(request);
                return acceptor.io.write(std::format("{}", resp)).transform_error([](auto &&e) {
                  return e.template cast_to<moderna::http::request::error_t>();
                });
              });
            })
            .transform_error([](auto &&e) {
              std::print(stderr, "{}\n", e.what());
              return e;
            })
        );
      }
    }));
  }
  pool.join();
}