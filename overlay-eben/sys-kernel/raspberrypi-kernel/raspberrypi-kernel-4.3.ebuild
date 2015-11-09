# Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
# Distributed under the terms of the GNU General Public License v2

EAPI=4

CROS_WORKON_REPO="git://github.com/raspberrypi"
CROS_WORKON_PROJECT="linux"
CROS_WORKON_EGIT_BRANCH="rpi-4.3.y"
CROS_WORKON_BLACKLIST="1"
CROS_WORKON_COMMIT="bd4c88f28a839ed7121bbb0a151f13d89e04dc44"

# This must be inherited *after* EGIT/CROS_WORKON variables defined
inherit git-2 cros-kernel2 cros-workon

DESCRIPTION="Chrome OS Kernel-raspberrypi2-kms"
KEYWORDS="arm"
KERNEL="kernel7"

DEPEND="!sys-kernel/chromeos-kernel-next
	!sys-kernel/chromeos-kernel
"
RDEPEND="${DEPEND}"

#TODO: install dtbs from source build.
src_install() {
	cros-kernel2_src_install
        "${FILESDIR}/mkknlimg" \
                "$(cros-workon_get_build_dir)/arch/arm/boot/zImage" \
                "${T}/dtImage"

        insinto /boot
        doins "${FILESDIR}"/{cmdline,config}.txt
        doins "${T}/dtImage"
	doins "${FILESDIR}/bcm2709-rpi-2-b.dtb"
}
