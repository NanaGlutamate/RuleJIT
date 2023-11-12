/**
 * @file bytecodemain.cpp
 * @author djw
 * @brief
 * @date 2023-09-19
 *
 * @details
 *
 * @par history
 * <table>
 * <tr><th>Author</th><th>Date</th><th>Changes</th></tr>
 * <tr><td>djw</td><td>2023-09-19</td><td>Initial version.</td></tr>
 * </table>
 */
#include <chrono>
#include <coroutine>
#include <iostream>

#include "bytecode/bytecode.h"

// struct ReturnObject{
//     struct promise_type{
//         ReturnObject get_return_object() { return {}; }
//         std::suspend_never initial_suspend() { return {}; }
//         std::suspend_never final_suspend() { return {}; }
//         void unhandled_exception() {}
//     };
// };

// struct Awaitable {
//     std::coroutine_handle<> *hp_;
//     constexpr bool await_ready() const noexcept { return false; }
//     void await_suspend(std::coroutine_handle<> h) { *hp_ = h; }
//     void await_resume() noexcept {
//         std::cout << "Event signaled, resuming." << std::endl;
//     }
// };

// ReturnObject counter(std::coroutine_handle<> *continuation_out) {
//     Awaitable a{continuation_out};
//     for (unsigned i = 0;; ++i) {
//         co_await a;
//         std::cout << "counter: " << i << std::endl;
//     }
// }

int fib_cpp(uint64_t n) {
    if (n == 0 || n == 1)
        return 1;
    else
        return fib_cpp(n - 1) + fib_cpp(n - 2);
}

int fib_cpp_tail(uint64_t n, uint64_t need_add) {
    if (n == 0 || n == 1)
        return need_add + 1;
    else
        return fib_cpp_tail(n - 2, fib_cpp_tail(n - 1, need_add));
}

int main() {
    using namespace std;
    using namespace bytecode;
    using namespace dynamicstruct;
    using namespace chrono;

    StructLayoutManager m;
    auto sf1 = m.addDefinition("add2d@frame", StructInfo{{"lhs", "f64"}, {"rhs", "f64"}});
    ByteCodeVM::Context c(m);
    ByteCodeVM vm{c};

    Function add2u{{
                       ByteCode{OPCode::STORE_VAR, DataType::U64, 8},
                       ByteCode{OPCode::STORE_VAR, DataType::U64, 0},
                       ByteCode{OPCode::LOAD_VAR, DataType::U64, 0},
                       ByteCode{OPCode::LOAD_VAR, DataType::U64, 8},
                       ByteCode{OPCode::BIN_OP, DataType::U64, BinaryOP::ADD},
                       ByteCode{OPCode::RET, DataType::NONE, 1},
                   },
                   {sf1, 16},
                   {"add2u"},
                   {0}};
    Function add2f{{
                       ByteCode{OPCode::STORE_VAR, DataType::F64, 8},
                       ByteCode{OPCode::STORE_VAR, DataType::F64, 0},
                       ByteCode{OPCode::LOAD_VAR, DataType::F64, 0},
                       ByteCode{OPCode::LOAD_VAR, DataType::F64, 8},
                       ByteCode{OPCode::BIN_OP, DataType::F64, BinaryOP::ADD},
                       ByteCode{OPCode::RET, DataType::NONE, 1},
                   },
                   {sf1, 16},
                   {"add2f"},
                   {0}};
    Function fib{{
                     ByteCode{OPCode::STORE_VAR, DataType::U64, 0},
                     ByteCode{OPCode::LOAD_IMM, DataType::NONE, 1},
                     ByteCode{OPCode::LOAD_VAR, DataType::U64, 0},
                     ByteCode{OPCode::BIN_OP, DataType::U64, BinaryOP::GE},
                     ByteCode{OPCode::BEZ, DataType::NONE, 7},
                     ByteCode{OPCode::LOAD_IMM, DataType::NONE, 1},
                     ByteCode{OPCode::RET, DataType::NONE, 1},
                     // LABEL:
                     ByteCode{OPCode::LOAD_VAR, DataType::U64, 0},
                     ByteCode{OPCode::LOAD_IMM, DataType::NONE, 1},
                     ByteCode{OPCode::BIN_OP, DataType::U64, BinaryOP::SUB},
                     ByteCode{OPCode::LOAD_CONST, DataType::U64, 0},
                     ByteCode{OPCode::CALL, DataType::NONE, 1},
                     ByteCode{OPCode::LOAD_VAR, DataType::U64, 0},
                     ByteCode{OPCode::LOAD_IMM, DataType::NONE, 2},
                     ByteCode{OPCode::BIN_OP, DataType::U64, BinaryOP::SUB},
                     ByteCode{OPCode::LOAD_CONST, DataType::U64, 0},
                     ByteCode{OPCode::CALL, DataType::NONE, 1},
                     ByteCode{OPCode::BIN_OP, DataType::U64, BinaryOP::ADD},
                     ByteCode{OPCode::RET, DataType::NONE, 1},
                 },
                 {m.addDefinition("fib@frame", StructInfo{{"n", "u64"}}), 8},
                 {"fib"},
                 {0}};
    Function fib2{{
                     ByteCode{OPCode::STORE_VAR, DataType::U64, 0},
                     ByteCode{OPCode::LOAD_IMM, DataType::NONE, 1},
                     ByteCode{OPCode::LOAD_VAR, DataType::U64, 0},
                     ByteCode{OPCode::BIN_OP, DataType::U64, BinaryOP::GE},
                     ByteCode{OPCode::BEZ, DataType::NONE, 3},
                     ByteCode{OPCode::LOAD_IMM, DataType::NONE, 1},
                     ByteCode{OPCode::BIN_OP, DataType::U64, BinaryOP::ADD},
                     ByteCode{OPCode::RET, DataType::NONE, 1},
                     // LABEL:
                     ByteCode{OPCode::LOAD_VAR, DataType::U64, 0},
                     ByteCode{OPCode::LOAD_IMM, DataType::NONE, 1},
                     ByteCode{OPCode::BIN_OP, DataType::U64, BinaryOP::SUB},
                     ByteCode{OPCode::LOAD_CONST, DataType::U64, 0},
                     ByteCode{OPCode::CALL, DataType::NONE, 1},
                     ByteCode{OPCode::LOAD_VAR, DataType::U64, 0},
                     ByteCode{OPCode::LOAD_IMM, DataType::NONE, 2},
                     ByteCode{OPCode::BIN_OP, DataType::U64, BinaryOP::SUB},
                     ByteCode{OPCode::BRANCH, DataType::NONE, -17},
                 },
                 {m.addDefinition("fib2@frame", StructInfo{{"n", "u64"}, {"need_add", "u64"}}), 16},
                 {"fib"},
                 {0}};

    cout << fib2.decompile();

    auto func = vm.getFunc<int(int, int)>(vm.loadFunction(move(add2u)));
    auto func2 = vm.getFunc<double(double, double)>(vm.loadFunction(move(add2f)));
    auto func3 = vm.getFuncUnsafe<uint64_t(uint64_t)>(vm.loadFunction(move(fib)));
    auto func4 = vm.getFunc<uint64_t(uint64_t, uint64_t)>(vm.loadFunction(move(fib2)));
    // cout << func(4, 5).value() << endl;
    // cout << func2(10.2, 0.3).value() << endl;

    constexpr uint64_t input = 20;

    auto start1 = high_resolution_clock::now();
    for(int i = 0; i < 10; ++i)if(fib_cpp_tail(input, 0) != 10946)cout << "err";
    auto end1 = high_resolution_clock::now();
    auto duration1 = duration_cast<nanoseconds>(end1 - start1);

    auto start2 = high_resolution_clock::now();
    for(int i = 0; i < 10; ++i)if(func4(input, 0) != 10946)cout << "err";
    auto end2 = high_resolution_clock::now();
    auto duration2 = duration_cast<nanoseconds>(end2 - start2);

    cout << duration1 / 10 << ": " << duration2 / 10 << endl;
    // seconds

#ifdef __BYTECODE_VM_PROFILING__
    auto& pf = vm.profile["fib"].ins;
    for(int i = 0; i < pf.size(); ++i){
        cout << std::format("{}: {} ns / ins ({} total)", i, pf[i].time.count() / pf[i].count, pf[i].count) << endl;
    }
#endif

    return 0;
}