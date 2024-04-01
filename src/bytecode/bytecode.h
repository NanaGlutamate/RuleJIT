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
#include <ranges>
#include <sstream>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

// #define __BYTECODE_VM_PROFILING__
// #define __BYTECODE_VM_SAFTY_CHECK__

#include "dynamicstruct.hpp"

#define READABLE_ENUM(name, ...)                                                                                       \
    struct name {                                                                                                      \
        template <typename Src>                                                                                        \
        constexpr name(Src s) : type(static_cast<Type>(s)) {}                                                          \
        enum Type { __VA_ARGS__, __END } type;                                                                         \
        inline static constexpr std::array<std::string_view, static_cast<size_t>(__END)> names =                       \
            split<static_cast<size_t>(__END)>(#__VA_ARGS__);                                                           \
        constexpr std::string_view getName() const { return names[static_cast<size_t>(type)]; }                        \
    };

#ifdef __BYTECODE_VM_SAFTY_CHECK__

#define DETECT_OVERFLOW(depth)                                                                                         \
    if (visitIllegal((depth))) {                                                                                       \
        err = Error::STACK_OVERFLOW;                                                                                   \
        break;                                                                                                         \
    }

#else

#define DETECT_OVERFLOW(x) ((void)0);

#endif

namespace bytecode {

template <size_t len>
consteval std::array<std::string_view, len> split(std::string_view src) {
    std::array<std::string_view, len> ret;
    size_t pos = 0;
    for (size_t i = 0; i < len; i++) {
        size_t next = src.find_first_of("=, ", pos);
        if (next == std::string_view::npos) {
            ret[i] = src.substr(pos);
            break;
        }
        ret[i] = src.substr(pos, next - pos);
        pos = src.find_first_not_of("=, ", next + 1);
    }
    return ret;
}

READABLE_ENUM(OPCode, NOP, COPY, LOAD_IMM, LOAD_VAR, LOAD_CONST, LOAD_MEM, STORE_VAR, STORE_MEM, VAR_ADDRESS,
              CONST_ADDRESS, TYPE_TRANS, BIN_OP, UNARY_OP, SYSCALL, BEZ, BNZ, BRANCH, CALL, RET, POP);

READABLE_ENUM(BinaryOP, ADD, SUB, MUL, DIV, MOD, AND, OR, BAND, BOR, BXOR, GE, GT, EQU);

READABLE_ENUM(UnaryOP, NEG, NOT);

READABLE_ENUM(SystemCall, PUT_C, PUT_S, IN_C, IN_S, IN_C_NB, GET_ATTR, STACK_SIZE, LINK_SYM, LOAD_IP);

inline constexpr std::array<size_t, SystemCall::__END> SystemCallParamCount = {1, 2, 0, 2, 0, 2, 0, 1, 1};

READABLE_ENUM(DataType, NONE, U8, U16, U32, U64, I8, I16, I32, I64, F32, F64, PTR);

// inline constexpr std::array<size_t, DataType::__END> DataTypeLen = {0, 1, 2, 4, 8, 1, 2, 4, 8, 4, 8, 8};

struct alignas(alignof(int32_t)) ByteCode {
    uint8_t opCode;
    uint8_t type;
    uint16_t data;
    using TypedData = int16_t;
};
static_assert(sizeof(int32_t) == sizeof(ByteCode));

using MemUnit = uint64_t;

struct Function {
    // struct DebugInfo {
    //     // diff -> name, type
    //     std::map<uint32_t, std::tuple<std::string, std::string>> varName;
    // };

    std::vector<ByteCode> code;

    struct VarTableInfo {
        dynamicstruct::ComplexType* stackFrameType;
        size_t byteSize;
        bool escaped;
    } varTableInfo;
    struct ConstTableInfo {
        size_t linkedFunctionCount;
        std::vector<MemUnit> compileTimeConstant;
    } constTableInfo;
    struct RegisterInfo {
        size_t registerStackSizeRequired;
    } registerInfo;

    // [0] = this_function_name
    std::vector<std::string> symbol;

    std::string decompile() const;
};

struct ByteCodeVM {
    struct Context {
        friend struct ByteCodeVM;
        Context(dynamicstruct::StructLayoutManager& typeManager) : typeManager(typeManager) {}
        Context(const Context& c) = delete;
        Context(Context&& c) = delete;

        std::optional<size_t> addFunction(const Function& f) {
            if (auto it = functionIndex.find(f.symbol[0]); it != functionIndex.end()) {
                return std::nullopt;
            }
            if (auto bc = f.code.back();
                bc.opCode != OPCode::RET && bc.opCode != OPCode::CALL && bc.opCode != OPCode::BRANCH) {
                // ill-formed function
                return std::nullopt;
            }
            auto id = functions.size();
            functions.push_back(f);
            functionIndex.emplace(f.symbol[0], id);
            return id;
        }

        std::optional<size_t> linkFunction(size_t functionID) {
            if (auto it = functionIndexInTextSegment.find(functionID); it != functionIndexInTextSegment.end()) {
                return it->second;
            }
            auto ip = textSegment.size();
            functionIndexInTextSegment.emplace(functionID, ip);
            auto& func = functions[functionID];
            textSegment.insert(textSegment.end(), func.code.cbegin(), func.code.cend());
            if (linkedFunctionInfo.size() <= functionID) {
                linkedFunctionInfo.resize(functionID);
            }
            std::vector<MemUnit> constant;
            auto& originalContant = func.constTableInfo.compileTimeConstant;
            constant.reserve(func.constTableInfo.linkedFunctionCount + originalContant.size());
            for (size_t i = 0; i < func.constTableInfo.linkedFunctionCount; ++i) {
                auto it = functionIndex.find(func.symbol[i]);
                // TODO: check?
                if(it == functionIndex.end()) {
                    return std::nullopt;
                }
                auto newID = linkFunction(it->second);
                if(!newID.has_value()) {
                    return std::nullopt;
                }
                constant.push_back(newID.value());
            }
            constant.insert(constant.end(), originalContant.begin(), originalContant.end());
            linkedFunctionInfo[functionID] = {ip, func.registerInfo.registerStackSizeRequired,
                                                         func.varTableInfo.byteSize, func.varTableInfo.escaped,
                                                         std::move(constant)};
            return ip;
        }

      private:
        dynamicstruct::StructLayoutManager& typeManager;

        struct LinkedFunctionInfo {
            size_t ip;
            size_t registerRequired;
            size_t varTableSize;
            bool varTableEscaped;
            std::vector<MemUnit> constant;
        };
        std::vector<LinkedFunctionInfo> linkedFunctionInfo{};

        // load time:
        std::vector<Function> functions{};
        std::unordered_map<std::string, size_t> functionIndex{};
        // link time:
        std::vector<ByteCode> textSegment{};
        std::unordered_map<size_t, size_t> functionIndexInTextSegment{};
    } & context;

    ByteCodeVM(Context& c) : context(c), registerStack(), functionStack(), input_buffer(), output_buffer() {
        registerStack.reserve(1024);
        functionStack.reserve(512);
    }
    struct FunctionStackFrame {
        FunctionStackFrame(Function& targetFunction, size_t stack_bottom)
            : varTable((targetFunction.varTableInfo.byteSize + sizeof(MemUnit) - 1) / sizeof(MemUnit), 0),
              func(&targetFunction), stack_bottom(stack_bottom), ip(0) {}
        const Function* func;
        std::vector<MemUnit> varTable;
        size_t stack_bottom;
        size_t ip;
    };

#ifdef __BYTECODE_VM_PROFILING__
    struct InstructionTime {
        std::chrono::nanoseconds time{0};
        size_t count{0};
    };
    struct FunctionTime {
        std::chrono::nanoseconds time{0};
        size_t count{0};
        std::vector<InstructionTime> ins;
    };
    std::map<std::string, FunctionTime> profile;
#endif

    std::stringstream input_buffer, output_buffer;

    std::vector<MemUnit> registerStack;
    std::vector<FunctionStackFrame> functionStack;
    enum struct Error {
        NONE,
        ILLEGAL_OP,
        CODE_OVERFLOW,
        STACK_OVERFLOW,
        TABLE_OVERFLOW,
        UNKNOWN_SYM,
        DIVIDED_ZERO
    } err = Error::NONE;

    // void run(std::string_view name) {}

    [[deprecated]] size_t loadFunction(const Function& f) { return loadFunction(Function{f}); }
    [[deprecated]] size_t loadFunction(Function&& f) {
        if (auto bc = f.code.back();
            bc.opCode != OPCode::RET && bc.opCode != OPCode::CALL && bc.opCode != OPCode::BRANCH) {
            // ill-formed function
            return -1;
        }
        size_t id = context.functions.size();
        context.symbolTable.emplace(f.symbol[0], id);
        context.functions.push_back(std::move(f));
        auto& realFunc = context.functions.back();
        for (size_t i = 0; i < realFunc.symbol.size(); ++i) {
            // link
            if (auto it = context.symbolTable.find(realFunc.symbol[i]); it != context.symbolTable.end()) {
                context.functions[id].constant[i] = it->second;
            }
        }
        return id;
    }

    // template <typename FuncType>
    // size_t loadFunction(FuncType&& f) {
    //     return 0;
    // }

  private:
    template <typename Ty>
    struct getFuncHelper;

    template <typename Ret, typename... Args>
    struct getFuncHelper<Ret(Args...)> {
        static auto get(ByteCodeVM* base, size_t id) {
            return [base, id](Args... args) -> std::optional<Ret> {
                using namespace std;
                auto bottom = base->registerStack.size();
                bool input = (... && base->pushStack(args));
                if (!input) {
                    return nullopt;
                }
                base->functionStack.emplace_back(base->context.functions[id], bottom);
                if (base->execute()) {
                    return nullopt;
                }
                auto back = base->registerStack.back();
                base->registerStack.pop_back();
                assert(bottom == base->registerStack.size());
                if constexpr (is_same_v<Ret, float_t> || is_same_v<Ret, double_t>) {
                    return static_cast<Ret>(bit_cast<double_t>(back));
                } else if constexpr (is_same_v<Ret, uint8_t> || is_same_v<Ret, uint16_t> || is_same_v<Ret, uint32_t> ||
                                     is_same_v<Ret, uint64_t>) {
                    return static_cast<Ret>(back);
                } else if constexpr (is_same_v<Ret, int8_t> || is_same_v<Ret, int16_t> || is_same_v<Ret, int32_t> ||
                                     is_same_v<Ret, int64_t>) {
                    return static_cast<Ret>(bit_cast<int64_t>(back));
                }
            };
        }
    };

    template <typename Ty>
    struct getFuncUnsafeHelper;

    template <typename Ret, typename... Args>
    struct getFuncUnsafeHelper<Ret(Args...)> {
        static auto get(ByteCodeVM* base, size_t id) {
            return [base, id](Args... args) -> Ret {
                using namespace std;
                bool input = (... && base->pushStack(args));
                base->functionStack.emplace_back(base->context.functions[id], 0);
                base->execute();
                auto back = base->registerStack.back();
                base->registerStack.pop_back();
                if constexpr (is_same_v<Ret, float_t> || is_same_v<Ret, double_t>) {
                    return static_cast<Ret>(bit_cast<double_t>(back));
                } else if constexpr (is_same_v<Ret, uint8_t> || is_same_v<Ret, uint16_t> || is_same_v<Ret, uint32_t> ||
                                     is_same_v<Ret, uint64_t>) {
                    return static_cast<Ret>(back);
                } else if constexpr (is_same_v<Ret, int8_t> || is_same_v<Ret, int16_t> || is_same_v<Ret, int32_t> ||
                                     is_same_v<Ret, int64_t>) {
                    return static_cast<Ret>(bit_cast<int64_t>(back));
                }
            };
        }
    };

  public:
    template <typename Ty>
    auto getFunc(size_t id) {
        return getFuncHelper<Ty>::get(this, id);
    }
    template <typename Ty>
    auto getFuncUnsafe(size_t id) {
        return getFuncUnsafeHelper<Ty>::get(this, id);
    }

  private:
    void load(uint8_t* src, uint64_t* dst, DataType::Type type);
    void store(uint8_t* dst, uint64_t* src, DataType::Type type);
    void typeTrans(uint64_t* target, DataType::Type type0, DataType::Type type1);
    uint64_t applyUnaryop(uint64_t lhs, DataType::Type type, UnaryOP::Type op);
    uint64_t applyBinop(uint64_t lhs, uint64_t rhs, DataType::Type type, BinaryOP::Type op);

    template <typename Ty>
    bool pushStack(Ty arg) {
        using namespace std;
        using namespace dynamicstruct;
        if constexpr (is_same_v<Ty, u8> || is_same_v<Ty, u16> || is_same_v<Ty, u32> || is_same_v<Ty, u64>) {
            registerStack.push_back(arg);
        } else if constexpr (is_same_v<Ty, i8> || is_same_v<Ty, i16> || is_same_v<Ty, i32> || is_same_v<Ty, i64>) {
            registerStack.push_back(bit_cast<uint64_t>(static_cast<int64_t>(arg)));
        } else if constexpr (is_same_v<Ty, f32> || is_same_v<Ty, f64>) {
            registerStack.push_back(bit_cast<uint64_t>(static_cast<double_t>(arg)));
        } else {
            return false;
        }
        return true;
    }
    bool visitIllegal(size_t depth) {
        if (functionStack.empty())
            return registerStack.size() < depth;
        return registerStack.size() - functionStack.back().stack_bottom < depth;
    }
    int execute();
};

} // namespace bytecode