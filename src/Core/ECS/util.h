#ifndef CORE_ECS_UTIL
#define CORE_ECS_UTIL

#include <type_traits>

namespace Engine {
    template <typename... Ts>
    using void_t = void;

    // try at getting something similar to: https://github.com/skypjack/entt/blob/master/src/entt/core/type_info.hpp
    // with limited C++ knowledge
    struct index_counter {
        // returns the current index count and increases it
        static unsigned int next() {
            static unsigned int value{};
            return value++;
        }
    };

    template<typename ComponentType, typename = void>
    struct type_index {
        static unsigned int value() {
            // this assignment is done once setting the index for all Components of the same type
            static const unsigned int value = index_counter::next();
            return value;
        }
    };
}

#endif

