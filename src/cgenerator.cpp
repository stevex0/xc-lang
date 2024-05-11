/// *==============================================================*
///  cgenerator.cpp
/// *==============================================================*

#include "include/cgenerator.hpp"

using namespace XC;

CGenerator::CGenerator(const std::unique_ptr<Module>& module)
    : module(module),
      code(std::make_unique<SourceFile>()),
      indention_level(0), 
      has_error(false) {
    generate();        
}

void CGenerator::generate(void) {
    code->filename = module->source->filename + ".c";

    writeLine("// -- AUTO-GENERATED CODE -- ");
    writeLine("");

    writeLine("#include <stdint.h>\n#include <stdbool.h>\n#include <stdlib.h>\n#include <stddef.h>");
    writeLine("");

    generateStructureDeclaration();
    generateFunctionDeclaration();
    generateStructureImplementation();
    generateFunctionImplementation();

    writeLine("// -- END OF AUTO-GENERATED CODE -- ");
    
    // for (const std::string& line : code->content) {
    //     std::cout << line << std::endl;
    // }
}

void CGenerator::writeLine(const std::string& line) {
    code->content.push_back(std::string(indention_level * 4, ' ') + line);
}

void CGenerator::generateStructureDeclaration(void) {
    const std::vector<Structure*> structures = module->symbols->getAllStructures();
    for (const Structure* structure : structures) {
        writeLine("typedef struct " + structure->name->lexeme + " " + structure->name->lexeme + ";");
    }
    writeLine("");
}

void CGenerator::generateFunctionDeclaration(void) {
    const std::vector<Function*> functions = module->symbols->getAllFunctions();
    for (const Function* function : functions) {
        writeLine(translateFunctionSignature(function) + ";");
    }
    writeLine("");
}

void CGenerator::generateStructureImplementation(void) {
    const std::vector<Structure*> structures = module->symbols->getAllStructures();
    for (const Structure* structure : structures) {
        writeLine("struct " + structure->name->lexeme);
        writeLine("{");
        addIndentation();

        if (structure->members != nullptr) {
            const std::vector<VariableDeclarator*> members = structure->members->members;
            for (const VariableDeclarator* member : members) {
                std::string buffer;
                buffer.append(translateDataType(member->data_type));
                buffer.push_back(' ');
                buffer.append(member->variable_name->lexeme + ";");
                writeLine(buffer);
            }
        }

        removeIndentation();
        writeLine("};");
        writeLine("");
    }
    writeLine("");
}

void CGenerator::generateFunctionImplementation(void) {
    const std::vector<Function*> functions = module->symbols->getAllFunctions();
    for (const Function* function : functions) {
        writeLine(translateFunctionSignature(function));

        generateBlockStatement(function->body);

        writeLine("");
    }
    writeLine("");
}

void CGenerator::generateBlockStatement(const BlockStatement* block) {
    writeLine("{");
    addIndentation();

    const std::vector<Statement*> statements = block->statements;
    for (const Statement* statement : statements) {
        generateStatement(statement);
    }

    removeIndentation();
    writeLine("}");
}

void CGenerator::generateStatement(const Statement* statement) {
    if (statement == nullptr) {
        return;
    }

    if (const ExpressionStatement* expression_statement = get_node_if(statement, ExpressionStatement)) {
        writeLine(translateExpression(expression_statement->expression) + ";"); 
    } 
    else if (const VariableDeclarationStatement* variable_declaration = get_node_if(statement, VariableDeclarationStatement)) {
        writeLine(translateVariableDeclaration(variable_declaration) + ";");
    } else if (const ConditionalStatement* conditional_statement = get_node_if(statement, ConditionalStatement)) {
        writeLine("if (" + translateExpression(conditional_statement->condition)  +")");
        generateBlockStatement(conditional_statement->body);

        if (conditional_statement->else_case != nullptr) {
            writeLine("else");

            if (const BlockStatement* else_case = get_node_if(conditional_statement->else_case, BlockStatement)) {
                generateBlockStatement(else_case);
            } else {
                generateStatement(conditional_statement->else_case);
            }
        }
    } else if (const WhileIteration* while_iteration = get_node_if(statement, WhileIteration)) {
        writeLine("while (" + translateExpression(while_iteration->condition) + ")");
        generateBlockStatement(while_iteration->body);
    } else if (const ForIteration* for_iteration = get_node_if(statement, ForIteration)) {
        std::string buffer;
        {
            
            buffer.append("for (");
            buffer.append(translateVariableDeclaration(for_iteration->initial) + ";");
            buffer.append(translateExpression(for_iteration->condition) + ";");
            buffer.append(translateExpression(for_iteration->update));
            buffer.append(")");
        }

        writeLine(buffer);
        generateBlockStatement(for_iteration->body);
    } else if (const ReturnStatement* return_statement = get_node_if(statement, ReturnStatement)) {
        writeLine("return " + (return_statement->expression != nullptr ? translateExpression(return_statement->expression) : "") + ";");
    } else if (node_is(statement, BreakStatement)) {
        writeLine("break;");
    } else if (node_is(statement, ContinueStatement)) {
        writeLine("continue;");
    } else {
        error();
    }
}

std::string CGenerator::error(void) {
    has_error = true;
    return "/* ERROR */";
}

std::string CGenerator::translateDataType(const DataType* data_type) {
    if (data_type == nullptr) {
        return "void";
    }

    std::string buffer;
    switch (data_type->type_name->type) {
        case TokenType::TYPE_BOOL: buffer.append("bool"); break;
        case TokenType::TYPE_FLOAT: buffer.append("float"); break;
        case TokenType::TYPE_DOUBLE: buffer.append("double"); break;
        case TokenType::TYPE_BYTE: buffer.append("int8_t"); break;
        case TokenType::TYPE_SHORT: buffer.append("int16_t"); break;
        case TokenType::TYPE_INT: buffer.append("int32_t"); break;
        case TokenType::TYPE_LONG: buffer.append("int64_t"); break;
        case TokenType::IDENTIFIER: buffer.append(data_type->type_name->lexeme); break;
        default: {
            error();
        };
    }

    if (data_type->is_reference) {
        buffer.push_back('*');
    }

    return buffer;
}

std::string CGenerator::translateFunctionSignature(const Function* function) {
    if (function == nullptr) {
        return "";
    }

    const IdentifierToken* owner = function->owner;
    const DataType* return_type = function->return_type;
    const IdentifierToken* name = function->name;
    const ParameterList* parameters = function->parameters;

    std::string buffer;

    // return type
    buffer.append(translateDataType(return_type));

    buffer.push_back(' ');

    // function name
    {
        if (owner != nullptr) {
            buffer.append(owner->lexeme + "_");
        }

        buffer.append(name->lexeme);
    }

    // function parameters
    {
        buffer.push_back('(');

        if (owner == nullptr && parameters == nullptr) {
            buffer.append("void");
        }

        if (owner != nullptr) {
            buffer.append(owner->lexeme + "* self");

            if (parameters != nullptr) {
                buffer.append(", ");
            }
        }

        if (parameters != nullptr) {
            for (const VariableDeclarator* parameter : parameters->parameters) {
                const DataType* data_type = parameter->data_type;
                const IdentifierToken* identifier = parameter->variable_name;

                buffer.append(translateDataType(data_type));
                buffer.append(" " + identifier->lexeme + ", ");
            }
        }

        if (buffer.back() == ' ') {
            buffer.pop_back();
            buffer.pop_back();
        }

        buffer.push_back(')');
    }

    return buffer;
}

std::string CGenerator::translateExpression(const Expression* expression) {
    if (expression == nullptr) {
        return "";
    }

    if (const PrefixUnaryExpression* prefix = get_node_if(expression, PrefixUnaryExpression)) {
        std::string op;

        switch (prefix->operation->type) {
            case TokenType::OP_INCREMENT: op = "++"; break;
            case TokenType::OP_DECREMENT: op = "--"; break;
            case TokenType::BOOLEAN_OP_NOT: op = "!"; break;
            case TokenType::BITWISE_OP_COMPLEMENT: op = "~"; break;
            case TokenType::ARITHMETIC_OP_SUB: op = "-"; break;
            case TokenType::BITWISE_OP_AND: op = "&"; break;
            default: op = error(); break;
        }

        return "(" + op + translateExpression(prefix->operand) + ")";
    } else if (const PostfixUnaryExpression* postfix = get_node_if(expression, PostfixUnaryExpression)) {
        std::string op;

        switch (postfix->operation->type) {
            case TokenType::OP_INCREMENT: op = "++"; break;
            case TokenType::OP_DECREMENT: op = "--"; break;
            default: op = error(); break;
        }

        return "(" + translateExpression(postfix->operand) + op + ")";
    } else if (const BinaryExpression* binary = get_node_if(expression, BinaryExpression)) {
        std::string op;

        switch (binary->operation->type) {
            case TokenType::ARITHMETIC_OP_ADD: op = "+"; break;
            case TokenType::ARITHMETIC_OP_SUB: op = "-"; break;
            case TokenType::ARITHMETIC_OP_MUL: op = "*"; break;
            case TokenType::ARITHMETIC_OP_DIV: op = "/"; break;
            case TokenType::ARITHMETIC_OP_MOD: op = "%"; break;
            case TokenType::BITWISE_OP_AND: op = "&"; break;
            case TokenType::BITWISE_OP_OR: op = "|"; break;
            case TokenType::BITWISE_OP_XOR: op = "^"; break;
            case TokenType::BITWISE_OP_LEFT_SHIFT: op = "<<"; break;
            case TokenType::BITWISE_OP_RIGHT_SHIFT: op = ">>"; break;
            case TokenType::RELATIONAL_OP_EQUALITY: op = "=="; break;
            case TokenType::RELATIONAL_OP_INEQUALITY: op = "!="; break;
            case TokenType::RELATIONAL_OP_LESS_THAN: op = "<"; break;
            case TokenType::RELATIONAL_OP_LESS_THAN_EQUAL: op = "<="; break;
            case TokenType::RELATIONAL_OP_GREATER_THAN: op = ">"; break;
            case TokenType::RELATIONAL_OP_GREATER_THAN_EQUAL: op = ">="; break;
            case TokenType::BOOLEAN_OP_AND: op = "&&"; break;
            case TokenType::BOOLEAN_OP_OR: op = "||"; break;
            case TokenType::BOOLEAN_OP_XOR: op = "^"; break;
            case TokenType::ASSIGNMENT_ASSIGN: op = '='; break;
            case TokenType::ASSIGNMENT_OP_ADD: op = "+="; break;
            case TokenType::ASSIGNMENT_OP_SUB: op = "-="; break;
            case TokenType::ASSIGNMENT_OP_MUL: op = "*="; break;
            case TokenType::ASSIGNMENT_OP_DIV: op = "/="; break;
            case TokenType::ASSIGNMENT_OP_MOD: op = "%="; break;
            case TokenType::ASSIGNMENT_OP_AND: op = "&="; break;
            case TokenType::ASSIGNMENT_OP_OR: op = "|="; break;
            case TokenType::ASSIGNMENT_OP_XOR: op = "^="; break;
            case TokenType::ASSIGNMENT_OP_LEFT_SHIFT: op = "<<="; break;
            case TokenType::ASSIGNMENT_OP_RIGHT_SHIFT: op = ">>="; break;
            default: op = error(); break;
        }

        return "(" + translateExpression(binary->left_operand) + " " + op + " " + translateExpression(binary->right_operand) + ")";
    } else if (const LiteralExpression* literal = get_node_if(expression, LiteralExpression)) {
        switch (literal->value->type) {
            case TokenType::LITERAL_BOOLEAN_TRUE: return "true";
            case TokenType::LITERAL_BOOLEAN_FALSE: return "false";
            case TokenType::LITERAL_REFERENCE_NULL: return "NULL";
            default: return error();
        }

    } else if (const NumberConstant* number = get_node_if(expression, NumberConstant)) {
        switch (number->value->type) {
            case TokenType::INTEGER_LITERAL: {
                std::string num = number->value->lexeme;

                if (num.size() > 2 && num.at(0) == '0' && num.at(1) == 'o') {
                    num.at(1) = '0';
                }

                return num;
            }
            case TokenType::FLOAT_LITERAL: {
                return number->value->lexeme + "f";
            }
            default: return error();
        }
    } else if (const FunctionCall* function_call = get_node_if(expression, FunctionCall)) {
        std::string function_name;
        std::string arguments;
        if (const MemberAccess* member_function = get_node_if(function_call->function, MemberAccess)) {
            if (member_function->owner->evaluated_type == nullptr) {
                return error();
            }

            function_name = member_function->owner->evaluated_type->type_name->lexeme + "_" + member_function->member->lexeme;
            arguments.push_back('(');

            arguments.append((member_function->owner->evaluated_type->is_reference ? "" : "&") + translateExpression(member_function->owner) + ", ");

            if (function_call->arguments != nullptr) {
                const std::vector<Expression*> args = function_call->arguments->expressions;

                for (const Expression* arg : args) {
                    arguments.append(translateExpression(arg) + ", ");
                }

            }

            if (arguments.back() == ' ') {
                arguments.pop_back();
                arguments.pop_back();
            }

            arguments.push_back(')');
        } else {
            function_name = translateExpression(function_call->function);
            arguments.push_back('(');

            if (function_call->arguments != nullptr) {
                const std::vector<Expression*> args = function_call->arguments->expressions;

                for (const Expression* arg : args) {
                    arguments.append(translateExpression(arg) + ", ");
                }

            }

            if (arguments.back() == ' ') {
                arguments.pop_back();
                arguments.pop_back();
            }

            arguments.push_back(')');
        }


        return function_name + arguments;
    } else if (const MemberAccess* member_access = get_node_if(expression, MemberAccess)) {
        return "(" + translateExpression(member_access->owner) + "." + member_access->member->lexeme + ")";
    } else if (const IdentifierConstant* identifier = get_node_if(expression, IdentifierConstant)) {
        if (identifier->evaluated_type != nullptr && identifier->evaluated_type->is_reference) {
            return "(*" + identifier->value->lexeme + ")";
        }
        return identifier->value->lexeme;
    } else {
        return error();
    }
}

std::string CGenerator::translateVariableDeclaration(const VariableDeclarationStatement* declaration) {
    if (declaration == nullptr) {
        return "";
    }

    const VariableDeclarator* variable_declarator = declaration->declarator;
    const Expression* initial_value = declaration->initial;

    std::string buffer;
    buffer.append(translateDataType(variable_declarator->data_type));
    buffer.push_back(' ');
    buffer.append(variable_declarator->variable_name->lexeme);

    if (initial_value != nullptr) {
        buffer.append(" = ");
        buffer.append(translateExpression(initial_value));
    }

    return buffer;
}

void CGenerator::addIndentation(void) {
    ++indention_level;
}

void CGenerator::removeIndentation(void) {
    if (indention_level > 0) {
        --indention_level;
    }
}

std::unique_ptr<SourceFile> CGenerator::generateCode(const std::unique_ptr<Module>& module) {
    CGenerator generator (module);
    return generator.has_error ? nullptr : std::move(generator.code);
}