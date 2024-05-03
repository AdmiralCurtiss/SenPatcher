#include <cstdint>
#include <limits>

// Originally from: https://gamedev.stackexchange.com/a/25746
// Public domain.

namespace HyoutaUtils::RNG {
struct xorshift {
    // Simple shift-and-xor random number generator. This
    // implementation provides an optimal period for its storage size,
    // passes most empirical tests, and is faster and smaller than
    // more popular approaches like MT.
    //
    // However, it is insecure. Don't use it as a source of
    // cryptographic randomness.
    //
    // See www.jstatsoft.org/v08/i14/paper for the algorithm, and
    // www.open-std.org/jtc1/sc22/wg21/docs/papers/2003/n1452.html
    // for the structure.

    typedef std::uint32_t result_type;

    struct state_type {
        result_type x;
        result_type y;
        result_type z;
        result_type w;
    };

    static constexpr xorshift::state_type s_default_seed = {123456789,
                                                            362436069,
                                                            521288629,
                                                            88675123};

    xorshift(void) : state_(s_default_seed) {}
    explicit xorshift(result_type r)
      : state_({s_default_seed.x, s_default_seed.y, s_default_seed.z, r}) {}
    explicit xorshift(const state_type& seed) : state_(seed) {}

    void seed(void) {
        state(s_default_seed);
    }
    void seed(result_type r) {
        auto seed = s_default_seed;
        seed.w = r;
        state(seed);
    }
    void seed(const state_type& seed) {
        state(seed);
    }

    // Generate a uniformly-distributed random integer of
    // result_type.
    result_type operator()(void) {
        result_type t = state_.x ^ (state_.x << 15);
        state_.x = state_.y;
        state_.y = state_.z;
        state_.z = state_.w;
        return state_.w = state_.w ^ (state_.w >> 21) ^ (t ^ (t >> 4));
    }

    // Discard the next z random values.
    void discard(size_t z) {
        while (z--)
            (*this)();
    }

    // Get or set the entire state. This can be used to store
    // and later re-load the state in any format.
    const state_type& state(void) const {
        return state_;
    }
    void state(const state_type& state) {
        state_ = state;
    }

    // Random number bounds. Used by random distributions; you
    // probably don't need to call these directly.
    static result_type min(void) {
        return std::numeric_limits<result_type>::min();
    }
    static result_type max(void) {
        return std::numeric_limits<result_type>::max();
    }

private:
    state_type state_;
};

// Two engines compare as equal if their states are
// bitwise-identical, i.e. if they would generate the same
// numbers forever.

inline bool operator==(const xorshift::state_type& lhs, const xorshift::state_type& rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

inline bool operator!=(const xorshift::state_type& lhs, const xorshift::state_type& rhs) {
    return !(lhs == rhs);
}

inline bool operator==(const xorshift& lhs, const xorshift& rhs) {
    return lhs.state() == rhs.state();
}

inline bool operator!=(const xorshift& lhs, const xorshift& rhs) {
    return !(lhs == rhs);
}
} // namespace HyoutaUtils::RNG
