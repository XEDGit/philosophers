NAME := philosophers

SRC_F := src

SRC := main.c

SRCS := $(addprefix $(SRC_F)/, $(SRC))

OBJ_F := obj

OBJ := $(SRC:.c=.o)

OBJS := $(addprefix $(OBJ_F)/, $(OBJ))

SRC_FLAGS := -Wall -Wextra -Werror -c -g

OBJ_FLAGS := -fsanitize=address

all: $(NAME)

$(NAME): $(OBJS)
	gcc $(OBJS) -o $(NAME)

$(OBJS): $(SRCS)
	mkdir -p $(OBJ_F)/
	gcc $(SRC_FLAGS) $(SRCS) 
	mv $(OBJ) $(OBJ_F)/

clean:
	rm -rf $(OBJ_F)/

fclean:
	rm -rf $(OBJ_F)/ $(NAME)

re: fclean $(NAME)
