all: cush.cpp
	clang++ cush.cpp -W -Wall -Wextra -pedantic -L/usr/include -lreadline -o cush
