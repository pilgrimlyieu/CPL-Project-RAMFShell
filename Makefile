.PHONY: all run binary clean submit git gdb

INC_PATH := include/
TOKEN := $(shell cat TOKEN)

all: compile

compile:
	@gcc -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell

test_address:
	@clang -fsanitize=address -fno-omit-frame-pointer -ftrapv -Wall -fdiagnostics-color=always -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell
	@./ramfs-shell

test_undefined:
	@clang -fsanitize=undefined -fno-omit-frame-pointer -ftrapv -Wall -fdiagnostics-color=always -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell
	@./ramfs-shell

test_leak:
	@clang -fsanitize=leak -fno-omit-frame-pointer -ftrapv -Wall -fdiagnostics-color=always -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell
	@./ramfs-shell

test_memory:
	@clang -fsanitize=memory -fno-omit-frame-pointer -ftrapv -Wall -fdiagnostics-color=always -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell
	@./ramfs-shell

test_thread:
	@clang -fsanitize=thread -fno-omit-frame-pointer -ftrapv -Wall -fdiagnostics-color=always -g -std=c17 -O2 -I$(INC_PATH) main.c fs/ramfs.c sh/shell.c -o ramfs-shell
	@./ramfs-shell

run: compile
	@./ramfs-shell

gdb: compile
	gdb ramfs-shell

git:
	@git add -A
	@git commit --allow-empty -m "compile"

clean:
	@rm test

submit:
	$(eval TEMP := $(shell mktemp -d))
	$(eval BASE := $(shell basename $(CURDIR)))
	$(eval FILE := ${TEMP}/${TOKEN}.zip)
	@cd .. && zip -qr ${FILE} ${BASE}/.git
	@echo "Created submission archive ${FILE}"
	@curl -m 15 -w "\n" -X POST -F "TOKEN=${TOKEN}" -F "FILE=@${FILE}" \
		https://public.oj.cpl.icu/api/v2/submission/lab
	@rm -r ${TEMP}