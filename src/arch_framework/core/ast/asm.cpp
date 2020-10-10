#include <functional>
#include <stdio.h>
#include <cmath>
#include "asm.h"
#include "op.h"

template <typename OP>
bool add_instruction(AstNode& node, asmjit::x86::Compiler& cc, OP& op) {
    auto n0 = node.nodes[0];
    auto n1 = node.nodes[1];
    if (not op.support(n0->value_type, n1->value_type)) {
        std::cout << "not support:" << n0->value_type << ", " << n1->value_type << "\n";
        return false;
    }
    node.value_type = op.result_type(n0->value_type, n1->value_type);
    auto ptr = op.fun(n0->value_type, n1->value_type);
    asmjit::x86::Gp arg1 = cc.newUInt64("arg1");
    cc.mov(arg1, n0->data_offset);
    cc.add(arg1, n0->data_pointer);
    asmjit::x86::Gp arg2 = cc.newUInt64("arg2");
    cc.mov(arg2, n1->data_offset);
    cc.add(arg2, n1->data_pointer);
    asmjit::x86::Gp arg3 = cc.newUInt64("arg3");
    cc.mov(arg3, node.data_offset);
    cc.add(arg3, node.data_pointer);

    asmjit::InvokeNode* n;
    cc.invoke(&n, asmjit::imm((void*)ptr),
              asmjit::FuncSignatureT<void, int64_t, int64_t, int64_t>(asmjit::CallConv::kIdFastCall));
    n->setArg(0, arg1);
    n->setArg(1, arg2);
    n->setArg(2, arg3);
    return true;
}

struct power {
    template <typename T, typename H>
    constexpr auto operator()(const T& lhs, const H& rhs) const {
        return std::pow((double)lhs, (double)rhs);
    }
};

struct modulus {
    template <typename T, typename H>
    constexpr auto operator()(const T& lhs, const H& rhs) const {
        int64_t l = lhs;
        int64_t r = rhs;
        return l % r;
    }
};

SimpleObjBinaryOp<std::plus<>> plus;
SimpleObjBinaryOp<std::minus<>> minus;
SimpleObjBinaryOp<std::multiplies<>> multiplies;
SimpleObjBinaryOp<std::divides<>> divides;
SimpleObjBinaryOp<modulus> _modulus;
SimpleObjBinaryOp<power> _power;
SimpleObjBinaryOp<std::equal_to<>> equal_to;
SimpleObjBinaryOp<std::not_equal_to<>> not_equal_to;
SimpleObjBinaryOp<std::greater<>> greater;
SimpleObjBinaryOp<std::less<>> less;
SimpleObjBinaryOp<std::greater_equal<>> greater_equal;
SimpleObjBinaryOp<std::less_equal<>> less_equal;
SimpleObjBinaryOp<std::logical_and<>> logical_and;
SimpleObjBinaryOp<std::logical_or<>> logical_or;

// 集合运算
SetObjBinaryOp in_op;

// 字符串集合运算
StringSetObjBinaryOp string_in_op;
AssignOp assign_op;

bool load_instruction_op(AstNode& node, asmjit::x86::Compiler& cc) {
    // SimpleObjBinaryOp<std::logical_not<>> logical_not;
    switch (node.operator_type) {
        case kCONST: {
            // ToDo: 把常量放到临时变量区
            node.value_type = get_value_type(node.result_varible);
        }; break;
        case kVAR: {
            // no op
        }; break;
        case kASIGN: {
            if (not add_instruction(node, cc, assign_op)) {
                return false;
            }
        }; break;
        case kADD: {
            if (not add_instruction(node, cc, plus)) {
                return false;
            }
        }; break;
        case kSUB: {
            if (not add_instruction(node, cc, minus)) {
                return false;
            }
        }; break;
        case kMUL: {
            if (not add_instruction(node, cc, multiplies)) {
                return false;
            }
        }; break;
        case kDIV: {
            if (not add_instruction(node, cc, divides)) {
                return false;
            }
        }; break;
        case kEQUAL: {
            if (not add_instruction(node, cc, equal_to)) {
                return false;
            }
        }; break;
        case kNOTEQUAL: {
            if (not add_instruction(node, cc, not_equal_to)) {
                return false;
            }
        }; break;
        case kGREATE: {
            if (not add_instruction(node, cc, greater)) {
                return false;
            }
        }; break;
        case kLOWER: {
            if (not add_instruction(node, cc, less)) {
                return false;
            }
        }; break;
        case kGREATEOREQUAL: {
            if (not add_instruction(node, cc, greater_equal)) {
                return false;
            }
        }; break;
        case kLOWEROREQUAL: {
            if (not add_instruction(node, cc, less_equal)) {
                return false;
            }
        }; break;
        case kIN: {
            auto v0 = node.nodes[0]->value_type;
            auto v1 = node.nodes[1]->value_type;
            if (v0 == vString or v0 == vStrings) {
                if (not add_instruction(node, cc, string_in_op)) {
                    return false;
                }
            } else if (not add_instruction(node, cc, in_op)) {
                return false;
            }
        }; break;
        case kPOW: {
            if (not add_instruction(node, cc, _power)) {
                return false;
            }
        }; break;
        case kMOD: {
            if (not add_instruction(node, cc, _modulus)) {
                return false;
            }
        }; break;
        default:
            return false;
            break;
    };
    return true;
}
