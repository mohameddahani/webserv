CXX = c++
CXXFLAGS =  -Wall -Wextra -Werror -std=c++98 
NAME = webserv

SRCS = 	main.cpp \
		./server/server.cpp \
		./request/request.cpp \
		./parse_config_file/parse_config_file.cpp \
		./response/response.cpp \
		./response/initMimeTypes.cpp \
		./webserv.cpp


OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
		$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

clean: 
	rm -rf $(OBJS)

fclean: clean
		rm -rf $(NAME)

re: fclean all

.SECONDARY: $(OBJS)
