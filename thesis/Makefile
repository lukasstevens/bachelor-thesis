FILE := main
OUT  := build

.PHONY: pdf
pdf:
	latexmk -bibtex -interaction=nonstopmode -outdir=$(OUT) -pdf -halt-on-error $(FILE)

.PHONY: watch
watch:
	latexmk -bibtex -interaction=nonstopmode -outdir=$(OUT) -pdf -pvc -halt-on-error $(FILE)

.PHONY: clean
clean:
	rm -rf $(filter-out $(OUT)/$(FILE).pdf, $(wildcard $(OUT)/*))

.PHONY: purge
purge:
	rm -rf $(OUT)
