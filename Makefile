CC = g++
CXXFLAGS = -std=c++20 -I./main -I./resource -Wall -MMD -MP
LDFLAGS = -lsfml-graphics -lsfml-window -lsfml-system
APPNAME = .uvu

SRCDIR = ./main
APPDIR = app
OBJDIR = $(APPDIR)
EXT = .cpp

SRC = $(shell find $(SRCDIR) -name "*$(EXT)")
OBJ = $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))
DEP = $(patsubst $(SRCDIR)/%.cpp, $(APPDIR)/%.d, $(SRC))

all: $(APPNAME)

$(APPNAME): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(dir $@)
	$(CC) $(CXXFLAGS) -c $< -o $@
	mv $(OBJDIR)/$*.d $(APPDIR)/$(notdir $*).d

-include $(DEP)

.PHONY: clean
clean:
	rm -rf $(APPDIR) $(APPNAME)