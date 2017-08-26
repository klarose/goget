CC ?= g++

CSTD := -std=c++14
CFLAGS = -Wall $(CSTD) $(CMDCFLAGS)

LDFLAGS := -lstdc++

OBJS := \
	goget.o \

STDLIBS := \
	net-cpp \
	boost_program_options \



%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ -Wall $<

all:: $(OBJS)
	$(CC) -o goget $(OBJS) $(addprefix -l,$(STDLIBS)) $(LDFLAGS)

clean:
	-rm -f $(OBJS)
	-rm -f goget

