#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

enum StrTokenType {
    TYPE_PRE = -1000,
    TYPE_ALGO_OPERATOR = 1,
    TYPE_VARIBLE = 2,
    TYPE_CMP_OPERATOR = 3,
    TYPE_CONST = 4,
    TYPE_COMMA = 5,
    TYPE_OTHERS = 6,
    TYPE_STRING = 7,
    TYPE_LEFT_PARENTHESES = 8,
    TYPE_RIGHT_PARENTHESES = 9,
    TYPE_BRANCH = 10,
    TYPE_NUMBER = 11,
    TYPE_OTERNARY = 12,
};

enum OperateType {
    kADD = 1,
    kSUB = 2,
    kMUL = 3,
    kDIV = 4,
    kIN = 5,
    kGREATE = 6,
    kGREATEOREQUAL = 7,
    kLOWER = 8,
    kLOWEROREQUAL = 9,
    kEQUAL = 10,
    kNOTEQUAL = 11,
    kNOT = 12,
    kAND = 13,
    kOR = 14,
    kLEFTPARENTHESIS = 15,   //(
    kRIGHTPARENTHESIS = 16,  //)
    kMOD = 17,
    kFUNC1 = 18,    // 1参数，一个返回值
    kFUNC2 = 19,    // 2参数，一个返回值
    kFUNC3 = 20,    // 3参数，一个返回值
    kFUNC4 = 21,    // 3参数，一个返回值
    kFUNC5 = 22,    // 3参数，一个返回值
    kVEC = 23,      // 一个数组，暂时只支持常量
    kSET = 24,      // 一个集合，暂时只支持常量
    kPOW = 25,      // 2 ^ 3 = 8
    kASIGN = 26,    // 赋值
    kBRANCH = 27,   // 逗号
    kTERNARY = 28,  // 条件运算符
    kCONST = 29,    // 常量
    kVAR = 30,      // 变量
};

enum ValueType {
    vNull = 0,

    vInt = 1,
    vLong = 2,
    vFloat = 3,
    vDouble = 4,
    vBool = 5,

    vInts = 6,
    vLongs = 7,
    vFloats = 8,
    vDoubles = 9,
    vBools = 10,

    vString = 11,
    vStrings = 12,
    vUserType = 13,
    vUserTypes = 14,

    vSetInt = 15,
    vSetLong = 16,
    vSetFloat = 17,
    vSetDouble = 18,
    vSetBool = 19,

    vSetString = 20,
};

enum TokenType {
    tOperator = 1,
    tConst = 2,
    tVarible = 3,
    tLeftParenthesis = 4,
    tInnerFunction = 100000,       // 占用10万个空位
    tFunctionBeginIndex = 200000,  //占用10万个空位
};

enum NodeLevel { nLeaf = 1, nAllChildIsLeaf = 2, nEtherChildIsLeaf = 3, nNoneChildIsLeaf = 4 };

enum MemType { mm_Const = 1, mm_Var = 2, mm_Alloc = 3 };

inline ValueType get_value_type(const std::type_info& ti) {
    ValueType vt = vNull;
    if (ti == typeid(int32_t)) {
        vt = vInt;
    } else if (ti == typeid(int64_t)) {
        vt = vLong;
    } else if (ti == typeid(float)) {
        vt = vFloat;
    } else if (ti == typeid(double)) {
        vt = vDouble;
    } else if (ti == typeid(bool)) {
        vt = vBool;
    } else if (ti == typeid(std::vector<int32_t>)) {
        vt = vInts;
    } else if (ti == typeid(std::vector<int64_t>)) {
        vt = vLongs;
    } else if (ti == typeid(std::vector<float>)) {
        vt = vFloats;
    } else if (ti == typeid(std::vector<double>)) {
        vt = vDoubles;
    } else if (ti == typeid(std::vector<bool>)) {
        vt = vBools;
    } else if (ti == typeid(std::string)) {
        vt = vString;
    } else if (ti == typeid(std::vector<std::string>)) {
        vt = vStrings;
    } else if (ti == typeid(std::unordered_set<int32_t>)) {
        vt = vSetInt;
    } else if (ti == typeid(std::unordered_set<int64_t>)) {
        vt = vSetLong;
    } else if (ti == typeid(std::unordered_set<float>)) {
        vt = vSetFloat;
    } else if (ti == typeid(std::unordered_set<double>)) {
        vt = vSetDouble;
    } else if (ti == typeid(std::unordered_set<bool>)) {
        vt = vSetBool;
    } else if (ti == typeid(std::unordered_set<std::string>)) {
        vt = vSetString;
    }
    return vt;
}
