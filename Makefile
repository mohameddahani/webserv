# Executable name
NAME     = webserv

# Colors
CYAN   = \033[0;36m
YELLOW = \033[0;33m
RED    = \033[0;31m
GREEN  = \033[0;32m
RESET  = \033[0m

# Compiler & it's flags
CXX      = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

# Directories
SRC_DIR  = src
OBJ_DIR  = obj

# Source files
SRCS     =  $(SRC_DIR)/main.cpp \
			$(SRC_DIR)/Webserv.cpp \
			$(SRC_DIR)/configParser/configParser.cpp \
			$(SRC_DIR)/server/Listener.cpp \
			$(SRC_DIR)/server/Epoll.cpp \
			$(SRC_DIR)/server/ConnectionManager.cpp \
			$(SRC_DIR)/server/Server.cpp \
			$(SRC_DIR)/Client.cpp \
			$(SRC_DIR)/cgi/CgiHandler.cpp \
			$(SRC_DIR)/cgi/CgiUtils.cpp \
			$(SRC_DIR)/http/Request.cpp \
			$(SRC_DIR)/http/Response.cpp \
			$(SRC_DIR)/http/initMimeTypes.cpp \
			$(SRC_DIR)/utils/Logger.cpp \
			$(SRC_DIR)/utils/Helper.cpp

# Object files
OBJS     = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)


# Create object directory structure
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(CYAN)🔨 Compiling $<$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@


# Rules
all: $(NAME)

$(NAME): $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)✅ webserv compiled successfully$(RESET)"

clean: 
	@rm -rf $(OBJ_DIR)
	@echo "$(YELLOW)🧹 Object files cleaned$(RESET)"

fclean: clean
	@rm -rf $(NAME)
	@echo "$(RED)🧹 Binary cleaned$(RESET)"

re: fclean all

.SECONDARY: $(OBJS)
