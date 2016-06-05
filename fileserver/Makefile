SUBDIRS = src

all debug release clean depend::
	@for i in $(SUBDIRS); do \
	    (cd $$i && make depend && make $@ ) || exit 1; \
	done

$(SUBDIRS)::
	@cd $@ && make
