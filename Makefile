CC = gcc
CFLAGS = -O2 -fPIC -Ilib/include
LDFLAGS = -lfl -Wl,-rpath=.
LIB=libweak_compiler.so

.PHONY: test clean build_dir all
all: build_dir test_files lex_file $(LIB) tests

build_dir:
	@! [[ -d build ]] \
	    && mkdir -p build/test_inputs \
	    || echo "Build direcory already exists..."

test_files:
	@cp -r tests/{front,back}_end/input/* build/test_inputs

lex_file:
	@flex --outfile=build/lex.yy.c lex/grammar.lex

SOURCES = $(shell find lib -name '*.c')
SOURCES += build/lex.yy.c

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o build/$(notdir $@)

OBJECTS = $(SOURCES:.c=.o)
LIB_DEPENDENCIES = $(addprefix build/,$(notdir $(SOURCES:.c=.o)))

# Why the fuck $(OBJECTS)???
$(LIB): $(OBJECTS)
	$(CC) $(CFLAGS) $(LIB_DEPENDENCIES) -shared -o build/$(LIB) $(LDFLAGS)

# Holy fuck
tests: analysis_test ast_storage_test ast_dump_test data_type_test tok_test parse_test

analysis_test: tests/front_end/analysis/analysis.c
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/analysis_test -Lbuild -lweak_compiler $(LDFLAGS)

ast_storage_test: tests/front_end/analysis/ast_storage.c
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/ast_storage_test -Lbuild -lweak_compiler $(LDFLAGS)

ast_dump_test: tests/front_end/ast/ast_dump.c
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/ast_dump_test -Lbuild -lweak_compiler $(LDFLAGS)

data_type_test: tests/front_end/lex/data_type.c
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/data_type_test -Lbuild -lweak_compiler $(LDFLAGS)

tok_test: tests/front_end/lex/tok.c
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/tok_test -Lbuild -lweak_compiler $(LDFLAGS)

parse_test: tests/front_end/parse/parse.c
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/parse_test -Lbuild -lweak_compiler $(LDFLAGS)

test:
	@(cd build; ./analysis_test)
	@(cd build; ./ast_storage_test)
	@(cd build; ./ast_dump_test)
	@(cd build; ./data_type_test)
	@(cd build; ./tok_test)
	@(cd build; ./parse_test)

clean:
	@rm -rf build $(OBJECTS) *.o
	@echo "Done"

