lstune: lstune.cc filter.cc notes.cc
	g++ -s -O2 notes.cc filter.cc lstune.cc -Wall -I. -l asound -l ncurses -o lstune

all: filter_test notes_test lstune

clean:
	rm -fv filter_test notes lstune notes_test

