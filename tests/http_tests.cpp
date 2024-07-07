import moderna.http;
import moderna.test_lib;
#include <format>

auto tests = std::make_tuple(
  moderna::test_lib::make_tester("http_header_basic")
    .add_test(
      "from_string_constructions",
      []() {
        auto header = moderna::http::header::make("Content-Type: application/json").value();
        moderna::test_lib::assert_equal(header.contains("Content-Type"), true);
        moderna::test_lib::assert_equal(header.get("Content-Type")->name, "Content-Type");
        moderna::test_lib::assert_equal(header.get("Content-Type")->content, "application/json");
      }
    )
    .add_test(
      "header_constructor",
      []() {
        auto header = moderna::http::header{
          {moderna::http::header_view::make("Content-Type", "application/json"),
           moderna::http::header_view::make("Content-Length", "20")}
        };
        auto content_type = header.get("Content-Type");
        auto content_length = header.get("Content-Length");
        moderna::test_lib::assert_equal(header.contains("Content-Type"), true);
        moderna::test_lib::assert_equal(header.contains("Content-Length"), true);
        moderna::test_lib::assert_equal(content_type.value().content, "application/json");
        moderna::test_lib::assert_equal(content_length.value().content, "20");
      }
    )
    .add_test(
      "header_remove",
      []() {
        auto header = moderna::http::header{
          {moderna::http::header_view::make("Content-Type", "application/json"),
           moderna::http::header_view::make("Content-Length", "20")}
        };
        auto removed_header = header.remove("Content-Type");
        moderna::test_lib::assert_equal(removed_header.has_value(), true);
        moderna::test_lib::assert_equal(header.contains("Content-Type"), false);
        moderna::test_lib::assert_equal(removed_header->content, "application/json");
      }
    )
    .add_test(
      "header_format_check",
      []() {
        auto header = moderna::http::header{
          {moderna::http::header_view::make("Content-Type", "application/json"),
           moderna::http::header_view::make("Content-Length", "20")}
        };
        moderna::test_lib::assert_equal(
          std::format("{}", header), "Content-Type: application/json\r\nContent-Length: 20\r\n"
        );
      }
    )
    .add_test(
      "header_modify",
      []() {
        auto header = moderna::http::header{
          {moderna::http::header_view::make("Content-Type", "application/json"),
           moderna::http::header_view::make("Content-Length", "20")}
        };
        header.modify(moderna::http::header_view::make("Content-Type", "text/plain"));
        auto content_type = header.get("Content-Type");
        moderna::test_lib::assert_equal(header.contains("Content-Type"), true);
        moderna::test_lib::assert_equal(content_type.value().content, "text/plain");
      }
    ),
  moderna::test_lib::make_tester("http_header_parse")
    .add_test(
      "error_parse",
      []() {
        auto to_parse = "Content-Type: Lol\r\nContent Type: Lol\r\n";
        auto new_header = moderna::http::header::make(to_parse);
        moderna::test_lib::assert_equal(new_header.has_value(), true);
        moderna::test_lib::assert_equal(new_header->size(), 1);
        moderna::test_lib::assert_equal(new_header->contains("Content-Type"), true);
        moderna::test_lib::assert_equal(new_header->get("Content-Type")->content, "Lol");
      }
    )
    .add_test(
      "error_runtime_parse",
      []() {
        std::string header_content = "application/json";
        header_content += "text/plain";
        auto new_header_line =
          moderna::http::header_view::vmake("Accept Some", header_content);
        moderna::test_lib::assert_equal(new_header_line.has_value(), false);
      }
    )
    .add_test(
      "parse_line", 
      [](){
        std::string header_line = "Content-Type: application/json";
        auto valid_header_view = moderna::http::header_view::vmake_from_line(header_line);
        moderna::test_lib::assert_equal(valid_header_view.has_value(), true);
        header_line = "Content Type: application/json";
        auto invalid_header_view = moderna::http::header_view::vmake_from_line(header_line);
        moderna::test_lib::assert_equal(invalid_header_view.has_value(), false);
      }
    ),
  moderna::test_lib::make_tester("http_status_basic")
    .add_test(
      "status_create",
      []() {
        auto status = moderna::http::status::create<moderna::http::status_code::HTTP_200_OK>();
        moderna::test_lib::assert_equal(status.code(), 200);
        moderna::test_lib::assert_equal(
          static_cast<int>(status.code_class()),
          static_cast<int>(moderna::http::status_code_class::SUCCESSFUL)
        );
      }
    )
    .add_test(
      "status_create_nonstandard",
      []() {
        auto status = moderna::http::status{105, "NON STANDARD MESSAGE"};
        moderna::test_lib::assert_equal(status.code(), 200);
        moderna::test_lib::assert_equal(status.msg(), "NON STANDARD MESSAGE");
      }
    )
    .add_test(
      "status_serialize",
      []() {
        auto status = moderna::http::status::create<moderna::http::status_code::HTTP_200_OK>();
        moderna::test_lib::assert_equal(std::format("{}", status), "200 OK");
      }
    )
);

template <typename Cur, typename... Args> void run_all_tests(Cur &&cur, Args &&...test_suites) {
  cur.print_or_exit();
  if constexpr (sizeof...(test_suites) != 0) {
    run_all_tests(std::forward<Args>(test_suites)...);
  }
}

int main() {
  std::apply([](auto &&...args) { run_all_tests(std::forward<decltype(args)>(args)...); }, tests);
}