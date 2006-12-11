# LOCALEDIR is defined in the top-level makefile

MOFILES = bg.mo

all: $(MOFILES) Makefile
	

bg.mo: bg.po
	msgfmt -c -v bg.po -o bg.mo

clean:
	rm -f *.mo

install: all
	for NAME in $(MOFILES); do \
	  install -v -d $(LOCALEDIR)/`basename $$NAME .mo`/LC_MESSAGES; \
	  install -v -m 644 $$NAME $(LOCALEDIR)/`basename $$NAME .mo`/LC_MESSAGES/isomaster.mo; \
	done

uninstall:
	for NAME in $(MOFILES); do \
	  rm -v -f $(LOCALEDIR)/`basename $$NAME .mo`/LC_MESSAGES/isomaster.mo; \
	done
