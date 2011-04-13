/* physlock: sysrq.c
 * Copyright (c) 2011 Bert Muennich <muennich at informatik.hu-berlin.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *  
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *  
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "physlock.h"
#include "sysrq.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define BUF_LEN 32

int get_sysrq_state() {
	char buf[BUF_LEN], *end;
	int len, state;
	FILE *ctl_file;

	ctl_file = fopen(SYSRQ_PATH, "r");
	if (ctl_file == NULL)
		DIE("could not open file: %s", SYSRQ_PATH);

	len = fread(buf, 1, BUF_LEN - 1, ctl_file);
	if (ferror(ctl_file))
		DIE("could not read file: %s: %s", SYSRQ_PATH, strerror(errno));

	fclose(ctl_file);

	buf[len] = '\0';
	state = strtol(buf, &end, 0);
	if (*end && *end != '\n')
		DIE("invalid file content: %s: %s", SYSRQ_PATH, buf);

	return state;
}

void set_sysrq_state(int new_state) {
	char buf[BUF_LEN];
	FILE *ctl_file;

	ctl_file = fopen(SYSRQ_PATH, "w+");
	if (ctl_file == NULL)
		DIE("could not open file: %s", SYSRQ_PATH);

	snprintf(buf, BUF_LEN, "%d\n", new_state);

	fwrite(buf, 1, strlen(buf), ctl_file);
	if (ferror(ctl_file))
		DIE("could not write file: %s: %s", SYSRQ_PATH, strerror(errno));

	fclose(ctl_file);
}
