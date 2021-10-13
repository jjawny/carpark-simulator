# ===================MAKEFILE FOR ALL 3 SOFTWARES===================
all:
	+$(MAKE) -C src-simulator
	+$(MAKE) -C src-manager
	+$(MAKE) -C src-fire-alarm-system
	echo "Done."

clean:
	rm SIMULATOR MANAGER FIRE-ALARM-SYSTEM src-simulator/*.o src-manager/*.o src-fire-alarm-system/*.o

.PHONY: all clean
