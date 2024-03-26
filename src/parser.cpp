/// *==============================================================*
///  parser.cpp
/// *==============================================================*
#include "include/parser.hpp"

using namespace XC;

Parser::Parser(const std::unique_ptr<Module>& module)
    : module(module),
      position(0),
      program(std::make_unique<Program>()),
      has_error(false) {
    parse();
}

void Parser::parse(void) {
    if (atEnd()) return;

    while (!atEnd()) {
        Declaration* declaration = (Declaration*) parseDeclaration();

        if (ErrorNode* error = get_node_if(declaration, ErrorNode)) {
            reportError(error);
            delete declaration;

            // try to recover
            while (!atEnd() && !(consumeIf(TokenType::PUNCTUATION_SEMI_COLON) || consumeIf(TokenType::PUNCTUATION_RIGHT_BRACE))) next();
        } else {
            program->declarations.push_back(declaration);
        }
    }

    // if (!has_error) {
    //     printTree(program.get(), "", true);
    // }
}

bool Parser::atEnd(void) {
    return current().type == TokenType::END_OF_FILE;
}

const Token& Parser::current(void) {
    return module->tokens->at(position);
}

const Token& Parser::next(void) {
    const Token& now = current();

    if (atEnd()) {
        return now;
    }

    ++position;

    return now;
}

const Token& Parser::peek(void) {
    if (atEnd()) {
        return current();
    }

    return module->tokens->at(position + 1);
}

bool Parser::match(const TokenType expect) {
    if (atEnd() && expect != TokenType::END_OF_FILE) {
        return false;
    }

    return current().type == expect;
}

bool Parser::matchNext(const TokenType expect) {
    if (atEnd() && expect != TokenType::END_OF_FILE) {
        return false;
    }

    return peek().type == expect;
}


bool Parser::consumeIf(const TokenType expect) {
    if (!match(expect)) {
        return false;
    }

    ++position;

    return true;
}

ErrorNode* Parser::error(const std::string message) {
    ErrorNode* err = new ErrorNode;

    err->reason = message;
    err->occurrence = position;

    return err;
}

void Parser::reportError(ErrorNode*& error) {
    // xc: error: message   |< header
    //  --> file:ln:col     |< info
    //    :                 |< divider
    // ln | content         |< line content
    //    : underline       |< footer

    const std::string& message = error->reason;
    Token& left_token = module->tokens->at(max_of(error->occurrence, 1) - 1);
    Token& right_token = module->tokens->at(error->occurrence);

    const uint32_t occurrence_line = left_token.line;
    const uint32_t occurrence_column = left_token.column + left_token.lexeme.size();

    const std::string header = "xc: \033[31merror\033[0m: " + message + '\n';
    const std::string line_column = std::to_string(occurrence_line + 1) + ':' + std::to_string(occurrence_column + 1);
    const std::string info = " --> " + module->source->filename + ':' + line_column + '\n';

    std::string preview;
    {
        const std::string line_number = std::to_string(occurrence_line + 1);
        const std::string divider = std::string(line_number.size() + 2, ' ') + ':';
        const std::string line = module->source->content.at(occurrence_line); 

        const std::string line_content = " " + line_number + " | " + line;

        std::string underline;
        {
            if (left_token.index == right_token.index) {
                underline.append(std::string(left_token.column + 1, ' '));
                underline.append(std::string(left_token.lexeme.size(), '^'));
            } else if (left_token.line != right_token.line) {
                underline.append(std::string(left_token.column + left_token.lexeme.size() + 1, ' '));
                underline.append(std::string(1, '^'));
            } else if (left_token.column + left_token.lexeme.size() == right_token.column) {
                underline.append(std::string(left_token.column + 1, ' '));
                underline.append(std::string(left_token.lexeme.size() + right_token.lexeme.size(), '^'));
            } else {
                underline.append(std::string(left_token.column + left_token.lexeme.size() + 1, ' '));
                underline.append(std::string(right_token.column - (left_token.column + left_token.lexeme.size()), '^'));
            }
        }

        const std::string footer = divider + underline;

        preview = divider + '\n' + line_content + footer;
    }

    std::cerr << header << info << preview << std::endl;

    has_error = true;

    if (error->additional_errors != nullptr) {
        reportError(error->additional_errors);
    }
}

AST* Parser::parseDeclaration(void) {
    return tryParse({
        parseStructure,
        parseFunction
    }, "expected declaration");
}

AST* Parser::parseFunction(void) {
    Function* function = new Function;
    ErrorNode* errors = new ErrorNode;

    if (match(TokenType::IDENTIFIER) && matchNext(TokenType::PUNCTUATION_DOUBLE_COLON)) {
        function->owner = &next();

        next(); // ::
    }

    function->return_type = (DataType*) parseDataType();
    if (ErrorNode* error_in_return_type = get_node_if(function->return_type, ErrorNode)) {
        function->return_type = nullptr;
        errors->appendError(error_in_return_type);
    }
    
    if (!match(TokenType::IDENTIFIER)) {
        errors->appendError(error("expected identifier"));
    } else {
        function->name = &next();
    }

    if (!consumeIf(TokenType::PUNCTUATION_LEFT_PARENTHESIS)) {
        errors->appendError(error("expected `(`"));
    }

    if (!consumeIf(TokenType::TYPE_VOID)) {
        function->parameters = (ParameterList*) tryParse({parseParameters}, "expected parameters");
        if (ErrorNode* error_in_parameters = get_node_if(function->parameters, ErrorNode)) {
            function->parameters = nullptr;
            errors->appendError(error_in_parameters);
        }
    }

    if (!consumeIf(TokenType::PUNCTUATION_RIGHT_PARENTHESIS)) {
        errors->appendError(error("expected `)`"));
    }

    function->body = (BlockStatement*) parseBlockStatement();
    if (ErrorNode* error_in_body = get_node_if(function->body, ErrorNode)) {
        function->body = nullptr;
        errors->appendError(error_in_body);
    }

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete function;
            return errors_found;
        }
    }

    return function;
}

AST* Parser::parseStructure(void) {
    if (!consumeIf(TokenType::KEYWORD_STRUCT)) {
        return error("expected keyword `struct`");
    }

    Structure* structure = new Structure;
    ErrorNode* errors = new ErrorNode;

    if (!match(TokenType::IDENTIFIER)) {
        errors->appendError(error("expected identifier"));
    } else {
        structure->name = &next();
    }

    if (!consumeIf(TokenType::PUNCTUATION_LEFT_BRACE)) {
        errors->appendError(error("expected `{`"));
    }

    structure->members = (StructureMembers*) tryParse({parseStructureMembers}, "expected structure members");
    if (ErrorNode* error_in_members = get_node_if(structure->members, ErrorNode)) {
        structure->members = nullptr;
        errors->appendError(error_in_members);
    }

    if (!consumeIf(TokenType::PUNCTUATION_RIGHT_BRACE)) {
        errors->appendError(error("expected `}`"));
    }

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete structure;
            return errors_found;
        }
    }

    return structure;
}

AST* Parser::parseStructureMembers(void) {
    StructureMembers* structure_members = new StructureMembers;

    ErrorNode* errors = new ErrorNode;

    do {
        VariableDeclarator* member = (VariableDeclarator*) parseVariableDeclarator();
        if (ErrorNode* error_in_member = get_node_if(member, ErrorNode)) {
            errors->appendError(error_in_member);

            // try to recover
            while (!atEnd() && !(consumeIf(TokenType::PUNCTUATION_SEMI_COLON) || match(TokenType::PUNCTUATION_RIGHT_BRACE))) next();
        } else {
            if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
                delete member;
                errors->appendError(error("expected `;`"));

            } else {
                structure_members->members.push_back(member);
            }
        }
    } while(!atEnd() && !match(TokenType::PUNCTUATION_RIGHT_BRACE));

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete structure_members;
            return errors_found;
        }
    }

    return structure_members;
}

AST* Parser::parseParameters(void) {
    ParameterList* parameter_list = new ParameterList;

    ErrorNode* errors = new ErrorNode;

    do {
        VariableDeclarator* parameter = (VariableDeclarator*) parseVariableDeclarator();

        if (ErrorNode* error_in_parameter = get_node_if(parameter, ErrorNode)) {
            errors->appendError(error_in_parameter);
        } else {
            parameter_list->parameters.push_back(parameter);
        }

    } while (!atEnd() && consumeIf(TokenType::PUNCTUATION_COMMA));

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete parameter_list;
            return errors_found;
        }
    }

    return parameter_list;
}

AST* Parser::parseBlockStatement(void) {
    ErrorNode* errors = new ErrorNode;

    if (!consumeIf(TokenType::PUNCTUATION_LEFT_BRACE)) {
        errors->appendError(error("expected `{"));
    }

    BlockStatement* block = new BlockStatement;

    while (!atEnd() && !match(TokenType::PUNCTUATION_RIGHT_BRACE)) {
        Statement* statement = (Statement*) parseStatement();

        if (ErrorNode* error_in_statement = get_node_if(statement, ErrorNode)) {
            errors->appendError(error_in_statement);

            // will try to recover
            while (!atEnd() && !consumeIf(TokenType::PUNCTUATION_RIGHT_BRACE)) next();
        } else {
            block->statements.push_back(statement);
        }
    }

    if (atEnd() || !consumeIf(TokenType::PUNCTUATION_RIGHT_BRACE)) {
        errors->appendError(error("expected `}`"));
    }

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete block;
            return errors_found;
        }
    }

    return block;
}

AST* Parser::parseStatement(void) {
    return tryParse({
        parseExpressionStatement,
        parseVariableDeclarationStatement,
        parseConditionalStatement,
        parseWhileIteration,
        parseForIteration,
        parseReturnStatement,
        parseContinueStatement,
        parseBreakStatement
    }, "expected statement");
}

AST* Parser::parseExpressionStatement(void) {
    ExpressionStatement* expression_statement = new ExpressionStatement;

    expression_statement->expression = (Expression*) parseExpression();
    if (ErrorNode* error_found = get_node_if(expression_statement->expression, ErrorNode)) {
        expression_statement->expression = nullptr;
        delete expression_statement;

        if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
            error_found->appendError(error("expected `;`"));
        }

        return error_found;
    }

    if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
        delete expression_statement;
        return error("expected `;`");
    }

    return expression_statement;
}

AST* Parser::parseVariableDeclarationStatement(void) {
    VariableDeclarationStatement* variable_declaration = new VariableDeclarationStatement;
    ErrorNode* errors = new ErrorNode;

    variable_declaration->declarator = (VariableDeclarator*) parseVariableDeclarator();
    if (ErrorNode* error_in_declarator = get_node_if(variable_declaration->declarator, ErrorNode)) {
        variable_declaration->declarator = nullptr;
        errors->appendError(error_in_declarator);
    }

    if (consumeIf(TokenType::ASSIGNMENT_ASSIGN)) {
        variable_declaration->initial = (Expression*) parseExpression();

        if (ErrorNode* error_in_inital_value = get_node_if(variable_declaration->initial, ErrorNode)) {
            variable_declaration->initial = nullptr;
            errors->appendError(error_in_inital_value);
        }
    }

    if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
        errors->appendError(error("expected `;`"));
    }

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete variable_declaration;
            return errors_found;
        }
    }

    return variable_declaration;
}

AST* Parser::parseWhileIteration(void) {
    if (!consumeIf(TokenType::KEYWORD_WHILE)) {
        return error("expected keyword `while`");
    }

    WhileIteration* iteration = new WhileIteration;
    ErrorNode* errors = new ErrorNode;

    iteration->condition = (Expression*) parseGrouping();
    if (ErrorNode* error_in_condition = get_node_if(iteration->condition, ErrorNode)) {
        iteration->condition = nullptr;
        errors->appendError(error_in_condition);
    }

    iteration->body = (BlockStatement*) parseBlockStatement();
    if (ErrorNode* error_in_body = get_node_if(iteration->body, ErrorNode)) {
        iteration->body = nullptr;
        errors->appendError(error_in_body);
    }

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete iteration;
            return errors_found;
        }
    }

    return iteration;
}

AST* Parser::parseForIteration(void) {
    if (!consumeIf(TokenType::KEYWORD_FOR)) {
        return error("expected keyword `for`");
    }

    ForIteration* iteration = new ForIteration;
    ErrorNode* errors = new ErrorNode;

    if (!consumeIf(TokenType::PUNCTUATION_LEFT_PARENTHESIS)) {
        errors->appendError(error("expected `(`"));
    }

    if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
        iteration->initial = (VariableDeclarationStatement*) parseVariableDeclarationStatement();
        if (ErrorNode* error_in_init = get_node_if(iteration->initial, ErrorNode)) {
            iteration->initial = nullptr;
            errors->appendError(error_in_init);
        }
    }

    if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
        iteration->condition = (Expression*) parseExpression();
        if (ErrorNode* error_in_condition = get_node_if(iteration->condition, ErrorNode)) {
            iteration->condition = nullptr;
            errors->appendError(error_in_condition);
        }

        if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
            errors->appendError(error("expected `;`"));
        }
    }

    if (!consumeIf(TokenType::PUNCTUATION_RIGHT_PARENTHESIS)) {
        iteration->update = (Expression*) parseExpression();
        if (ErrorNode* error_in_update = get_node_if(iteration->update, ErrorNode)) {
            iteration->update = nullptr;
            errors->appendError(error_in_update);
        }

        if (!consumeIf(TokenType::PUNCTUATION_RIGHT_PARENTHESIS)) {
            errors->appendError(error("expected `)`"));
        }
    }

    iteration->body = (BlockStatement*) parseBlockStatement();
    if (ErrorNode* error_in_body = get_node_if(iteration->body, ErrorNode)) {
        iteration->body = nullptr;
        errors->appendError(error_in_body);
    }

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete iteration;
            return errors_found;
        }
    }

    return iteration;
}

AST* Parser::parseReturnStatement(void) {
    if (!consumeIf(TokenType::KEYWORD_RETURN)) {
        return error("expected keyword `return`");
    }

    ReturnStatement* _return = new ReturnStatement;

    if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
        _return->expression = (Expression*) parseExpression();
        if (ErrorNode* error_found = get_node_if(_return->expression, ErrorNode)) {
            _return->expression = nullptr;
            delete _return;

            if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
                error_found->appendError(error("expected `;`"));
            }

            return error_found;
        }

        if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
            delete _return;
            return error("expected `;`");
        }
    }

    return _return;
}

AST* Parser::parseContinueStatement(void) {
    if (!consumeIf(TokenType::KEYWORD_CONTINUE)) {
        return error("expected keyword `continue`");
    }

    if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
        return error("expected `;`");
    }

    return new ContinueStatement;
}

AST* Parser::parseBreakStatement(void) {
    if (!consumeIf(TokenType::KEYWORD_BREAK)) {
        return error("expected keyword `break`");
    }

    if (!consumeIf(TokenType::PUNCTUATION_SEMI_COLON)) {
        return error("expected `;`");
    }

    return new BreakStatement;
}

AST* Parser::parseConditionalStatement(void) {
    if (!consumeIf(TokenType::KEYWORD_IF)) {
        return error("expected keyword `if`");
    }

    ConditionalStatement* conditional = new ConditionalStatement;
    ErrorNode* errors = new ErrorNode;

    conditional->condition = (Expression*) parseGrouping();
    if (ErrorNode* error = get_node_if(conditional->condition, ErrorNode)) {
        errors->appendError(error);
        conditional->condition = nullptr;
    }

    conditional->body = (BlockStatement*) parseBlockStatement();
    if (ErrorNode* error = get_node_if(conditional->body, ErrorNode)) {
        errors->appendError(error);
        conditional->body = nullptr;
    }

    if (consumeIf(TokenType::KEYWORD_ELSE)) {
        conditional->else_case = (ConditionalStatement*) tryParse({
            parseConditionalStatement,
            parseBlockStatement
        }, "expected keyword `if` or `{`");
    }

    if (ErrorNode* error = get_node_if(conditional->else_case, ErrorNode)) {
        errors->appendError(error);
        conditional->else_case = nullptr;
    }

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete conditional;
            return errors_found;
        }
    }

    return conditional;
}

AST* Parser::parseDataType(void) {
    DataType* data_type = new DataType;

    data_type->is_reference = consumeIf(TokenType::BITWISE_OP_AND);

    data_type->type_name = nullptr;
    if (
        !(match(TokenType::IDENTIFIER)
        || match(TokenType::TYPE_BOOL)
        || match(TokenType::TYPE_BYTE)
        || match(TokenType::TYPE_SHORT)
        || match(TokenType::TYPE_INT)
        || match(TokenType::TYPE_LONG)
        || match(TokenType::TYPE_FLOAT)
        || match(TokenType::TYPE_DOUBLE)
        || match(TokenType::TYPE_VOID))
    ) {
        delete data_type;
        return error("expected identifier, or type");
    }

    data_type->type_name = &next();

    data_type->dimensions = 0;

    while (!atEnd() && (match(TokenType::PUNCTUATION_LEFT_BRACKET) && matchNext(TokenType::PUNCTUATION_RIGHT_BRACKET))) {
        next(); // [
        next(); // ]
        ++data_type->dimensions;
    }

    return data_type;
}

AST* Parser::parseVariableDeclarator(void) {
    VariableDeclarator* declarator = new VariableDeclarator;
    ErrorNode* errors = new ErrorNode;

    declarator->data_type = (DataType*) parseDataType();
    if (ErrorNode* error_in_data_type = get_node_if(declarator->data_type, ErrorNode)) {
        declarator->data_type = nullptr;
        errors->appendError(error_in_data_type);
    }

    if (!match(TokenType::IDENTIFIER)) {
        errors->appendError(error("expected identifier"));
    } else {
        declarator->variable_name = &next();
    }

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete declarator;
            return errors_found;
        }
    }

    return declarator;
}


AST* Parser::parseExpression(void) {
    return parseAssignment();
}

AST* Parser::parseAssignment(void) {
    Expression* expression = (Expression*) parseBooleanOR();

    while (
        match(TokenType::ASSIGNMENT_ASSIGN)
        || match(TokenType::ASSIGNMENT_OP_ADD)
        || match(TokenType::ASSIGNMENT_OP_SUB)
        || match(TokenType::ASSIGNMENT_OP_MUL)
        || match(TokenType::ASSIGNMENT_OP_DIV)
        || match(TokenType::ASSIGNMENT_OP_MOD)
        || match(TokenType::ASSIGNMENT_OP_OR)
        || match(TokenType::ASSIGNMENT_OP_XOR)
        || match(TokenType::ASSIGNMENT_OP_AND)
        || match(TokenType::ASSIGNMENT_OP_LEFT_SHIFT)
        || match(TokenType::ASSIGNMENT_OP_RIGHT_SHIFT)
    ) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parseBooleanOR()
        );
    }

    return expression;
}

AST* Parser::parseBooleanOR(void) {
    Expression* expression = (Expression*) parseBooleanXOR();

    while (match(TokenType::BOOLEAN_OP_OR)) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parseBooleanXOR()
        );
    }

    return expression;
}

AST* Parser::parseBooleanXOR(void) {
    Expression* expression = (Expression*) parseBooleanAND();

    while (match(TokenType::BOOLEAN_OP_XOR)) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parseBooleanAND()
        );
    }

    return expression;
}

AST* Parser::parseBooleanAND(void) {
    Expression* expression = (Expression*) parseBitwiseOR();

    while (match(TokenType::BOOLEAN_OP_AND)) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parseBitwiseOR()
        );
    }

    return expression;
}

AST* Parser::parseBitwiseOR(void) {
    Expression* expression = (Expression*) parseBitwiseXOR();

    while (match(TokenType::BITWISE_OP_OR)) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parseBitwiseXOR()
        );
    }

    return expression;
}

AST* Parser::parseBitwiseXOR(void) {
    Expression* expression = (Expression*) parseBitwiseAND();
    
    while (match(TokenType::BITWISE_OP_XOR)) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parseBitwiseAND()
        );
    }

    return expression;
}

AST* Parser::parseBitwiseAND(void) {
    Expression* expression = (Expression*) parseEquality();

    while (match(TokenType::BITWISE_OP_AND)) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parseEquality()
        );
    }

    return expression;
}

AST* Parser::parseEquality(void) {
    Expression* expression = (Expression*) parseRelational();
    
    while (
        match(TokenType::RELATIONAL_OP_EQUALITY)
        || match(TokenType::RELATIONAL_OP_INEQUALITY)
    ) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parseRelational()
        );
    }

    return expression;
}

AST* Parser::parseRelational(void) {
    Expression* expression = (Expression*) parseBitwiseShift();

    while (
        match(TokenType::RELATIONAL_OP_LESS_THAN)
        || match(TokenType::RELATIONAL_OP_GREATER_THAN)
        || match(TokenType::RELATIONAL_OP_LESS_THAN_EQUAL)
        || match(TokenType::RELATIONAL_OP_GREATER_THAN_EQUAL)
    ) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parseBitwiseShift()
        );
    }

    return expression;
}

AST* Parser::parseBitwiseShift(void) {
    Expression* expression = (Expression*) parseAdditive();

    while (
        match(TokenType::BITWISE_OP_LEFT_SHIFT)
        || match(TokenType::BITWISE_OP_RIGHT_SHIFT)
    ) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parseAdditive()
        );
    }

    return expression;
}

AST* Parser::parseAdditive(void) {
    Expression* expression = (Expression*) parseMultiplicative();

    while (
        match(TokenType::ARITHMETIC_OP_ADD)
        || match(TokenType::ARITHMETIC_OP_SUB)
    ) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parseMultiplicative()
        );
    }

    return expression;
}

AST* Parser::parseMultiplicative(void) {
    Expression* expression = (Expression*) parsePrefix();

    while (
        match(TokenType::ARITHMETIC_OP_MUL)
        || match(TokenType::ARITHMETIC_OP_DIV)
        || match(TokenType::ARITHMETIC_OP_MOD)
    ) {
        OperatorToken* _operator = &next();
        expression = newBinaryExpression(
            _operator,
            expression,
            (Expression*) parsePrefix()
        );
    }

    return expression;
}

AST* Parser::parsePrefix(void) {
    if (
        match(TokenType::OP_INCREMENT)
        || match(TokenType::OP_DECREMENT)
        || match(TokenType::ARITHMETIC_OP_SUB)
        || match(TokenType::BOOLEAN_OP_NOT)
        || match(TokenType::BITWISE_OP_COMPLEMENT)
        || match(TokenType::BITWISE_OP_AND)
    ) {
        OperatorToken* _operator = &next();
        return newPrefixExpression(_operator, (Expression*) parsePostfix());
    }

    return parsePostfix();
}

AST* Parser::parsePostfix(void) {
    Expression* expression = (Expression*) parsePrimary();

    while (
        match(TokenType::PUNCTUATION_DOT)
        || match(TokenType::PUNCTUATION_LEFT_PARENTHESIS)
        || match(TokenType::PUNCTUATION_LEFT_BRACKET)
    ) {
        if (consumeIf(TokenType::PUNCTUATION_DOT)) {
            MemberAccess* member_access = new MemberAccess;
            ErrorNode* errors = new ErrorNode;

            member_access->owner = expression;
            if (ErrorNode* error_in_owner = get_node_if(member_access->owner, ErrorNode)) {
                member_access->owner = nullptr;
                errors->appendError(error_in_owner);
            }

            if (!match(TokenType::IDENTIFIER)) {
                errors->appendError(error("expected identifier"));
            } else {
                member_access->member = &next(); 
            }

            if (ErrorNode* errors_found = errors->additional_errors) {
                if (errors_found == nullptr) {
                    delete errors;
                } else {
                    errors->additional_errors = nullptr;
                    delete errors;
                    delete member_access;
                    member_access = (MemberAccess*) errors_found;
                }
            }

            expression = member_access;
        } else if (consumeIf(TokenType::PUNCTUATION_LEFT_PARENTHESIS)) {
            FunctionCall* function_call = new FunctionCall;
            ErrorNode* errors = new ErrorNode;

            function_call->function = expression;
            if (ErrorNode* error_in_function = get_node_if(function_call->function, ErrorNode)) {
                function_call->function = nullptr;
                errors->appendError(error_in_function);
            }

            if (!consumeIf(TokenType::PUNCTUATION_RIGHT_PARENTHESIS)) {
                function_call->arguments = (ExpressionList*) parseExpressionList();

                if (ErrorNode* error_in_arguments = get_node_if(function_call->arguments, ErrorNode)) {
                    function_call->arguments = nullptr;
                    errors->appendError(error_in_arguments);
                }

                if (!consumeIf(TokenType::PUNCTUATION_RIGHT_PARENTHESIS)) {
                    errors->appendError(error("expected `)`"));
                }
            }

            if (ErrorNode* errors_found = errors->additional_errors) {
                if (errors_found == nullptr) {
                    delete errors;
                } else {
                    errors->additional_errors = nullptr;
                    delete errors;
                    delete function_call;
                    function_call = (FunctionCall*) errors_found;
                }
            }

            expression = function_call;
        } else if (consumeIf(TokenType::PUNCTUATION_LEFT_BRACKET)) {
            ArrayAccess* array_access = new ArrayAccess;
            ErrorNode* errors = new ErrorNode;

            array_access->array = expression;
            if (ErrorNode* error_in_array = get_node_if(array_access->array, ErrorNode)) {
                array_access->array = nullptr;
                errors->appendError(error_in_array);
            }

            array_access->index = (Expression*) parseExpression();
            if (ErrorNode* error_in_index = get_node_if(array_access->index, ErrorNode)) {
                array_access->index = nullptr;
                errors->appendError(error_in_index);
            }

            if (!consumeIf(TokenType::PUNCTUATION_RIGHT_BRACKET)) {
                errors->appendError(error("expected `]`"));
            }

            if (ErrorNode* errors_found = errors->additional_errors) {
                if (errors_found == nullptr) {
                    delete errors;
                } else {
                    errors->additional_errors = nullptr;
                    delete errors;
                    delete array_access;
                    array_access = (ArrayAccess*) errors_found;
                }
            }

            expression = array_access;
        } else {
            break; // this is an impossible case
        }
    }

    if (
        match(TokenType::OP_INCREMENT)
        || match(TokenType::OP_DECREMENT)
    ) {
        OperatorToken* _operator = &next();
        expression = newPostfixExpression(_operator, expression);
    }

    return expression;
}

AST* Parser::parsePrimary(void) {
    return tryParse({
            parseGrouping,
            parseLiteral,
            parseNumberConstant,
            parseIdentifierConstant
            // ,parseCastExpression TODO
        }, "expected primary expression");
}

AST* Parser::parseLiteral(void) {
    if (
        !(match(TokenType::LITERAL_BOOLEAN_TRUE)
        || match(TokenType::LITERAL_BOOLEAN_FALSE)
        || match(TokenType::LITERAL_REFERENCE_NULL))
    ) {
        return error("expected literal value");
    }

    LiteralExpression* literal = new LiteralExpression;

    literal->value = &next();

    return literal;
}

AST* Parser::parseNumberConstant(void) {
    if (
        !(match(TokenType::INTEGER_LITERAL)
        || match(TokenType::FLOAT_LITERAL))
    ) {
        return error("expected numeric constant");
    }

    NumberConstant* number = new NumberConstant;

    number->value = &next();

    return number;
}

AST* Parser::parseIdentifierConstant(void) {
    if (!match(TokenType::IDENTIFIER)) {
        return error("expected an identifier");
    }

    IdentifierConstant* identifier = new IdentifierConstant;

    identifier->value = &next();

    return identifier;
}

AST* Parser::parseGrouping(void) {
    if (!consumeIf(TokenType::PUNCTUATION_LEFT_PARENTHESIS)) {
        return error("expected `(`");
    }

    Expression* group = (Expression*) parseExpression();
    if (ErrorNode* error_in_grouping = get_node_if(group, ErrorNode)) {
        if (!consumeIf(TokenType::PUNCTUATION_RIGHT_PARENTHESIS)) {
            error_in_grouping->appendError(error("expected `)`"));
        }
        return error_in_grouping;
    }

    if (!consumeIf(TokenType::PUNCTUATION_RIGHT_PARENTHESIS)) {
        delete group;
        return error("expected `)`");
    }

    return group;
}

AST* Parser::parseExpressionList(void) {
    ExpressionList* expression_list = new ExpressionList;
    ErrorNode* errors = new ErrorNode;

    do {
        Expression* expression = (Expression*) parseExpression();

        if (ErrorNode* error_in_expression = get_node_if(expression, ErrorNode)) {
            errors->appendError(error_in_expression);
        } else {
            expression_list->expressions.push_back(expression);
        }
    } while (!atEnd() && consumeIf(TokenType::PUNCTUATION_COMMA));

    if (ErrorNode* errors_found = errors->additional_errors) {
        if (errors_found == nullptr) {
            delete errors;
        } else {
            errors->additional_errors = nullptr;
            delete errors;
            delete expression_list;
            return errors_found;
        }
    }

    return expression_list;
}

AST* Parser::tryParse(const std::vector<ProductionRule>& rules, const std::string& error_message) {
    const uint32_t current_position = position;

    ErrorNode* potential_error = nullptr;

    for (const ProductionRule& rule : rules) {
        AST* tree = (this->*rule)();

        if (ErrorNode* err = get_node_if(tree, ErrorNode)) {
            if ((err->occurrence > current_position) && (potential_error == nullptr || (err->occurrence > potential_error->occurrence))) {
                potential_error = err;
            }
        } else {
            return tree;
        }

        position = current_position; // reset position to try again
    }

    // All rules failed
    ErrorNode* result_error = (potential_error != nullptr) ? potential_error : error(error_message);
    position = result_error->occurrence;

    return result_error;
}

std::optional<std::unique_ptr<Program>> Parser::getProgramTree() {
    return has_error ? none() : some(std::move(program));
}
