/*
 * SPDX-FileCopyrightText: 2022 Kyunghwan Kwon <k@mononn.com>
 *
 * SPDX-License-Identifier: MIT
 */

#ifndef LIBMCU_BQ25180_OVERRIDES_H
#define LIBMCU_BQ25180_OVERRIDES_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Read a register value via I2C
 *
 * @param[in] addr device address
 * @param[in] reg register address to read from
 * @param[in] buf buffer to get the value in
 * @param[in] bufsize size of @ref buf
 *
 * @return The number of bytes read on success. Otherwise a negative integer
 *         error code
 *
 * @note This function should be implemented for the specific platform or
 *       board. The default one does nothing being linked weak.
 */
int bq25180_read(uint8_t addr, uint8_t reg, void *buf, size_t bufsize);

/**
 * @brief Write a value to a register via I2C
 *
 * @param[in] addr device address
 * @param[in] reg register address to write to
 * @param[in] data data to be written in @ref reg
 * @param[in] data_len length of @ref data
 *
 * @return The number of bytes written on success. Otherwise a negative integer
 *         error code
 *
 * @note This function should be implemented for the specific platform or
 *       board. The default one does nothing being linked weak.
 */
int bq25180_write(uint8_t addr, uint8_t reg, const void *data, size_t data_len);

#if defined(__cplusplus)
}
#endif

#endif /* LIBMCU_BQ25180_OVERRIDES_H */
