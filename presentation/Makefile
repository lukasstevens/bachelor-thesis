file := presentation
out  := build
options :=-bibtex -interaction=nonstopmode -halt-on-error -pdf -pdflatex="xelatex %O %S"

.PHONY: pdf
pdf:
	latexmk $(options) -outdir=$(out) $(file)

.PHONY: force
force:
	latexmk $(options) -gg -outdir=$(out) $(file)

.PHONY: watch
watch:
	latexmk $(options) -pvc -outdir=$(out) $(file)

.PHONY: clean
clean:
	rm -rf $(filter-out $(out)/$(file).pdf, $(wildcard $(out)/*))

.PHONY: purge
purge:
	rm -rf $(out)
