##################################
# Compiler flags                 #
##################################
CFLAGS  = -I../tests -I../lib
LDFLAGS = -L../build/lib -lweak_compiler -lfl
BIN     = weak_compiler
SRC     = compiler.c
OBJ     = $(SRC:.c=.o)

##################################
# Targets                        #
##################################
all: $(OBJ)

$(OBJ): $(SRC)
	@echo [CC] $(@F)
	@$(CC) $(CFLAGS) $(@:.o=.c) -o ../build/bin/$(notdir $(@:.o=)) $(LDFLAGS)