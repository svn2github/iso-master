# Copyright 1999-2006 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# $Header: $

DESCRIPTION="Graphical CD Image editor."
HOMEPAGE="http://littlesvr.ca/isomaster/"
SRC_URI="http://littlesvr.ca/isomaster/releases/${P}.tar.bz2"

LICENSE="GPL-2"
KEYWORDS="x86"

SLOT="0"
IUSE=""

DEPEND=">=x11-libs/gtk+-2.0"

src_unpack() {
    unpack ${A}
    cd "${S}"
}

src_compile() {
    emake || die "emake failed"
}

src_install() {
    emake PREFIX="/usr" DESTDIR="${D}" install || die "Install failed"
}
