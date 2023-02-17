CC = gcc
CFLAGS = -O0 -ggdb   -fPIC -Ilib/include
LDFLAGS = -lfl -Wl,-R
LIB=libweak_compiler.so

.PHONY: test clean all
all: build_dir test_files $(LIB) tests

build_dir:
	! [[ -d build ]] \
	    && mkdir -p build/test_inputs \
	    || echo "Build direcory already exists..."; \
	flex --outfile=build/lex.yy.c lex/grammar.lex

test_files: | build_dir
	cp -r tests/{front,back}_end/input/* build/test_inputs

SOURCES = $(shell find lib -name '*.c')
SOURCES += build/lex.yy.c

%.o: %.c
	$(CC) -c $(CFLAGS) $^ -o build/$(notdir $@)

OBJECTS = $(SOURCES:.c=.o)
LIB_DEPENDENCIES = $(addprefix build/,$(notdir $(SOURCES:.c=.o)))

# Why the fuck $(OBJECTS)???
$(LIB): $(OBJECTS) | build_dir
	$(CC) $(CFLAGS) $(LIB_DEPENDENCIES) -shared -o build/$(LIB) $(LDFLAGS)

# Holy fuck
tests: analysis_test ast_storage_test ast_dump_test data_type_test tok_test parse_test code_gen_test

analysis_test: tests/front_end/analysis/analysis.c | $(LIB)
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/analysis_test -Lbuild -lweak_compiler $(LDFLAGS)

ast_storage_test: tests/front_end/analysis/ast_storage.c | $(LIB)
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/ast_storage_test -Lbuild -lweak_compiler $(LDFLAGS)

ast_dump_test: tests/front_end/ast/ast_dump.c | $(LIB)
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/ast_dump_test -Lbuild -lweak_compiler $(LDFLAGS)

data_type_test: tests/front_end/lex/data_type.c | $(LIB)
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/data_type_test -Lbuild -lweak_compiler $(LDFLAGS)

tok_test: tests/front_end/lex/tok.c | $(LIB)
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/tok_test -Lbuild -lweak_compiler $(LDFLAGS)

parse_test: tests/front_end/parse/parse.c | $(LIB)
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/parse_test -Lbuild -lweak_compiler $(LDFLAGS)

code_gen_test: tests/back_end/code_gen/code_gen.c | $(LIB)
	$(CC) -Itests $(CFLAGS) $^ build/lex.yy.o -o build/code_gen_test -Lbuild -lweak_compiler $(LDFLAGS)

test:
	@(cd build; LD_LIBRARY_PATH=. ./analysis_test)
	@(cd build; LD_LIBRARY_PATH=. ./ast_storage_test)
	@(cd build; LD_LIBRARY_PATH=. ./ast_dump_test)
	@(cd build; LD_LIBRARY_PATH=. ./data_type_test)
	@(cd build; LD_LIBRARY_PATH=. ./tok_test)
	@(cd build; LD_LIBRARY_PATH=. ./parse_test)
	@(cd build; LD_LIBRARY_PATH=. ./code_gen_test)

clean:
	@rm -rf build $(OBJECTS) *.o
	@echo "Done"

