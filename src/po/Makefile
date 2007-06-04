# LOCALEDIR, RM, INSTALL defined in parent makefile

MOFILES = bg.mo ca.mo de.mo el.mo es.mo fi.mo fr.mo hr.mo it.mo nb.mo nl.mo nn.mo pl.mo pt.mo ro.mo ru.mo sk.mo sr@latin.mo sr@Latn.mo sq.mo sv.mo uk.mo zh_CN.mo zh_TW.mo

all: $(MOFILES) Makefile
	
# static pattern rule
$(MOFILES): %.mo: %.po
	msgfmt -c -v $< -o $@

clean:
	$(RM) *.mo

# for info about DESTDIR see http://www.gnu.org/prep/standards/html_node/DESTDIR.html

install: all
	for NAME in $(MOFILES); do \
	  $(INSTALL) -v -d $(DESTDIR)$(LOCALEDIR)/`basename $$NAME .mo`/LC_MESSAGES; \
	  $(INSTALL) -v -m 644 $$NAME $(DESTDIR)$(LOCALEDIR)/`basename $$NAME .mo`/LC_MESSAGES/isomaster.mo; \
	done

uninstall:
	for NAME in $(MOFILES); do \
	  $(RM) -v $(DESTDIR)$(LOCALEDIR)/`basename $$NAME .mo`/LC_MESSAGES/isomaster.mo; \
	done
