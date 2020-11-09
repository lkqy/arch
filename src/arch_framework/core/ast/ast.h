#pragma once

#include <queue>
#include <strstream>
#include <iostream>
#include <memory>
#include <functional>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <stack>
#include <asmjit/x86.h>
#include "parse.h"
#include "enum.h"
#include "../reflection.h"

struct AstNode {
    AstNode(){};
    ~AstNode() {
        for (const auto& p : nodes) {
            if (p)
                delete p;
        }
    };
    OperateType operator_type;
    ValueType value_type;
    std::string result_varible;

    //操作数
    std::vector<AstNode*> nodes;

    asmjit::x86::Gp data_pointer;
    size_t data_offset;  // 地址偏移

    void dfs(std::function<void(AstNode&)> visitor) {
        for (auto n : nodes) {
            n->dfs(visitor);
        }
        visitor(*this);
    }
    bool dfs_bool(std::function<bool(AstNode&)> visitor) {
        for (auto n : nodes) {
            auto ret = n->dfs_bool(visitor);
            if (not ret)
                return ret;
        }
        return visitor(*this);
    }

    void print(int depth) {
        for (int i = 0; i < depth; ++i) {
            std::cout << " ";
        }
        std::cout << result_varible << ":" << operator_type << " " << value_type << "\n";
        for (auto n : nodes) {
            n->print(depth + 4);
        }
    }
};

template <typename T>
class Ast {
public:
    Ast() {
        T t;
        for_each_address(t, [&](auto&& field_name, char* address) { _offsets[field_name] = address - (char*)&t; });
        for_each_type(t,
                      [&](auto&& field_name, const std::type_info& ti) { _var_type[field_name] = get_value_type(ti); });
    }
    size_t offset(const AstNode& n) {
        // 返回地址偏移
        return 0;
    }
    std::string error_log() {
        logger.freeze(false);
        return std::string(logger.str(), logger.pcount());
    }

    bool init(const std::string& expression) {
        std::unordered_set<std::string> varible_names;
        std::queue<std::pair<std::string, int>> queue;
        if (not build_visit_queue(expression, queue)) {
            return false;
        }
        size_t q_size = queue.size();
        auto expression_root = build_expression_tree(queue, varible_names);
        if (not expression_root) {
            return false;
        }
        _node = expression_root;
        return true;
    }
    void print() {
        _node->print(0);
    }
    void dfs(std::function<void(AstNode&)> visitor) {
        _node->dfs(visitor);
    }
    bool dfs_bool(std::function<bool(AstNode&)> visitor) {
        return _node->dfs_bool(visitor);
    }
    size_t offset(const std::string& var) {
        return _offsets[var];
    }
    ValueType type(const std::string& var) {
        return _var_type[var];
    }

private:
    bool build_visit_queue(const std::string& expression, std::queue<std::pair<std::string, int>>& queue) {
        const char* input = expression.c_str();
        const char* end = input + strlen(input);
        const char* start = input;

        std::stack<std::pair<std::string, int>> stack;
        std::stack<int> stack_op_count;
        StrTokenType cur_type;
        StrTokenType pre_type = TYPE_PRE;
        int params_number = 0;
        stack_op_count.push(0);
        std::string pre_token = "";
        while (start < end) {
            const char* p = nullptr;
            std::string token = next_token(start, &p, cur_type, pre_type);
            if (token == "in" and pre_token == "not") {
                logger << "not support [ not in ] operation. ";
                return false;
            }
            pre_type = cur_type;
            start = p;
            if (token.empty()) {
                break;
            } else if (is_operator(token)) {
                while (!stack.empty() && is_operator(stack.top().first)) {
                    if ((op_left_assoc(token) && op_preced(token) <= op_preced(stack.top().first)) ||
                        (!op_left_assoc(token) && op_preced(token) < op_preced(stack.top().first))) {
                        queue.push(stack.top());
                        stack.pop();
                        params_number = stack_op_count.top();
                        stack_op_count.pop();
                    } else {
                        break;
                    }
                }
                // ToDo: 三目运算太扎眼了
                if (token == "?") {
                    continue;
                }
                stack.push(std::make_pair(token, tOperator));
                stack_op_count.push(0);
            } else if (is_const(token)) {
                queue.push(std::make_pair(token, tConst));
                stack_op_count.top() += 1;
            } else if (is_inner_function(token)) {
                stack_op_count.top() += 1;
                stack.push(std::make_pair(token, tInnerFunction));
                stack_op_count.push(0);
            } else if (is_function(token)) {
                stack_op_count.top() += 1;
                stack.push(std::make_pair(token, tFunctionBeginIndex));
                stack_op_count.push(0);
            } else if (is_varible(token)) {
                queue.push(std::make_pair(token, tVarible));
                stack_op_count.top() += 1;
            } else if (token == ",") {
                bool find = false;
                while (!stack.empty()) {
                    if (stack.top().first == "(") {
                        find = true;
                        break;
                    } else {
                        queue.push(stack.top());
                        stack.pop();
                        params_number = stack_op_count.top();
                        stack_op_count.pop();
                    }
                }
                if (!find) {
                    logger << "function parentheses mismatched. ";
                    return false;
                }
            } else if (token == "(") {
                stack.push(std::make_pair(token, tLeftParenthesis));
                stack_op_count.push(0);
            } else if (token == ")") {
                bool find = false;
                while (!stack.empty()) {
                    if (stack.top().first == "(") {
                        find = true;
                        stack.pop();
                        params_number = stack_op_count.top();
                        stack_op_count.pop();
                        break;
                    } else {
                        queue.push(stack.top());
                        stack.pop();
                        params_number = stack_op_count.top();
                        stack_op_count.pop();
                    }
                }
                if (!find) {
                    logger << "parentheses mismatched. ";
                    return false;
                }
                if (!stack.empty()) {
                    int token_type = stack.top().second;
                    if (token_type >= tInnerFunction || token_type >= tFunctionBeginIndex) {
                        stack.top().second += params_number;
                        queue.push(stack.top());
                        stack.pop();
                        params_number = stack_op_count.top();
                        stack_op_count.pop();
                    } else {
                        // 如果不是函数，操作数要累加1
                        stack_op_count.top() += 1;
                    }
                }
            } else {
                logger << "unknown token [" << token << "]. ";
                return false;
            }
            pre_token = token;
        }
        while (!stack.empty()) {
            if (stack.top().first == "(" || stack.top().first == ")") {
                logger << "parentheses mismatched. ";
                return false;
            }
            queue.push(stack.top());
            stack.pop();
            stack_op_count.pop();
        }
        return true;
    }

    AstNode* build_expression_tree(std::queue<std::pair<std::string, int>>& queue,
                                   std::unordered_set<std::string>& varible_names) {
        std::stack<AstNode*> stack;
        int temporal_varible_index = 0;

        static std::unordered_map<int, OperateType> function_map = {{0, kFUNC0}, {1, kFUNC1}, {2, kFUNC2}, {3, kFUNC3}};

        auto asign_nodes = [&](AstNode* node, int op_count) {
            std::stack<AstNode*> temp_stack;
            for (int i = 0; i < op_count; ++i) {
                temp_stack.push(stack.top());
                stack.pop();
            }
            while (!temp_stack.empty()) {
                node->nodes.push_back(temp_stack.top());
                temp_stack.pop();
            }
            stack.push(node);
        };

        auto process_const = [&](const auto& token) {
            auto node = new AstNode();
            node->result_varible = token;
            node->operator_type = kCONST;
            stack.push(node);
            return true;
        };
        auto process_varible = [&](const auto& token) {
            //如果变量是同一个，就索引
            auto node = new AstNode();
            node->result_varible = token;
            node->operator_type = kVAR;
            varible_names.insert(token);
            stack.push(node);
            return true;
        };
        auto process_operator = [&](const auto& token) {
            size_t count = op_arg_count(token);
            if (stack.size() < count) {
                logger << "operator count is wrong:" << std::to_string(stack.size()) << " vs " << count << ". ";
                return false;
            }
            auto node = new AstNode();
            node->operator_type = get_operator(token);
            //不存结果
            // node->value = std::make_shared<BoolValue>(false);
            asign_nodes(node, count);
            return true;
        };

        auto process_function = [&](auto& token, int param_number) {
            long stack_size = stack.size();
            if (stack_size < param_number) {
                logger << "function params count [" << param_number << "] is not right. ";
                return false;
            }
            auto node = new AstNode();
            node->result_varible = token;
            // ToDo: 解决函数问题
            auto it = function_map.find(param_number);
            if (it == function_map.end()) {
                logger << "function params count [" << param_number << "] is not right. ";
                return false;
            }
            node->operator_type = it->second;
            asign_nodes(node, param_number);
            return true;
        };

        while (!queue.empty()) {
            ++temporal_varible_index;
            const std::pair<std::string, int>& token = queue.front();
            if (token.second == tConst) {
                if (!process_const(token.first)) {
                    logger << "process const failed [" << token.first << "]. ";
                    return nullptr;
                }
            } else if (token.second == tOperator) {
                if (!process_operator(token.first)) {
                    logger << "operator [" << token.first << "] not support. ";
                    return nullptr;
                }
            } else if (token.second == tVarible) {
                if (_offsets.find(token.first) == _offsets.end()) {
                    logger << "missing varible [" << token.first << "]. ";
                    return nullptr;
                }
                if (!process_varible(token.first)) {
                    logger << "invalid varible [" << token.first << "]. ";
                    return nullptr;
                }
            } else if (token.second >= tFunctionBeginIndex) {  //按函数处理
                if (!process_function(token.first, token.second - tFunctionBeginIndex)) {
                    // logger<<"invalid function ["<<token.first<<"]. ";
                    return nullptr;
                }
            } else {
                logger << "error type [" << token.first << "]. ";
                return nullptr;
            }
            queue.pop();
        }
        if (stack.size() == 1) {
            return stack.top();
        }
        std::string elements;
        while (stack.size() > 1) {
            stack.pop();
            elements += "'" + stack.top()->result_varible + "'";
        }

        logger << "dangling elements [" << elements << "]. ";
        return nullptr;
    }

private:
    AstNode* _node;
    std::unordered_map<std::string, size_t> _offsets;
    std::unordered_map<std::string, ValueType> _var_type;

    std::string _expression;
    std::ostrstream logger;
};
