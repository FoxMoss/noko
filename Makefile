# I decided to make another one of my make macro files.
# yes you can figure these commands out on your own.
# I am just lazy and don't want to write a script.

.PHONY: clean xephyr run


default: build/
	cmake --build build

configure:
	cmake -S . -B build

# Can't firgure out how to make a makefile work nicely for noko orchestration
# without writing a script so for now these run targets are more or less pointless.
# run: run-xephyr

run-xephyr:
	Xephyr -br -ac -noreset -screen 1000x500 :2 & \
	Xephyr :2 -screen 800x600 & \
	sleep 1 && \
	DISPLAY=:2 ./build/noko-desktop &

rnWM:
	cd ./build
	DISPLAY=:2 ./build/noko

rnDSK:
	cd ./build
	DISPLAY=:2 ./build/noko-desktop
	#TODO:
	# relative file indexing makes noko-desktop fail. We will have to make it search the system for fonts or use an absolute env configure via cmake instead.
	# DISPLAY=:2 ./build/noko-desktop & \


#careful, redownloading all of the libs takes a while.
clean:
	rm -rf build
