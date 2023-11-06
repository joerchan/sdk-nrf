#include "cmsis.h"
#include "uart_stdout.h"
#include "tfm_spm_log.h"
#include "config_tfm.h"
#include "exception_info.h"
#include "tfm_arch.h"

// #if defined(CONFIG_TFM_ALLOW_NON_SECURE_FAULT_HANDLING)
void tfm_hal_system_halt(void)
{
	/*
	 * Disable IRQs to stop all threads, not just the thread that
	 * halted the system.
	 */
	__disable_irq();

	/*
	 * Enter sleep to reduce power consumption and do it in a loop in
	 * case a signal wakes up the CPU.
	 */
	while (1) {
		__WFE();
	}
}

#define SECUREFAULT_EXCEPTION_NUMBER (NVIC_USER_IRQ_OFFSET + SecureFault_IRQn)
#define HARDFAULT_EXCEPTION_NUMBER   (NVIC_USER_IRQ_OFFSET + HardFault_IRQn)
#define BUSFAULT_EXCEPTION_NUMBER    (NVIC_USER_IRQ_OFFSET + BusFault_IRQn)

#define SPUFAULT_EXCEPTION_NUMBER    (NVIC_USER_IRQ_OFFSET + SPU_IRQn)

#if defined(TFM_EXCEPTION_INFO_DUMP) && defined(TRUSTZONE_PRESENT)

__attribute__((naked)) static void handle_fault_from_ns(
	uint32_t fault_handler_fn, uint32_t exc_return) {
	/* scrub secure register state before jumping to ns */
	__ASM volatile(
		"mov  lr, r1 \n"
		"movs r1, #0 \n"
		"movs r2, #0 \n"
#if (CONFIG_TFM_FLOAT_ABI >= 1)
		"vmov d0, r1, r2 \n"
		"vmov d1, r1, r2 \n"
		"vmov d2, r1, r2 \n"
		"vmov d3, r1, r2 \n"
		"vmov d4, r1, r2 \n"
		"vmov d5, r1, r2 \n"
		"vmov d6, r1, r2 \n"
		"vmov d7, r1, r2 \n"
		"mrs r2, control \n"
		"bic r2, r2, #4 \n"
		"msr control, r2 \n"
		"isb \n"
#endif
		"ldr  r1, ="M2S(STACK_SEAL_PATTERN)" \n"
		"push {r1, r2} \n"
		"movs r1, #0 \n"
		"movs r3, #0 \n"
		"movs r4, #0 \n"
		"movs r5, #0 \n"
		"movs r6, #0 \n"
		"movs r7, #0 \n"
		"movs r8, #0 \n"
		"movs r9, #0 \n"
		"movs r10, #0 \n"
		"movs r11, #0 \n"
		"movs r12, #0 \n"
		"bic r0, r0, #1 \n"
		"bxns r0 \n"
	);
}

void tfm_hal_system_reset(void)
{
	struct exception_info_t exc_ctx;
	
	tfm_exception_info_get_context(&exc_ctx);

	const bool exc_ctx_valid = exc_ctx.EXC_RETURN != 0x0;
	const uint8_t active_exception_number = (exc_ctx.xPSR & 0xff);

	/* The goal of this feature is to allow the non-secure to handle the exceptions that are
         * triggered by non-secure but the exception is targeting secure.
         *
         * Banked Exceptions:
         * These exceptions have their individual pending bits and will target the security state
         * that they are taken from.
         * 
         * These exceptions are banked:
         *  - HardFault
         *  - MemManageFault
         *  - UsageFault
         *  - SVCall
         *  - PendSV
         *  - Systick
         * 
         * These exceptions are not banked:
         *  - Reset
         *  - NMI
         *  - BusFault
         *  - SecureFault
         *  - DebugMonitor
         *  - External Interrupt
         * 
         * AICR.PRIS bit:
         * Prioritize Secure interrupts. All secure exceptions take priority over the non-secure
         * TF-M enables this
         * 
         * AICR.BFHFNMINS:
         * Enable BusFault HardFault and NMI to target non-secure.
         * Since HardFault is banked this wil target the security state it is taken from, the others
         * will always target non-secure.
         * The effect of enabling this and PRIS at the same time is UNDEFINED. 
         *
         * External interrupts target security state based on NVIC target state configuration.
	 *
	 */
	const bool securefault_active = (active_exception_number == SECUREFAULT_EXCEPTION_NUMBER);
	const bool busfault_active = (active_exception_number == BUSFAULT_EXCEPTION_NUMBER);
	const bool hardfault_active = (active_exception_number == HARDFAULT_EXCEPTION_NUMBER);
	const bool spufault_active = (active_exception_number == SPUFAULT_EXCEPTION_NUMBER);

	SPMLOG_ERRMSGVAL("Active exception number", active_exception_number);

	if (!exc_ctx_valid ||
	    is_return_secure_stack(exc_ctx.EXC_RETURN) ||
	    !(securefault_active || busfault_active || spufault_active)) {
		while(true);
		NVIC_SystemReset();
	}

	/*
	 * If we get here, we are taking a reset path where a fault was generated
	 * from the NS firmware running on the device. If we just reset, it will be
	 * impossible to extract the root cause of the error on the NS side.
	 *
	 * To allow for root cause analysis, let's call the NS HardFault handler.  Any error from
	 * the NS fault handler will land us back in the Secure HardFault handler where we will not
	 * enter this path and simply reset the device.
	 */

	uint32_t *vtor = (uint32_t *)tfm_hal_get_ns_VTOR();

	uint32_t hardfault_handler_fn = vtor[HARDFAULT_EXCEPTION_NUMBER];

	/* bit 0 needs to be cleared to transition to NS */
	hardfault_handler_fn &= ~0x1;

	/* Adjust EXC_RETURN value to emulate NS exception entry */
	uint32_t ns_exc_return = exc_ctx.EXC_RETURN & ~EXC_RETURN_ES;
	/* Update SPSEL to reflect correct CONTROL_NS.SPSEL setting */
	ns_exc_return &= ~(EXC_RETURN_SPSEL);
	CONTROL_Type ctrl_ns;
	ctrl_ns.w = __TZ_get_CONTROL_NS();
	if (ctrl_ns.b.SPSEL) {
		ns_exc_return |= EXC_RETURN_SPSEL;
	}

	// while(true);
	handle_fault_from_ns(hardfault_handler_fn, ns_exc_return);

	while(true);

	NVIC_SystemReset();
}
#else
void tfm_hal_system_reset(void)
{
	NVIC_SystemReset();
}
#endif /* defined(TRUSTZONE_PRESENT) && defined(TFM_EXCEPTION_INFO_DUMP) */
// #endif /* CONFIG_TFM_ALLOW_NON_SECURE_FAULT_HANDLING */
