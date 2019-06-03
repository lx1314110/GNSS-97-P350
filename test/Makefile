.PHONY : all
all :
	$(MAKE) -C fpga
	$(MAKE) -C fpga_addr
	$(MAKE) -C fpga_addr_print
	$(MAKE) -C time_ratio
	$(MAKE) -C time_ratio_ns
	$(MAKE) -C printf_delay
.PHONY : clean
clean :
	$(MAKE) clean -C fpga
	$(MAKE) clean -C fpga_addr
	$(MAKE) clean -C fpga_addr_print
	$(MAKE) clean -C time_ratio
	$(MAKE) clean -C time_ratio_ns
	$(MAKE) clean -C printf_delay
