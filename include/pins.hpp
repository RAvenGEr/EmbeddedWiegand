#ifndef EMBEDDED_WIEGAND_PINS_HPP
#define EMBEDDED_WIEGAND_PINS_HPP
/**
 * @file
 * @author David Webb (ravenger@dpwlabs.com)
 * @brief Pin Abstraction for EmbeddedWiegand
 * 
 * @copyright Copyright (c) 2023
 */

#define EMBEDDED_WIEGAND_MBED 1

namespace embedded_wiegand {

#if EMBEDDED_WIEGAND_MBED

#include <mbed.h>

/** An IO pin with open drain output and input read capabilities */
class Pin
{
public:
    Pin(PinName name) : pin_{name, PIN_INPUT, OpenDrainNoPull, 1} {}

    /** Set pin to input mode */
    void input() { pin_.input(); }

    /** Set pin to output open drain mode */
    void output()
    {
        pin_.output();
        pin_.mode(OpenDrainNoPull);
        pin_.write(1);
    }

    /** Set pin to low level (driven) */
    void set_low() { pin_.write(0); }

    /** Set pin to high level (pulled-up by bus) */
    void set_high() { pin_.write(1); }

    int read() const { pin_.re }

private:
    DigitalInOut pin_;
};

#endif

} // namespace embedded_wiegand

#endif // EMBEDDED_WIEGAND_PINS_HPP
