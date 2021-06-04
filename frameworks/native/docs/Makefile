HEADERS := $(wildcard ../include/android/*.h)

all: html website

html: $(HEADERS) Doxyfile
	mkdir -p html
	doxygen

website: $(HEADERS) Doxyfile header.html
	mkdir -p website
	HTML_HEADER=header.html HTML_FOOTER=footer.html HTML_OUTPUT=website doxygen
	rm -f website/index.html
