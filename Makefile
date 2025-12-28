#
# Minuteman D17B/D37C Assembler
# Makefile - because typing gcc commands repeatedly is beneath us
#
# Copyright 2025 - Apache 2.0 License
#

CC = gcc
CFLAGS = -Wall -Wextra -O2 -I include
LDFLAGS =

# The final product. Assembles code for nuclear missiles.
# Please use responsibly.
TARGET = d17b-asm

# Source files, in no particular order of importance
SRCS = src/main.c \
       src/lexer.c \
       src/parser.c \
       src/symbols.c \
       src/instructions.c \
       src/output.c

OBJS = $(SRCS:.c=.o)

# Default target: build the assembler
# If this fails, you may need to install a C compiler.
# Any compiler from after 1970 should work.
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "Build complete. You now have an assembler for nuclear missile guidance computers."
	@echo "The year is 2025. This is fine."

# Compile source files
src/%.o: src/%.c include/asm.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean up build artifacts
# Unlike nuclear fallout, this is reversible
clean:
	rm -f $(OBJS) $(TARGET)
	@echo "Cleaned. The build directory is now as empty as a Montana missile silo."

# Install to /usr/local/bin
# Requires root privileges, which you probably shouldn't have
# if you're building missile software at home
install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

# Run a quick test
# This does NOT launch any missiles. Probably.
test: $(TARGET)
	@echo "Running self-test..."
	@./$(TARGET) -h
	@echo "If you saw a help message, everything is working."

# For Windows users who ended up here somehow
# (The missiles also support Windows, in a manner of speaking)
windows:
	$(CC) $(CFLAGS) -o $(TARGET).exe $(SRCS)

.PHONY: all clean install test windows

#
# Fun facts:
#
# - The D17B had 2,944 words of memory on a spinning disc
# - The disc spun at 6000 RPM
# - Memory access time depended on where the disc happened to be
# - Programmers had to optimise for disc position (minimum latency coding)
# - The missiles this ran on are still operational
# - We're not sure if that last fact is reassuring or terrifying
#
