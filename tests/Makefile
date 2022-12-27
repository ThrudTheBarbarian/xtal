SRCS 	= $(wildcard *.xt)
BINS	= $(SRCS:.xt=.exe)

XC		= xtal

##############################################################################
# atarisim from https://github.com/dmsc/mini65-sim
##############################################################################
SIM		= atarisim

all: $(BINS)

redo: clean all

clean:
	@echo "Cleaning up ..."
	@$(RM) -f $(BINS) *.out
	
%.exe : %.xt
	- @$(XC) $< -o $@ > $*.out 2>&1 || true
	- @$(SIM) $@ >> $*.out >> $*.out 2>&1 || true
	@cmp -s $*.out expected/$*.run; \
	RETVAL=$$?; \
	printf "%-28s" $<; \
	if [ $$RETVAL -eq 0 ]; then \
		$(call print, 2, " PASS"); \
	else \
		$(call print, 1, " FAIL"); \
	fi
	
.PHONY = all


define print
      tput setaf $1 ; echo $2 ; tput sgr0
endef

