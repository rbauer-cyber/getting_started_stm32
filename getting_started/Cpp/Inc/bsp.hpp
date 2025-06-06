//============================================================================
// Blinky example
// Last Updated for Version: 7.3.0
// Date of the Last Update:  2023-08-24
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef BSP_HPP_
#define BSP_HPP_

#include <cstdint>

namespace BSP {

extern "C" {
volatile uint16_t QF_getSysAppEvent();
volatile void QF_clearSysAppEvent();
} // extern "C"

constexpr std::uint32_t TICKS_PER_SEC {2000U};

void init();
void start();
int bspMain();
void terminate(std::int16_t const result);
void ledOn(void);
void ledOff(void);

} // namespace BSP

#endif // BSP_HPP_

