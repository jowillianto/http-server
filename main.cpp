#include <format>
#include <print>
import moderna.io;
import moderna.thread_plus;
import moderna.http.middleware;
import moderna.http;
import moderna.http.server;
import moderna.variant_error;

using error_t = moderna::variant_error<moderna::http::is_a_directory_error, std::system_error>;
int main(int argc, char **argv) {
  int port = std::atoi(argv[1]);
  moderna::http::dir_router::create_router("/")
    .transform_error([](auto &&e) { return e.template cast_to<error_t>(); })
    .and_then([&](auto &&dir_router) {
      return moderna::http::create_server(
               moderna::http::logger_middleware{moderna::http::router{dir_router}}, port, 50, 10
      )
        .transform_error([](auto &&e) { return e.template cast_to<error_t>(); });
    });
}