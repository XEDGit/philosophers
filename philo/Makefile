NAME := philo

SRC_F := src

SRC := main.c \
	routine.c \
	wait.c \
	philosophers/philo_actions.c \
	philosophers/philo_utils.c \
	philosophers/philo_forks.c \
	utils/argv_utils.c \
	utils/time_utils.c

SRCS := $(addprefix $(SRC_F)/, $(SRC))

OBJ_F := obj

OBJ := $(notdir $(SRC:.c=.o))

OBJS := $(addprefix $(OBJ_F)/, $(OBJ))

SRC_FLAGS := -Wall -Wextra -Werror -Wpedantic -c -Iincludes -g3 -fsanitize=thread

all: $(NAME)

$(NAME): $(OBJS)
	gcc $(OBJS) -o $(NAME) -g3 -fsanitize=thread

$(OBJS): $(SRCS)
	mkdir -p $(OBJ_F)/
	gcc $(SRC_FLAGS) $(SRCS) 
	mv $(OBJ) $(OBJ_F)/

clean:
	rm -rf $(OBJ_F)/

fclean:
	rm -rf $(OBJ_F)/ $(NAME)

re: fclean $(NAME)
