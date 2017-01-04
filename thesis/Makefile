FILE = thesis
BUILDDIR = build

all: $(FILE).pdf

clean: 
	rm -rf $(BUILDDIR)

watch:
	latexmk -pdf -pvc -outdir=$(BUILDDIR) $(FILE).tex

$(FILE).pdf: $(FILE).tex
	latexmk -pdf -outdir=$(BUILDDIR) $(FILE).tex

.PHONY: all clean watch
