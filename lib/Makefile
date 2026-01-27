NAME	= libwifi2c
ANAME	= $(NAME).a
SONAME	= $(NAME).so
OBJ	= wifi2c.o
SRC	= $(OBJ:.o=.c)
INC	= $(OBJ:.o=.h)

CFLAGS	= -Wall -Wextra -O2 -c
LFLAGS	= -shared

INST_PREFIX	= /usr/local
INST_INC	= $(INST_PREFIX)/include
INST_LIB	= $(INST_PREFIX)/lib

CC	= cc
AR	= ar
RM	= rm
INSTALL	= install

.PHONY: all
all: $(ANAME) $(SONAME)

$(ANAME): $(OBJ)
	$(AR) -rcs $(ANAME) $(OBJ)

$(SONAME): $(OBJ)
	$(CC) $(LFLAGS) -o $(SONAME) $(OBJ)

$(OBJ): $(SRC) $(INC)
	$(CC) $(CFLAGS) -o $(OBJ) $(SRC)

.PHONY: clean
clean:
	$(RM) -f $(OBJ) $(ANAME) $(SONAME)

.PHONY: install
install: $(ANAME) $(SONAME)
	$(INSTALL) -m 0644 $(INC) $(INST_INC)
	$(INSTALL) -m 0644 $(ANAME) $(INST_LIB)
	$(INSTALL) -m 0755 $(SONAME) $(INST_LIB)
