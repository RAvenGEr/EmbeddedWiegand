# Embedded Wiegand

Wiegand interface for embedded devices built with C++11 (or later)

This header only library provides a simple to use Wiegand interface for an MCU.

Example usage for mbed

```cpp
#include <mbed.h>

#include <cstddef>

#include "wiegand_interface.hpp"

int main()
{
    using namespace embedded_wiegand;
    Pin pc1{PC_1};
    Pin pc3{PC_3};

    WiegandInterface w{pc1, pc3};

    Ticker t;
    t.attach_us(callback(&w, &WiegandInterface::tick), 80);

    // Send a 26 bit code
    w.send(WiegandInterface::code(1, 101));
    ThisThread::sleep_for(2000);

    // Send a 34 bit code
    w.send(WiegandInterface::code<34>(1, 101), 34);
}

```

pin.h contains an implementation of Pin for the mbed OS platform.

### TODO
 - Receive
