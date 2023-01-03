#ifndef WIEGAND_INTERFACE_HPP
#define WIEGAND_INTERFACE_HPP
/**
 * @file
 * @author David Webb (ravenger@dpwlabs.com)
 * @brief Wiegand Interface
 * 
 * @copyright Copyright (c) 2023
 */

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "pins.hpp"

namespace embedded_wiegand {

/**
 * @brief Wiegand interface for microcontrollers
 * 
 */
class WiegandInterface
{
    /** Default size for wiegand send */
    static constexpr size_t SEND_BITS = 26;

public:
    /**
     * @brief Construct a new Wiegand Interface object
     * 
     * @param d0_pin IO pin for wiegand bus D0
     * @param d1_pin IO pin for wiegand bus D1
     */
    WiegandInterface(Pin& d0_pin, Pin& d1_pin) : d0_pin_{d0_pin}, d1_pin_{d1_pin} {}

    /**
     * @brief Generate on the wire code
     * 
     * @tparam BITS number of bits in the Wiegand message, including parity
     * @param facility 8 bit facility code (0-255)
     * @param id ID portion
     * @return code with parity for sending over the wire
     */
    template<size_t BITS = SEND_BITS, typename T>
    static constexpr auto code(uint8_t facility, T id)
    {
        using R = typename std::conditional<BITS <= 32, uint32_t, uint64_t>::type;

        const R code = (static_cast<R>(facility) << (BITS - 10)) | id;
        return add_parity<BITS>(code);
    }

    /**
     * @brief Add parity bits to a code for sending on the wire
     * 
     * @tparam BITS number of bits in the Wiegand message, including parity
     * @param code The code to send
     * @return code with parity for sending over the wire
     */
    template<size_t BITS = SEND_BITS, typename T>
    static constexpr auto add_parity(T code)
    {
        static_assert(BITS % 2 == 0, "BITS must be even");
        using R = typename std::conditional<BITS <= 32, uint32_t, uint64_t>::type;

        const R code_shifted = code << 1;
        R parity_even = 0;
        R parity_odd = 1;
        for (size_t i = 0; i < (BITS / 2) - 1; ++i) {
            parity_odd ^= code & 1;
            code >>= 1;
        }
        for (size_t i = 0; i < (BITS / 2) - 1; ++i) {
            parity_even ^= code & 1;
            code >>= 1;
        }
        return code_shifted | (parity_even << (BITS - 1)) | parity_odd;
    }

    /**
     * @brief Send data over the Wiegand interface
     * 
     * @param code the code to send
     * @param send_bits the number of bits to send
     * @return true if the code is queued to send
     */
    template<typename T>
    bool send(T code, size_t send_bits = SEND_BITS)
    {
        if (state != IDLE) {
            return false;
        }
        using S = typename std::common_type<T, uint32_t>::type;
        if (send_bits > 32) {
            code_ = std::is_same<S, uint32_t>::value ? 0 : static_cast<uint32_t>(code >> 32);
            code_ext_ = static_cast<uint32_t>(code);
            mask_bit_ = 1 << (send_bits - 1 - 32);
        } else {
            code_ = code;
            mask_bit_ = 1 << (send_bits - 1);
        }
        send_pos_ = 0;
        send_bits_ = send_bits;
        state_ = SENDING;
        return true;
    }

    /**
     * @brief Progress the sending state
     * 
     * @note This function must be called at a fixed interval while sending,
     * the recommended interval is 80 milliseconds. This is safe to call from
     * an interrupt.
     */
    void tick()
    {
        if (state_ != SENDING) {
            return;
        }
        if (send_pos_ == 0) {
            d0_pin_.output();
            d1_pin_.output();
        }
        const size_t tick_state = send_pos_ % 4;
        switch (tick_state) {
        case 0:
            write_bit(code_ & mask_bit_);
            mask_bit_ >>= 1;
            break;
        case 1:
            // Clear bits
            d0_pin_.set_high();
            d1_pin_.set_high();
            if (--send_bits_ == 0) {
                d0_pin_.input();
                d1_pin_.input();
                state_ = IDLE;
                return;
            }
            break;
        default:
            break;
        }
        if (mask_bit_ == 0) {
            mask_bit_ = 1 << 31;
            code_ = code_ext_;
        }
        ++send_pos_;
    }

private:
    enum State
    {
        IDLE,
        RECEIVING,
        SENDING
    };

    State state_{IDLE};

    Pin& d0_pin_;
    Pin& d1_pin_;

    uint32_t code_{0};
    uint32_t code_ext_{0};

    uint32_t mask_bit_{1};

    size_t send_pos_{0};
    size_t send_bits_{26};

    /** Write a single bit over the wire */
    void write_bit(unsigned val)
    {
        if (val == 0) {
            d0_pin_.set_low();
        } else {
            d1_pin_.set_low();
        }
    }
};

} // namespace embedded_wiegand

#endif // WIEGAND_INTERFACE_HPP
