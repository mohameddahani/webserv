NAME = webserv

SRCS = main.cpp webserv.cpp ./server/server.cpp ./request/request.cpp ./response/response.cpp

OBJS = $(SRCS:.cpp=.o)

CPP = c++

CPPFLAGS = -Wall -Wextra -Werror -std=c++98

RM = rm -rf

%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CPP) $(CPPFLAGS) -o $(NAME) $(OBJS)

all: $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re