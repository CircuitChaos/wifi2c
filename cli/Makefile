NAME	= wifi2c-cli
OBJ	= wifi2c-cli.o
SRC	= $(OBJ:.o=.c)

PREFIX		= /usr/local

LIB_PREFIX	= $(PREFIX)
LIB_INC		= $(LIB_PREFIX)/include
LIB_LIB		= $(LIB_PREFIX)/lib
LIB_NAME	= wifi2c

INST_PREFIX	= $(PREFIX)
INST_DIR	= $(INST_PREFIX)/bin

CFLAGS	= -Wall -Wextra -O2 -c -I$(LIB_INC)
LFLAGS	= -L$(LIB_LIB) -l$(LIB_NAME)

CC	= cc
AR	= ar
RM	= rm
INSTALL	= install

.PHONY: all
all: $(NAME)

$(NAME): $(OBJ)
	$(CC) -o $(NAME) $(OBJ) $(LFLAGS)

$(OBJ): $(SRC)
	$(CC) -o $(OBJ) $(SRC) $(CFLAGS)

.PHONY: clean
clean:
	$(RM) -f $(OBJ) $(NAME)

.PHONY: install
install: $(NAME)
	$(INSTALL) -m 0755 $(NAME) $(INST_DIR)
