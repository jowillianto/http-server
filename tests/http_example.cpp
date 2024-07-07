import moderna.http;
#include <iostream>
#include <format>

int main(int argc, char** argv) {
  if (argc < 2){
    std::cerr << "Requires minimal 1 arguments\n";
    return 1;
  }
  // This line will check the header name for standard compliance. 
  auto header_line = moderna::http::header_view::make("Accept").set_content(argv[1]);
  auto header = moderna::http::header::make(std::format("{}", header_line));
  std::cout<<std::format("{}", header_line)<<std::endl;
}