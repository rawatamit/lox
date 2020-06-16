#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ErrorHandler.h"
#include "Parser.h"
#include "Scanner.h"
//#include "ASTPrinter.h"
#include "Interpreter.h"
#include "Resolver.h"
#include "RuntimeException.h"

namespace lox {
static std::shared_ptr<Interpreter> interpreter =
    std::make_shared<Interpreter>();
static bool hadRuntimeError = false;

static int run(const std::string &source, ErrorHandler &errorHandler) {
  /// scanner
  Scanner scanner(source, errorHandler);
  const auto tokens = scanner.scanAndGetTokens();
  // if found error during scanning, report
  if (errorHandler.foundError) {
    errorHandler.report();
    return 65;
  }
  /// parser
  Parser parser(tokens, errorHandler);
  auto stmts = parser.parse();
  // if found error during parsing, report
  if (errorHandler.foundError) {
    errorHandler.report();
    return 65;
  }
  /// print ast
#if 0
	ASTPrinter pp;
	pp.print(stmts);
	std::cout << std::endl;
#endif
  Resolver resolver(interpreter, errorHandler);
  resolver.resolve(stmts);
  if (errorHandler.foundError) {
    errorHandler.report();
    return 65;
  }

  try {
    interpreter->interpret(stmts);
  } catch (const RuntimeException &e) {
    std::cerr << e.what() << '\n' << "[line " << e.tok.line << "]" << '\n';
    return 65;
  }
  return 0;
}

static int runFile(const std::string &path, ErrorHandler &errorHandler) {
  std::ifstream file(path);
  std::ostringstream stream;
  stream << file.rdbuf();
  file.close();
  return run(stream.str(), errorHandler);
}

static void runPrompt(ErrorHandler &errorHandler) {
  while (true) {
    std::cout << "> ";
    std::string line;
    getline(std::cin, line);
    run(line, errorHandler);
    if (hadRuntimeError) {
      exit(70);
    }
  }
}
} // namespace lox

int main(int argc, char **argv) {
  lox::ErrorHandler errorHandler;
  if (argc > 2) {
    std::cout << "Usage: lox [filename]" << std::endl;
  } else if (argc == 2) {
    return lox::runFile(argv[1], errorHandler);
  } else {
    lox::runPrompt(errorHandler);
  }
  return 0;
}
