# presidentma <maliang.pr@qq.com>
TARGET=cb
CC=gcc
DIR=src
LINK=-lncursesw
O_DIR=objs
BINDIR=/usr/local/bin
OBJS = $(patsubst %.c,%.o,$(wildcard $(DIR)/*.c $(DIR)/include/*.c))
$(TARGET):$(OBJS)
	$(CC) $(patsubst %.o,$(O_DIR)/%.o,$(notdir $^)) -o $@ $(LINK)
%.o:%.c
	@if [ ! -d $(O_DIR) ];then mkdir $(O_DIR);fi
	$(CC) -c $< -o $(O_DIR)/$(notdir $@)
install:
	echo $(BINDIR)
	install -m 751 cb $(BINDIR)
uninstall:
	rm $(BINDIR)/cb
clean:
	rm $(O_DIR)/*.o $(TARGET)