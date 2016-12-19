/* physlock: issue.c
 * Copyright (c) 2013 Bert Muennich <be.muennich at gmail.com>
 *
 * This file contains functions that are based on agetty(8) by
 * Peter Orbaek <poe@daimi.aau.dk> and Werner Fink <werner@suse.de>.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "physlock.h"
#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <utmpx.h>
#include <sys/utsname.h>
#include <langinfo.h>
#include <time.h>
#include <errno.h>

static void handle_special_char(unsigned char c, vt_t vt, int oldvt, FILE *fp)
{
    struct utsname uts;

    switch (c) {
        case 's':
            uname(&uts);
            fprintf(vt.ios, "%s", uts.sysname);
            break;
        case 'n':
            uname(&uts);
            fprintf(vt.ios, "%s", uts.nodename);
            break;
        case 'r':
            uname(&uts);
            fprintf(vt.ios, "%s", uts.release);
            break;
        case 'v':
            uname(&uts);
            fprintf(vt.ios, "%s", uts.version);
            break;
        case 'm':
            uname(&uts);
            fprintf(vt.ios, "%s", uts.machine);
            break;
        case 'd':
        case 't':
            {
                time_t now;
                struct tm *tm;

                time(&now);
                tm = localtime(&now);

                if (!tm)
                    break;

                if (c == 'd') /* ISO 8601 */
                    fprintf(vt.ios,
                            "%s %s %d  %d",
                            nl_langinfo(ABDAY_1 + tm->tm_wday),
                            nl_langinfo(ABMON_1 + tm->tm_mon),
                            tm->tm_mday,
                            tm->tm_year < 70 ? tm->tm_year + 2000 :
                            tm->tm_year + 1900);
                else
                    fprintf(vt.ios,
                            "%02d:%02d:%02d",
                            tm->tm_hour, tm->tm_min, tm->tm_sec);
                break;
            }
        case 'l':
            fprintf(vt.ios, "tty%d", oldvt);
            break;
        case 'u':
        case 'U':
            {
                int users = 0;
                struct utmpx *ut;
                setutxent();
                while ((ut = getutxent()))
                    if (ut->ut_type == USER_PROCESS)
                        users++;
                endutxent();
                if (c == 'U')
                    fprintf(vt.ios, P_("%d user", "%d users", users), users);
                else
                    fprintf (vt.ios, "%d ", users);
                break;
            }
        default:
            fputc(c, vt.ios);
            break;
    }
}

void print_issue_file(vt_t vt, int oldvt)
{
    FILE *fd;
    if ((fd = fopen(ISSUE_FILE_PATH, "r"))) {
        int c;

        while ((c = getc(fd)) != EOF) {
            if (c == '\\')
                handle_special_char(getc(fd), vt, oldvt, fd);  
            else
                fputc(c, vt.ios);
        }
        fflush(vt.ios);
        fclose(fd);
    }
    else {
        error(EXIT_FAILURE, errno, "Unable to open %s", ISSUE_FILE_PATH);
    }
}
