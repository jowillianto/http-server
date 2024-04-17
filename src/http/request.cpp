module;
#include <curl/curl.h>
#include <expected>
#include <format>
#include <string>
#include <string_view>
module http;

/*
  Implement for Method
*/
#define DECL_METHOD(METHOD) \
  template <> http::Method http::Method::create<http::CommonMethod::METHOD>() noexcept { \
    return http::Method{.method = #METHOD}; \
  }
DECL_METHOD(HEAD);
DECL_METHOD(GET);
DECL_METHOD(OPTIONS);
DECL_METHOD(PUT);
DECL_METHOD(PATCH);
DECL_METHOD(DELETE);
DECL_METHOD(CONNECT);
DECL_METHOD(TRACE);
#undef DECL_METHOD
/*
  Implementating for Path
*/
std::string_view http::Path::encoded() const noexcept {
  return _encoded;
}
std::string_view http::Path::raw() const noexcept {
  return _raw;
}
std::expected<http::Path, http::ParseError> http::Path::from_raw(std::string_view raw) noexcept {
  auto path = http::Path::encode(raw);
  if (!path.has_value()) return std::unexpected{path.error()};
  return http::Path{std::move(path.value()), std::string{raw}};
}
std::expected<std::string, http::ParseError> http::Path::encode(std::string_view raw) noexcept {
  const auto encoded_value = curl_easy_escape(nullptr, raw.data(), static_cast<int>(raw.length()));
  if (encoded_value == NULL) return std::unexpected{std::format("libcurl : Fail to parse {}", raw)};
  std::string result{encoded_value};
  curl_free(encoded_value);
  return result;
}

std::expected<http::Path, http::ParseError> http::Path::from_encoded(std::string_view encoded
) noexcept {
  auto decoded_path = http::Path::decode(encoded);
  if (!decoded_path.has_value()) return std::unexpected{decoded_path.error()};
  return http::Path{std::string{encoded}, std::move(decoded_path.value())};
}

std::expected<std::string, http::ParseError> http::Path::decode(std::string_view encoded) noexcept {
  int output_length;
  const auto decoded_value =
    curl_easy_unescape(nullptr, encoded.data(), static_cast<int>(encoded.length()), &output_length);
  if (decoded_value == NULL)
    return std::unexpected{http::ParseError{std::format("libcurl : Fail to parse {}", encoded)}};
  std::string result{decoded_value, static_cast<size_t>(output_length)};
  curl_free(decoded_value);
  return result;
}

http::Path::Path(std::string encoded, std::string raw) :
  _encoded(std::move(encoded)), _raw(std::move(raw)) {}

/*
  Implementation for Request
*/
std::expected<http::Request, http::ParseError> http::Request::from_string(std::string_view v
) noexcept {
  // Get First Line
  auto first_line_end = v.find("\r\n");
  if (first_line_end == std::string::npos)
    return std::unexpected{
      http::ParseError{std::format("{} since the end of the first line cannot be found", v)}
    };
  // Find Status string
  auto header_sect_end = v.find("\r\n\r\n", first_line_end + 2);
  if (header_sect_end == std::string::npos)
    return std::unexpected{http::ParseError{std::format(
      "{} is not a valid http request as the end of the header section cannot be found", v
    )}};
  auto first_line = http::strip_whitespace(std::string_view{v.begin(), v.begin() + first_line_end});
  // Find method
  auto method_sep = std::find(first_line.begin(), first_line.end(), ' ');
  if (method_sep == first_line.end())
    return std::unexpected{http::ParseError{std::format("{} ambiguous method", first_line)}};
  auto method =
    Method{.method{http::strip_whitespace(std::string_view{first_line.begin(), method_sep})}};
  // Find path
  auto path_str = http::strip_whitespace(std::string_view{method_sep + 1, first_line.end()});
  auto path_sep = std::find(path_str.begin(), path_str.end(), ' ');
  if (path_sep == path_str.end())
    return std::unexpected{http::ParseError{std::format("{} ambiguous path", path_str)}};
  auto path = Path::from_encoded(std::string_view{path_str.begin(), path_sep});
  if (!path.has_value()) return std::unexpected{path.error()};
  // Create Header
  auto header_sect = std::string_view{v.begin() + first_line_end + 2, v.begin() + header_sect_end};
  auto header = http::Header::from_string(header_sect);
  if (!header.has_value()) return std::unexpected{header.error()};
  // Content string
  auto content =
    std::string{v.begin() + header_sect_end + 4, v.end()};
  return http::Request{method, path.value(), header.value(), content};
}
std::string_view http::Request::get_content() const noexcept {
  return _content;
}
void http::Request::set_content(std::string v) noexcept {
  _content = std::move(v);
  _sync_header_and_content_length();
}
void http::Request::set_content(std::string v, std::string_view content_type) noexcept {
  _content = std::move(v);
  _sync_header_and_content_length();
  header.add_or_modify_header({.name = "Content-Type", .content = content_type});
}
http::Request::Request(
  http::Method method, http::Path path, http::Header header, std::string content
) noexcept :
  method(method),
  path(std::move(path)), header(std::move(header)), _content(std::move(content)) {
  _sync_header_and_content_length();
  if (!header.get_content("Content-Type").has_value())
    header.add_header({.name = "Content-Type", .content = "text/plain"});
}

void http::Request::_sync_header_and_content_length() noexcept {
  header.add_or_modify_header(
    {.name = "Content-Length", .content = std::to_string(_content.length())}
  );
}