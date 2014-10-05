/*
 * mon_register6809.c - The VICE built-in monitor 6809 register functions.
 *
 * Written by
 *  Olaf Seibert <rhialto@falu.nl>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

/* #define DEBUG_MON_REGS */

#include "vice.h"

#include <stdio.h>
#include <string.h>

#include "lib.h"
#include "log.h"
#include "mon_register.h"
#include "mon_util.h"
#include "montypes.h"
#include "uimon.h"
#include "h6809regs.h"

#ifdef DEBUG_MON_REGS
#define DBG(_x_) printf _x_
#else
#define DBG(_x_)
#endif

static mon_reg_list_t mon_reg_list_6809[11] = {
    {       "X",     e_X, 16,                      0, 0, &mon_reg_list_6809[1], 0 },
    {       "Y",     e_Y, 16,                      0, 0, &mon_reg_list_6809[2], 0 },
    {       "U",     e_U, 16,                      0, 0, &mon_reg_list_6809[3], 0 },
    {       "S",    e_SP, 16,                      0, 0, &mon_reg_list_6809[4], 0 },
    {      "PC",    e_PC, 16,                      0, 0, &mon_reg_list_6809[5], 0 },
    {      "DP",    e_DP,  8,                      0, 0, &mon_reg_list_6809[6], 0 },
    {      "CC", e_FLAGS,  8,                      0, 0, &mon_reg_list_6809[7], 0 },
    {"EFHINZVC", e_FLAGS,  8,  MON_REGISTER_IS_FLAGS, 0, &mon_reg_list_6809[8], 0 },
    {       "A",     e_A,  8,                      0, 0, &mon_reg_list_6809[9], 0 },
    {       "B",     e_B,  8,                      0, 0, &mon_reg_list_6809[10], 0 },
    {       "D",     e_D, 16,                      0, 0, NULL, 0 }
};

/* TODO: this function is generic, move it into mon_register.c and remove
         mon_register_valid from the monitor_cpu_type_t struct
*/
/* check if register id is valid, returns 1 on valid, 0 on invalid */
static int mon_register_valid(int mem, int reg_id)
{
    mon_reg_list_t *mon_reg_list, *regs;
    int ret = 0;

    DBG(("mon_register_valid mem: %d id: %d\n", mem, reg_id));

    if (monitor_diskspace_dnr(mem) >= 0) {
        if (!check_drive_emu_level_ok(monitor_diskspace_dnr(mem) + 8)) {
            return 0;
        }
    }

    mon_reg_list = regs = mon_register_list_get(mem);

    do {
        if ((!(regs->flags & MON_REGISTER_IS_MEMORY)) && (regs->id == reg_id)) {
            ret = 1;
            break;
        }
        regs = regs->next;
    } while (regs != NULL);

    lib_free(mon_reg_list);

    return ret;
}

static unsigned int mon_register_get_val(int mem, int reg_id)
{
    h6809_regs_t *reg_ptr;

    if (monitor_diskspace_dnr(mem) >= 0) {
        if (!check_drive_emu_level_ok(monitor_diskspace_dnr(mem) + 8)) {
            return 0;
        }
    }

    reg_ptr = mon_interfaces[mem]->h6809_cpu_regs;

    switch (reg_id) {
        case e_X:
            return H6809_REGS_GET_X(reg_ptr);
        case e_Y:
            return H6809_REGS_GET_Y(reg_ptr);
        case e_U:
            return H6809_REGS_GET_U(reg_ptr);
        case e_SP:
            return H6809_REGS_GET_S(reg_ptr);
        case e_PC:
            return H6809_REGS_GET_PC(reg_ptr);
        case e_DP:
            return H6809_REGS_GET_DP(reg_ptr);
        case e_FLAGS:
            return H6809_REGS_GET_CC(reg_ptr);
        case e_A:
            return H6809_REGS_GET_A(reg_ptr);
        case e_B:
            return H6809_REGS_GET_B(reg_ptr);
        case e_D:
            return H6809_REGS_GET_D(reg_ptr);
        default:
            log_error(LOG_ERR, "Unknown register!");
    }
    return 0;
}

static void mon_register_set_val(int mem, int reg_id, WORD val)
{
    h6809_regs_t *reg_ptr;

    if (monitor_diskspace_dnr(mem) >= 0) {
        if (!check_drive_emu_level_ok(monitor_diskspace_dnr(mem) + 8)) {
            return;
        }
    }

    reg_ptr = mon_interfaces[mem]->h6809_cpu_regs;

    switch (reg_id) {
        case e_X:
            H6809_REGS_SET_X(reg_ptr, val);
            break;
        case e_Y:
            H6809_REGS_SET_Y(reg_ptr, val);
            break;
        case e_U:
            H6809_REGS_SET_U(reg_ptr, val);
            break;
        case e_SP:
            H6809_REGS_SET_S(reg_ptr, val);
            break;
        case e_PC:
            H6809_REGS_SET_PC(reg_ptr, val);
            break;
        case e_DP:
            H6809_REGS_SET_DP(reg_ptr, (BYTE)val);
            break;
        case e_FLAGS:
            H6809_REGS_SET_CC(reg_ptr, (BYTE)val);
            break;
        case e_A:
            H6809_REGS_SET_A(reg_ptr, (BYTE)val);
            break;
        case e_B:
            H6809_REGS_SET_B(reg_ptr, (BYTE)val);
            break;
        case e_D:
            H6809_REGS_SET_D(reg_ptr, val);
            break;
        default:
            log_error(LOG_ERR, "Unknown register!");
            return;
    }
    force_array[mem] = 1;
}

static void mon_register_print(int mem)
{
    h6809_regs_t *regs;

    if (monitor_diskspace_dnr(mem) >= 0) {
        if (!check_drive_emu_level_ok(monitor_diskspace_dnr(mem) + 8)) {
            return;
        }
    } else if (mem != e_comp_space) {
        log_error(LOG_ERR, "Unknown memory space!");
        return;
    }
    regs = mon_interfaces[mem]->h6809_cpu_regs;

    mon_out("  ADDR A  B  X    Y    SP   U    DP EFHINZVC\n");
    mon_out(".;%04x %02x %02x %04x %04x %04x %04x %02x %c%c%c%c%c%c%c%c\n",
            addr_location(mon_register_get_val(mem, e_PC)),
            mon_register_get_val(mem, e_A),
            mon_register_get_val(mem, e_B),
            mon_register_get_val(mem, e_X),
            mon_register_get_val(mem, e_Y),
            mon_register_get_val(mem, e_SP),
            mon_register_get_val(mem, e_U),
            mon_register_get_val(mem, e_DP),
            (H6809_REGS_TEST_E(regs) ? '1' : '.'),
            (H6809_REGS_TEST_F(regs) ? '1' : '.'),
            (H6809_REGS_TEST_H(regs) ? '1' : '.'),
            (H6809_REGS_TEST_I(regs) ? '1' : '.'),
            (H6809_REGS_TEST_N(regs) ? '1' : '.'),
            (H6809_REGS_TEST_Z(regs) ? '1' : '.'),
            (H6809_REGS_TEST_V(regs) ? '1' : '.'),
            (H6809_REGS_TEST_C(regs) ? '1' : '.')
            );
}

static const char* mon_register_print_ex(int mem)
{
    static char buff[80];
    h6809_regs_t *regs;

    if (monitor_diskspace_dnr(mem) >= 0) {
        if (!check_drive_emu_level_ok(monitor_diskspace_dnr(mem) + 8)) {
            return "";
        }
    } else if (mem != e_comp_space) {
        log_error(LOG_ERR, "Unknown memory space!");
        return "";
    }

    regs = mon_interfaces[mem]->h6809_cpu_regs;

    sprintf(buff, "A:%02X B:%02X X:%04X Y:%04X SP:%04X U:%04X DP:%02x %c%c%c%c%c%c%c%c",
            mon_register_get_val(mem, e_A),
            mon_register_get_val(mem, e_B),
            mon_register_get_val(mem, e_X),
            mon_register_get_val(mem, e_Y),
            mon_register_get_val(mem, e_SP),
            mon_register_get_val(mem, e_U),
            mon_register_get_val(mem, e_DP),
            H6809_REGS_TEST_E(regs) ? 'E' : '.',
            H6809_REGS_TEST_F(regs) ? 'F' : '.',
            H6809_REGS_TEST_H(regs) ? 'H' : '.',
            H6809_REGS_TEST_I(regs) ? 'I' : '.',
            H6809_REGS_TEST_N(regs) ? 'N' : '.',
            H6809_REGS_TEST_Z(regs) ? 'Z' : '.',
            H6809_REGS_TEST_V(regs) ? 'V' : '.',
            H6809_REGS_TEST_C(regs) ? 'C' : '.'
            );

    return buff;
}

/* TODO: try to make this a generic function, move it into mon_register.c and
         remove mon_register_list_get from the monitor_cpu_type_t struct */
static mon_reg_list_t *mon_register_list_get6809(int mem)
{
    mon_reg_list_t *mon_reg_list, *regs;

    mon_reg_list = regs = lib_malloc(sizeof(mon_reg_list_t) * 11);
    memcpy(mon_reg_list, mon_reg_list_6809, sizeof(mon_reg_list_t) * 11);

    do {
        regs->val = (unsigned int)mon_register_get_val(mem, regs->id);
        regs = regs->next;
    } while (regs != NULL);

    return mon_reg_list;
}

void mon_register6809_init(monitor_cpu_type_t *monitor_cpu_type)
{
    monitor_cpu_type->mon_register_get_val = mon_register_get_val;
    monitor_cpu_type->mon_register_set_val = mon_register_set_val;
    monitor_cpu_type->mon_register_print = mon_register_print;
    monitor_cpu_type->mon_register_print_ex = mon_register_print_ex;
    monitor_cpu_type->mon_register_list_get = mon_register_list_get6809;
    monitor_cpu_type->mon_register_valid = mon_register_valid;
}
