#pragma once
#include <string>
#include "enum.h"

ValueType get_value_type(const std::string& token);
bool paser_const_value(ValueType& vt, char* ptr, const std::string& token);
const char* get_last_pointer_from_number_str(const char* str);
bool is_number(char c, StrTokenType pre_type);
StrTokenType get_token_type(char c, StrTokenType pre_type);
bool op_left_assoc(const std::string& op);
int op_preced(const std::string& op);
size_t op_arg_count(const std::string& op);
std::string next_token(const char* input, const char** next_token, StrTokenType& cur_type,
                       const StrTokenType& pre_type);
bool is_terminate(char c);
bool skip_char(char c);
bool is_parentheses(char c);
bool is_const(const std::string& token);
bool is_operator(const std::string& token);
bool is_varible(const std::string& token);
bool is_function(const std::string& token);
bool is_inner_function(const std::string& token);

OperateType get_operator(const std::string& token);

std::string to_string(OperateType op);
