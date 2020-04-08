all: cush.cpp
	g++ cush.cpp -W -Wall -Wextra -pedantic -L/usr/include -lreadline -o cush
