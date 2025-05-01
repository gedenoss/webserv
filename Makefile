CC = c++
CFLAGS = -Wall -Werror -Wextra -std=c++98 -g3 -O3
NAME = webserv

SRC_DIR = srcs/
INC_DIR = includes/
BUILD = build/
UPLOAD = upload/

SRC = config.cpp main.cpp request.cpp response.cpp cgi.cpp utils.cpp request_utils.cpp errors.cpp server.cpp path.cpp initResponse.cpp upload.cpp
HEADERS = request.hpp response.hpp server.hpp utils.hpp  errors.hpp config.hpp server.hpp upload.hpp

SRCS = $(addprefix $(SRC_DIR), $(SRC))
OBJS = $(addprefix $(BUILD), $(SRC:.cpp=.o))

all : $(NAME) $(BUILD)
	@echo "\033[0;32mCompilation finished :D\033[0m"

$(BUILD):
	@mkdir -p $(BUILD)
	@mkdir -p $(UPLOAD)

$(NAME): $(BUILD) $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(BUILD)%.o: $(SRC_DIR)%.cpp $(addprefix $(INC_DIR), $(HEADERS))
	@$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

clean:
	@rm -rf build
	@rm -rf upload

fclean: clean
	@rm -f $(NAME)
	@echo "\033[0;32mClean finished ^^\033[0m"

re : fclean all