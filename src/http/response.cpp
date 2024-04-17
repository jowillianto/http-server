module;
#include <expected>
#include <format>
#include <string>
#include <string_view>
module http;

std::expected<http::Response, http::ParseError> http::Response::from_string(std::string_view v
) noexcept {
  // Locate the first line
  auto first_sep = v.find("\r\n");
  if (first_sep == std::string::npos)
    return std::unexpected{http::ParseError{std::format("{} is not a valid http response", v)}};
  // Locate the end of the header section
  auto second_sep = v.find("\r\n\r\n", first_sep + 2);
  if (second_sep == std::string::npos)
    return std::unexpected{http::ParseError{std::format("{} is not a valid http response", v)}};
  // Remove HTTP/1.1 the version string from the first line
  auto first_line = std::string_view{v.begin(), v.begin() + first_sep};
  auto status_sep = first_line.find(' ');
  if (status_sep == std::string::npos)
    return std::unexpected{http::ParseError{std::format("{} is not a valid http response", v)}};
  // This is the final result, the rest is the content.
  auto status_str = std::string_view{first_line.begin() + status_sep, first_line.end()};
  auto header_str = std::string_view{v.begin() + first_sep + 2, v.begin() + second_sep};
  auto status = http::Status::from_string(status_str);
  auto header = http::Header::from_string(header_str);
  if (!status.has_value()) return std::unexpected{status.error()};
  if (!header.has_value()) return std::unexpected{header.error()};
  return http::Response{
    status.value(), header.value(), std::string{v.begin() + second_sep + 4, v.end()}
  };
}

void http::Response::set_content(std::string v) noexcept {
  _content = std::move(v);
  _sync_header_and_content_length();
}
void http::Response::set_content(std::string v, std::string_view content_type) noexcept {
  _content = std::move(v);
  _sync_header_and_content_length();
  header.add_or_modify_header({.name = "Content-Type", .content = content_type});
}

std::string_view http::Response::get_content() const noexcept {
  return _content;
}

void http::Response::_sync_header_and_content_length() noexcept {
  header.add_or_modify_header(
    {.name = "Content-Length", .content = std::to_string(_content.length())}
  );
}

http::Response::Response(http::Status status, http::Header header, std::string content) noexcept :
  status(std::move(status)), header(std::move(header)), _content{content} {
  _sync_header_and_content_length();
  if (!header.get_content("Content-Type").has_value())
    header.add_header({.name = "Content-Type", .content = "text/plain"});
}