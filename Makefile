CC := g++-13
CFLAGS := -std=c++20 -Wall

SRCDIR := src
BUILDDIR := build
OUTPUTDIR := build/output
TARGET := $(OUTPUTDIR)/GreedyNMS

# 查找所有.cpp文件
SOURCES := $(wildcard $(SRCDIR)/**/*.cpp)
# 生成对应的.o文件列表
OBJECTS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	@echo " Linking $@"
	@$(CC) $^ -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo " Compiling $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo " Cleaning..."
	@echo " $(RM) -r $(BUILDDIR)"; $(RM) -r $(BUILDDIR)

.PHONY: all clean
