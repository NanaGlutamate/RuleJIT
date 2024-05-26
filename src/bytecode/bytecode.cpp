#include <chrono>
#include <expected>

#include "bytecode.h"

namespace {

using namespace bytecode;

/**
 * @brief cast p1 into real type, and use p1 as real type as param1,
 * 64bit type that *p1 can trans to as param2 to call functor f
 *
 * @tparam Functor
 * @param p1
 * @param type
 * @param f
 */
template <typename Functor>
ByteCodeVM::Error dataTypeVisit(uint8_t* p1, DataType::Type type, Functor f) {
    switch (type) {
    case DataType::I8:
        f(reinterpret_cast<int8_t*>(p1), int64_t{});
        break;
    case DataType::I16:
        f(reinterpret_cast<int16_t*>(p1), int64_t{});
        break;
    case DataType::I32:
        f(reinterpret_cast<int32_t*>(p1), int64_t{});
        break;
    case DataType::I64:
        f(reinterpret_cast<int64_t*>(p1), int64_t{});
        break;
    case DataType::U8:
        f(reinterpret_cast<uint8_t*>(p1), uint64_t{});
        break;
    case DataType::U16:
        f(reinterpret_cast<uint16_t*>(p1), uint64_t{});
        break;
    case DataType::U32:
        f(reinterpret_cast<uint32_t*>(p1), uint64_t{});
        break;
    case DataType::U64:
        f(reinterpret_cast<uint64_t*>(p1), uint64_t{});
        break;
    case DataType::F32:
        f(reinterpret_cast<float_t*>(p1), double_t{});
        break;
    case DataType::F64:
        f(reinterpret_cast<double_t*>(p1), double_t{});
        break;
    default:
        return ByteCodeVM::Error::ILLEGAL_OP;
    }
    return ByteCodeVM::Error::NONE;
}

template <typename Functor>
ByteCodeVM::Error registerTypeVisit(uint64_t p1, DataType::Type type, Functor f) {
    switch (type) {
    case DataType::I64:
        f(std::bit_cast<int64_t>(p1));
        break;
    case DataType::U64:
        f(std::bit_cast<uint64_t>(p1));
        break;
    case DataType::F64:
        f(std::bit_cast<double_t>(p1));
        break;
    default:
        return ByteCodeVM::Error::ILLEGAL_OP;
    }
    return ByteCodeVM::Error::NONE;
}

} // namespace

namespace bytecode {

std::string Function::decompile() const {
    std::string ret = symbol[0] + ":\n    symbol: ";
    for (auto& sym : symbol) {
        ret += std::format("\"{}\", ", sym);
    }
    ret += "\n    constant: ";
    for (size_t i = 0; i < constTableInfo.compileTimeConstant.size() * 4; ++i) {
        ret += std::format("{:#x} ", reinterpret_cast<const uint8_t*>(constTableInfo.compileTimeConstant.data())[i]);
    }
    ret += std::format("\n    stack frame size: {} byte\n\n", varTableInfo.stackFrameType->getLayout().size);
    for (size_t i = 0; i < code.size(); i++) {
        auto opCode = code[i].opCode;
        auto opCodeName = OPCode(opCode).getName();
        auto typeName = code[i].type == DataType::NONE ? "-" : DataType(code[i].type).getName();
        auto data = code[i].data;
        std::string attachedInfo;
        if (opCode == OPCode::LOAD_VAR || opCode == OPCode::STORE_VAR || opCode == OPCode::VAR_ADDRESS) {
            if (varTableInfo.stackFrameType) {
                auto def = varTableInfo.stackFrameType->getDefines();
                auto diff = data;
                for (auto& [name, _] : def) {
                    auto diff_this = varTableInfo.stackFrameType->getOffset(name);
                    if (diff_this > diff) {
                        attachedInfo = std::format("({} + {})", name, diff_this - diff);
                        break;
                    } else if (diff_this == diff) {
                        attachedInfo = std::format("({})", name);
                        break;
                    }
                }
            }
        } else if (opCode == OPCode::BIN_OP) {
            attachedInfo = std::format("({})", BinaryOP(data).getName());
        } else if (opCode == OPCode::UNARY_OP) {
            attachedInfo = std::format("({})", UnaryOP(data).getName());
        } else if (opCode == OPCode::SYSCALL) {
            attachedInfo = std::format("({})", SystemCall(data).getName());
        }else if (opCode == OPCode::TYPE_TRANS) {
            attachedInfo = std::format("({})", DataType(data).getName());
        } 
        std::string dataStr;
        if (opCode == OPCode::LOAD_IMM) {
            dataStr = std::format("{:#010x}", data);
        } else if (opCode == OPCode::COPY || opCode == OPCode::LOAD_VAR || opCode == OPCode::LOAD_CONST ||
                   opCode == OPCode::LOAD_MEM || opCode == OPCode::STORE_VAR || opCode == OPCode::STORE_MEM ||
                   opCode == OPCode::VAR_ADDRESS || opCode == OPCode::CONST_ADDRESS || opCode == OPCode::BIN_OP ||
                   opCode == OPCode::UNARY_OP || opCode == OPCode::SYSCALL || opCode == OPCode::BEZ ||
                   opCode == OPCode::CALL || opCode == OPCode::RET) {
            dataStr = std::format("{:>10}", data);
        } else {
            dataStr = "          ";
        }
        ret += std::format(" {:<6} {:12}{:4}  {}{}\n", i, opCodeName, typeName, dataStr, attachedInfo);
    }
    return ret;
}

void ByteCodeVM::load(uint8_t* src, uint64_t* dst, DataType::Type type) {
    err = dataTypeVisit(src, type,
                        [p2{dst}](auto p1, auto p3) { *p2 = std::bit_cast<uint64_t>(static_cast<decltype(p3)>(*p1)); });
}

void ByteCodeVM::store(uint8_t* dst, uint64_t* src, DataType::Type type) {
    err = dataTypeVisit(dst, type, [p2{src}](auto p1, auto p3) {
        *p1 = static_cast<std::remove_cvref_t<decltype(*p1)>>(std::bit_cast<decltype(p3)>(*p2));
    });
}

void ByteCodeVM::typeTrans(uint64_t* target, DataType::Type type0, DataType::Type type1) {
    auto tmp = dataTypeVisit(nullptr, type0, [target, type1, this](auto, auto arg_typed1) {
        auto err1 = dataTypeVisit(nullptr, type1, [target](auto, auto arg_typed2) {
            *target = std::bit_cast<uint64_t>(
                static_cast<decltype(arg_typed2)>(std::bit_cast<decltype(arg_typed1)>(*target)));
        });
        if (err1 != ByteCodeVM::Error::NONE) {
            this->err = ByteCodeVM::Error::ILLEGAL_OP;
        }
    });
    if (tmp != Error::NONE) {
        err = tmp;
    }
}

uint64_t ByteCodeVM::applyUnaryop(uint64_t lhs, DataType::Type type, UnaryOP::Type op) {
    switch (op) {
    case UnaryOP::NEG:
        err = registerTypeVisit(lhs, type, [&lhs](auto a) {
            if constexpr (std::is_same_v<decltype(a), uint64_t>) {
                lhs = std::bit_cast<uint64_t>(-std::bit_cast<int64_t>(a));
            } else {
                lhs = std::bit_cast<uint64_t>(-a);
            }
        });
    case UnaryOP::NOT:
        lhs = !lhs;
        break;
    default:
        err = Error::ILLEGAL_OP;
        break;
    }
    return lhs;
}

uint64_t ByteCodeVM::applyBinop(uint64_t lhs, uint64_t rhs, DataType::Type type, BinaryOP::Type op) {
    switch (op) {
    case BinaryOP::ADD:
        err = registerTypeVisit(
            rhs, type, [&lhs](auto rhs) { lhs = std::bit_cast<uint64_t>(std::bit_cast<decltype(rhs)>(lhs) + rhs); });
        break;
    case BinaryOP::SUB:
        err = registerTypeVisit(
            rhs, type, [&lhs](auto rhs) { lhs = std::bit_cast<uint64_t>(std::bit_cast<decltype(rhs)>(lhs) - rhs); });
        break;
    case BinaryOP::MUL:
        err = registerTypeVisit(
            rhs, type, [&lhs](auto rhs) { lhs = std::bit_cast<uint64_t>(std::bit_cast<decltype(rhs)>(lhs) * rhs); });
        break;
    case BinaryOP::DIV:
        err = registerTypeVisit(rhs, type, [&lhs, this](auto rhs) {
            if (rhs == 0) {
                this->err = Error::DIVIDED_ZERO;
                return;
            }
            lhs = std::bit_cast<uint64_t>(std::bit_cast<decltype(rhs)>(lhs) / rhs);
        });
        break;
    case BinaryOP::MOD:
        err = registerTypeVisit(rhs, type, [&lhs, this](auto rhs) {
            if constexpr (std::is_same_v<decltype(rhs), double_t>) {
                this->err = Error::ILLEGAL_OP;
            } else {
                lhs = std::bit_cast<uint64_t>(std::bit_cast<decltype(rhs)>(lhs) % rhs);
            }
        });
        break;
    case BinaryOP::AND:
        lhs = lhs && rhs;
        break;
    case BinaryOP::OR:
        lhs = lhs || rhs;
        break;
    case BinaryOP::BAND:
        err = registerTypeVisit(rhs, type, [&lhs, this](auto rhs) {
            if constexpr (std::is_same_v<decltype(rhs), double_t>) {
                this->err = Error::ILLEGAL_OP;
            } else {
                lhs = std::bit_cast<uint64_t>(std::bit_cast<decltype(rhs)>(lhs) & rhs);
            }
        });
        break;
    case BinaryOP::BOR:
        err = registerTypeVisit(rhs, type, [&lhs, this](auto rhs) {
            if constexpr (std::is_same_v<decltype(rhs), double_t>) {
                this->err = Error::ILLEGAL_OP;
            } else {
                lhs = std::bit_cast<uint64_t>(std::bit_cast<decltype(rhs)>(lhs) | rhs);
            }
        });
        break;
    case BinaryOP::BXOR:
        err = registerTypeVisit(rhs, type, [&lhs, this](auto rhs) {
            if constexpr (std::is_same_v<decltype(rhs), double_t>) {
                this->err = Error::ILLEGAL_OP;
            } else {
                lhs = std::bit_cast<uint64_t>(std::bit_cast<decltype(rhs)>(lhs) ^ rhs);
            }
        });
        break;
    case BinaryOP::GE:
        err = registerTypeVisit(
            rhs, type, [&lhs](auto rhs) { lhs = static_cast<uint64_t>(std::bit_cast<decltype(rhs)>(lhs) >= rhs); });
        break;
    case BinaryOP::GT:
        err = registerTypeVisit(
            rhs, type, [&lhs](auto rhs) { lhs = static_cast<uint64_t>(std::bit_cast<decltype(rhs)>(lhs) > rhs); });
        break;
    case BinaryOP::EQU:
        err = registerTypeVisit(
            rhs, type, [&lhs](auto rhs) { lhs = static_cast<uint64_t>(std::bit_cast<decltype(rhs)>(lhs) == rhs); });
        break;
    default:
        err = Error::ILLEGAL_OP;
        break;
    }
    return lhs;
}

int ByteCodeVM::execute() {
    while (functionStack.size()) {
        // load function context
        auto& thisFunc = functionStack.back();
#ifdef __BYTECODE_VM_PROFILING__
        auto& func_profile = profile[thisFunc.func->symbol[0]];
        func_profile.ins.resize(thisFunc.func->code.size());
        auto time_function_start = std::chrono::high_resolution_clock::now();
#endif
        auto& code = thisFunc.func->code;
        const uint8_t* constTable = reinterpret_cast<const uint8_t*>(thisFunc.func->constant.data());
        uint8_t* varTable = reinterpret_cast<uint8_t*>(thisFunc.varTable.data());
        auto ip = thisFunc.ip;
        bool same_function = true;
        while (same_function) {
            // execute byte code
            if (ip >= code.size()) {
                err = Error::CODE_OVERFLOW;
                return -1;
            }
#ifdef __BYTECODE_VM_PROFILING__
            auto this_ip = ip;
            auto time_instruction_start = std::chrono::high_resolution_clock::now();
#endif
            auto [opCode, type0, data] = code[ip];
            ip++;
            switch (static_cast<OPCode::Type>(opCode)) {
            case OPCode::NOP: { // NOP - - - () -> ()
                break;
            }
            case OPCode::COPY: { // COPY - - LENGTH (data[n]) -> (data[n], data[n])
                DETECT_OVERFLOW(data);
                auto size = registerStack.size();
                for (auto i = size - data; i < size; ++i) {
                    registerStack.push_back(registerStack[i]);
                }
                break;
            }
            case OPCode::LOAD_IMM: { // LOAD_IMM - - IMM () -> (data)
                registerStack.push_back(static_cast<uint64_t>(data));
                break;
            }
            case OPCode::LOAD_VAR: { // LOAD_VAR VAR_TYPE - BYTE_DIFF () -> (data)
                registerStack.push_back(0);
                load(varTable + data, &registerStack.back(), static_cast<DataType::Type>(type0));
                break;
            }
            case OPCode::LOAD_CONST: { // LOAD_CONST VAR_TYPE - BYTE_DIFF () -> (data)
                registerStack.push_back(0);
                load(const_cast<uint8_t*>(constTable) + data, &registerStack.back(),
                     static_cast<DataType::Type>(type0));
                break;
            }
            case OPCode::LOAD_MEM: { // LOAD_MEM VAR_TYPE - BYTE_DIFF (addr) -> (addr, data)
                DETECT_OVERFLOW(1);
                uint8_t* p = std::bit_cast<uint8_t*>(static_cast<size_t>(registerStack.back()));
                registerStack.push_back(0);
                load(p + data, &registerStack.back(), static_cast<DataType::Type>(type0));
                break;
            }
            case OPCode::STORE_VAR: { // STORE_VAR VAR_TYPE - BYTE_DIFF (data) -> ()
                DETECT_OVERFLOW(1);
                store(varTable + data, &registerStack.back(), static_cast<DataType::Type>(type0));
                registerStack.pop_back();
                break;
            }
            case OPCode::STORE_MEM: { // STORE_MEM VAR_TYPE - BYTE_DIFF (addr, data) -> (addr)
                DETECT_OVERFLOW(2);
                size_t back = registerStack.back();
                registerStack.pop_back();
                store(std::bit_cast<uint8_t*>(static_cast<size_t>(registerStack.back())), &back,
                      static_cast<DataType::Type>(type0));
                break;
            }
            case OPCode::VAR_ADDRESS: { // VAR_ADDRESS VAR_TYPE - BYTE_DIFF () -> (addr)
                registerStack.push_back(static_cast<uint64_t>(reinterpret_cast<size_t>(varTable + data)));
                break;
            }
            case OPCode::CONST_ADDRESS: { // CONST_ADDRESS VAR_TYPE - BYTE_DIFF () -> (addr)
                registerStack.push_back(static_cast<uint64_t>(reinterpret_cast<size_t>(constTable + data)));
                break;
            }
            case OPCode::TYPE_TRANS: { // TYPE_TRANS VAR_TYPE0 VAR_TYPE1 - (data) -> (data); from type0 to type1
                DETECT_OVERFLOW(1);
                typeTrans(&registerStack.back(), static_cast<DataType::Type>(type0),
                          static_cast<DataType::Type>(data));
                break;
            }
            case OPCode::BIN_OP: { // BIN_OP TYPE0 - TYPE (data, data) -> (data)
                DETECT_OVERFLOW(2);
                uint64_t back = registerStack.back();
                registerStack.pop_back();
                registerStack.back() = applyBinop(registerStack.back(), back, static_cast<DataType::Type>(type0),
                                                  static_cast<BinaryOP::Type>(data));
                break;
            }
            case OPCode::UNARY_OP: { // UNARY TYPE0 - TYPE (data) -> (data)
                DETECT_OVERFLOW(1);
                registerStack.back() = applyUnaryop(registerStack.back(), static_cast<DataType::Type>(type0),
                                                    static_cast<UnaryOP::Type>(data));
                break;
            }
            case OPCode::SYSCALL: {
                if (data >= SystemCall::__END) {
                    err = Error::ILLEGAL_OP;
                    break;
                }
                DETECT_OVERFLOW(SystemCallParamCount[data]);
                switch (static_cast<SystemCall::Type>(data)) {
                // PUT_C, PUT_S, IN_C, IN_S, IN_C_NB, GET_ATTR, STACK_SIZE, LINK_SYM, LOAD_IP
                // 1, 2, 0, 2, 0, 2, 0, 1, 1
                case SystemCall::Type::PUT_C: { // (c) -> ()
                    output_buffer << static_cast<char>(registerStack.back());
                    registerStack.pop_back();
                    break;
                }
                case SystemCall::Type::PUT_S: { // (ptr, len) -> ()
                    auto len = registerStack.back();
                    registerStack.pop_back();
                    auto p = std::bit_cast<char*>(registerStack.back());
                    registerStack.pop_back();
                    output_buffer << std::string_view{p, len};
                    break;
                }
                case SystemCall::Type::IN_C: { // () -> (c)
                    char c;
                    input_buffer >> c;
                    registerStack.push_back(static_cast<uint64_t>(c));
                    break;
                }
                case SystemCall::Type::IN_S: { // (ptr, len) -> ()
                    auto len = registerStack.back();
                    registerStack.pop_back();
                    auto p = std::bit_cast<char*>(registerStack.back());
                    registerStack.pop_back();
                    input_buffer.get(p, len);
                    break;
                }
                case SystemCall::Type::IN_C_NB: { // () -> (c)
                    err = Error::ILLEGAL_OP;
                    break;
                }
                case SystemCall::Type::GET_ATTR: { // (ptr, sym) -> (ptr, ptr)
                    auto id = registerStack.back();
                    registerStack.pop_back();
                    auto p = std::bit_cast<uint64_t*>(registerStack.back());
                    // TODO: attr
                    break;
                }
                case SystemCall::Type::STACK_SIZE: { // () -> (l); len before push
                    registerStack.push_back(registerStack.size() - functionStack.back().stack_bottom);
                    break;
                }
                case SystemCall::Type::LINK_SYM: { // (sym) -> (id)
                    auto& sym = functionStack.back().func->symbol[registerStack.back()];
                    auto it = context.symbolTable.find(sym);
                    if (it == context.symbolTable.end()) {
                        registerStack.back() = -1;
                    } else {
                        registerStack.back() = it->second;
                    }
                    break;
                }
                case SystemCall::Type::LOAD_IP: { // (data) -> ()
                    ip = registerStack.back();
                    registerStack.pop_back();
                    break;
                }
#ifdef __BYTECODE_VM_SAFTY_CHECK__
                default: {
                    err = Error::ILLEGAL_OP;
                    break;
                }
#endif
                }
                break;
            }
            case OPCode::BEZ: { // BEZ - - TARGET (data) -> ()
                DETECT_OVERFLOW(1);
                if (registerStack.back() == 0) {
                    // TODO: size independent
                    ip += std::bit_cast<ByteCode::TypedData>(data);
                }
                registerStack.pop_back();
                break;
            }
            case OPCode::BNZ: { // BEZ - - TARGET (data) -> ()
                DETECT_OVERFLOW(1);
                if (registerStack.back() != 0) {
                    ip += std::bit_cast<ByteCode::TypedData>(data);
                }
                registerStack.pop_back();
                break;
            }
            case OPCode::BRANCH: { // BRANCH - - TARGET () -> ()
                ip += std::bit_cast<ByteCode::TypedData>(data);
                break;
            }
            case OPCode::CALL: { // CALL - - ARG_CNT (data[cnt], func) -> (), (data[cnt])
                DETECT_OVERFLOW(data + 1);
                uint64_t id = registerStack.back();
                if (id > context.functions.size()) {
                    err = Error::TABLE_OVERFLOW;
                    break;
                }
                thisFunc.ip = ip;
                same_function = false;
                registerStack.pop_back();
                functionStack.push_back(FunctionStackFrame(context.functions[id], registerStack.size() - data));
                break;
            }
            case OPCode::RET: { // RET - - ARG_CNT (), (data[...], data[cnt]) -> (data[cnt])
                DETECT_OVERFLOW(data);
                registerStack.erase(registerStack.begin() + thisFunc.stack_bottom, registerStack.end() - data);
                same_function = false;
                functionStack.pop_back();
                break;
            }
            case OPCode::POP: { // POP - - - (data) -> ()
                DETECT_OVERFLOW(1);
                registerStack.pop_back();
                break;
            }
#ifdef __BYTECODE_VM_SAFTY_CHECK__
            default: {
                err = Error::ILLEGAL_OP;
                break;
            }
#endif
            }
            if (err != Error::NONE) {
                thisFunc.ip = ip;
                return -1;
            }
#ifdef __BYTECODE_VM_PROFILING__
            func_profile.ins[this_ip].time += std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now() - time_instruction_start);
            func_profile.ins[this_ip].count++;
#endif
        }
#ifdef __BYTECODE_VM_PROFILING__
        func_profile.time += std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now() - time_function_start);
        func_profile.count++;
#endif
    }
    return 0;
}

} // namespace bytecode