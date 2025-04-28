SRC = newZombie.cpp randomChump.cpp Zombie.cpp main.cpp
OBJ = $(SRC:.cpp=.o)

CFLAGS = -Wall -Werror -Wextra -std=c++98 
CC = c++

NAME = Zombie

all: $(NAME)

$(NAME): $(OBJ)
		$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
		$(CC) $(CFLAGS) -c $< -o $@

clean :
		rm -rf $(OBJ)

fclean: clean
		rm -rf $(NAME)

re: fclean all