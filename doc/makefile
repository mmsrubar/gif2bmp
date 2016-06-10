NAME=gif2bmp

$(NAME).pdf: $(NAME).dvi
	dvipdf $(NAME).dvi

$(NAME).dvi: $(NAME).tex
	vlna -l -m -n $(NAME).tex
	latex  -shell-escape $(NAME).tex
	latex  -shell-escape $(NAME).tex

clean:
	rm -rf $(NAME).dvi $(NAME).aux $(NAME).toc $(NAME).log $(NAME).out
