/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#include "CppUTest/TestHarness.h"
#include "CppUTest/TestHarness_c.h"
#include "CppUTestExt/MockSupport.h"

#include "bq25180.h"
#include "bq25180_overrides.h"

#if defined(__cplusplus)
extern "C" {
#endif
void fake_assert(bool exp) {
	if (exp) {
		return;
	}

	mock().actualCall(__func__);
	TEST_EXIT;
}
#if defined(__cplusplus)
}
#endif

int bq25180_read(uint8_t addr, uint8_t reg, void *buf, size_t bufsize) {
	return mock().actualCall(__func__)
			.withParameter("addr", addr)
			.withParameter("reg", reg)
			.withOutputParameter("buf", buf)
			.withParameter("bufsize", bufsize)
			.returnIntValueOrDefault((int)bufsize);
}

int bq25180_write(uint8_t addr, uint8_t reg, const void *data, size_t data_len) {
	return mock().actualCall(__func__)
			.withParameter("addr", addr)
			.withParameter("reg", reg)
			.withMemoryBufferParameter("data", (const uint8_t *)data, data_len)
			.returnIntValueOrDefault((int)data_len);
}

TEST_GROUP(BQ25180) {
	void setup(void) {
		mock().strictOrder();
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}

	void expect_reg_read(uint8_t reg, uint8_t *p) {
		mock().expectOneCall("bq25180_read")
			.withParameter("addr", BQ25180_DEVICE_ADDRESS)
			.withParameter("reg", reg)
			.withOutputParameterReturning("buf", p, sizeof(*p))
			.withParameter("bufsize", 1);
	}
	void expect_reg_write(uint8_t reg, uint8_t *p) {
		mock().expectOneCall("bq25180_write")
			.withParameter("addr", BQ25180_DEVICE_ADDRESS)
			.withParameter("reg", reg)
			.withMemoryBufferParameter("data", p, sizeof(*p));
	}
	void expect_reg(uint8_t reg, uint8_t reset_val, uint8_t expected) {
		static uint8_t v1, v2;

		v1 = reset_val;
		v2 = expected;
		expect_reg_read(reg, &v1);
		expect_reg_write(reg, &v2);
	}
};

TEST(BQ25180, reset_ShouldDoSoftReset_WhenHardwareResetFlagIsFalse) {
	expect_reg(0x09/*SHIP_RST*/, 0x11, 0x91);
	bq25180_reset(0);
}

TEST(BQ25180, reset_ShouldDoHardwareReset_WhenHardwareResetFlagIsTrue) {
	expect_reg(0x09/*SHIP_RST*/, 0x11, 0x71);
	bq25180_reset(true);
}

TEST(BQ25180, read_event_ShouldReturnEvents_WhenNoEventOccured) {
	uint8_t default_flag0 = 0;
	struct bq25180_event expected = { 0, };
	struct bq25180_event actual;

	expect_reg_read(0x02, &default_flag0);

	LONGS_EQUAL(true, bq25180_read_event(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_event_ShouldReturnEvents_WhenAllEventsOccured) {
	uint8_t flag0 = 0xff;
	struct bq25180_event expected = {
		.battery_overcurrent = 1,
		.battery_undervoltage = 1,
		.input_overvoltage = 1,
		.thermal_regulation = 1,
		.vindpm_fault = 1,
		.vdppm_fault = 1,
		.ilim_fault = 1,
		.battery_thermal_fault = 1,
	};
	struct bq25180_event actual;

	expect_reg_read(0x02, &flag0);

	LONGS_EQUAL(true, bq25180_read_event(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_event_ShouldReturnEvents_WhenSomeEventsOccured) {
	uint8_t flag0 = 0x55;
	struct bq25180_event expected = {
		.battery_overcurrent = 1,
		.battery_undervoltage = 0,
		.input_overvoltage = 1,
		.thermal_regulation = 0,
		.vindpm_fault = 1,
		.vdppm_fault = 0,
		.ilim_fault = 1,
		.battery_thermal_fault = 0,
	};
	struct bq25180_event actual;

	expect_reg_read(0x02, &flag0);

	LONGS_EQUAL(true, bq25180_read_event(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_event_ShouldAssertParam_WhenNullParamGiven) {
	mock().expectOneCall("fake_assert");
	bq25180_read_event(NULL);
}

TEST(BQ25180, read_state_ShouldReturnState) {
	uint8_t stat0 = 0;
	uint8_t stat1 = 0;
	struct bq25180_state expected = { 0, };
	struct bq25180_state actual;

	expect_reg_read(0x00, &stat0);
	expect_reg_read(0x01, &stat1);

	LONGS_EQUAL(true, bq25180_read_state(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_state_ShouldReturnState_WhenAllStateAreSetToOnes) {
	uint8_t stat0 = 0xff;
	uint8_t stat1 = 0xff;
	struct bq25180_state expected = {
		.vin_good = 1,
		.thermal_regulation_active = 1,
		.vindpm_active = 1,
		.vdppm_active = 1,
		.ilim_active = 1,
		.charging_status = 3,
		.tsmr_open = 1,
		.wake2_raised = 1,
		.wake1_raised = 1,
		.safety_timer_fault = 1,
		.ts_status = 3,
		.battery_undervoltage_active = 1,
		.vin_overvoltage_active = 1,
	};

	struct bq25180_state actual;

	expect_reg_read(0x00, &stat0);
	expect_reg_read(0x01, &stat1);

	LONGS_EQUAL(true, bq25180_read_state(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_state_ShouldReturnState_WhenSomeStateAreSetToOnes) {
	uint8_t stat0 = 0x35;
	uint8_t stat1 = 0x95;
	struct bq25180_state expected = {
		.vin_good = 1,
		.thermal_regulation_active = 0,
		.vindpm_active = 1,
		.vdppm_active = 0,
		.ilim_active = 1,
		.charging_status = 1,
		.tsmr_open = 0,
		.wake2_raised = 1,
		.wake1_raised = 0,
		.safety_timer_fault = 1,
		.ts_status = 2,
		.battery_undervoltage_active = 0,
		.vin_overvoltage_active = 1,
	};

	struct bq25180_state actual;

	expect_reg_read(0x00, &stat0);
	expect_reg_read(0x01, &stat1);

	LONGS_EQUAL(true, bq25180_read_state(&actual));
	MEMCMP_EQUAL(&expected, &actual, sizeof(expected));
}

TEST(BQ25180, read_state_ShouldAssertParam_WhenNullParamGiven) {
	mock().expectOneCall("fake_assert");
	bq25180_read_state(NULL);
}

TEST(BQ25180, battery_charging_ShouldDisable) {
	expect_reg(0x04/*ICHG_CTRL*/, 0x05, 0x85);
	bq25180_enable_battery_charging(false);
}

TEST(BQ25180, battery_charging_ShouldEnable) {
	expect_reg(0x04/*ICHG_CTRL*/, 0x85, 0x05);
	bq25180_enable_battery_charging(true);
}

TEST(BQ25180, safety_timer_ShouldBeSet_When3HGiven) {
	expect_reg(0x07/*IC_CTRL*/, 0x84, 0x80);
	bq25180_set_safety_timer(BQ25180_SAFETY_3H);
}

TEST(BQ25180, safety_timer_ShouldBeDisabled_WhenRequested) {
	expect_reg(0x07/*IC_CTRL*/, 0x84, 0x8c);
	bq25180_set_safety_timer(BQ25180_SAFETY_DISABLE);
}

TEST(BQ25180, watchdog_timer_ShouldBeSet_When40sGiven) {
	expect_reg(0x07/*IC_CTRL*/, 0x84, 0x86);
	bq25180_set_watchdog_timer(BQ25180_WDT_40_SEC);
}

TEST(BQ25180, watchdog_timer_ShouldBeDisabled_WhenRequested) {
	expect_reg(0x07/*IC_CTRL*/, 0x84, 0x87);
	bq25180_set_watchdog_timer(BQ25180_WDT_DISABLE);
}

TEST(BQ25180, battery_regulation_ShouldSetVoltage_WhenMilliVoltageGiven) {
	uint8_t expected = 0x46;
	expect_reg_write(0x03/*VBAT_CTRL*/, &expected);
	bq25180_set_battery_regulation_voltage(4200);
	expected = 0x00, expect_reg_write(0x03/*VBAT_CTRL*/, &expected);
	bq25180_set_battery_regulation_voltage(3500);
	expected = 0x73, expect_reg_write(0x03/*VBAT_CTRL*/, &expected);
	bq25180_set_battery_regulation_voltage(4650);
}

TEST(BQ25180, battery_regulation_ShouldAssertParam_WhenBelowTheRange) {
	mock().expectOneCall("fake_assert");
	bq25180_set_battery_regulation_voltage(3500-1);
}

TEST(BQ25180, battery_regulation_ShouldAssertParam_WhenAboveTheRange) {
	mock().expectOneCall("fake_assert");
	bq25180_set_battery_regulation_voltage(4650+1);
}

TEST(BQ25180, battery_discharge_current_ShouldSetOCP) {
	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0x16);
	bq25180_set_battery_discharge_current(BQ25180_BAT_DISCHAGE_500mA);

	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0x56);
	bq25180_set_battery_discharge_current(BQ25180_BAT_DISCHAGE_1000mA);

	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0x96);
	bq25180_set_battery_discharge_current(BQ25180_BAT_DISCHAGE_1500mA);

	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0xd6);
	bq25180_set_battery_discharge_current(BQ25180_BAT_DISCHAGE_DISABLE);
}

TEST(BQ25180, battery_undervoltage_ShouldSetUVLO) {
	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0x7e);
	bq25180_set_battery_under_voltage(2000);

	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0x76);
	bq25180_set_battery_under_voltage(2200);

	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0x6e);
	bq25180_set_battery_under_voltage(2400);

	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0x66);
	bq25180_set_battery_under_voltage(2600);

	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0x5e);
	bq25180_set_battery_under_voltage(2800);

	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0x56);
	bq25180_set_battery_under_voltage(3000);
}

TEST(BQ25180, battery_undervoltage_ShouldAssertParam_WhenAboveTheRange) {
	mock().expectOneCall("fake_assert");
	bq25180_set_battery_under_voltage(3000+1);
}

TEST(BQ25180, battery_undervoltage_ShouldAssertParam_WhenBelowTheRange) {
	mock().expectOneCall("fake_assert");
	bq25180_set_battery_under_voltage(2000-1);
}

TEST(BQ25180, precharge_threshold_ShouldSetVLOWV) {
	expect_reg(0x07/*IC_CTRL*/, 0x84, 0xc4);
	bq25180_set_precharge_threshold(2800);

	expect_reg(0x07/*IC_CTRL*/, 0x84, 0x84);
	bq25180_set_precharge_threshold(3000);
}

TEST(BQ25180, precharge_current_ShouldSetIPRECHG) {
	expect_reg(0x05/*CHARGECTRL0*/, 0x2c, 0x6c);
	bq25180_set_precharge_current(false);

	expect_reg(0x05/*CHARGECTRL0*/, 0x2c, 0x2c);
	bq25180_set_precharge_current(true);
}

TEST(BQ25180, fastcharge_current_ShouldAssertParam_WhenAboveTheRange) {
	mock().expectOneCall("fake_assert");
	bq25180_set_fastcharge_current(1000+1);
}

TEST(BQ25180, fastcharge_current_ShouldAssertParam_WhenBelowTheRange) {
	mock().expectOneCall("fake_assert");
	bq25180_set_fastcharge_current(5-1);
}

TEST(BQ25180, fastcharge_ShouldSetICHG) {
	expect_reg(0x04/*ICHG_CTRL*/, 0x05, 0x05);
	bq25180_set_fastcharge_current(10);

	expect_reg(0x04/*ICHG_CTRL*/, 0x05, 0x1e);
	bq25180_set_fastcharge_current(35);

	expect_reg(0x04/*ICHG_CTRL*/, 0x05, 0x1f);
	bq25180_set_fastcharge_current(40);

	expect_reg(0x04/*ICHG_CTRL*/, 0x05, 0x7f);
	bq25180_set_fastcharge_current(1000);
}

TEST(BQ25180, terminante_current_ShouldSetITERM) {
	expect_reg(0x05/*CHARGECTRL0*/, 0x2c, 0x1c);
	bq25180_set_termination_current(5);

	expect_reg(0x05/*CHARGECTRL0*/, 0x2c, 0x2c);
	bq25180_set_termination_current(10);

	expect_reg(0x05/*CHARGECTRL0*/, 0x2c, 0x3c);
	bq25180_set_termination_current(20);

	expect_reg(0x05/*CHARGECTRL0*/, 0x2c, 0x0c);
	bq25180_set_termination_current(0);
}

TEST(BQ25180, vindpm_SholudSetVINDPM) {
	expect_reg(0x05/*CHARGECTRL0*/, 0x2c, 0x20);
	bq25180_enable_vindpm(BQ25180_VINDPM_4200mV);

	expect_reg(0x05/*CHARGECTRL0*/, 0x2c, 0x24);
	bq25180_enable_vindpm(BQ25180_VINDPM_4500mV);

	expect_reg(0x05/*CHARGECTRL0*/, 0x2c, 0x28);
	bq25180_enable_vindpm(BQ25180_VINDPM_4700mV);

	expect_reg(0x05/*CHARGECTRL0*/, 0x2c, 0x2c);
	bq25180_enable_vindpm(BQ25180_VINDPM_DISABLE);
}

TEST(BQ25180, dppm_ShouldSetVDPPM) {
	expect_reg(0x0a/*SYS_REG*/, 0x40, 0x41);
	bq25180_enable_dppm(0);

	expect_reg(0x0a/*SYS_REG*/, 0x40, 0x40);
	bq25180_enable_dppm(true);
}

TEST(BQ25180, input_current_ShouldSetILIM) {
	expect_reg(0x08/*TMR_ILIM*/, 0x4d, 0x48);
	bq25180_set_input_current(50);

	expect_reg(0x08/*TMR_ILIM*/, 0x4d, 0x49);
	bq25180_set_input_current(100);

	expect_reg(0x08/*TMR_ILIM*/, 0x4d, 0x4a);
	bq25180_set_input_current(200);

	expect_reg(0x08/*TMR_ILIM*/, 0x4d, 0x4b);
	bq25180_set_input_current(300);

	expect_reg(0x08/*TMR_ILIM*/, 0x4d, 0x4c);
	bq25180_set_input_current(400);

	expect_reg(0x08/*TMR_ILIM*/, 0x4d, 0x4d);
	bq25180_set_input_current(500);

	expect_reg(0x08/*TMR_ILIM*/, 0x4d, 0x4e);
	bq25180_set_input_current(700);

	expect_reg(0x08/*TMR_ILIM*/, 0x4d, 0x4f);
	bq25180_set_input_current(1100);
}

TEST(BQ25180, sys_source_ShouldSetSYSMODE) {
	expect_reg(0x0a/*SYS_REG*/, 0x40, 0x48);
	bq25180_set_sys_source(BQ25180_SYS_SRC_NONE_FLOATING);

	expect_reg(0x0a/*SYS_REG*/, 0x40, 0x4c);
	bq25180_set_sys_source(BQ25180_SYS_SRC_NONE_PULLDOWN);

	expect_reg(0x0a/*SYS_REG*/, 0x40, 0x44);
	bq25180_set_sys_source(BQ25180_SYS_SRC_VBAT);

	expect_reg(0x0a/*SYS_REG*/, 0x40, 0x40);
	bq25180_set_sys_source(BQ25180_SYS_SRC_VIN_VBAT);
}

TEST(BQ25180, sys_voltage_ShouldSetSYSREGCTRL) {
	expect_reg(0x0a/*SYS_REG*/, 0x40, 0x00);
	bq25180_set_sys_voltage(BQ25180_SYS_REG_VBAT);

	expect_reg(0x0a/*SYS_REG*/, 0x40, 0x20);
	bq25180_set_sys_voltage(BQ25180_SYS_REG_V4_4);

	expect_reg(0x0a/*SYS_REG*/, 0x40, 0x40);
	bq25180_set_sys_voltage(BQ25180_SYS_REG_V4_5);

	expect_reg(0x0a/*SYS_REG*/, 0x40, 0x60);
	bq25180_set_sys_voltage(BQ25180_SYS_REG_V4_6);

	expect_reg(0x0a/*SYS_REG*/, 0x40, 0x80);
	bq25180_set_sys_voltage(BQ25180_SYS_REG_V4_7);

	expect_reg(0x0a/*SYS_REG*/, 0x40, 0xa0);
	bq25180_set_sys_voltage(BQ25180_SYS_REG_V4_8);

	expect_reg(0x0a/*SYS_REG*/, 0x40, 0xc0);
	bq25180_set_sys_voltage(BQ25180_SYS_REG_V4_9);

	expect_reg(0x0a/*SYS_REG*/, 0x40, 0xe0);
	bq25180_set_sys_voltage(BQ25180_SYS_REG_PASS_THROUGH);
}

TEST(BQ25180, thermal_protection_ShouldDisable) {
	expect_reg(0x07/*IC_CTRL*/, 0x84, 0x4);
	bq25180_enable_thermal_protection(false);
}

TEST(BQ25180, thermal_protection_ShouldEnable) {
	expect_reg(0x07/*IC_CTRL*/, 0x4, 0x84);
	bq25180_enable_thermal_protection(true);
}

TEST(BQ25180, enable_push_button_ShouldDisable) {
	expect_reg(0x09/*SHIP_RST*/, 0x11, 0x10);
	bq25180_enable_push_button(false);
}

TEST(BQ25180, enable_push_button_ShouldEnable) {
	expect_reg(0x09/*SHIP_RST*/, 0x10, 0x11);
	bq25180_enable_push_button(true);
}

TEST(BQ25180, enable_interrupt_ShouldEnable) {
	expect_reg(0x0C/*MASK_ID*/, 0x00, 0x80);
	bq25180_enable_interrupt(BQ25180_INTR_THERMAL_FAULT);
	expect_reg(0x0C/*MASK_ID*/, 0x00, 0x40);
	bq25180_enable_interrupt(BQ25180_INTR_THERMAL_REGULATION);
	expect_reg(0x0C/*MASK_ID*/, 0x00, 0x20);
	bq25180_enable_interrupt(BQ25180_INTR_BATTERY_RANGE);
	expect_reg(0x0C/*MASK_ID*/, 0x00, 0x10);
	bq25180_enable_interrupt(BQ25180_INTR_POWER_ERROR);

	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0x52);
	bq25180_enable_interrupt(BQ25180_INTR_CHARGING_STATUS);
	expect_reg(0x06/*CHARGECTRL1*/, 0x56, 0x54);
	bq25180_enable_interrupt(BQ25180_INTR_CURRENT_LIMIT);
	expect_reg(0x06/*CHARGECTRL1*/, 0x57, 0x56);
	bq25180_enable_interrupt(BQ25180_INTR_VDPM);
}

TEST(BQ25180, disable_interrupt_ShouldDisable) {
	expect_reg(0x0C/*MASK_ID*/, 0xf0, 0x70);
	bq25180_disable_interrupt(BQ25180_INTR_THERMAL_FAULT);
	expect_reg(0x0C/*MASK_ID*/, 0xf0, 0xb0);
	bq25180_disable_interrupt(BQ25180_INTR_THERMAL_REGULATION);
	expect_reg(0x0C/*MASK_ID*/, 0xf0, 0xd0);
	bq25180_disable_interrupt(BQ25180_INTR_BATTERY_RANGE);
	expect_reg(0x0C/*MASK_ID*/, 0xf0, 0xe0);
	bq25180_disable_interrupt(BQ25180_INTR_POWER_ERROR);

	expect_reg(0x06/*CHARGECTRL1*/, 0, 4);
	bq25180_disable_interrupt(BQ25180_INTR_CHARGING_STATUS);
	expect_reg(0x06/*CHARGECTRL1*/, 0, 2);
	bq25180_disable_interrupt(BQ25180_INTR_CURRENT_LIMIT);
	expect_reg(0x06/*CHARGECTRL1*/, 0, 1);
	bq25180_disable_interrupt(BQ25180_INTR_VDPM);
}
