# Copyright 1999-2006 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

DESCRIPTION="GTK2 (bootable) CD ISO Image editor."
HOMEPAGE="http://littlesvr.ca/isomaster/"
SRC_URI="http://littlesvr.ca/isomaster/releases/${P}.tar.bz2"

LICENSE="GPL-2"
KEYWORDS="~x86"

SLOT="0"
IUSE=""
S="${WORKDIR}/${PN}"

DEPEND=">=x11-libs/gtk+-2.0"

src_compile() {
    emake PREFIX="/usr" || die "emake failed"
}

src_install() {
    emake PREFIX="/usr" DESTDIR="${D}" install || die "Install failed"

	cd ${S}
	dodoc *.TXT bk/TODO.TXT
}
