all:
	$(MAKE) -C common
	$(MAKE) -C client
	$(MAKE) -C server
clean:
	$(MAKE) -C common clean
	$(MAKE) -C client clean
	$(MAKE) -C server clean

.PHONY: all clean
