.SUFFIXES: .dia .tex .pdf

IMAGES = $(patsubst %.dia,%.pdf,$(wildcard *.dia))
PDF = libsf1r.pdf
DOCDIR = ../../../../docs/pdf/

all: images doc

images: $(IMAGES)

doc: $(PDF)

.tex.pdf:
	@echo "pdflatex $<"
	@pdflatex $*.tex >/dev/null && pdflatex $*.tex >/dev/null

.dia.pdf:
	@echo "image $<"
	@dia -t eps $*.dia >/dev/null
	@epstopdf $*.eps
	@touch $(patsubst %.pdf,%.tex,$(PDF))

.PHONY: clean 

clean:
	@rm -f *.bbl *.blg *.aux *.log *.out *.toc
	@rm -f *~ *backup
	@rm -f *.pdf *.eps

copy:
	@echo Copying $(PDF) to $(DOCDIR)
	@cp -i $(PDF) $(DOCDIR)
