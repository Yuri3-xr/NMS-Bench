CC := g++
CFLAGS := -std=c++17 -Wall -ffast-math -funroll-loops -O3 
LDFLAGS=-ltbb

SRCDIR := src
BUILDDIR := build
OUTPUTDIR := build/app
TARGET := $(OUTPUTDIR)/main


SOURCES := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	@echo " Linking $@"
	@$(CC) $^ -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo " Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo " Cleaning..."
	@echo " $(RM) -r $(BUILDDIR)"; $(RM) -r $(BUILDDIR)

.PHONY: all clean
