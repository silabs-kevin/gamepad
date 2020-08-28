build=GNU\ ARM\ v7\.2\.1\ -\ Debug

all:
	bear make -C $(build) -j12 all

flags:
	make -C $(build)

clean:
	make -C $(build) clean && rm -rf compile_commands.json

.PHONY: all clean
