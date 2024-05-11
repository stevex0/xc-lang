/// *==============================================================*
///  analyzer.cpp
/// *==============================================================*

#include "include/analyzer.hpp"

using namespace XC;

Analyzer::Analyzer(const std::unique_ptr<Module>& module)
    : module(module),
      symbol_table(std::make_unique<SymbolTable>()),
      has_error(false) {
    checkSemantics();
}

void Analyzer::checkSemantics(void) {
    // load declarations into symbol table
    // for each declaration 
    //   struct -> check members
    //   function -> check return type, identifier, parameters, statements
    loadSymbols();
    validateStructures();
    validateFunctions();
}

void Analyzer::loadSymbols(void) {
    for (Declaration*& declaration : module->program->declarations) {
        if (Function* function = get_node_if(declaration, Function)) {
            if (!symbol_table->loadFunction(function)) {
                error("function `" + function->name->lexeme + "` is already defined", function->name);
            }
        } else if (Structure* structure = get_node_if(declaration, Structure)) {
            if (!symbol_table->loadStructure(structure)) {
                error("struct `" + structure->name->lexeme + "` already defined", structure->name);
            }
        }
    }
}

void Analyzer::validateStructures(void) {
    const std::vector<Structure*> structures = symbol_table->getAllStructures();

    // for each structure ->
    //   validate members ->
    //      for each member -> validate type and name 
    for (const Structure* structure : structures) {
        validateStructureMember(structure);
    }
}

void Analyzer::validateFunctions(void) {
    const std::vector<Function*> functions = symbol_table->getAllFunctions();

    for (const Function* function : functions) {
        validateFunctionOwner(function);
        validateFunctionReturnType(function);
        validateFunctionParameters(function);
        validateFunctionBody(function);
    }
}

void Analyzer::validateStructureMember(const Structure* structure) {
    std::unordered_set<std::string> member_table;

    for (const VariableDeclarator* member : structure->members->members) {
        const DataType* member_type = member->data_type;
        const IdentifierToken* member_identifier = member->variable_name;

        if (member_type->type_name->type == TokenType::IDENTIFIER) {
            const Structure* symbol = symbol_table->lookupStructure(member_type->type_name->lexeme);
            
            if (symbol == nullptr) {
                error("type `" + member_type->type_name->lexeme + "` is undefined", member_type->type_name);
            } else {
                const bool contains_self = symbol == structure;
                const bool has_non_direct_reference = member_type->is_reference || member_type->dimensions > 0;
                const bool is_self_referencing = contains_self && !has_non_direct_reference;

                if (is_self_referencing) {
                    error("struct `" + structure->name->lexeme + "` contains a self referencing member" , member->variable_name);
                }
            }
        }

        if (member_table.count(member_identifier->lexeme) <= 0) {
            member_table.insert(member_identifier->lexeme);
        } else {
            error("struct `" + structure->name->lexeme + "` has multiple members of `" + member_identifier->lexeme + "`", member_identifier);
        }
    }
}

void Analyzer::validateFunctionOwner(const Function* function) {
    const IdentifierToken* owner = function->owner;

    if (owner == nullptr) {
        return;
    }

    if (symbol_table->lookupStructure(owner->lexeme) == nullptr) {
        error("struct `" + owner->lexeme + "` is undefined", owner);
    }
}

void Analyzer::validateFunctionReturnType(const Function* function) {
    const DataType* return_type = function->return_type;

    // No need to validate return type
    const bool returns_void = return_type == nullptr;
    if (returns_void) {
        return;
    }

    // Check if is a builtin type. We do not need to check if a builtin type is defined
    if (return_type->type_name->type != TokenType::IDENTIFIER) {
        return;
    }

    if (symbol_table->lookupStructure(return_type->type_name->lexeme) == nullptr) {
        error("struct `" + return_type->type_name->lexeme + "` is undefined", return_type->type_name);
    }
}

void Analyzer::validateFunctionParameters(const Function* function) {
    ParameterList* parameters = function->parameters;

    const bool has_parameters = parameters != nullptr;
    if (!has_parameters) {
        return;
    }

    std::unordered_set<std::string> parameter_table;

    const bool has_owner = function->owner != nullptr;
    if (has_owner) {
        parameter_table.insert("self");
    }

    for (const VariableDeclarator* parameter : parameters->parameters) {
        const DataType* parameter_type = parameter->data_type;
        const IdentifierToken* parameter_identifier = parameter->variable_name;

        if (parameter_type->type_name->type == TokenType::IDENTIFIER) {
            if (symbol_table->lookupStructure(parameter_type->type_name->lexeme) == nullptr) {
                error("type `" + parameter_type->type_name->lexeme + " is undefined", parameter_type->type_name);
            }
        }

        if (parameter_table.count(parameter_identifier->lexeme) <= 0) {
            parameter_table.insert(parameter_identifier->lexeme);
        } else {
            error("multiple parameter name of `" + parameter_identifier->lexeme + "`", parameter_identifier);
        }
    }
}

void Analyzer::validateFunctionBody(const Function* function) {
    const BlockStatement* body = function->body;

    SymbolStack symbol_stack;

    symbol_stack.pushStack(function);

    pushParametersToStack(symbol_stack, function);

    validateBlockStatement(symbol_stack, body);

    symbol_stack.popStack();
}

DataType* Analyzer::getTypeOfExpression(SymbolStack& symbols, Expression* expression) {
    if (expression == nullptr) {
        return nullptr;
    }

    if (expression->evaluated_type != nullptr) {
        return expression->evaluated_type;
    }

    if (PrefixUnaryExpression* prefix = get_node_if(expression, PrefixUnaryExpression)) {
        expression->evaluated_type = getTypeOfPrefixExpression(symbols, prefix);
    }
    else if (PostfixUnaryExpression* postfix = get_node_if(expression,  PostfixUnaryExpression)) {
        expression->evaluated_type = getTypeOfPostfixExpression(symbols, postfix);
    }
    else if (BinaryExpression* binary = get_node_if(expression, BinaryExpression)) {
        expression->evaluated_type = getTypeOfBinaryExpression(symbols, binary);
    }
    else if (NumberConstant* number = get_node_if(expression, NumberConstant)) {
        expression->evaluated_type = getTypeOfNumberExpression(number);
    }
    else if (IdentifierConstant* identifier = get_node_if(expression, IdentifierConstant)) {
        expression->evaluated_type = getTypeOfIdentifier(symbols, identifier);
    }
    else if (LiteralExpression* literal = get_node_if(expression, LiteralExpression)) {
        expression->evaluated_type = getTypeOfLiteral(literal);
    } 
    else if (MemberAccess* member_access = get_node_if(expression, MemberAccess)) {
        expression->evaluated_type = getTypeOfMemberAccess(symbols, member_access);
    }
    else if (FunctionCall* function_call = get_node_if(expression, FunctionCall)) {
        expression->evaluated_type = getTypeOfFunctionCall(symbols, function_call);
    }

    return expression->evaluated_type;
}

void Analyzer::validateBlockStatement(SymbolStack& stack, const BlockStatement* block) {
    for (const Statement* statement : block->statements) {
        validateStatement(stack, statement);
    }
}

void Analyzer::validateStatement(SymbolStack& stack, const Statement* statement) {
    if (const VariableDeclarationStatement* variable_declaration = get_node_if(statement, VariableDeclarationStatement)) {
        const DataType* variable_type = variable_declaration->declarator->data_type;
        const IdentifierToken* variable_name = variable_declaration->declarator->variable_name;
        const Expression* initial_value = variable_declaration->initial;

        // check type
        if (variable_type->type_name->type == TokenType::IDENTIFIER) {
            const Structure* symbol = symbol_table->lookupStructure(variable_type->type_name->lexeme);

            if (symbol == nullptr) {
                error("type `" + variable_type->type_name->lexeme + "` is undefined", variable_type->type_name);
            }
        }

        // check name (not used already)
        if (stack.lookupSymbol(variable_name->lexeme) != nullptr) {
            error("variable name of `" + variable_name->lexeme + "` is already defined", variable_name);
        } else {
            stack.addSymbol(variable_name->lexeme, variable_type);
        }

        if (initial_value != nullptr) {
            const DataType* initial_value_type = getTypeOfExpression(stack, (Expression*) initial_value);

            if (initial_value_type == nullptr) {
                error("could not assign initial value", variable_name);
                return;
            }

            if (!isSameType(variable_type, initial_value_type)) {
                error("`" + variable_name->lexeme + "` was declared as `" + variable_type->type_name->lexeme + "` but was initialize as `" + initial_value_type->type_name->lexeme + "`", variable_name);
            }
        }
    }
    else if (const ExpressionStatement* expression_statement = get_node_if(statement, ExpressionStatement)) {
        getTypeOfExpression(stack, expression_statement->expression);
    }
    else if (const WhileIteration* while_iteration = get_node_if(statement, WhileIteration)) {
        if (!isBooleanType(getTypeOfExpression(stack, while_iteration->condition))) {
            error("while condition should evaluate to be bool", nullptr);
        }

        stack.pushStack(while_iteration);

        validateBlockStatement(stack, while_iteration->body);

        stack.popStack();
    }
    else if (const ForIteration* for_iteration = get_node_if(statement, ForIteration)) {
        const VariableDeclarationStatement* initial = for_iteration->initial;
        const Expression* condition = for_iteration->condition;
        const Expression* update = for_iteration->update;
        const BlockStatement* body = for_iteration->body;

        if (initial != nullptr) {
            validateStatement(stack, initial);
        }

        if (condition != nullptr && !isBooleanType(getTypeOfExpression(stack, (Expression*) condition))) {
            error("for loop condition must evaluate to a bool", nullptr);
        }

        if (update != nullptr) {
            getTypeOfExpression(stack, (Expression*) update);
        }

        validateBlockStatement(stack, body);
    }
    else if (const ConditionalStatement* conditional = get_node_if(statement, ConditionalStatement)) {
        validateConditionalStatement(stack, conditional);
    }
    else if (const ReturnStatement* return_statement = get_node_if(statement, ReturnStatement)) {
        // get function return type 
        const Function* function = getParentFunctionFromStack(stack);

        if (function == nullptr) {
            error("could not determine function return type", nullptr);
        }

        if (return_statement->expression == nullptr && function->return_type == nullptr) {
            return;
        }

        if (return_statement->expression == nullptr && function->return_type != nullptr) {
            error("expected a return value", nullptr);
        }

        if (return_statement->expression != nullptr && function->return_type == nullptr) {
            error("given a return value when the function should not return anything", nullptr);
        }

        const DataType* return_value_type = getTypeOfExpression(stack, return_statement->expression);

        if (!isSameType(return_value_type, function->return_type)) {
            error("mismatch in return type", nullptr);
        }
    }
    else if (node_is(statement, ContinueStatement)) {
        if (!withinLoop(stack)) {
            error("`continue` statement must be within a loop", nullptr);
        }
    }
    else if (node_is(statement, BreakStatement)) {
        if (!withinLoop(stack)) {
            error("`break` statement must be within a loop", nullptr);
        }
    }
}

DataType* Analyzer::getTypeOfPrefixExpression(SymbolStack& symbols, PrefixUnaryExpression* expression) {
    const OperatorToken* operation = expression->operation;
    const Expression* operand = expression->operand;

    switch (operation->type) {
        case TokenType::OP_INCREMENT:
        case TokenType::OP_DECREMENT: {
            if (const IdentifierConstant* identifier = get_node_if(operand, IdentifierConstant)) {
                if (const DataType* type = symbols.lookupSymbol(identifier->value->lexeme); isIntegerType(type) && type->dimensions == 0) {
                    return copyDataType(getTypeOfExpression(symbols, (Expression*) identifier));
                }
            }
            return error("invalid operand for prefix `" + operation->lexeme + "`", operation);
        }
        case TokenType::BOOLEAN_OP_NOT: {
            const DataType* operand_type = getTypeOfExpression(symbols, (Expression*) operand);

            if (operand_type != nullptr && operand_type->type_name->lexeme == "bool" && operand_type->dimensions == 0) {
                return copyDataType(operand_type);
            }

            return error("invalid operand for prefix `" + operation->lexeme + "`", operation);
        }
        case TokenType::BITWISE_OP_COMPLEMENT: {
            const DataType* operand_type = getTypeOfExpression(symbols, (Expression*) operand);

            if (isIntegerType(operand_type) && operand_type->dimensions == 0) {
                return copyDataType(operand_type);
            }

            return error("invalid operand for prefix `" + operation->lexeme + "`", operation);
        }
        case TokenType::ARITHMETIC_OP_SUB: {
            if (const NumberConstant* number = get_node_if(operand, NumberConstant)) {
                return copyDataType(getTypeOfExpression(symbols, (Expression*) number));
            }

            return error("invalid operand for prefix `" + operation->lexeme + "`", operation);
        }
        case TokenType::BITWISE_OP_AND: {
            // operand must be a variable or member access
            if (!node_is(operand, IdentifierConstant) && !node_is(operand, MemberAccess)) {
                return error("cannot get the reference", operation);
            }

            const DataType* operand_type = getTypeOfExpression(symbols, (Expression*) operand);
            
            if (operand_type == nullptr) {
                return error("could not infer type", operation);
            }

            if (operand_type->is_reference || operand_type->dimensions > 0) {
                return error("cannot get the reference", operation);
            }

            DataType* with_ref = copyDataType(operand_type);
            with_ref->is_reference = true;

            return with_ref;


        }
        default: return nullptr;
    }
}

DataType* Analyzer::getTypeOfPostfixExpression(SymbolStack& symbols, PostfixUnaryExpression* expression) {
    if (const IdentifierConstant* identifier = get_node_if(expression->operand, IdentifierConstant)) {
        if (const DataType* type = symbols.lookupSymbol(identifier->value->lexeme); isIntegerType(type) && type->dimensions == 0) {
            return copyDataType(getTypeOfExpression(symbols, (Expression*) identifier));
        }
    }

    return error("invalid operand for postfix `" + expression->operation->lexeme + "`", expression->operation);
}

DataType* Analyzer::getTypeOfBinaryExpression(SymbolStack& symbols, BinaryExpression* expression) {
    const OperatorToken* operation = expression->operation;
    const Expression* left_operand = expression->left_operand;
    const Expression* right_operand = expression->right_operand;
    const DataType* left_type = getTypeOfExpression(symbols, (Expression*) left_operand);
    const DataType* right_type = getTypeOfExpression(symbols, (Expression*) right_operand);

    if (left_type == nullptr || right_type == nullptr) {
        return nullptr;
    }

    switch (operation->type) {
        case TokenType::ARITHMETIC_OP_ADD:
        case TokenType::ARITHMETIC_OP_SUB:
        case TokenType::ARITHMETIC_OP_MUL:
        case TokenType::ARITHMETIC_OP_DIV: {
            if (isIntegerType(left_type) && isIntegerType(right_type)) {
                return copyDataType(left_type);
            } else if (isIntegerType(left_type) && isFloatingPointType(right_type)) {
                return copyDataType(right_type);
            } else if (isFloatingPointType(left_type) && isIntegerType(right_type)) {
                return copyDataType(left_type);
            } else if (isFloatingPointType(left_type) && isFloatingPointType(right_type)) {
                return copyDataType(left_type);
            } else {
                return error("no support for `" + operation->lexeme + "` operation between `" + left_type->type_name->lexeme + "` and `" + right_type->type_name->lexeme + "`", operation);
            }
        }
        case TokenType::ARITHMETIC_OP_MOD:
        case TokenType::BITWISE_OP_AND:
        case TokenType::BITWISE_OP_OR:
        case TokenType::BITWISE_OP_XOR:
        case TokenType::BITWISE_OP_LEFT_SHIFT:
        case TokenType::BITWISE_OP_RIGHT_SHIFT: {
            if (isIntegerType(left_type) && isIntegerType(right_type)) {
                return copyDataType(left_type);
            }
            
            // both operands must be int types
            return error("no support for `" + operation->lexeme + "` operation between `" + left_type->type_name->lexeme + "` and `" + right_type->type_name->lexeme + "`", operation);
        }
        case TokenType::RELATIONAL_OP_EQUALITY:
        case TokenType::RELATIONAL_OP_INEQUALITY: {
            if (isBooleanType(left_type) && isBooleanType(right_type)) {
                return copyDataType(left_type);
            }

            if (
                (isIntegerType(left_type) && isIntegerType(right_type))
                || (isIntegerType(left_type) && isFloatingPointType(right_type))
                || (isFloatingPointType(left_type) && isIntegerType(right_type))
                || (isFloatingPointType(left_type) && isFloatingPointType(right_type))
            ) {
                
                DataType* type = new DataType;

                type->dimensions = 0;
                type->is_reference = false;
                
                Token* bool_type = new Token;

                bool_type->lexeme = "bool";
                bool_type->column = operation->column;
                bool_type->line = operation->line;
                bool_type->index = operation->index;
                bool_type->type = TokenType::TYPE_BOOL;

                type->type_name = bool_type;

                return type;

            }

            return error("no support for `" + operation->lexeme + "` operation between `" + left_type->type_name->lexeme + "` and `" + right_type->type_name->lexeme + "`", operation);
        }

        case TokenType::RELATIONAL_OP_LESS_THAN:
        case TokenType::RELATIONAL_OP_LESS_THAN_EQUAL:
        case TokenType::RELATIONAL_OP_GREATER_THAN:
        case TokenType::RELATIONAL_OP_GREATER_THAN_EQUAL: {
            if (
                (isIntegerType(left_type) && isIntegerType(right_type))
                || (isIntegerType(left_type) && isFloatingPointType(right_type))
                || (isFloatingPointType(left_type) && isIntegerType(right_type))
                || (isFloatingPointType(left_type) && isFloatingPointType(right_type))
            ) {
                DataType* type = new DataType;

                type->dimensions = 0;
                type->is_reference = false;
                
                Token* bool_type = new Token;

                bool_type->lexeme = "bool";
                bool_type->column = operation->column;
                bool_type->line = operation->line;
                bool_type->index = operation->index;
                bool_type->type = TokenType::TYPE_BOOL;

                type->type_name = bool_type;

                return type;
            }

            return error("no support for `" + operation->lexeme + "` operation between `" + left_type->type_name->lexeme + "` and `" + right_type->type_name->lexeme + "`", operation);
        }

        case TokenType::BOOLEAN_OP_AND:
        case TokenType::BOOLEAN_OP_OR:
        case TokenType::BOOLEAN_OP_XOR: {
            if (isBooleanType(left_type) && isBooleanType(right_type)) {
                return copyDataType(left_type);
            }

            return error("no support for `" + operation->lexeme + "` operation between `" + left_type->type_name->lexeme + "` and `" + right_type->type_name->lexeme + "`", operation);
        }
        case TokenType::ASSIGNMENT_ASSIGN: {
            // left must be a variable
            // right must be the same type
            if (left_operand->type() != ASTType::IdentifierConstant && left_operand->type() != ASTType::MemberAccess) {
                return error("left operand must be assignable", operation);
            }

            if (!isSameType(left_type, right_type)) {
                return error("could not assign `" + left_type->type_name->lexeme + "` to `" + right_type->type_name->lexeme + "`", operation);
            }


            return copyDataType(left_type);
        }
        case TokenType::ASSIGNMENT_OP_ADD:
        case TokenType::ASSIGNMENT_OP_SUB:
        case TokenType::ASSIGNMENT_OP_MUL:
        case TokenType::ASSIGNMENT_OP_DIV: {
            // left must a variable of type int or float
            // right must be a int or float type
            if (left_operand->type() != ASTType::IdentifierConstant && left_operand->type() != ASTType::MemberAccess) {
                return error("left operand must be assignable", operation);
            }

            if (!isIntegerType(left_type) && !isFloatingPointType(left_type)) {
                return error("left operand must be either an integer type or floating point type", operation);
            }

            if (!isIntegerType(right_type) && !isFloatingPointType(right_type)) {
                return error("right operand must be either an integer type or floating point type", operation);
            }

            return copyDataType(left_type);
        }
        case TokenType::ASSIGNMENT_OP_MOD:
        case TokenType::ASSIGNMENT_OP_AND:
        case TokenType::ASSIGNMENT_OP_OR:
        case TokenType::ASSIGNMENT_OP_XOR:
        case TokenType::ASSIGNMENT_OP_LEFT_SHIFT:
        case TokenType::ASSIGNMENT_OP_RIGHT_SHIFT: {
            // left must be a variable with int type
            // right must be a int type
            if (left_operand->type() != ASTType::IdentifierConstant && left_operand->type() != ASTType::MemberAccess) {
                return error("left operand must be assignable", operation);
            }

            if (!isIntegerType(left_type)) {
                return error("left operand must be an integer type", operation);
            }

            if (!isIntegerType(right_type)) {
                return error("right operand must be either an integer type", operation);
            }

            return copyDataType(left_type);
        }

        default: return nullptr;
    }
}

DataType* Analyzer::getTypeOfNumberExpression(NumberConstant* number) {
    switch (number->value->type) {
        case TokenType::INTEGER_LITERAL: {
            DataType* type = new DataType;

            type->dimensions = 0;
            type->is_reference = false;
            
            Token* number_type = new Token;

            number_type->lexeme = "int";
            number_type->column = number->value->column;
            number_type->line = number->value->line;
            number_type->index = number->value->index;
            number_type->type = TokenType::TYPE_INT;

            type->type_name = number_type;

            return type;
        }
        case TokenType::FLOAT_LITERAL: {
            DataType* type = new DataType;

            type->dimensions = 0;
            type->is_reference = false;
            
            Token* number_type = new Token;

            number_type->lexeme = "float";
            number_type->column = number->value->column;
            number_type->line = number->value->line;
            number_type->index = number->value->index;
            number_type->type = TokenType::TYPE_FLOAT;

            type->type_name = number_type;

            return type;
        }
        default: return nullptr;
    }
}

DataType* Analyzer::getTypeOfIdentifier(SymbolStack& symbols, IdentifierConstant* identifier) {
    if (const DataType* type = symbols.lookupSymbol(identifier->value->lexeme)) {
        return copyDataType(type);
    }

    return error("`" + identifier->value->lexeme + "` is undefined", identifier->value);
}

DataType* Analyzer::getTypeOfLiteral(LiteralExpression* literal) {
    switch (literal->value->type) {
        case TokenType::LITERAL_BOOLEAN_TRUE:
        case TokenType::LITERAL_BOOLEAN_FALSE: {
            DataType* type = new DataType;

            type->dimensions = 0;
            type->is_reference = false;
            
            Token* bool_type = new Token;

            bool_type->lexeme = "bool";
            bool_type->column = literal->value->column;
            bool_type->line = literal->value->line;
            bool_type->index = literal->value->index;
            bool_type->type = TokenType::TYPE_BOOL;

            type->type_name = bool_type;

            return type;
        }
        case TokenType::LITERAL_REFERENCE_NULL: {
            DataType* type = new DataType;

            type->dimensions = 0;
            type->is_reference = true;
            
            Token* reference_type = new Token;

            reference_type->lexeme = "null";
            reference_type->column = literal->value->column;
            reference_type->line = literal->value->line;
            reference_type->index = literal->value->index;
            reference_type->type = TokenType::LITERAL_REFERENCE_NULL;

            type->type_name = reference_type;

            return type;
        }

        default: return nullptr;
    }
}

DataType* Analyzer::getTypeOfMemberAccess(SymbolStack& symbols, MemberAccess* member_access) {
    const Expression* owner = member_access->owner;
    const IdentifierToken* member = member_access->member;

    const DataType* owner_type = getTypeOfExpression(symbols, (Expression*) owner);

    if (owner_type == nullptr) {
        return error("could not determine what `" + member->lexeme + "` is", member);
    }

    const Structure* structure = symbol_table->lookupStructure(owner_type->type_name->lexeme);
    const StructureMembers* members = structure->members;
    if (members == nullptr) {
        return error("member `" + member->lexeme + "` does not exist", member);
    }

    for (const VariableDeclarator* variable_declarator : members->members) {
        if (variable_declarator->variable_name->lexeme == member->lexeme) {
            return copyDataType(variable_declarator->data_type);
        }
    }

    return error("struct `" + structure->name->lexeme + "` does not have a member `" + member->lexeme + "`", member);
}

DataType* Analyzer::getTypeOfFunctionCall(SymbolStack& symbols, FunctionCall* function_call) {
    if (const IdentifierConstant* identifier = get_node_if(function_call->function, IdentifierConstant)) {
        const Function* function = symbol_table->lookupFunction(identifier->value->lexeme);

        if (function == nullptr) {
            return error("`" + identifier->value->lexeme + "()` is undefined", identifier->value);
        }

        if (function->owner != nullptr) {
            return error("`" + function->owner->lexeme + "::" + function->name->lexeme + "()` cannot be called here", identifier->value);
        }

        const uint32_t args_count = function_call->arguments == nullptr ? 0 : function_call->arguments->expressions.size();
        const uint32_t require_args = function->parameters == nullptr ? 0 : function->parameters->parameters.size();

        if (args_count != require_args) {
            return error("`" + function->name->lexeme + "()` requires " + std::to_string(require_args) + " arguments but were given " + std::to_string(args_count), identifier->value);
        }

        for (uint32_t i = 0; i < args_count; ++i) {
            const DataType* given_type = getTypeOfExpression(symbols, function_call->arguments->expressions.at(i));
            const DataType* require_type = function->parameters->parameters.at(i)->data_type;

            if (!isSameType(given_type, require_type)) {
                return error("invalid arguments", identifier->value);
            }
        }
        
        return copyDataType(function->return_type);
    }
    else if (const MemberAccess* member_access = get_node_if(function_call->function, MemberAccess)) {
        const Expression* operand = member_access->owner;
        const IdentifierToken* member_function = member_access->member;

        const DataType* operand_type = getTypeOfExpression(symbols, (Expression*) operand);
        const Function* function = symbol_table->lookupFunction(member_function->lexeme);

        if (operand_type == nullptr) {
            return error("could not determine where this member function comes from", member_function);
        }

        // function does not exist or function has no owner or the owner has no member function of this name
        if (function == nullptr || function->owner == nullptr || operand_type->type_name->lexeme != function->owner->lexeme) {
            return error("`" + operand_type->type_name->lexeme + "` does not have a member function `" + member_function->lexeme + "()`", member_function);
        }

        if (operand_type->dimensions != 0) {
            return error("array types do not have member functions: " + std::to_string(operand_type->dimensions), member_function);
        }

        const uint32_t args_count = function_call->arguments == nullptr ? 0 : function_call->arguments->expressions.size();
        const uint32_t require_args = function->parameters == nullptr ? 0 : function->parameters->parameters.size();

        if (args_count != require_args) {
            return error("`" + function->name->lexeme + "()` requires " + std::to_string(require_args) + " arguments but were given " + std::to_string(args_count), member_function);
        }

        for (uint32_t i = 0; i < args_count; ++i) {
            const DataType* given_type = getTypeOfExpression(symbols, function_call->arguments->expressions.at(i));
            const DataType* require_type = function->parameters->parameters.at(i)->data_type;

            if (!isSameType(given_type, require_type)) {
                return error("invalid arguments", member_function);
            }
        }
        
        return copyDataType(function->return_type);
    }
    // else if (const ArrayAccess* array_access = get_node_if(function_call->function, ArrayAccess)) {
    //     return error("array access", nullptr);
    // }
    else {
        return error("cannot call function", nullptr);
    }
}

DataType* Analyzer::copyDataType(const DataType* type) {
    if (type == nullptr) {
        return nullptr;
    }

    DataType* copy = new DataType;

    copy->dimensions = type->dimensions;
    copy->is_reference = type->is_reference;

    Token* type_name = new Token;

    type_name->lexeme = type->type_name->lexeme;
    type_name->column = type->type_name->column;
    type_name->line = type->type_name->line;
    type_name->index = type->type_name->index;
    type_name->type = type->type_name->type;

    copy->type_name = type_name;

    return copy;
}

void Analyzer::validateConditionalStatement(SymbolStack& symbols, const ConditionalStatement* conditional) {
    if (conditional == nullptr) {
        return;
    }

    if (!isBooleanType(getTypeOfExpression(symbols, conditional->condition))) {
        error("if condition should evaluate to be bool", nullptr);
    }

    symbols.pushStack(conditional);

    validateBlockStatement(symbols, conditional->body);

    symbols.popStack();

    if (const BlockStatement* else_case = get_node_if(conditional->else_case, BlockStatement)) {
        symbols.pushStack(conditional);

        validateBlockStatement(symbols, else_case);

        symbols.popStack();
    } else {
        validateConditionalStatement(symbols, conditional->else_case);
    }
}

std::nullptr_t Analyzer::error(const std::string& message, const Token* token) {
    has_error = true;

    if (token == nullptr) {
        std::cerr << message << std::endl;
        return nullptr;
    }

    // xc: error: message   |< header
    //  --> file:ln:col     |< info
    //    :                 |< divider
    // ln | content         |< line content
    //    : underline       |< footer

    const std::string header = "xc: \033[31merror\033[0m: " + message + '\n';
    const std::string line_column = std::to_string(token->line + 1) + ':' + std::to_string(token->column + 1);
    const std::string info = " --> " + module->source->filename + ':' + line_column + '\n';

    std::string preview;
    {
        const std::string line_number = std::to_string(token->line + 1);
        const std::string divider = std::string(line_number.size() + 2, ' ') + ':';

        const std::string line_preview = module->source->content.at(token->line); 
        const std::string line_content = " " + line_number + " | " + line_preview;
        const std::string underline = std::string(min_of(token->lexeme.size(), line_preview.size() - token->column), '^');
        const std::string footer = divider + std::string(token->column + 1, ' ') + underline;

        preview = divider + '\n' + line_content + footer;
    }

    std::cerr << header << info << preview << std::endl;

    return nullptr;
}

bool Analyzer::isIntegerType(const DataType* type) {
    if (type == nullptr) {
        return false;
    }

    return type->type_name->type == TokenType::TYPE_BYTE
        || type->type_name->type == TokenType::TYPE_SHORT
        || type->type_name->type == TokenType::TYPE_INT
        || type->type_name->type == TokenType::TYPE_LONG;
}

bool Analyzer::isFloatingPointType(const DataType* type) {
    if (type == nullptr) {
        return false;
    }

    return type->type_name->type == TokenType::TYPE_FLOAT || type->type_name->type == TokenType::TYPE_DOUBLE;
}

bool Analyzer::isBooleanType(const DataType* type) {
        if (type == nullptr) {
        return false;
    }

    return type->type_name->type == TokenType::TYPE_BOOL;
}

bool Analyzer::isSameType(const DataType* type_1, const DataType* type_2) {
    if (type_1 == nullptr && type_2 == nullptr) {
        return true;
    }

    if (type_1 == nullptr || type_2 == nullptr) {
        return false;
    }

    if (type_1->is_reference && type_2->type_name->type == TokenType::LITERAL_REFERENCE_NULL) {
        return true;
    }

    return type_1->dimensions == type_2->dimensions 
        && type_1->is_reference == type_2->is_reference
        && type_1->type_name->lexeme == type_2->type_name->lexeme;
}

bool Analyzer::withinLoop(SymbolStack& stack) {
    for (const std::pair<const AST*, std::unordered_map<std::string, const DataType*>>& data : stack.stack) {
        const AST* parent = data.first;

        if (node_is(parent, WhileIteration) || node_is(parent, ForIteration)) {
            return true;
        }
    }

    return false;
}

const Function* Analyzer::getParentFunctionFromStack(SymbolStack& stack) {
    for (const std::pair<const AST*, std::unordered_map<std::string, const DataType*>>& data : stack.stack) {
        const AST* parent = data.first;

        if (node_is(parent, Function)) {
            return (const Function*) parent;
        }
    }

    return nullptr;
}

void Analyzer::pushParametersToStack(SymbolStack& stack, const Function* function) {
    if (function == nullptr) {
        return;
    }

    if (function->parameters != nullptr) {
        for (const VariableDeclarator* variables : function->parameters->parameters) {
            stack.addSymbol(variables->variable_name->lexeme, variables->data_type);
        }
    }

    if (function->owner != nullptr) {
        DataType* type = new DataType;

        type->dimensions = 0;
        type->is_reference = true;
        
        Token* ref_type = new Token;

        ref_type->lexeme = function->owner->lexeme;
        ref_type->column = function->owner->column;
        ref_type->line = function->owner->line;
        ref_type->index = function->owner->index;
        ref_type->type = function->owner->type;

        type->type_name = ref_type;

        stack.addSymbol("self", type);
    }
}

std::unique_ptr<SymbolTable> Analyzer::validateSemantics(const std::unique_ptr<Module>& module) {
    Analyzer analyzer(module);
    return analyzer.has_error ? none() : some(std::move(analyzer.symbol_table));
} 