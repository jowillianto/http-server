import server;
import socket;
import http;
#include <cassert>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <string>

struct DirLoader {
  DirLoader(std::filesystem::path root_dir) {
    _root_dir = root_dir;
    assert(std::filesystem::is_directory(root_dir));
  }
  std::expected<std::string, http::Response> sanitize_path(std::string_view path) {
    auto find_dot_dot = path.find("..");
    if (find_dot_dot != std::string::npos) {
      return std::unexpected{http::Response{
        http::Status::create<http::StatusCode::HTTP_400_BAD_REQUEST>(),
        http::Header{},
        std::format(".. cannot exist in path '{}'", path)
      }};
    }
    auto strip_question_mark = std::find(path.begin(), path.end(), '?');
    return std::string{path.begin() + 1, strip_question_mark};
  }
  http::Response read_file_at_path(std::string_view child_path) {
    auto full_path = _root_dir / child_path;
    if (std::filesystem::is_regular_file(full_path)) {
      std::stringstream s;
      s << std::fstream{full_path, std::ios_base::in}.rdbuf();
      return http::Response(
        http::Status::create<http::StatusCode::HTTP_200_OK>(),
        http::Header{http::HeaderLine{.name = "Content-Type", .content = "text/plain"}},
        std::move(s.str())
      );
    } else if (std::filesystem::is_directory(full_path)) {
      std::string dir_list;
      for (const auto &dir : std::filesystem::directory_iterator{full_path}) {
        dir_list += std::format(
          R"(<p><a href = "/{}" > {} </a></p>)",
          std::filesystem::relative(dir.path(), _root_dir).string(),
          dir.path().filename().string()
        );
      }
      return http::Response{
        http::Status::create<http::StatusCode::HTTP_200_OK>(),
        http::Header{{http::HeaderLine{.name = "Content-Type", .content = "text/html"}}},
        std::move(dir_list)
      };
    }
    return http::Response{
      http::Status::create<http::StatusCode::HTTP_404_NOT_FOUND>(),
      http::Header{},
      std::format("{} cannot be found", child_path)
    };
  }
  http::Response operator()(http::Request &request) {
    auto child_path = sanitize_path(request.path.raw());
    if (!child_path.has_value()) return child_path.error();
    return read_file_at_path(child_path.value());
  }

private:
  std::filesystem::path _root_dir;
};

int main(int argc, char **argv) {
  std::string address = argv[1];
  int port = atoi(argv[2]);

  server::Threaded server{sock::UnixTCPListener{address, port, 20}};
  server.run(server::HTTPMiddleware<sock::UnixTCPHandler, DirLoader>{DirLoader{argv[3]}});
}