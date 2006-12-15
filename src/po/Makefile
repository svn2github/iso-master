# LOCALEDIR is defined in the top-level makefile

MOFILES = bg.mo de.mo fr.mo pl.mo ru.mo zh_TW.mo

all: $(MOFILES) Makefile
	

bg.mo: bg.po
	msgfmt -c -v bg.po -o bg.mo

de.mo: de.po
	msgfmt -c -v de.po -o de.mo

fr.mo: fr.po
	msgfmt -c -v fr.po -o fr.mo

ru.mo: ru.po
	msgfmt -c -v ru.po -o ru.mo

pl.mo: pl.po
	msgfmt -c -v pl.po -o pl.mo

zh_TW.mo: zh_TW.po
	msgfmt -c -v zh_TW.po -o zh_TW.mo

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
