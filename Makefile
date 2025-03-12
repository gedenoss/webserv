CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98 -g3 -O3
NAME = webserv

SRC_DIR = srcs/
INC_DIR = includes/
BUILD = build/

SRC = cgi.cpp config.cpp main.cpp request.cpp response.cpp utils.cpp webserv.cpp
HEADERS = cgi.hpp request.hpp response.hpp server.hpp utils.hpp webserv.hpp config.hpp

SRCS = $(addprefix $(SRC_DIR), $(SRC))
OBJS = $(addprefix $(BUILD), $(SRC:.cpp=.o))

all : $(NAME) $(BUILD)

$(BUILD):
	mkdir -p $(BUILD)

$(NAME): $(BUILD) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(BUILD)%.o: $(SRC_DIR)%.cpp $(addprefix $(INC_DIR), $(HEADERS))
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	rm -rf build

fclean: clean
	rm -f $(NAME)

re : fclean all