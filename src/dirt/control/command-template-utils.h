#pragma once

#include <tuple>
#include <utility>
#include "../uni/common.h"

namespace control {
using namespace uni;

template <class... Args>
using StatementStorage = std::tuple<std::remove_cvref_t<Args>...>;

template <class... Args>
struct CommandWrapper {
    template <usize... is>
    static void unwrap (void* function, void* args) {
        auto real_f = *reinterpret_cast<void(**)(Args...)>(&function);
         // If Args... is empty, real_args ends up unused, lol
        auto real_args [[maybe_unused]]
            = reinterpret_cast<StatementStorage<Args...>*>(args);
        real_f(std::get<is>(*real_args)...);
    }
    template <usize... is>
    static constexpr decltype(&unwrap<is...>) get_unwrap (std::index_sequence<is...>) {
        return &unwrap<is...>;
    }
};

} // namespace control
