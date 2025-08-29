NAME = webserv
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -MMD -MP
SRC = $(wildcard *.cpp source/*.cpp source/*/*.cpp)
OBJDIR = objets
OBJ = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRC))
DEP = $(OBJ:.o=.d)
INC = -I include

GREEN = \033[0;32m
YELLOW = \033[0;33m
RED = \033[0;31m
RESET = \033[0m

all: $(NAME)
	@echo "$(GREEN)$(NAME) compilé$(RESET)"

$(NAME): $(OBJ)
	@echo "$(YELLOW)$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)$(RESET)"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@$(RESET)"
	@$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

clean:
	@echo "$(RED)rm -rf $(OBJDIR)$(RESET)"
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "$(RED)rm -f $(NAME)$(RESET)"
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

-include $(DEP)
