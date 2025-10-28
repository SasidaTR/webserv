NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -MMD -MP
SRC = source/main.cpp \
	source/http/HttpServer.cpp \
	source/http/Request.cpp \
	source/http/Response.cpp \
	source/http/Router.cpp \
	source/http/handlers/DeleteHandler.cpp \
	source/http/handlers/DirectoryHandler.cpp \
	source/http/handlers/StaticFileHandler.cpp \
	source/http/handlers/UploadHandler.cpp \
	source/configuration/configParse.cpp \
	source/configuration/getlinefd.cpp \
	source/configuration/parseLocation.cpp \
	source/cgi/CGIHandler.cpp \
	source/cgi/CGIProcess.cpp \
	source/cgi/CGIResponseBuilder.cpp \
	source/cgi/CGIEnvironment.cpp \
	source/cgi/CGIUtils.cpp \
	source/cgi/alt_cgi.cpp
OBJDIR = objets
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))
DEP = $(OBJ:.o=.d)
INC = -I include

GREEN = \033[0;32m
YELLOW = \033[0;33m
RED = \033[0;31m
RESET = \033[0m

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(INC) $(OBJ) -o $(NAME)
	@echo "$(GREEN)$(NAME) compiled$(RESET)"

$(OBJDIR)/.compiling:
	@echo "$(YELLOW)Compiling$(RESET)"
	@mkdir -p $(OBJDIR)
	@touch $@

$(OBJDIR)/%.o: %.cpp $(OBJDIR)/.compiling
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
	@echo "$(RED)Cleaning$(RESET)"
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include $(DEP)
