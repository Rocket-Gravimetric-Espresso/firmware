init:
	west init -l app
	west update

build:
	west build -b nrf52840dk_nrf52840 app

rebuild:
	rm -rf build
	west build -b nrf52840dk_nrf52840 app

flash:
	west flash

reset:
	nrfjprog --reset
