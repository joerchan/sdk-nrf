#include "cmsis.h"
#include "uart_stdout.h"
#include "tfm_spm_log.h"
#include "config_tfm.h"
#include "exception_info.h"
#include "tfm_arch.h"
#include "tfm_ioctl_api.h"

#if NRF_ALLOW_NON_SECURE_FAULT_HANDLING

#define SECUREFAULT_EXCEPTION_NUMBER (NVIC_USER_IRQ_OFFSET + SecureFault_IRQn)
#define HARDFAULT_EXCEPTION_NUMBER   (NVIC_USER_IRQ_OFFSET + HardFault_IRQn)
#define BUSFAULT_EXCEPTION_NUMBER    (NVIC_USER_IRQ_OFFSET + BusFault_IRQn)

#define SPUFAULT_EXCEPTION_NUMBER    (NVIC_USER_IRQ_OFFSET + SPU_IRQn)

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

typedef void (*ns_funcptr) (void) __attribute__((cmse_nonsecure_call));

static struct tfm_ns_fault_service_handler_context  *ns_callback_context;
static ns_funcptr ns_callback;

void ns_fault_service_save_spu_events(void)
{
	if (ns_callback_context) {
		ns_callback_context->status.spu_events = 0;

		if(NRF_SPU->EVENTS_RAMACCERR) {
			ns_callback_context->status.spu_events |=  TFM_SPU_EVENT_RAMACCERR;
		}
		if(NRF_SPU->EVENTS_PERIPHACCERR) {
			ns_callback_context->status.spu_events |=  TFM_SPU_EVENT_PERIPHACCERR;
		}
		if(NRF_SPU->EVENTS_FLASHACCERR) {
			ns_callback_context->status.spu_events |=  TFM_SPU_EVENT_FLASHACCERR;
		}
	}
}

int ns_fault_service_set_handler(struct tfm_ns_fault_service_handler_context  *context,
					tfm_ns_fault_service_handler_callback callback)
{
	ns_callback_context = context;
	ns_callback = (ns_funcptr)callback;

	return 0;
}

void call_ns_callback(struct exception_info_t *exc_info)
{
	ns_callback_context->frame.r0 = exc_info->EXC_FRAME_COPY[0];
	ns_callback_context->frame.r1 = exc_info->EXC_FRAME_COPY[1];
	ns_callback_context->frame.r2 = exc_info->EXC_FRAME_COPY[2];
	ns_callback_context->frame.r3 = exc_info->EXC_FRAME_COPY[3];
	ns_callback_context->frame.r12 = exc_info->EXC_FRAME_COPY[4];
	ns_callback_context->frame.lr = exc_info->EXC_FRAME_COPY[5];
	ns_callback_context->frame.pc = exc_info->EXC_FRAME_COPY[6];
	ns_callback_context->frame.xpsr = exc_info->EXC_FRAME_COPY[7];

	/* TODO: make TF-M save callee-saved registers. */
	memset(&ns_callback_context->registers, 0,sizeof(struct tfm_ns_fault_service_handler_context_registers));

	ns_callback_context->status.cfsr = exc_info->CFSR;
	ns_callback_context->status.hfsr = exc_info->HFSR;
	ns_callback_context->status.sfsr = exc_info->SFSR;
	ns_callback_context->status.bfar = exc_info->BFAR;
	ns_callback_context->status.mmfar = exc_info->MMFAR;
	ns_callback_context->status.sfar = exc_info->SFAR;

	// ns_callback_context->status.msp = exc_info->MSP;
	// ns_callback_context->status.psp = exc_info->PSP;
	ns_callback_context->status.msp = __TZ_get_MSP_NS();
	ns_callback_context->status.psp = __TZ_get_PSP_NS();

	ns_callback_context->status.exc_return = exc_info->EXC_RETURN;


	/* Already saved */
	// ns_callback_context->status.spu_events;

	ns_callback();
}

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
 * AICR.BFHFNMINS bit:
 * Enable BusFault HardFault and NMI to target non-secure.
 * Since HardFault is banked this wil target the security state it is taken from, the others
 * will always target non-secure.
 * The effect of enabling this and PRIS at the same time is UNDEFINED.
 *
 * External interrupts target security state based on NVIC target state configuration.
 * The SPU interrupt has been configured to target secure state in target_cfg.c
 *
 * The zephyr interrupt handling for ARM uses a common fault handler for all types of
 * Exceptions. This one also handles exceptions that have not been enabled or configured,
 * and external interrupts that have not been enabled or configured.
 * These are handled the same with just a difference in the message that is printed:
 * "Reserved Exception" and "Spurious interrupt".
 *
 * The SPU fault handler is just an extension to of the BusFault or SecureFault handler
 * that is triggered by events external to the CPU, such as an EasyDMA access.
 * Handling this by the same handler as BusFault or SecureFault therefore shouldn't cause
 * any problems.
 *
 * When triggering the fault handler in non-secure application the non-secure fault handler
 * does not have access to the fault status registers.
 * The fault handler as a result will therefore not be able to provide any fault information.
 */

void ns_fault_handling(void)
{
	struct exception_info_t exc_ctx;
	
	tfm_exception_info_get_context(&exc_ctx);

	const bool exc_ctx_valid = exc_ctx.EXC_RETURN != 0x0;
	const uint8_t active_exception_number = (exc_ctx.xPSR & 0xff);

	const bool securefault_active = (active_exception_number == SECUREFAULT_EXCEPTION_NUMBER);
	const bool busfault_active = (active_exception_number == BUSFAULT_EXCEPTION_NUMBER);
	const bool hardfault_active = (active_exception_number == HARDFAULT_EXCEPTION_NUMBER);
	const bool spufault_active = (active_exception_number == SPUFAULT_EXCEPTION_NUMBER);

	SPMLOG_ERRMSGVAL("Active exception number", active_exception_number);

	if (!exc_ctx_valid ||
	    is_return_secure_stack(exc_ctx.EXC_RETURN) ||
	    !(securefault_active || busfault_active || spufault_active)) {
		return;
	}

	if (ns_callback) {
		call_ns_callback(&exc_ctx);
	}
}
#endif /* CONFIG_TFM_ALLOW_NON_SECURE_FAULT_HANDLING */

void tfm_hal_system_halt(void)
{
#if NRF_ALLOW_NON_SECURE_FAULT_HANDLING
	ns_fault_handling();
#endif

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

void tfm_hal_system_reset(void)
{
#if NRF_ALLOW_NON_SECURE_FAULT_HANDLING
	ns_fault_handling();
#endif

	NVIC_SystemReset();
}
