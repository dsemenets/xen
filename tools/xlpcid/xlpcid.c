/*
    Pcid daemon that acts as a server for the client in the libxl PCI

    Copyright (C) 2021 EPAM Systems Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE  // required for strchrnul()

#include <libxl_utils.h>
#include <libxlutil.h>

//#include "xl.h"
//#include "xl_utils.h"
//#include "xl_parse.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>

#include <xlpcid.h>
#include <xenstore.h>
#include <assert.h>

libxl_ctx *ctx;
xentoollog_logger_stdiostream *logger;
libxl_bitmap global_vm_affinity_mask;
libxl_bitmap global_hvm_affinity_mask;
libxl_bitmap global_pv_affinity_mask;
char *lockfile;
int logfile;

static void help(const char *str)
{
    printf("%s\n", str);
    printf(" --foreground -f      -  run in foreground mode\n");
    printf(" --pidfile <pidfile>  -  specify pid file\n");
}
/*
 * TODO: Running this code in multi-threaded environment
 * Now the code is designed so that only one request to the server
 * from the client is made in one domain. In the future, it is necessary
 * to take into account cases when from different domains there can be
 * several requests from a client at the same time. Therefore, it will be
 * necessary to regulate the multithreading of processes for global variables.
 */
static void xlpcid_ctx_alloc(void)
{
    if (libxl_ctx_alloc(&ctx, LIBXL_VERSION, 0, (xentoollog_logger*)logger)) {                                                                                                                         
        fprintf(stderr, "cannot init xl context\n");
        exit(1);
    }

    libxl_bitmap_init(&global_vm_affinity_mask);
    libxl_bitmap_init(&global_hvm_affinity_mask);
    libxl_bitmap_init(&global_pv_affinity_mask);
//    libxl_childproc_setmode(ctx, &childproc_hooks, 0);
}

static void xlpcid_ctx_free(void)
{
    libxl_bitmap_dispose(&global_pv_affinity_mask);
    libxl_bitmap_dispose(&global_hvm_affinity_mask);
    libxl_bitmap_dispose(&global_vm_affinity_mask);
    if (ctx) {
        libxl_ctx_free(ctx);
        ctx = NULL;
    }
    if (logger) {
        xtl_logger_destroy((xentoollog_logger*)logger);
        logger = NULL;
    }
    if (lockfile) {
        free(lockfile);
        lockfile = NULL;
    }
}

static int do_daemonize(const char *name, const char *pidfile)
{
    char *fullname;
    int nullfd, ret = 0;

    ret = libxl_create_logfile(ctx, name, &fullname);
    if (ret) {
        fprintf(stderr, "Failed to open logfile %s: %s", fullname, strerror(errno));
        exit(-1);
    }

    logfile = open(fullname, O_WRONLY|O_CREAT|O_APPEND, 0644);
    free(fullname);
    assert(logfile >= 3);

    nullfd = open("/dev/null", O_RDONLY);
    assert(nullfd >= 3);

    dup2(nullfd, 0);
    dup2(logfile, 1);
    dup2(logfile, 2);

    close(nullfd);

    if (daemon(0, 1)) {
        perror("daemon");
        close(logfile);
        return 1;
    }

    if (pidfile) {
        int fd = open(pidfile, O_RDWR | O_CREAT, S_IRUSR|S_IWUSR);
        char *pid = NULL;

        if (fd == -1) {
            perror("Unable to open pidfile");
            exit(1);
        }

        if (asprintf(&pid, "%ld\n", (long)getpid()) == -1) {
            perror("Formatting pid");
            exit(1);
        }

        if (write(fd, pid, strlen(pid)) < 0) {
            perror("Writing pid");
            exit(1);
        }

        if (close(fd) < 0) {
            perror("Closing pidfile");
            exit(1);
        }

        free(pid);
    }

    return ret;
}

int main(int argc, char *argv[])
{
    int opt = 0, daemonize = 1, ret;
    const char *pidfile = NULL;
    unsigned int xtl_flags = 0;
    bool progress_use_cr = 0;
    bool timestamps = 0;
    xentoollog_level minmsglevel = XTL_PROGRESS;
    static const struct option opts[] = {
        {"pidfile", 1, 0, 'p'},
        {"foreground", 0, 0, 'f'},
        {"help", 0, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hfitTvp:", opts, NULL)) != -1)
        switch (opt) {
        case 'f':
            daemonize = 0;
            break;
        case 'p':
            pidfile = optarg;
            break;
        case 't':
            timestamps = 1;
            break;
        case 'T':
            progress_use_cr = 1;
            break;
        case 'v':
            if (minmsglevel > 0)
                minmsglevel--;
            break;
        case 'h':
            help("xlpcid");
            exit(1);
            break;
        }

    if (progress_use_cr)
        xtl_flags |= XTL_STDIOSTREAM_PROGRESS_USE_CR;
    if (timestamps)
        xtl_flags |= XTL_STDIOSTREAM_SHOW_DATE | XTL_STDIOSTREAM_SHOW_PID;
    logger = xtl_createlogger_stdiostream(stderr, minmsglevel, xtl_flags);
    if (!logger)
        exit(EXIT_FAILURE);

    xlpcid_ctx_alloc();

    if (daemonize) {
        ret = do_daemonize("xlpcid", pidfile);
        if (ret) {
            ret = (ret == 1) ? 0 : ret;
            goto out_daemon;
        }
    }

    libxl_pcid_process(ctx);

    ret = 0;

out_daemon:
    xlpcid_ctx_free();
    exit(ret);
}
