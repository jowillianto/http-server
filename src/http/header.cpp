module;
#include <expected>
#include <format>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
module http;

namespace http {
  std::expected<HeaderLine, http::ParseError> http::HeaderLine::from_string(std::string_view v
  ) noexcept {
    auto header_sep = std::find(v.begin(), v.end(), ':');
    if (header_sep == v.end())
      return std::unexpected { std::format("{} is not a valid header string", v)};
    return HeaderLine {
      .name = std::string_view { v.begin(), header_sep }, 
      .content = std::string_view { header_sep + 1, v.end()}
    };
  }
  void http::HeaderLine::validate_name() const {
    auto space_it = std::find_if(name.begin(), name.end(), isspace);
    if (space_it != name.end())
      throw http::ParseError{
        std::format("{} is an illegal character in header \'{}\'", *space_it, name)
      };
  }
  std::expected<Header, ParseError> Header::from_string(std::string_view header_string) noexcept {
    std::vector<std::string_view> separated_by_newline = split_on_separator(header_string, "\r\n");
    Header header{};
    for (auto &line : separated_by_newline) {
      try {
        auto header_line = HeaderLine::from_string(line);
        if (!header_line.has_value()) return std::unexpected{header_line.error()};
        header.add_header(header_line.value());
      } catch (const http::ParseError &e) {
        return std::unexpected{e};
      }
    }
    return header;
  }

  Header::Header(std::initializer_list<HeaderLine> list) {
    for (auto& line : list) 
      add_header(std::move(line));
  }

  void Header::add_header(HeaderLine h) {
    if (_headers.find(h.name) != _headers.end())
      throw ParseError{std::format("{} already exist", h.name)};
    h.validate_name();
    _headers[std::string{h.name}] = std::string{h.content};
  }

  void Header::remove_header(std::string_view header_name) {
    auto header_it = _headers.find(header_name);
    if (header_it == _headers.end())
      throw ParseError{std::format("{} does not exist", header_name)};
    _headers.erase(header_it);
  }

  void Header::modify_header(HeaderLine h) {
    auto header_it = _headers.find(h.name);
    if (header_it == _headers.end()) throw ParseError{std::format("{} does not exist", h.name)};
    header_it->second = h.content;
  }

  void Header::add_or_modify_header(HeaderLine h) {
    try {
      modify_header(h);
    } catch (const ParseError &e) {
      add_header(h);
    }
  }

  std::expected<std::string_view, std::out_of_range> Header::get_content(
    std::string_view header_name
  ) const noexcept {
    auto header_it = _headers.find(header_name);
    if (header_it == _headers.end()) {
      return std::unexpected{
        std::out_of_range{std::format("{} is not a registered header", header_name)}
      };
    }
    return std::string_view{header_it->first};
  }

  Header::container_t::const_iterator Header::begin() const noexcept {
    return _headers.begin();
  }

  Header::container_t::const_iterator Header::end() const noexcept {
    return _headers.end();
  }
}