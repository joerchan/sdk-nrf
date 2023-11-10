#include <zephyr/init.h>
#include <tfm_ioctl_api.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(nonsecure, LOG_LEVEL_DBG);

static struct tfm_ns_fault_service_handler_context g_context;
void tfm_ns_fault_handler_callback(void)
{
#if 0
	LOG_ERR("NS FAULT HANDLER!!");

        LOG_ERR("r0: %x", g_context.frame.r0);
        LOG_ERR("r1: %x", g_context.frame.r1);
        LOG_ERR("r2: %x", g_context.frame.r2);
        LOG_ERR("r3: %x", g_context.frame.r3);
        LOG_ERR("r12: %x", g_context.frame.r12);
        LOG_ERR("lr: %x", g_context.frame.lr);
        LOG_ERR("pc: %x", g_context.frame.pc);
        LOG_ERR("xpsr: %x", g_context.frame.xpsr);


	LOG_ERR("cfsr: %x", g_context.status.cfsr);
        uint32_t mfsr = g_context.status.cfsr & 0xff;
        uint32_t bfsr = (g_context.status.cfsr >> 8) & 0xff;
        uint32_t ufsr = (g_context.status.cfsr >>16) & 0xffff;

        LOG_ERR("mfsr: %x", mfsr);
        LOG_ERR("bfsr: %x", bfsr);
        LOG_ERR("ufsr: %x", ufsr);

	LOG_ERR("hfsr: %x", g_context.status.hfsr);
	LOG_ERR("sfsr: %x", g_context.status.sfsr);
	LOG_ERR("bfar: %x", g_context.status.bfar);
	LOG_ERR("mmfar: %x", g_context.status.mmfar);
	LOG_ERR("sfar: %x", g_context.status.sfar);
	LOG_ERR("spu_events: %x", g_context.status.spu_events);
#endif
        // z_arm_fatal_error(K_ERR_CPU_EXCEPTION, NULL);
        z_arch_esf_t esf;

        esf.basic.r0 = g_context.frame.r0;
        esf.basic.r1 = g_context.frame.r1;
        esf.basic.r2 = g_context.frame.r2;
        esf.basic.r3 = g_context.frame.r3;
        esf.basic.r12 = g_context.frame.r12;
        esf.basic.lr = g_context.frame.lr;
        esf.basic.pc = g_context.frame.pc;
        esf.basic.xpsr = g_context.frame.xpsr;
#if defined(CONFIG_EXTRA_EXCEPTION_INFO)
        _callee_saved_t callee_regs;

        esf.extra_info.exc_return = g_context.status.exc_return;
        esf.extra_info.msp = g_context.status.msp;

	callee_regs.psp = g_context.status.psp;

        callee_regs.v1 = g_context.registers.r4;
	callee_regs.v2 = g_context.registers.r5;
	callee_regs.v3 = g_context.registers.r6;
	callee_regs.v4 = g_context.registers.r7;
	callee_regs.v5 = g_context.registers.r8;
	callee_regs.v6 = g_context.registers.r9;
	callee_regs.v7 = g_context.registers.r10;
	callee_regs.v8 = g_context.registers.r11;

        esf.extra_info.callee = &callee_regs;
#endif
        
        z_arm_fatal_error(K_ERR_CPU_EXCEPTION, &esf);
}

static int nonsecure_init(void)
{
	int err = 0;

	err = tfm_platform_ns_fault_set_handler(&g_context, &tfm_ns_fault_handler_callback);
	if (err) {
		LOG_ERR("TF-M non-secure callback initialization failed, error: %d", err);
		return err;
	}

        return err;
}

SYS_INIT(nonsecure_init, PRE_KERNEL_1, 0);
