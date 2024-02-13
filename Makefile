.DEFAULT_GOAL = all
NAME = ft_ping

CFLAGS += -Wall -Wextra -fsanitize=address -g
SRC_DIR = src
SRC = $(shell find $(SRC_DIR) -type f -name *.c)
INC_DIR = inc
INC = $(wildcard $(INC_DIR)/*.h)
OBJ_DIR = .obj
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

$(NAME) : $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@ -I $(INC_DIR)

# Mandatory Rules
.PHONY: all clean fclean re run

all : $(NAME)

clean :
	rm -rf $(OBJ_DIR)

fclean : clean
	rm -f $(NAME)

re : fclean
	$(MAKE) all

# Automatic Dependencies Tracking
$(OBJ_DIR)/%.d : $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -I $(INC_DIR) -MM -MG -MT $(@:.d=.o) $< -o $@

-include $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.d)
