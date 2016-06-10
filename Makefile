BIN=gif2bmp
LIB=libgif2bmp.a
CCFLAGS=-std=c11 -Wall -Wextra -pedantic
OBJFILES=gif.o gif_print.o gif_dict.o gif_interlance.o gif_lzw.o gif_rgb.o bmp.o gif2bmp.o
LOGIN=xsruba03
ZIP=kko.proj3.xsruba03.zip
TAR=kko.proj3.xsruba03.tar
DIR=$(basename $(ZIP))
DOC=gif2bmp.pdf

all: $(BIN)

debug: CCFLAGS += -g -DDEBUG
debug: $(BIN)

gif.o: gif.c gif.h list.h
	$(CC) $(CCFLAGS) -c $<
gif_print.o: gif_print.c gif_print.h
	$(CC) $(CCFLAGS) -c $<
gif_dict.o: gif_dict.c gif_dict.h
	$(CC) $(CCFLAGS) -c $<
gif_interlance.o: gif_interlance.c gif_interlance.h
	$(CC) $(CCFLAGS) -c $<
gif_lzw.o: gif_lzw.c gif_lzw.h
	$(CC) $(CCFLAGS) -c $<
gif_rgb.o: gif_rgb.c gif_rgb.h
	$(CC) $(CCFLAGS) -c $<
bmp.o: bmp.c bmp.h gif_bits.h
	$(CC) $(CCFLAGS) -c $<
gif2bmp.o: gif2bmp.c gif2bmp.h bmp.o
	$(CC) $(CCFLAGS) -c $<

$(LIB): $(OBJFILES)
	ar crv $@ $(OBJFILES)
$(BIN): main.c $(LIB)
	$(CC) $(CCFLAGS) main.c $(LIB) -o $@

$(DOC):
	make -C doc

check: tests/test.sh $(BIN)
	cp $(BIN) tests
	(cd tests; sh test.sh)
valgrind: tests/test.sh $(BIN)
	cp $(BIN) tests
	(cd tests; sh test.sh valgrind)

zip: $(DOC)
	rm -rf $(DIR)
	mkdir $(DIR)
	cp -a gif.{c,h} gif_print.{c,h} gif_dict.{c,h} gif_interlance.{c,h} gif_lzw.{c,h} gif_rgb.{c,h} bmp.{c,h} gif2bmp.{c,h} gif_bits.h list.h main.c Makefile doc/$(DOC) $(DIR)
	zip -r $(ZIP) $(DIR)
clean:
	make clean -C doc
	rm -rf $(OBJFILES) $(BIN) gif_lzw_test test_gif_dict tests/$(BIN) $(LIB) $(TAR) $(ZIP) $(DIR)
