CC = g++
CXXFLAGS = -std=c++20 -I./main -I./resource -Wall -MMD -MP
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system
APPNAME = .uvu

SRCDIR = ./main
APPDIR = app
OBJDIR = $(APPDIR)
EXT = .cpp

ifeq ($(OS),Windows_NT)
	RM = del /Q
	MKDIR = if not exist $(subst /,\\,$(1)) mkdir $(subst /,\\,$(1))
	FIND = for /R $(SRCDIR) %%f in (*$(EXT)) do @echo %%f
	SRC := $(shell $(FIND))
else
	RM = rm -rf
	MKDIR = mkdir -p $(1)
	SRC := $(shell find $(SRCDIR) -name "*$(EXT)")
endif

OBJ = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRC))
DEP = $(patsubst $(SRCDIR)/%.cpp,$(APPDIR)/%.d,$(SRC))

all: $(APPNAME)

$(APPNAME): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(call MKDIR,$(dir $@))
	$(CC) $(CXXFLAGS) -c $< -o $@
	- mv $(OBJDIR)/$*.d $(APPDIR)/$(notdir $*).d

-include $(DEP)

.PHONY: clean
clean:
	$(RM) $(APPDIR) $(APPNAME)
