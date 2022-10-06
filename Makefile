NAME := philosophers

SRC := main.c

$(NAME): $(SRC) all

all:
	gcc $(SRC) -fsanitize=address -g -o $(NAME)

clean:
	rm $(NAME)

re: clean $(NAME)