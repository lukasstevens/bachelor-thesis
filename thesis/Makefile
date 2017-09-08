file := main
out  := build
options := -bibtex -interaction=nonstopmode -halt-on-error -pdf -pdflatex="pdflatex -shell-escape %O %S"

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
	rm -rf figures/**/*.aux figures/**/*.log figures/**/*.out figures/**/*.pdf

.PHONY: purge
purge:
	rm -rf $(out)
	rm -rf figures/**/*.aux figures/**/*.log figures/**/*.pdf
