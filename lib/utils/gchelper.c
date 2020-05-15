/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>

#include "py/mpstate.h"
#include "py/gc.h"
#include "lib/utils/gchelper.h"

#if MICROPY_ENABLE_GC

// Even if we have specific support for an architecture, it is
// possible to force use of setjmp-based implementation.

#if MICROPY_GCREGS_SETJMP

STATIC void gc_helper_get_regs(gc_helper_regs_t arr) {
    setjmp(arr);
}

// Explicitly mark this as noinline to make sure the regs variable
// is effectively at the top of the stack: otherwise, in builds where
// LTO is enabled and a lot of inlining takes place we risk a stack
// layout where regs is lower on the stack than pointers which have
// just been allocated but not yet marked, and get incorrectly sweeped.
MP_NOINLINE STATIC uintptr_t gc_helper_get_regs_and_sp(uintptr_t * regs) {
    gc_helper_regs_t regs;
    setjmp(regs);
    return (uintptr_t)&regs;
}

#else

#if defined(__x86_64__) || defined(__i386__) || defined(__thumb2__) || defined(__thumb__) || defined(__arm__)

// provided by gchelper_*.s
uintptr_t gc_helper_get_regs_and_sp(uintptr_t *regs);

#else

#error "Architecture not supported for gc_helper_get_regs. Set MICROPY_GCREGS_SETJMP to use the fallback implementation."

#endif

#endif // MICROPY_GCREGS_SETJMP

MP_NOINLINE void gc_helper_collect_regs_and_stack(void) {
    // get the registers and the sp
    gc_helper_regs_t regs;
    uintptr_t sp = gc_helper_get_regs_and_sp(regs);

    // trace the stack, including the registers (since they live on the stack in this function)
    gc_collect_root((void **)sp, ((uint32_t)MP_STATE_THREAD(stack_top) - sp) / sizeof(uint32_t));
}

#endif

