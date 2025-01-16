init:
	west init -l app
	west update

build-nordic:
	rm -fr build
	west build -b nrf52840dk/nrf52840 app

build-xiao:
	rm -fr build
	west build -b xiao_custom_pcb app -DBOARD_ROOT=./

flash-nordic:
	west flash

flash-xiao:
	west flash -r uf2

reset:
	nrfjprog --reset
