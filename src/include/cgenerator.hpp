/// *==============================================================*
///  cgenerator.hpp
/// *==============================================================*

#ifndef CGENERATOR_HPP
#define CGENERATOR_HPP

#include "common.hpp"
#include "xc.hpp"
#include "sourcefile.hpp"

namespace XC {

    class CGenerator {
    public:
        CGenerator(const std::unique_ptr<Module>& module);

        static std::unique_ptr<SourceFile> generateCode(const std::unique_ptr<Module>& module);

    private:
        const std::unique_ptr<Module>& module;

        std::unique_ptr<SourceFile> code;

        uint32_t indention_level;

        bool has_error;

        void generate(void);

        void writeLine(const std::string& line);
        void addIndentation(void);
        void removeIndentation(void);

        void generateStructureDeclaration(void);
        void generateFunctionDeclaration(void);
        void generateStructureImplementation(void);
        void generateFunctionImplementation(void);

        void generateBlockStatement(const BlockStatement* block);
        void generateStatement(const Statement* statement);

        std::string error(void);

        std::string translateDataType(const DataType* data_type);
        std::string translateFunctionSignature(const Function* function);
        std::string translateExpression(const Expression* expression);
        std::string translateVariableDeclaration(const VariableDeclarationStatement* declaration);
    };

}

#endif /* CGENERATOR_HPP */