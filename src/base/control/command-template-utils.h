#pragma once

#include <tuple>
#include <utility>
#include "../uni/common.h"

namespace control {

template <class... Args>
using StatementStorage = std::tuple<std::remove_cvref_t<Args>...>;

template <class... Args>
struct CommandWrapper {
    template <usize... is>
    static void unwrap (void* function, void* args) {
        auto real_f = *reinterpret_cast<void(**)(Args...)>(&function);
        auto real_args = reinterpret_cast<StatementStorage<Args...>*>(args);
        real_f(std::get<is>(*real_args)...);
    }
    template <usize... is>
    static CE decltype(&unwrap<is...>) get_unwrap (std::index_sequence<is...>) {
        return &unwrap<is...>;
    }
};

} // namespace control
