/*
 * Use the last periodic interval timer (PIT3) as the system clock.
 *
 *  $Id$
 */

#include <rtems.h>
#include <bsp.h>
#include <mcf5282/mcf5282.h>

/*
 * Use INTC0 base
 */
#define CLOCK_VECTOR (64+58)

/*
 * Periodic interval timer interrupt handler
 */
#define Clock_driver_support_at_tick()             \
    do {                                           \
        MCF5282_PIT3_PCSR |= MCF5282_PIT_PCSR_PIF; \
    } while (0)                                    \

/*
 * Attach clock interrupt handler
 */
#define Clock_driver_support_install_isr( _new, _old )             \
    do {                                                           \
        _old = (rtems_isr_entry)set_vector(_new, CLOCK_VECTOR, 1);  \
    } while(0)

/*
 * Turn off the clock
 */
#define Clock_driver_support_shutdown_hardware()   \
    do {                                           \
        MCF5282_PIT3_PCSR &= ~MCF5282_PIT_PCSR_EN; \
    } while(0)

/*
 * Set up the clock hardware
 *
 * Prescale so that it counts in microseconds
 * System clock frequency better be 2**n (1<=n<=16) MHz!
 */
#define Clock_driver_support_initialize_hardware()                       \
    do {                                                                 \
        int level;                                                       \
        int preScaleCode = -2;                                           \
        int preScaleDivisor = bsp_get_CPU_clock_speed() / 1000000;       \
        while (preScaleDivisor) {                                        \
            preScaleDivisor >>= 1;                                       \
            preScaleCode++;                                              \
        }                                                                \
        bsp_allocate_interrupt(PIT3_IRQ_LEVEL, PIT3_IRQ_PRIORITY);       \
        MCF5282_INTC0_ICR58 = MCF5282_INTC_ICR_IL(PIT3_IRQ_LEVEL) |      \
                              MCF5282_INTC_ICR_IP(PIT3_IRQ_PRIORITY);    \
        rtems_interrupt_disable( level );                                \
        MCF5282_INTC0_IMRH &= ~MCF5282_INTC_IMRH_INT58;                  \
		MCF5282_PIT3_PCSR &= ~MCF5282_PIT_PCSR_EN;                       \
        rtems_interrupt_enable( level );                                 \
		MCF5282_PIT3_PMR = BSP_Configuration.microseconds_per_tick - 1;  \
		MCF5282_PIT3_PCSR = MCF5282_PIT_PCSR_PRE(preScaleCode) |         \
                            MCF5282_PIT_PCSR_PIE |                       \
                            MCF5282_PIT_PCSR_RLD |                       \
                            MCF5282_PIT_PCSR_EN;                         \
    } while (0)

#include "../../../shared/clockdrv_shell.c"
