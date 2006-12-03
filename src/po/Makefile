# LOCALEDIR is defined in the top-level makefile

#MOFILES = fr.mo

all: $(MOFILES) Makefile
	

fr.mo: fr.po
	msgfmt -c -v fr.po -o fr.mo
	echo $(LOCALEDIR) a

clean:
	rm -f *.mo

install: all
	for NAME in $(MOFILES); do \
	  install -v -d $(LOCALEDIR)/`basename $$NAME .mo`/LC_MESSAGES; \
	  install -v $$NAME $(LOCALEDIR)/`basename $$NAME .mo`/LC_MESSAGES/isomaster.mo; \
	done

uninstall:
	for NAME in $(MOFILES); do \
	  rm -v -f $(LOCALEDIR)/`basename $$NAME .mo`/LC_MESSAGES/isomaster.mo; \
	done
