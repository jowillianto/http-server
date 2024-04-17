module;

#include <algorithm>
#include <expected>
#include <format>
#include <string>
#include <string_view>

module http;

http::Status::Status(uint32_t code, std::string msg) noexcept : _code(code), _msg(std::move(msg))  {
  _classify_code();
}
std::expected<const http::Status, http::ParseError> http::Status::from_string(
  std::string_view status_string
) noexcept {
  // Strip Trailing whitespaces.
  std::string_view cleaned = strip_whitespace(status_string);
  auto first_sep = std::find(cleaned.begin(), cleaned.end(), ' ');
  // Space cannot be found. The standard mandates that the status number and message is space
  // separated.
  if (first_sep == cleaned.end()) {
    return std::unexpected{
      http::ParseError(std::format("{} has an ambiguous status code", status_string))
    };
  }
  return Status{
    // TODO: Potentially Unsafe since const char* data has no end identifier.
    static_cast<uint32_t>(std::atoi(std::string_view{cleaned.begin(), first_sep}.data())),
    std::string{first_sep + 1, cleaned.end()}
  };
}
uint32_t http::Status::code() const noexcept {
  return _code;
}
const std::string &http::Status::msg() const noexcept {
  return _msg;
}
http::StatusCodeClass http::Status::code_class() const noexcept {
  return _code_class;
}
void http::Status::change_code(uint32_t code, std::string msg) noexcept {
  _code = code;
  _msg = std::move(msg);
  _classify_code();
}
void http::Status::Status::_classify_code() {
  if (_code < 100) throw http::ParseError(std::format("{} is an invalid status code", _code));
  else if (_code < 200)
    _code_class = StatusCodeClass::INFORMATIONAL_RESPONSE;
  else if (_code < 300)
    _code_class = StatusCodeClass::SUCCESSFUL;
  else if (_code < 400)
    _code_class = StatusCodeClass::REDIRECTION;
  else if (_code < 500)
    _code_class = StatusCodeClass::CLIENT_ERROR;
  else if (_code < 600)
    _code_class = StatusCodeClass::SERVER_ERROR;
  else
    throw http::ParseError(std::format("{} is an invalid status code", _code));
}

/*
  HTTP Common Codes Template Specialization Definition
*/

template <auto N> consteval auto replace_sub_str(char const (&src)[N]) {
  std::array<char, N> res = {};
  for (uint32_t i = 0; i < N; i += 1)
    res[i] = src[i] == '_' ? ' ' : src[i];
  return res;
}

#ifndef DECLARE_FOR_COMMON_HTTP
  #define DECLARE_FOR_COMMON_HTTP(STATUS) \
    template <> http::Status http::Status::create<http::StatusCode::STATUS>() noexcept { \
      constexpr auto msg = replace_sub_str(#STATUS); \
      std::string status_msg{msg.begin() + 9, msg.end()}; \
      return Status{http::StatusCode::STATUS, std::move(status_msg)}; \
    } \
    template <> void http::Status::change_code<http::StatusCode::STATUS>() noexcept { \
      constexpr auto msg = replace_sub_str(#STATUS); \
      _msg = std::string{msg.begin() + 9, msg.end()}; \
    }
DECLARE_FOR_COMMON_HTTP(HTTP_100_CONTINUE);
DECLARE_FOR_COMMON_HTTP(HTTP_101_SWITCHING_PROTOCOLS);
DECLARE_FOR_COMMON_HTTP(HTTP_102_PROCESSING);
DECLARE_FOR_COMMON_HTTP(HTTP_103_EARLY_HINTS);
DECLARE_FOR_COMMON_HTTP(HTTP_200_OK);
DECLARE_FOR_COMMON_HTTP(HTTP_201_CREATED);
DECLARE_FOR_COMMON_HTTP(HTTP_202_ACCEPTED);
DECLARE_FOR_COMMON_HTTP(HTTP_203_NON_AUTHORITATIVE_INFORMATION);
DECLARE_FOR_COMMON_HTTP(HTTP_204_NO_CONTENT);
DECLARE_FOR_COMMON_HTTP(HTTP_205_RESET_CONTENT);
DECLARE_FOR_COMMON_HTTP(HTTP_206_PARTIAL_CONTENT);
DECLARE_FOR_COMMON_HTTP(HTTP_207_MULTI_STATUS);
DECLARE_FOR_COMMON_HTTP(HTTP_208_ALREADY_REPORTED);
DECLARE_FOR_COMMON_HTTP(HTTP_226_IM_USED);
DECLARE_FOR_COMMON_HTTP(HTTP_300_MULTIPLE_CHOICES);
DECLARE_FOR_COMMON_HTTP(HTTP_301_MOVED_PERMANENTLY);
DECLARE_FOR_COMMON_HTTP(HTTP_302_FOUND);
DECLARE_FOR_COMMON_HTTP(HTTP_303_SEE_OTHER);
DECLARE_FOR_COMMON_HTTP(HTTP_304_NOT_MODIFIED);
DECLARE_FOR_COMMON_HTTP(HTTP_305_USE_PROXY);
DECLARE_FOR_COMMON_HTTP(HTTP_307_TEMPORARY_REDIRECT);
DECLARE_FOR_COMMON_HTTP(HTTP_308_PERMANENT_REDIRECT);
DECLARE_FOR_COMMON_HTTP(HTTP_400_BAD_REQUEST);
DECLARE_FOR_COMMON_HTTP(HTTP_401_UNAUTHORIZED);
DECLARE_FOR_COMMON_HTTP(HTTP_402_PAYMENT_REQUIRED);
DECLARE_FOR_COMMON_HTTP(HTTP_403_FORBIDDEN);
DECLARE_FOR_COMMON_HTTP(HTTP_404_NOT_FOUND);
DECLARE_FOR_COMMON_HTTP(HTTP_405_METHOD_NOT_ALLOWED);
DECLARE_FOR_COMMON_HTTP(HTTP_406_NOT_ACCEPTABLE);
DECLARE_FOR_COMMON_HTTP(HTTP_407_PROXY_AUTHENTICATION_REQUIRED);
DECLARE_FOR_COMMON_HTTP(HTTP_408_REQUEST_TIMEOUT);
DECLARE_FOR_COMMON_HTTP(HTTP_409_CONFLICT);
DECLARE_FOR_COMMON_HTTP(HTTP_410_GONE);
DECLARE_FOR_COMMON_HTTP(HTTP_411_LENGTH_REQUIRED);
DECLARE_FOR_COMMON_HTTP(HTTP_412_PRECONDITION_FAILED);
DECLARE_FOR_COMMON_HTTP(HTTP_413_PAYLOAD_TOO_LARGE);
DECLARE_FOR_COMMON_HTTP(HTTP_414_URI_TOO_LONG);
DECLARE_FOR_COMMON_HTTP(HTTP_415_UNSUPPORTED_MEDIA_TYPE);
DECLARE_FOR_COMMON_HTTP(HTTP_416_RANGE_NOT_SATISFIABLE);
DECLARE_FOR_COMMON_HTTP(HTTP_417_EXPECTATION_FAILED);
DECLARE_FOR_COMMON_HTTP(HTTP_421_MISDIRECTED_REQUEST);
DECLARE_FOR_COMMON_HTTP(HTTP_422_UNPROCESSABLE_ENTITY);
DECLARE_FOR_COMMON_HTTP(HTTP_423_LOCKED);
DECLARE_FOR_COMMON_HTTP(HTTP_424_FAILED_DEPENDENCY);
DECLARE_FOR_COMMON_HTTP(HTTP_500_INTERNAL_SERVER_ERROR);
DECLARE_FOR_COMMON_HTTP(HTTP_501_NOT_IMPLEMENTED);
DECLARE_FOR_COMMON_HTTP(HTTP_502_BAD_GATEWAY);
DECLARE_FOR_COMMON_HTTP(HTTP_503_SERVICE_UNAVAILABLE);
DECLARE_FOR_COMMON_HTTP(HTTP_504_GATEWAY_TIMEOUT);
DECLARE_FOR_COMMON_HTTP(HTTP_505_HTTP_VERSION_NOT_SUPPORTED);
DECLARE_FOR_COMMON_HTTP(HTTP_506_VARIANT_ALSO_NEGOTIATES);
DECLARE_FOR_COMMON_HTTP(HTTP_507_INSUFFICIENT_STORAGE);
DECLARE_FOR_COMMON_HTTP(HTTP_508_LOOP_DETECTED);
DECLARE_FOR_COMMON_HTTP(HTTP_510_NOT_EXTENDED);
DECLARE_FOR_COMMON_HTTP(HTTP_511_NETWORK_AUTHENTICATION_REQUIRED);
  #undef DECLARE_FOR_COMMON_HTTP
#endif