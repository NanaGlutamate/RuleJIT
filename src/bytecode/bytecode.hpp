/**
 * @file bytecode.hpp
 * @author djw
 * @brief
 * @date 2023-09-18
 *
 * @details
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-09-18</td><td>Initial version.</td></tr>
 * </table>
 */
#pragma once

#include <array>
#include <bit>
#include <format>
#include <map>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

#define READABLE_ENUM(name, ...)                                                                                       \
    struct name {                                                                                                      \
        template <typename Src>                                                                                        \
        constexpr name(Src s) : type(static_cast<Type>(s)) {}                                                          \
        enum Type { __VA_ARGS__, __END } type;                                                                         \
        inline static constexpr std::array<std::string_view, static_cast<size_t>(__END)> names =                       \
            split<static_cast<size_t>(__END)>(#__VA_ARGS__);                                                           \
        constexpr std::string_view getName() const { return names[static_cast<size_t>(type)]; }                        \
    };

namespace bytecode {

template <size_t len>
consteval std::array<std::string_view, len> split(std::string_view src) {
    std::array<std::string_view, len> ret;
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        size_t next = src.find_first_of(", ", pos);
        if (next == std::string_view::npos) {
            ret[i] = src.substr(pos);
            break;
        }
        ret[i] = src.substr(pos, next - pos);
        pos = src.find_first_not_of(", ", next + 1);
    }
    return ret;
}

READABLE_ENUM(OPCode, NOP, LOAD_IMM_H, LOAD_IMM_L, LOAD_VAR, LOAD_CONST, LOAD_MEM, STORE_VAR, STORE_MEM, VAR_ADDRESS,
              CONST_ADDRESS, TYPE_TRANS, BIN_OP, UNARY_OP, ALLOC, SYSCALL, BNZ, CALL, RET);

READABLE_ENUM(BinaryOP, ADD, SUB, MUL, DIV, MOD, GE, GT, EQU);

READABLE_ENUM(UnaryOP, NEG, NOT);

READABLE_ENUM(SystemCall, PRINT);

READABLE_ENUM(DataType, NONE, U8, U16, U32, U64, I8, I16, I32, I64, F32, F64, PTR);

inline constexpr std::array<size_t, DataType::__END> DataTypeLen = {0, 1, 2, 4, 8, 1, 2, 4, 8, 4, 8, 8};

struct alignas(alignof(uint64_t)) ByteCode {
    uint16_t opCode;
    uint8_t type0;
    uint8_t type1;
    uint32_t data;
};

using MemUnit = uint64_t;

struct Function {
    struct VarTableInfo {
        uint64_t unitCount;
    };
    struct DebugInfo {
        // diff -> name, size
        std::map<uint32_t, std::tuple<std::string, uint32_t>> varName;
    };
    // struct alignas(alignof(uint64_t)) MemUnit {
    //     uint8_t data[64 / 8];
    // };

    std::string thisSymbol;
    std::vector<std::string> symbol;

    std::vector<ByteCode> code;
    std::vector<MemUnit> constant;
    VarTableInfo varTableInfo;
    std::unique_ptr<DebugInfo> debugInfo;

    std::string decompile() const {
        std::string ret = thisSymbol + ":\n\n";
        for (size_t i = 0; i < code.size(); i++) {
            auto opCode = code[i].opCode;
            auto opCodeName = OPCode(opCode).getName();
            auto type0Name = code[i].type0 == DataType::NONE ? "" : DataType(code[i].type0).getName();
            auto type1Name = code[i].type1 == DataType::NONE ? "" : DataType(code[i].type1).getName();
            auto data = code[i].data;
            std::string attachedInfo;
            if (opCode == OPCode::LOAD_VAR || opCode == OPCode::STORE_VAR || opCode == OPCode::VAR_ADDRESS) {
                if (debugInfo) {
                    auto it = debugInfo->varName.lower_bound(data);
                    attachedInfo = std::format("({})", std::get<0>(*it));
                }
            } else if (opCode == OPCode::BIN_OP) {
                attachedInfo = std::format("({})", BinaryOP(data).getName());
            } else if (opCode == OPCode::UNARY_OP) {
                attachedInfo = std::format("({})", UnaryOP(data).getName());
            } else if (opCode == OPCode::SYSCALL) {
                attachedInfo = std::format("({})", SystemCall(data).getName());
            }
            ret += std::format(" {:<6} {:<12}{:<3}{:<3}{:>12#x}{}\n", i, opCodeName, type0Name, type1Name, data,
                               attachedInfo);
        }
        return ret;
    }
};

struct ByteCodeVM {
    struct FunctionStackFrame {
        FunctionStackFrame(Function& targetFunction)
            : varTable(targetFunction.varTableInfo.unitCount, 0), func(&targetFunction) {}
        const Function* func;
        std::vector<MemUnit> varTable;
        size_t stack_bottom;
        size_t ip;
    };
    std::unordered_map<std::string, Function> symbolTable;
    std::vector<MemUnit> registerStack;
    std::vector<FunctionStackFrame> functionStack;
    enum struct Error { NONE, ILLEGAL_OP } err;

    void run(std::string_view name) {}

  private:
    template <typename Functor>
    void dataTypeVisit(uint8_t* p1, uint64_t* p2, Functor f, DataType::Type type){
        switch (type) {
        case DataType::I8:
            f.trans<int64_t>(reinterpret_cast<int8_t*>(p1), p2);
            break;
        case DataType::I16:
            f.trans<int64_t>(reinterpret_cast<int16_t*>(p1), p2);
            break;
        case DataType::I32:
            f.trans<int64_t>(reinterpret_cast<int32_t*>(p1), p2);
            break;
        case DataType::I64:
            f.trans<int64_t>(reinterpret_cast<int64_t*>(p1), p2);
            break;
        case DataType::U8:
            f.trans<uint64_t>(reinterpret_cast<uint8_t*>(p1), p2);
            break;
        case DataType::U16:
            f.trans<uint64_t>(reinterpret_cast<uint16_t*>(p1), p2);
            break;
        case DataType::U32:
            f.trans<uint64_t>(reinterpret_cast<uint32_t*>(p1), p2);
            break;
        case DataType::U64:
            f.trans<uint64_t>(reinterpret_cast<uint64_t*>(p1), p2);
            break;
        case DataType::F32:
            f.trans<double_t>(reinterpret_cast<float_t*>(p1), p2);
            break;
        case DataType::F64:
            f.trans<double_t>(reinterpret_cast<double_t*>(p1), p2);
            break;
        default:
            err = Error::ILLEGAL_OP;
            break;
        }
    }
    void load(uint8_t* src, uint64_t& dst, DataType::Type type) {
        switch (type) {
        case DataType::I8:
            dst = std::bit_cast<uint64_t>(static_cast<int64_t>(*reinterpret_cast<int8_t*>(src)));
            break;
        case DataType::I16:
            dst = std::bit_cast<uint64_t>(static_cast<int64_t>(*reinterpret_cast<int16_t*>(src)));
            break;
        case DataType::I32:
            dst = std::bit_cast<uint64_t>(static_cast<int64_t>(*reinterpret_cast<int32_t*>(src)));
            break;
        case DataType::I64:
            dst = std::bit_cast<uint64_t>(static_cast<int64_t>(*reinterpret_cast<int64_t*>(src)));
            break;
        case DataType::U8:
            dst = std::bit_cast<uint64_t>(static_cast<uint64_t>(*reinterpret_cast<uint8_t*>(src)));
            break;
        case DataType::U16:
            dst = std::bit_cast<uint64_t>(static_cast<uint64_t>(*reinterpret_cast<uint16_t*>(src)));
            break;
        case DataType::U32:
            dst = std::bit_cast<uint64_t>(static_cast<uint64_t>(*reinterpret_cast<uint32_t*>(src)));
            break;
        case DataType::U64:
            dst = std::bit_cast<uint64_t>(static_cast<uint64_t>(*reinterpret_cast<uint64_t*>(src)));
            break;
        case DataType::F32:
            dst = std::bit_cast<uint64_t>(static_cast<double_t>(*reinterpret_cast<float_t*>(src)));
            break;
        case DataType::F64:
            dst = std::bit_cast<uint64_t>(static_cast<double_t>(*reinterpret_cast<double_t*>(src)));
            break;
        default:
            err = Error::ILLEGAL_OP;
            break;
        }
    }
    void store(uint8_t* dst, uint64_t& src, DataType::Type type) {
        switch (type) {
        case DataType::I8:
            *reinterpret_cast<int8_t*>(dst) = std::bit_cast<int64_t>(src);
            break;
        case DataType::I16:
            *reinterpret_cast<int16_t*>(dst) = std::bit_cast<int64_t>(src);
            break;
        case DataType::I32:
            *reinterpret_cast<int32_t*>(dst) = std::bit_cast<int64_t>(src);
            break;
        case DataType::I64:
            *reinterpret_cast<int64_t*>(dst) = std::bit_cast<int64_t>(src);
            break;
        case DataType::U8:
            *reinterpret_cast<uint8_t*>(dst) = std::bit_cast<uint64_t>(src);
            break;
        case DataType::U16:
            *reinterpret_cast<uint16_t*>(dst) = std::bit_cast<uint64_t>(src);
            break;
        case DataType::U32:
            *reinterpret_cast<uint32_t*>(dst) = std::bit_cast<uint64_t>(src);
            break;
        case DataType::U64:
            *reinterpret_cast<uint64_t*>(dst) = std::bit_cast<uint64_t>(src);
            break;
        case DataType::F32:
            *reinterpret_cast<float_t*>(dst) = std::bit_cast<double_t>(src);
            break;
        case DataType::F64:
            *reinterpret_cast<double_t*>(dst) = std::bit_cast<double_t>(src);
            break;
        default:
            err = Error::ILLEGAL_OP;
            break;
        }
    }
    int execute() {
        while (functionStack.size()) {
            auto& thisFunc = functionStack.back();
            auto& code = thisFunc.func->code;
            uint8_t* varTable = std::launder(reinterpret_cast<uint8_t*>(thisFunc.varTable.data()));
            auto ip = thisFunc.ip;
            while (true) {
                if (ip >= code.size())
                    return -1;
                auto [opCode, type0, type1, data] = code[ip];
                // NOP, LOAD_VAR, LOAD_IMM_H, LOAD_IMM_L, LOAD_MEM, STORE_VAR, TYPE_TRANS, BIN_OP, UNARY_OP, ALLOC,
                // VAR_ADDRESS, BNZ, CALL, RET
                switch (opCode) {
                case OPCode::NOP:
                    break;
                case OPCode::LOAD_VAR: // LOAD_VAR VAR_TYPE - BYTE_DIFF
                    registerStack.push_back(0);
                    load(varTable + data, registerStack.back(), static_cast<DataType::Type>(type0));
                    break;
                case OPCode::LOAD_IMM_H: // LOAD_IMM_H - - IMM
                    registerStack.push_back(static_cast<uint64_t>(data) << 32);
                    break;
                case OPCode::LOAD_IMM_L: // LOAD_IMM_L - - IMM
                    registerStack.push_back(static_cast<uint64_t>(data));
                    break;
                case OPCode::LOAD_MEM: // LOAD_MEM VAR_TYPE - BYTE_DIFF
                    uint8_t* p = std::bit_cast<uint8_t*>(static_cast<size_t>(registerStack.back()));
                    load(p + data, registerStack.back(), static_cast<DataType::Type>(type0));
                    break;
                case OPCode::STORE_VAR: // STORE_VAR VAR_TYPE - BYTE_DIFF
                    store(varTable + data, registerStack.back(), static_cast<DataType::Type>(type0));
                    registerStack.pop_back();
                    break;
                case OPCode::TYPE_TRANS: // TYPE_TRANS VAR_TYPE0 VAR_TYPE1 -

                    break;
                }
                ip++;
            }
        }
    }
};

} // namespace bytecode