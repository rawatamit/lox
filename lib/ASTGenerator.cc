#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <algorithm>
#include <vector>

namespace lox {
    // source:
    // https://stackoverflow.com/questions/14265581/parse-split-a-string-in-c-using-string-delimiter-standard-c
    static std::vector<std::string> split(const std::string& str,
                                          const std::string& delim) {
        std::vector<std::string> tokens;
        size_t prev = 0, pos = 0;
        do {
            pos = str.find(delim, prev);
            if (pos == std::string::npos)
                pos    = str.length();
            auto token = str.substr(prev, pos - prev);
            if (!token.empty())
                tokens.push_back(token);
            prev = pos + delim.length();
        } while (pos < str.length() && prev < str.length());
        return tokens;
    }
}

class ASTGenerator {
  public:
    using ASTSpecification = std::pair<std::string, std::vector<std::string>>;
    ASTGenerator(const std::string& aDir, const ASTSpecification aSpec)
        : outDir(aDir)
        , astSpec(aSpec) {}
    void generate() {
        std::cout << outDir << std::endl;
        defineAST();
    }
    void defineAST() {
        auto baseName = astSpec.first;
        auto path     = outDir + "/" + baseName + ".h";
        std::ofstream file(path);
        if (!file.is_open()) {
            std::cout << "Unable to open file." << std::endl;
            return;
        }

        /// #ifndef guard
        file << "#ifndef " + baseName + "_H_" << std::endl;
        file << "#define " + baseName + "_H_" << '\n' << '\n';

        // Expr base abstract interface
        file << "#include \"Token.h\"" << '\n';
        file << "#include <any>" << '\n';
        file << "#include <vector>" << '\n';
        file << "using namespace lox;" << '\n' << '\n';

        // forward declarations
        file << "class " << baseName << ";" << std::endl;
        for (auto& type : astSpec.second) {
            auto className = type.substr(0, type.find(":"));
            // remove spaces from className
            className.erase(std::remove(className.begin(), className.end(), ' '),
                            className.end());
            file << "class " << className << ';' << '\n';
        }

        file << '\n';
        defineVisitor(file, baseName);
        file << '\n';

        file << "class " << baseName << " {" << std::endl;
        file << "public:" << std::endl;
        file << "  virtual ~" << baseName << "() {}" << std::endl;
        file << "  virtual std::any accept(" << baseName + "Visitor* visitor) = 0;"
             << '\n';
        file << "};" << '\n' << '\n';

        // Derived concrete classes
        for (auto type : astSpec.second) {
            auto className = type.substr(0, type.find(":"));
            auto fields    = type.substr(type.find(":") + 1, type.size());
            defineType(file, baseName, className, fields);
        }

        /// #endif for #ifndef
        file << "#endif" << '\n';

        file.close();
    }

    void defineType(std::ofstream& file, const std::string& baseName,
                    const std::string& className, const std::string fields) {
        file << "class " + className + " : public " + baseName + " { "
             << std::endl;
        file << "public: " << '\n';
        file << "  " << className + "(";
        auto fieldList = lox::split(fields, ",");
        bool first     = true;
        for (auto field : fieldList) {
            if (!first)
                file << ", ";
            if (first)
                first      = false;
            file << "  " << field << '\n';
        }
        file << ")  : ";
        first = true;
        for (auto field : fieldList) {
            if (!first)
                file << ", ";
            if (first)
                first      = false;
            auto fieldName = lox::split(field, " ")[1];
            file << fieldName + "(" + fieldName + ")";
        }
        file << " {}" << std::endl;
        file << "  std::any accept(" << baseName + "Visitor* visitor) override {"
             << std::endl;
        file << "    return visitor->visit" << className << "(this);" << std::endl;
        file << "  }" << std::endl;
        file << "public: " << std::endl;
        for (auto field : fieldList) {
            file << "  " << field << ';' << '\n';
        }
        file << "};" << '\n' << '\n';
    }

    void defineVisitor(std::ofstream& file, const std::string& baseName) {
        auto visitorClassName = baseName + "Visitor";
        file << "class " << visitorClassName << " {" << std::endl;
        file << "public:" << std::endl;
        file << "  virtual ~" << visitorClassName << "() {}" << std::endl;
        for (auto type : astSpec.second) {
            auto className = type.substr(0, type.find(":"));
            file << "  virtual std::any "
                 << "    visit" + className << "(" << className << "* " << baseName
                 << ") = 0;" << std::endl;
        }
        file << "};" << std::endl;
    }

  private:
    const std::string outDir;
    const ASTSpecification astSpec;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: ast_generator <output directory>" << std::endl;
    } else {
        const std::string outDir                     = argv[1];
        const ASTGenerator::ASTSpecification exprSpec = {
            "Expr",
            {"Assign       :Token name, Expr* value",
             "BinaryExpr   :Expr* left, Token Operator, Expr* right",
             "GroupingExpr :Expr* expression",
             "LiteralExpr  :TokenType type, std::string value",
             "UnaryExpr    :Token Operator, Expr* right",
             "Variable     :Token name"}};
        ASTGenerator exprGenerator(outDir, exprSpec);
        exprGenerator.generate();

        const ASTGenerator::ASTSpecification stmtSpec = {
            "Stmt",
            {"Block      : std::vector<Stmt*> stmts",
             "Expression   :Expr* expr",
             "Print :Expr* expr",
             "Var        : Token name, Expr* init"}};
        ASTGenerator stmtGenerator(outDir, stmtSpec);
        stmtGenerator.generate();
    }
    return 0;
}
