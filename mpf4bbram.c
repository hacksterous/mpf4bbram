//(c) 2019 Anirban Banerjee
//Licensed under:
//GNU General Public License version 3

//see also: https://stackoverflow.com/questions/20667754/how-to-use-backup-sram-as-eeprom-in-stm32f4

#include "py/obj.h"
#include "py/runtime.h"
#include "py/builtin.h"
#ifdef BARE_M
#define RTC_BKPREG_BASE (RTC_BASE + 0x50)
#include <stm32f4xx_hal_pwr.h>
#include <stm32f4xx_hal_rcc.h>

#include "mpconfig.h"
#endif

uint32_t enable_bak_reg_pwr (void) {
#ifdef BARE_M
	uint32_t bit, timeout;
	bit = READ_BIT(PWR->CSR, PWR_CSR_BRR);
	//if Backup Regulator ready is already set, return
	if (bit & PWR_CSR_BRR)
		return (1);
	//set Backup regulator enable
	SET_BIT(PWR->CSR, PWR_CSR_BRE);
	//read Backup Regulator ready a max. of 32 times
	for (timeout = 0; timeout < 31; timeout++){
		bit = READ_BIT(PWR->CSR, PWR_CSR_BRR);
		bit = READ_BIT(PWR->CSR, PWR_CSR_BRR);
		if (bit & PWR_CSR_BRR)
			break;
	}
	return ((bit & PWR_CSR_BRR) >> PWR_CSR_BRR_Pos);
#endif
}

void disable_bak_reg_pwr (void) {
#ifdef BARE_M
	//clear Backup regulator enable
	PWR->CSR &= ~PWR_CSR_BRE;
#endif
}

STATIC mp_obj_t mpf4bbram_enable (void) {
#ifdef BARE_M
	//enable power interface clock
    __HAL_RCC_PWR_CLK_ENABLE();
	
	//enable BBRAM interface clock
	SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_BKPSRAMEN);

	//Enable access to RTC domain by setting the DBP bit
	//DBP = Disable Backup Protection
	HAL_PWR_EnableBkUpAccess();
#endif
    return mp_const_none;
}

STATIC mp_obj_t mpf4bbram_disable (void) {
#ifdef BARE_M
	HAL_PWR_DisableBkUpAccess();
	RCC->AHB1ENR &= ~RCC_AHB1ENR_BKPSRAMEN;
    __HAL_RCC_PWR_CLK_DISABLE();
#endif
    return mp_const_none;
}

STATIC mp_obj_t mpf4bbram_init (void) {
	//is called once in in boot.py or main.py
	uint32_t regulator_pwr = 0;
#ifdef BARE_M
	regulator_pwr = enable_bak_reg_pwr();
#endif
    return mp_obj_new_int(regulator_pwr);
}

STATIC mp_obj_t mpf4bbram_kill (void) {
	//will kill BBRAM data when main power
	//is unplugged
#ifdef BARE_M
	mpf4bbram_disable();
	disable_bak_reg_pwr();
#endif
    return mp_const_none;
}

STATIC mp_obj_t mpf4bbram_write32_bbreg (mp_obj_t oindex, mp_obj_t odata) {
#ifdef BARE_M
	uint32_t index = (uint32_t) mp_obj_get_int(oindex);
	uint32_t data = (uint32_t) mp_obj_get_int(odata);
	if (index < 20){
		*(__IO uint32_t *) (RTC_BKPREG_BASE + (index << 2)) = data;
	}
#endif
    return mp_const_none;
}

STATIC mp_obj_t mpf4bbram_write32_bbram (mp_obj_t oindex, mp_obj_t odata) {
	//odata could be int, uint32 or float (single precision)
#ifdef BARE_M
	uint32_t index = (uint32_t) mp_obj_get_int(oindex);
	uint32_t data = (uint32_t) mp_obj_get_int(odata);
	*(__IO uint32_t *) (BKPSRAM_BASE + (index << 2)) = data;
#endif
    return mp_const_none;
}

STATIC mp_obj_t mpf4bbram_read32_bbreg (mp_obj_t oindex) {
#ifdef BARE_M
	__IO uint32_t data;
	uint32_t index = (uint32_t) mp_obj_get_int(oindex);
	data = *(__IO uint32_t *) (RTC_BKPREG_BASE + index);
	return mp_obj_new_int(data);
#else
    return mp_obj_new_int(0);
#endif
}

STATIC mp_obj_t mpf4bbram_read32_bbram (mp_obj_t oindex) {
	//odata could be int, uint32 or float (single precision)
#ifdef BARE_M
	__IO uint32_t data;
	uint32_t index = (uint32_t) mp_obj_get_int(oindex);
	data = *(__IO uint32_t *) (BKPSRAM_BASE + index);
    return mp_obj_new_int(data);
#else
    return mp_obj_new_int(0);
#endif
}

STATIC MP_DEFINE_CONST_FUN_OBJ_0(mpf4bbram_enable_obj, mpf4bbram_enable);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mpf4bbram_disable_obj, mpf4bbram_disable);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mpf4bbram_init_obj, mpf4bbram_init);
STATIC MP_DEFINE_CONST_FUN_OBJ_0(mpf4bbram_kill_obj, mpf4bbram_kill);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mpf4bbram_write32_bbreg_obj, mpf4bbram_write32_bbreg);
STATIC MP_DEFINE_CONST_FUN_OBJ_2(mpf4bbram_write32_bbram_obj, mpf4bbram_write32_bbram);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mpf4bbram_read32_bbreg_obj, mpf4bbram_read32_bbreg);
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mpf4bbram_read32_bbram_obj, mpf4bbram_read32_bbram);

STATIC const mp_rom_map_elem_t mpf4bbram_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mpf4bbram) },
    { MP_ROM_QSTR(MP_QSTR_enable), MP_ROM_PTR(&mpf4bbram_enable_obj) },
    { MP_ROM_QSTR(MP_QSTR_disable), MP_ROM_PTR(&mpf4bbram_disable_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&mpf4bbram_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_kill), MP_ROM_PTR(&mpf4bbram_kill_obj) },
    { MP_ROM_QSTR(MP_QSTR_write32_bbreg), MP_ROM_PTR(&mpf4bbram_write32_bbreg_obj) },
    { MP_ROM_QSTR(MP_QSTR_write32_bbram), MP_ROM_PTR(&mpf4bbram_write32_bbram_obj) },
    { MP_ROM_QSTR(MP_QSTR_read32_bbreg), MP_ROM_PTR(&mpf4bbram_read32_bbreg_obj) },
    { MP_ROM_QSTR(MP_QSTR_read32_bbram), MP_ROM_PTR(&mpf4bbram_read32_bbram_obj) }
};

STATIC MP_DEFINE_CONST_DICT(mpf4bbram_module_globals, mpf4bbram_module_globals_table);

const mp_obj_module_t mpf4bbram_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mpf4bbram_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_mpf4bbram, mpf4bbram_user_cmodule, MODULE_MPF4BBRAM_ENABLED);


