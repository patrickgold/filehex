# makefile for creating filexhex in linux

help:
	@echo run \'make build\' for building this project.

build:	filehex/filehex.c
	gcc filehex/filehex.c -o fhex -Wall

cleanup:
	rm fhex
