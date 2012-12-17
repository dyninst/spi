TEX=/s/texlive-2011/bin/pdflatex
MAIN=spi
DEP=spi.tex \
		section/*.tex \


$(MAIN).pdf: $(DEP)
	$(TEX) $(MAIN)
	bibtex $(MAIN)
	$(TEX) $(MAIN)
	$(TEX) $(MAIN)

clean:
	@rm -f *.dvi *.log *.aux *.ent *.bak *.bbl *.blg *.out *.toc $(MAIN).pdf

read:
	@xpdf spi.pdf
