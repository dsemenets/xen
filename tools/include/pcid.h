/*
    Common definitions for Xen PCI client-server protocol.
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

#ifndef PCID_H
#define PCID_H

#define PCID_XS_DIR             "/local/domain/"
#define PCID_XS_PATH            "/data/pcid-vchan"

#define PCI_RECEIVE_BUFFER_SIZE 4096
#define PCI_MAX_SIZE_RX_BUF     MB(1)

#define PCID_MSG_FIELD_ID        "id"
#define PCID_MSG_FIELD_ARGS      "arguments"

#define PCID_PCIBACK_DRIVER      "pciback_driver"

#if defined(__linux__)
#define SYSFS_PCIBACK_DRIVER   "/sys/bus/pci/drivers/pciback"
#endif

int libxl_pcid_process(libxl_ctx *ctx);

#endif /* PCID_H */

/*
 * Local variables:
 *  mode: C
 *  c-file-style: "linux"
 *  indent-tabs-mode: t
 *  c-basic-offset: 8
 *  tab-width: 8
 * End:
 */
