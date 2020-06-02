#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ErrorHandler.h"
#include "Parser.h"
#include "Scanner.h"
#include "ASTPrinter.h"
#include "Interpreter.h"

namespace lox {
    static Interpreter* interpreter = new Interpreter();
    static bool hadRuntimeError = false;

    static void run(const std::string& source, ErrorHandler& errorHandler) {
        /// scanner
        Scanner scanner(source, errorHandler);
        const auto tokens = scanner.scanAndGetTokens();
        // if found error during scanning, report
        if (errorHandler.foundError) {
            errorHandler.report();
            return;
        }
        /// parser
        Parser parser(tokens, errorHandler);
        auto stmts = parser.parse();
	// if found error during parsing, report
        if (errorHandler.foundError) {
            errorHandler.report();
            return;
        }
	/// print ast
	//ASTPrinter pp;
	//pp.print(expr);
	//std::cout << std::endl;
        interpreter->interpret(stmts);
    }

    static void runFile(const std::string& path, ErrorHandler& errorHandler) {
        std::ifstream file(path);
        std::ostringstream stream;
        stream << file.rdbuf();
        file.close();
        run(stream.str(), errorHandler);
    }

    static void runPrompt(ErrorHandler& errorHandler) {
        while (true) {
            std::cout << "> ";
            std::string line;
            getline(std::cin, line);
            run(line, errorHandler);
            if (errorHandler.foundError) {
              break;
            }
            if (hadRuntimeError)
            {
              exit(70);
            }
        }
    }
}

#if 1
int main(int argc, char** argv) {
    lox::ErrorHandler errorHandler;
    if (argc > 2) {
        std::cout << "Usage: lox [filename]" << std::endl;
    } else if (argc == 2) {
        lox::runFile(argv[1], errorHandler);
    } else {
        lox::runPrompt(errorHandler);
    }
    return 0;
}
#endif

#if 0
#include <any>
#include <type_traits>

int main()
{
  std::any a = std::make_any<bool>(false);
  std::cout << typeid(a).name() << '\n';
}
#endif
