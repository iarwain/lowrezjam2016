# GNU Make solution makefile autogenerated by Premake
# Type "make help" for usage help

ifndef config
  config=debug64
endif
export config

PROJECTS := lrj

.PHONY: all clean help $(PROJECTS)

all: $(PROJECTS)

lrj: 
	@echo "==== Building lrj ($(config)) ===="
	@${MAKE} --no-print-directory -C . -f lrj.make

clean:
	@${MAKE} --no-print-directory -C . -f lrj.make clean

help:
	@echo "Usage: make [config=name] [target]"
	@echo ""
	@echo "CONFIGURATIONS:"
	@echo "   debug64"
	@echo "   release64"
	@echo "   debug32"
	@echo "   release32"
	@echo ""
	@echo "TARGETS:"
	@echo "   all (default)"
	@echo "   clean"
	@echo "   lrj"
	@echo ""
	@echo "For more information, see http://industriousone.com/premake/quick-start"
