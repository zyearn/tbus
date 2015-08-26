TARGETS = SHELL

all: $(TARGETS)

SHELL:
	@cd tbusapi && $(MAKE)
	@cd tbusmgr && $(MAKE)

clean:
	@cd tbusapi && $(MAKE) clean
	@cd tbusmgr && $(MAKE) clean
