/// *==============================================================*
///  ast.cpp
/// *==============================================================*
#include "include/ast.hpp"

using namespace XC;

Expression* XC::newBinaryExpression(OperatorToken* _operator, Expression* left_operand, Expression* right_operand) {
    BinaryExpression* binary = new BinaryExpression;
    ErrorNode* errors = new ErrorNode;

    binary->operation = _operator;

    binary->left_operand = left_operand;
    if (ErrorNode* error_in_left_operand = get_node_if(binary->left_operand, ErrorNode)) {
        binary->left_operand = nullptr;
        errors->appendError(error_in_left_operand);
    }

    binary->right_operand = right_operand;
    if (ErrorNode* error_in_right_operand = get_node_if(binary->right_operand, ErrorNode)) {
        binary->right_operand = nullptr;
        errors->appendError(error_in_right_operand);
    }

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete binary;
            return (Expression*) errors_found;
        }
    }

    return binary;
}

Expression* XC::newPrefixExpression(OperatorToken* _operator, Expression* operand) {
    PrefixUnaryExpression* prefix = new PrefixUnaryExpression;

    prefix->operation = _operator;
    prefix->operand = operand;

    if (ErrorNode* error = get_node_if(prefix->operand, ErrorNode)) {
        prefix->operand = nullptr;
        delete prefix;
        return (Expression*) error;
    }

    return prefix;
}

Expression* XC::newPostfixExpression(OperatorToken* _operator, Expression* operand) {
    PostfixUnaryExpression* post = new PostfixUnaryExpression;

    post->operation = _operator;
    post->operand = operand;

    if (ErrorNode* error = get_node_if(post->operand, ErrorNode)) {
        post->operand = nullptr;
        delete post;
        return (Expression*) error;
    }

    return post;
}


void XC::printTree(AST* node, std::string indent, bool last) {
    if (node == nullptr) return;

    std::cout << indent;
    if (last) {
        std::cout << "  `--";
        indent += "     ";
    } else {
        std::cout << "  |--";
        indent += "  |  ";
    }

    if (const auto* x = get_node_if(node, PrefixUnaryExpression)) {
        std::cout << "( " << x->operation->lexeme << " ) pre" << std::endl;
        printTree(x->operand, indent, true);
    }

    else if (const auto* x = get_node_if(node, PostfixUnaryExpression)) {
        std::cout << "( " << x->operation->lexeme << " ) post" << std::endl;
        printTree(x->operand, indent, true);
    }

    else if (const auto* x = get_node_if(node, BinaryExpression)) {
        std::cout << "( " << x->operation->lexeme << " )" << std::endl;
        printTree(x->left_operand, indent, false);
        printTree(x->right_operand, indent, true);
    }

    else if (const auto* x = get_node_if(node, LiteralExpression)) {
        std::cout << "( \'" << x->value->lexeme << "\' )" << std::endl;
    }

    else if (const auto* x = get_node_if(node, NumberConstant)) {
        std::cout << "( \'" << x->value->lexeme << "\' )" << std::endl;
    }

    else if (const auto* x = get_node_if(node, IdentifierConstant)) {
        std::cout << "( \'" << x->value->lexeme << "\' )" << std::endl;
    }

    else if (const auto* x = get_node_if(node, MemberAccess)) {
        std::cout << "( . )" << std::endl;
        printTree(x->owner, indent, false);
        std::cout << indent <<"  `--( \'" << x->member->lexeme << "\' )" << std::endl; 
    }

    else if (const auto* x = get_node_if(node, ArrayAccess)) {
        std::cout << "( [] )" << std::endl;
        printTree(x->array, indent, false);
        printTree(x->index, indent, true);
    } 

    else if (const auto* x = get_node_if(node, ExpressionList)) {
        std::cout << "(   )" << std::endl;
        if (!x->expressions.empty()) {
            for (uint32_t i = 0; i < x->expressions.size() - 1; ++i) {
                printTree(x->expressions.at(i), indent, false);
            }
            printTree(x->expressions.back(), indent, true);
        }
    } 
    
    else if (const auto* x = get_node_if(node, FunctionCall)) {
        std::cout << "( () )" << std::endl;
        printTree(x->function, indent, x->arguments == nullptr);
        printTree(x->arguments, indent, true);
    }
    else if (const auto* x = get_node_if(node, DataType)) {
        std::string name = "";
        if (x->is_reference) name.push_back('&');
        if (x->type_name != nullptr) name.append(x->type_name->lexeme);
        for (uint32_t i = 0; i < x->dimensions; ++i) {
            name.append("[]");
        }

        std::cout << "( \'" << name << "\' )" << std::endl;
    }

    // TODO
    // else if (const auto* x = get_if(node, CastExpression)) {
    //     std::cout << "( CAST )" << std::endl;
    //     printTree(x->data_type, indent, false);
    //     printTree(x->expression, indent, true);
    // }

    else if (const auto* x = get_node_if(node, ExpressionStatement)) {
        std::cout << "( EXPRESSION STATEMENT )" << std::endl;
        printTree(x->expression, indent, true);
    }

    else if (const auto* x = get_node_if(node, VariableDeclarator)) {
        std::cout << "( VARIABLE DECLARATOR )" << std::endl;
        printTree(x->data_type, indent, false);
        std::cout << indent << "  `--( \'"  <<  x->variable_name->lexeme << "\')" << std::endl;
    }

    else if (const auto* x = get_node_if(node, VariableDeclarationStatement)) {
        std::cout << "( VARIABLE DECLARATION )" << std::endl;
        printTree(x->declarator, indent, x->initial == nullptr);
        printTree(x->initial, indent, true);
    } 

    else if (const auto* x = get_node_if(node, ReturnStatement)) {
        std::cout << "( RETURN )" << std::endl;
        printTree(x->expression, indent, true);
    }

    else if (node_is(node, ContinueStatement)) {
        std::cout << "( CONTINUE )" << std::endl;
    }

    else if (node_is(node, BreakStatement)) {
        std::cout << "( BREAK )" << std::endl;
    }

    else if (const auto* x = get_node_if(node, BlockStatement)) {
        std::cout << "( BLOCK STATEMENT )" << std::endl;
        if (!x->statements.empty()) {
            for (uint32_t i = 0; i < x->statements.size() - 1; ++i) {
                printTree(x->statements.at(i), indent, false);
            }
            printTree(x->statements.back(), indent, true);
        }
    }

    else if (const auto* x = get_node_if(node, WhileIteration)) {
        std::cout << "( WHILE )" << std::endl;
        printTree(x->condition, indent, false);
        printTree(x->body, indent, true);
    }

    else if (const auto* x = get_node_if(node, ForIteration)) {
        std::cout << "( FOR )" << std::endl;
        printTree(x->initial, indent, false);
        printTree(x->condition, indent, false);
        printTree(x->update, indent, false);
        printTree(x->body, indent, true);
    }

    else if (const auto* x = get_node_if(node, ParameterList)) {
        std::cout << "( PARAMETERS )" << std::endl;
        if (!x->parameters.empty()) {
            for (uint32_t i = 0; i < x->parameters.size() - 1; ++i) {
                printTree(x->parameters.at(i), indent, false);
            }
            printTree(x->parameters.back(), indent, true);
        }
    }

    else if (const auto* x = get_node_if(node, ConditionalStatement)) {
        std::cout << "( IF )" << std::endl;
        printTree(x->condition, indent, false);
        printTree(x->body, indent, x->else_case == nullptr);
        printTree(x->else_case, indent, true);
    }

    else if (const auto* x = get_node_if(node, Function)) {
        std::string name = "";
        if (x->owner != nullptr) name.append(x->owner->lexeme).append(" :: ");
        name.append(x->name->lexeme);
        std::cout << "FUNCTION ( " << name << " )" << std::endl;
        printTree(x->return_type, indent, false);
        printTree(x->parameters, indent, false);
        printTree(x->body, indent, true);
    }

    else if (const auto* x = get_node_if(node, StructureMembers)) {
        std::cout << "(   )" << std::endl;
        if (!x->members.empty()) {
            for (uint32_t i = 0; i < x->members.size() - 1; ++i) {
                printTree(x->members.at(i), indent, false);
            }
            printTree(x->members.back(), indent, true);
        }
    }

    else if (const auto* x = get_node_if(node, Structure)) {
        std::cout << "STRUCTURE ( " << x->name->lexeme << " )" << std::endl;
        printTree(x->members, indent, true);
    }

    else if (const auto* x = get_node_if(node, Program)) {
        std::cout << "( PROGRAM )" << std::endl;
        if (!x->declarations.empty()) {
            for (uint32_t i = 0; i < x->declarations.size() - 1; ++i) {
                printTree(x->declarations.at(i), indent, false);
            }
            printTree(x->declarations.back(), indent, true);
        }
    }

    else if (const auto* x = get_node_if(node, ErrorNode)) {
        std::cout << "( ERROR: \"" << x->reason << "\" )" << std::endl;
    }
    
    else {
        std::cout << "( ? )" << std::endl;
    }
}
